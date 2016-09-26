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
#include "room.h"
#include "game_state.h"
#include "hero.h"


#include <ctype.h>

//Used to hold the new hero position
Coord nh;

//do_run: Start the hero running
void do_run(byte ch)
{
  running = true;
  counts_as_turn = false;
  run_character = ch;
}

bool is_gone(Room* rp)
{
    return ((rp->is_gone()) && (rp->is_maze()) == 0);
}

//do_move: Check to see that a move is legal.  If it is handle the consequences (fighting, picking up, etc.)
void do_move(int dy, int dx)
{
  byte ch;
  int fl;

  firstmove = false;
  if (invalid_position) {
      invalid_position = false;
      msg("the crack widens ... "); 
      descend("");
      return;
  }
  if (no_move) {no_move--; msg("you are still stuck in the bear trap"); return;}
  //Do a confused move (maybe)
  if (game->hero().is_confused() && rnd(5) != 0) rndmove(&game->hero(), &nh);
  else
  {
over:
    nh.y = game->hero().pos.y+dy;
    nh.x = game->hero().pos.x+dx;
  }
  //Check if he tried to move off the screen or make an illegal diagonal move, and stop him if he did. fudge it for 40/80 jll -- 2/7/84
  if (offmap(nh)) goto hit_bound;
  if (!diag_ok(&game->hero().pos, &nh)) {counts_as_turn = false; running = false; return;}
  //If you are running and the move does not get you anywhere stop running
  if (running && equal(game->hero().pos, nh)) counts_as_turn = running = false;
  fl = Level::get_flags(nh);
  ch = get_tile_or_monster(nh);
  //When the hero is on the door do not allow him to run until he enters the room all the way
  if ((Level::get_tile(game->hero().pos)==DOOR) && (ch==FLOOR)) running = false;
  if (!(fl&F_REAL) && ch==FLOOR) {
    ch = TRAP;
    Level::set_tile(nh, TRAP); 
    Level::set_flag(nh, F_REAL);
  }
  else if (game->hero().is_held() && ch != 'F') { //TODO: remove direct check for F
      msg("you are being held"); 
      return; 
  }
  switch (ch)
  {
  case ' ': case VWALL: case HWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL:
hit_bound:
    if (running && is_gone(game->hero().room) && !game->hero().is_blind())
    {
      bool b1, b2;

      switch (run_character)
      {
      case 'h': case 'l':
        b1 = (game->hero().pos.y>1 && ((Level::get_flags({game->hero().pos.x,game->hero().pos.y-1})&F_PASS) || Level::get_tile({game->hero().pos.x,game->hero().pos.y-1})==DOOR));
        b2 = (game->hero().pos.y<maxrow-1 && ((Level::get_flags({game->hero().pos.x,game->hero().pos.y+1})&F_PASS) || Level::get_tile({game->hero().pos.x,game->hero().pos.y+1})==DOOR));
        if (!(b1^b2)) break;
        if (b1) {
            run_character = 'k'; 
            dy = -1;
        }
        else {
            run_character = 'j';
            dy = 1;
        }
        dx = 0;
        goto over;

      case 'j': case 'k':
        b1 = (game->hero().pos.x>1 && ((Level::get_flags({game->hero().pos.x-1,game->hero().pos.y})&F_PASS) || Level::get_tile({game->hero().pos.x-1,game->hero().pos.y})==DOOR));
        b2 = (game->hero().pos.x<COLS-2 && ((Level::get_flags({game->hero().pos.x+1,game->hero().pos.y})&F_PASS) || Level::get_tile({game->hero().pos.x+1,game->hero().pos.y})==DOOR));
        if (!(b1^b2))
            break;
        if (b1) {
            run_character = 'h'; 
            dx = -1;
        }
        else {
            run_character = 'l';
            dx = 1;
        }
        dy = 0;
        goto over;
      }
    }
    counts_as_turn = running = false;
    break;

  case DOOR:
    running = false;
    if (Level::get_flags(game->hero().pos)&F_PASS) 
        enter_room(&nh);
    goto move_stuff;

  case TRAP:
    ch = be_trapped(&nh);
    if (ch==T_DOOR || ch==T_TELEP) return;

  case PASSAGE:
    goto move_stuff;

  case FLOOR:
    if (!(fl&F_REAL)) 
        be_trapped(&game->hero().pos);
    goto move_stuff;

  default:
    running = false;
    if (isupper(ch) || monster_at(nh))
      fight(&nh, get_current_weapon(), false);
    else
    {
      running = false;
      if (ch!=STAIRS) take = ch;
move_stuff:
      Screen::DrawChar(game->hero().pos, Level::get_tile(game->hero().pos));
      if ((fl&F_PASS) && (Level::get_tile(oldpos)==DOOR || (Level::get_flags(oldpos)&F_MAZE))) leave_room(&nh);
      if ((fl&F_MAZE) && (Level::get_flags(oldpos)&F_MAZE)==0) enter_room(&nh);
      game->hero().pos = nh;
    }
  }
}

