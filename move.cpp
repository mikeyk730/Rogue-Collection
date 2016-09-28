//Hero movement commands
//move.c      1.4 (A.I. Design)       12/22/84
#include <ctype.h>

#include "rogue.h"
#include "move.h"
#include "thing.h"
#include "weapons.h"
#include "output_interface.h"
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
#include "monster.h"

//Used to hold the new hero position
Coord nh;

//do_run: Start the hero running
void do_run(byte ch)
{
  game->modifiers.m_running = true;
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
  const int COLS = game->screen().columns();
  byte ch;
  int fl;

  game->modifiers.m_first_move = false;
  if (invalid_position) {
      invalid_position = false;
      msg("the crack widens ... "); 
      descend("");
      return;
  }
  if (game->no_move) {
      game->no_move--; 
      msg("you are still stuck in the bear trap");
      return;
  }
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
  if (!diag_ok(&game->hero().pos, &nh)) {
      counts_as_turn = false;
      game->modifiers.m_running = false; 
      return;
  }
  //If you are running and the move does not get you anywhere stop running
  if (game->modifiers.is_running() && equal(game->hero().pos, nh)) { 
      counts_as_turn = false;
      game->modifiers.m_running = false;
  }
  fl = game->level().get_flags(nh);
  ch = game->level().get_tile_or_monster(nh);
  //When the hero is on the door do not allow him to run until he enters the room all the way
  if ((game->level().get_tile(game->hero().pos)==DOOR) && (ch==FLOOR))
      game->modifiers.m_running = false;
  if (!(fl&F_REAL) && ch==FLOOR) {
    ch = TRAP;
    game->level().set_tile(nh, TRAP); 
    game->level().set_flag(nh, F_REAL);
  }
  else if (game->hero().is_held() && ch != 'F') { //TODO: remove direct check for F
      msg("you are being held"); 
      return; 
  }
  switch (ch)
  {
  case ' ': case VWALL: case HWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL:
hit_bound:
    if (game->modifiers.is_running() && is_gone(game->hero().room) && !game->hero().is_blind())
    {
      bool b1, b2;

      switch (run_character)
      {
      case 'h': case 'l':
        b1 = (game->hero().pos.y>1 && ((game->level().get_flags({game->hero().pos.x,game->hero().pos.y-1})&F_PASS) || game->level().get_tile({game->hero().pos.x,game->hero().pos.y-1})==DOOR));
        b2 = (game->hero().pos.y<maxrow-1 && ((game->level().get_flags({game->hero().pos.x,game->hero().pos.y+1})&F_PASS) || game->level().get_tile({game->hero().pos.x,game->hero().pos.y+1})==DOOR));
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
        b1 = (game->hero().pos.x>1 && ((game->level().get_flags({game->hero().pos.x-1,game->hero().pos.y})&F_PASS) || game->level().get_tile({game->hero().pos.x-1,game->hero().pos.y})==DOOR));
        b2 = (game->hero().pos.x<COLS-2 && ((game->level().get_flags({game->hero().pos.x+1,game->hero().pos.y})&F_PASS) || game->level().get_tile({game->hero().pos.x+1,game->hero().pos.y})==DOOR));
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
    counts_as_turn = false;
    game->modifiers.m_running = false;
    break;

  case DOOR:
    game->modifiers.m_running = false;
    if (game->level().get_flags(game->hero().pos)&F_PASS) 
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
    game->modifiers.m_running = false;
    if (isupper(ch) || game->level().monster_at(nh))
        game->hero().fight(&nh, get_current_weapon(), false);
    else
    {
      game->modifiers.m_running = false;
      if (ch!=STAIRS) take = ch;
move_stuff:
      game->level().draw_char(game->hero().pos);
      if ((fl&F_PASS) && (game->level().get_tile(oldpos)==DOOR || (game->level().get_flags(oldpos)&F_MAZE))) 
          leave_room(&nh);
      if ((fl&F_MAZE) && (game->level().get_flags(oldpos)&F_MAZE)==0)
          enter_room(&nh);
      game->hero().pos = nh;
    }
  }
}

//door_open: Called to illuminate a room.  If it is dark, remove anything that might move.
void door_open(struct Room *room)
{
    int j, k;
    byte ch;
    Monster* item;

    if (!(room->is_gone()) && !game->hero().is_blind())
        for (j = room->pos.y; j < room->pos.y + room->size.y; j++)
            for (k = room->pos.x; k < room->pos.x + room->size.x; k++)
            {
                ch = game->level().get_tile_or_monster({ k,j });
                if (isupper(ch))
                {
                    item = wake_monster({ k,j });
                    if (item->oldch == ' ' && !(room->is_dark()) && !game->hero().is_blind())
                        item->oldch = game->level().get_tile({ k,j });
                }
            }
}

//be_trapped: The guy stepped on a trap.... Make him pay.
int be_trapped(Coord *tc)
{
  byte tr;
  const int COLS = game->screen().columns();

  repeat_cmd_count = game->modifiers.m_running = false;
  game->level().set_tile(*tc, TRAP);
  tr = game->level().get_trap_type(*tc);
  was_trapped = 1;
  switch (tr)
  {
  case T_DOOR:
    descend("you fell into a trap!");
    break;

  case T_BEAR:
    game->no_move += BEAR_TIME;
    msg("you are caught in a bear trap");
    break;

  case T_SLEEP:
    game->sleep_timer += SLEEP_TIME;
    game->hero().set_running(false);
    msg("a %smist envelops you and you fall asleep", noterse("strange white "));
    break;

  case T_ARROW:
    game->log("battle", "Arrow trap 1d6 attack on player");
    if (attempt_swing(game->hero().stats.level-1, game->hero().stats.ac, 1))
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
    game->hero().teleport();
    game->screen().mvaddch(*tc, TRAP); //since the hero's leaving, look() won't put it on for us
    was_trapped++;//todo:look at this
    break;

  case T_DART:
    game->log("battle", "Dart trap 1d4 attack on player");
    if (attempt_swing(game->hero().stats.level+1, game->hero().stats.ac, 1))
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
  game->level().new_level(true);
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
  const int COLS = game->screen().columns();

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
    ch = game->level().get_tile_or_monster({x, y});
    if (!step_ok(ch))
        goto bad;
    if (ch == SCROLL)
    {
        for (auto it = game->level().items.begin(); it != game->level().items.end(); ++it){
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
