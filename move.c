//Hero movement commands
//move.c      1.4 (A.I. Design)       12/22/84

#include "rogue.h"
#include "move.h"
#include "thing.h"
#include "weapons.h"
#include "curses.h"
#include "io.h"
#include "rip.h"
#include "wizard.h"
#include "monsters.h"
#include "rooms.h"
#include "misc.h"
#include "main.h"
#include "fight.h"
#include "new_leve.h"
#include "chase.h"
#include "mach_dep.h"
#include "level.h"
#include "rings.h"
#include "scrolls.h"
#include "pack.h"

#include <ctype.h>

//Used to hold the new hero position
Coord nh;

//do_run: Start the hero running
void do_run(byte ch)
{
  running = TRUE;
  after = FALSE;
  runch = ch;
}

//do_move: Check to see that a move is legal.  If it is handle the consequences (fighting, picking up, etc.)
void do_move(int dy, int dx)
{
  byte ch;
  int fl;

  firstmove = FALSE;
  if (bailout) {bailout = 0; msg("the crack widens ... "); descend(""); return;}
  if (no_move) {no_move--; msg("you are still stuck in the bear trap"); return;}
  //Do a confused move (maybe)
  if (on(player, ISHUH) && rnd(5)!=0) rndmove(&player, &nh);
  else
  {
over:
    nh.y = player.pos.y+dy;
    nh.x = player.pos.x+dx;
  }
  //Check if he tried to move off the screen or make an illegal diagonal move, and stop him if he did. fudge it for 40/80 jll -- 2/7/84
  if (offmap(nh.y, nh.x)) goto hit_bound;
  if (!diag_ok(&player.pos, &nh)) {after = FALSE; running = FALSE; return;}
  //If you are running and the move does not get you anywhere stop running
  if (running && equal(player.pos, nh)) after = running = FALSE;
  fl = get_flags(nh.y, nh.x);
  ch = get_tile_or_monster(nh.y, nh.x);
  //When the hero is on the door do not allow him to run until he enters the room all the way
  if ((get_tile(player.pos.y, player.pos.x)==DOOR) && (ch==FLOOR)) running = FALSE;
  if (!(fl&F_REAL) && ch==FLOOR) {
    ch = TRAP;
    set_tile(nh.y, nh.x, TRAP); 
    set_flag(nh.y, nh.x, F_REAL);
  }
  else if (on(player, ISHELD) && ch!='F') {msg("you are being held"); return;}
  switch (ch)
  {
  case ' ': case VWALL: case HWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL:
hit_bound:
    if (running && isgone(player.room) && !on(player, ISBLIND))
    {
      bool b1, b2;

      switch (runch)
      {
      case 'h': case 'l':
        b1 = (player.pos.y>1 && ((get_flags(player.pos.y-1, player.pos.x)&F_PASS) || get_tile(player.pos.y-1, player.pos.x)==DOOR));
        b2 = (player.pos.y<maxrow-1 && ((get_flags(player.pos.y+1, player.pos.x)&F_PASS) || get_tile(player.pos.y+1, player.pos.x)==DOOR));
        if (!(b1^b2)) break;
        if (b1) {runch = 'k'; dy = -1;}
        else {runch = 'j'; dy = 1;}
        dx = 0;
        goto over;

      case 'j': case 'k':
        b1 = (player.pos.x>1 && ((get_flags(player.pos.y, player.pos.x-1)&F_PASS) || get_tile(player.pos.y, player.pos.x-1)==DOOR));
        b2 = (player.pos.x<COLS-2 && ((get_flags(player.pos.y, player.pos.x+1)&F_PASS) || get_tile(player.pos.y, player.pos.x+1)==DOOR));
        if (!(b1^b2)) break;
        if (b1) {runch = 'h'; dx = -1;}
        else {runch = 'l'; dx = 1;}
        dy = 0;
        goto over;
      }
    }
    after = running = FALSE;
    break;

  case DOOR:
    running = FALSE;
    if (get_flags(player.pos.y, player.pos.x)&F_PASS) enter_room(&nh);
    goto move_stuff;

  case TRAP:
    ch = be_trapped(&nh);
    if (ch==T_DOOR || ch==T_TELEP) return;

  case PASSAGE:
    goto move_stuff;

  case FLOOR:
    if (!(fl&F_REAL)) be_trapped(&player.pos);
    goto move_stuff;

  default:
    running = FALSE;
    if (isupper(ch) || monster_at(nh.y, nh.x))
      fight(&nh, ch, get_current_weapon(), FALSE);
    else
    {
      running = FALSE;
      if (ch!=STAIRS) take = ch;
move_stuff:
      mvaddch(player.pos.y, player.pos.x, get_tile(player.pos.y, player.pos.x));
      if ((fl&F_PASS) && (get_tile(oldpos.y, oldpos.x)==DOOR || (get_flags(oldpos.y, oldpos.x)&F_MAZE))) leave_room(&nh);
      if ((fl&F_MAZE) && (get_flags(oldpos.y, oldpos.x)&F_MAZE)==0) enter_room(&nh);
      player.pos = nh;
    }
  }
}