//door_open: Called to illuminate a room.  If it is dark, remove anything that might move.
void door_open(struct Room *room)
{
  int j, k;
  byte ch;
  Agent *item;

  if (!(room->is_gone()) && !game->hero().is_blind())
    for (j = room->pos.y; j<room->pos.y+room->size.y; j++)
      for (k = room->pos.x; k<room->pos.x+room->size.x; k++)
      {
        ch = get_tile_or_monster({k,j});
        if (isupper(ch))
        {
          item = wake_monster({k,j});
          if (item->oldch==' ' && !(room->is_dark()) && !game->hero().is_blind())
              item->oldch = Level::get_tile({k,j});
        }
      }
}

//be_trapped: The guy stepped on a trap.... Make him pay.
int be_trapped(Coord *tc)
{
  byte tr;

  repeat_cmd_count = running = false;
  Level::set_tile(*tc, TRAP);
  tr = Level::get_trap_type(*tc);
  was_trapped = 1;
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
    sleep_timer += SLEEP_TIME;
    game->hero().set_running(false);
    msg("a %smist envelops you and you fall asleep", noterse("strange white "));
    break;

  case T_ARROW:
    if (swing(game->hero().stats.level-1, game->hero().stats.ac, 1))
    {
      if (!game->hero().decrease_hp(roll(1, 6), true)) {
          msg("an arrow killed you"); 
          death('a');
      }
      else 
          msg("oh no! An arrow shot you");
    }
    else
    {
      Item *arrow;

      if ((arrow = new Weapon(ARROW, 0, 0))!=NULL)
      {
        arrow->count = 1;
        arrow->pos = game->hero().pos;
        fall(arrow, false);
      }
      msg("an arrow shoots past you");
    }
    break;

  case T_TELEP:
    teleport();
    Screen::DrawChar(*tc, TRAP); //since the hero's leaving, look() won't put it on for us
    was_trapped++;//todo:look at this
    break;

  case T_DART:
    if (swing(game->hero().stats.level+1, game->hero().stats.ac, 1))
    {
      if (!game->hero().decrease_hp(roll(1, 4), true)) {
          msg("a poisoned dart killed you"); 
          death('d');
      }
      if (!is_wearing_ring(R_SUSTSTR) && !save(VS_POISON)) 
          game->hero().adjust_strength(-1);
      msg("a dart just hit you in the shoulder");
    }
    else 
        msg("a dart whizzes by your ear and vanishes");
    break;
  }
  clear_typeahead_buffer();
  return tr;
}

void descend(char *mesg)
{
  next_level();
  if (*mesg==0) msg(" ");
  new_level(true);
  msg("");
  msg(mesg);
  if (!save(VS_LUCK))
  {
    msg("you are damaged by the fall");
    if (!game->hero().decrease_hp(roll(1,8), true))
        death('f');
  }
}

//rndmove: Move in a random direction if the monster/person is confused
void rndmove(Agent *who, Coord *newmv)
{
  int x, y;
  byte ch;
  Item *obj;

  y = newmv->y = who->pos.y+rnd(3)-1;
  x = newmv->x = who->pos.x+rnd(3)-1;
  //Now check to see if that's a legal move.  If not, don't move. (I.e., bump into the wall or whatever)
  if (y==who->pos.y && x==who->pos.x) return;
  if ((y<1 || y>=maxrow) || (x<0 || x>=COLS))
      goto bad;
  else if (!diag_ok(&who->pos, newmv)) 
      goto bad;
  else
  {
    ch = get_tile_or_monster({x, y});
    if (!step_ok(ch))
        goto bad;
    if (ch == SCROLL)
    {
        for (auto it = level_items.begin(); it != level_items.end(); ++it){
            obj = *it;
            if (y == obj->pos.y && x == obj->pos.x)
                break;
        }
        if (is_scare_monster_scroll(obj)) 
            goto bad;
    }
  }
  return;

bad:

  (*newmv) = who->pos;
  return;
}