//door_open: Called to illuminate a room.  If it is dark, remove anything that might move.
void door_open(struct Room *room)
{
  int j, k;
  byte ch;
  AGENT *item;

  if (!(room->flags&ISGONE) && !on(player, ISBLIND))
    for (j = room->pos.y; j<room->pos.y+room->size.y; j++)
      for (k = room->pos.x; k<room->pos.x+room->size.x; k++)
      {
        ch = get_tile_or_monster(j, k);
        if (isupper(ch))
        {
          item = wake_monster(j, k);
          if (item->oldch==' ' && !(room->flags&ISDARK) && !on(player, ISBLIND)) item->oldch = get_tile(j, k);
        }
      }
}

//be_trapped: The guy stepped on a trap.... Make him pay.
int be_trapped(Coord *tc)
{
  byte tr;

  count = running = FALSE;
  set_tile(tc->y, tc->x, TRAP);
  tr = get_flags(tc->y, tc->x)&F_TMASK;
  was_trapped = TRUE;
  switch (tr)
  {
  case T_DOOR:
    descend("you fell into a trap!");
    break;

  case T_BEAR:
    no_move += BEAR_TIME;
    msg("you are caught in a bear trap");
    break;

  case T_SLEEP:
    no_command += SLEEP_TIME;
    player.flags &= ~ISRUN;
    msg("a %smist envelops you and you fall asleep", noterse("strange white "));
    break;

  case T_ARROW:
    if (swing(player.stats.level-1, player.stats.ac, 1))
    {
      player.stats.hp -= roll(1, 6);
      if (player.stats.hp<=0) {msg("an arrow killed you"); death('a');}
      else msg("oh no! An arrow shot you");
    }
    else
    {
      ITEM *arrow;

      if ((arrow = create_item(WEAPON, ARROW))!=NULL)
      {
        init_weapon(arrow, ARROW);
        arrow->count = 1;
        arrow->pos = player.pos;
        fall(arrow, FALSE);
      }
      msg("an arrow shoots past you");
    }
    break;

  case T_TELEP:
    teleport();
    mvaddch(tc->y, tc->x, TRAP); //since the hero's leaving, look() won't put it on for us
    was_trapped++;
    break;

  case T_DART:
    if (swing(player.stats.level+1, player.stats.ac, 1))
    {
      player.stats.hp -= roll(1, 4);
      if (player.stats.hp<=0) {msg("a poisoned dart killed you"); death('d');}
      if (!is_wearing_ring(R_SUSTSTR) && !save(VS_POISON)) chg_str(-1);
      msg("a dart just hit you in the shoulder");
    }
    else msg("a dart whizzes by your ear and vanishes");
    break;
  }
  flush_type();
  return tr;
}

void descend(char *mesg)
{
  next_level();
  if (*mesg==0) msg(" ");
  new_level(TRUE);
  msg("");
  msg(mesg);
  if (!save(VS_LUCK))
  {
    msg("you are damaged by the fall");
    if ((player.stats.hp -= roll(1,8))<=0) death('f');
  }
}

//rndmove: Move in a random direction if the monster/person is confused
void rndmove(AGENT *who, Coord *newmv)
{
  int x, y;
  byte ch;
  ITEM *obj;

  y = newmv->y = who->pos.y+rnd(3)-1;
  x = newmv->x = who->pos.x+rnd(3)-1;
  //Now check to see if that's a legal move.  If not, don't move. (I.e., bump into the wall or whatever)
  if (y==who->pos.y && x==who->pos.x) return;
  if ((y<1 || y>=maxrow) || (x<0 || x>=COLS)) goto bad;
  else if (!diag_ok(&who->pos, newmv)) goto bad;
  else
  {
    ch = get_tile_or_monster(y, x);
    if (!step_ok(ch)) goto bad;
    if (ch==SCROLL)
    {
      for (obj = lvl_obj; obj!=NULL; obj = next(obj)) if (y==obj->pos.y && x==obj->pos.x) break;
      if (is_scare_monster_scroll(obj)) goto bad;
    }
  }
  return;

bad:

  (*newmv) = who->pos;
  return;
}
