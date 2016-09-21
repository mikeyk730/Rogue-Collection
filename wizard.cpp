//Special wizard commands (some of which are also non-wizard commands under strange circumstances)
//wizard.c    1.4 (AI Design) 12/14/84

#include <ctype.h>
#include <stdlib.h>

#include "rogue.h"
#include "game_state.h"
#include "daemons.h"
#include "pack.h"
#include "thing.h"
#include "curses.h"
#include "io.h"
#include "wizard.h"
#include "daemon.h"
#include "main.h"
#include "things.h"
#include "rooms.h"
#include "misc.h"
#include "new_leve.h"
#include "monsters.h"
#include "mach_dep.h"
#include "sticks.h"
#include "rings.h"
#include "scrolls.h"
#include "potions.h"
#include "weapons.h"
#include "level.h"
#include "armor.h"
#include "hero.h"
#include "food.h"
#include "scrolls.h"

//whatis: What a certain object is
void whatis()
{
    if (player.pack.empty()) {
        msg("You don't have anything in your pack to identify");
        return;
    }

    Item *obj;
    for (;;) {
        if ((obj = get_item("identify", 0)) == NULL)
        {
            msg("You must identify something");
            msg(" ");
            msg_position = 0;
        }
        else
            break;
    }

    switch (obj->type)
    {
    case SCROLL:
    case POTION:
        game->item_class(obj->type).discover(obj->which);
        break;

    case RING:
    case STICK:
        game->item_class(obj->type).discover(obj->which);
        obj->set_known();
        break;

    case WEAPON:
    case ARMOR:
        obj->set_known();
        break;
    }

    //If it is vorpally enchanted, then reveal what type of monster it is vorpally enchanted against
    if (obj->is_vorpalized())
        obj->set_revealed();

    msg(inv_name(obj, false));
}

int get_which(int type, int limit)
{
    byte ch;
    msg_position = 0;
    msg("which %c do you want? (0-%x)", type, limit);
    int option = (isdigit((ch = readchar())) ? ch - '0' : ch - 'a' + 10);
    return (option >= 0 && option <= limit) ? option : 0;
}

char get_bless_char(){
    msg("blessing? (+,-,n)");
    byte bless = readchar();
    msg_position = 0;
    return bless;
}

//summon_object: Wizard command for getting anything he wants
void summon_object()
{
  int which;
  Item* obj;
  
  msg("type of item !:%c ?:%c /:%c =:%c ):%c ]:%c ,:%c $:%c ", POTION, SCROLL, STICK,RING, WEAPON, ARMOR, AMULET, FOOD);
  switch (readchar())
  {
  case '!': 
      which = get_which(POTION, MAXPOTIONS - 1);
      obj = new Potion(which); 
      break;
  case '?': 
      which = get_which(SCROLL,MAXSCROLLS-1);
      obj = new Scroll(which);
      break;
  case '/':
      which = get_which(STICK,MAXSTICKS-1);
      obj = new Stick(which);
      break;
  case '=':
  {
      char bless;
      which = get_which(RING, MAXRINGS - 1);
      
      int level = 0;
      switch (which)
      {
      case R_PROTECT: case R_ADDSTR: case R_ADDHIT: case R_ADDDAM:
          bless = get_bless_char();
          level = (bless == '-') ? -1 : rnd(2) + 1;
          break;
      }

      obj = new Ring(which, level);
      break;
  }
  case ')':
  {
      which = get_which(WEAPON, MAXWEAPONS - 1); 
      char bless = get_bless_char();
      int hit_plus = 0;
      if (bless == '-')
          hit_plus -= rnd(3) + 1;
      else if (bless == '+')
          hit_plus += rnd(3) + 1;

      obj = new Weapon(which, hit_plus, 0);
      break;
  }
  case ']': 
  {
      which = get_which(ARMOR, MAXARMORS - 1);
      char bless = get_bless_char();

      int ac_mod = 0;
      if (bless == '-')
          ac_mod += rnd(3) + 1;
      else if (bless == '+')
          ac_mod -= rnd(3) + 1;

      obj = new Armor(which, ac_mod);
      break;
  }
  case ',':
      which = get_which(AMULET,0);
      obj = new Amulet();
      break;
  default:
      which = get_which(FOOD,1);
      obj = new Food(which);
      break;
  }

  obj->group = 0;
  obj->count = 1;
  obj->damage = obj->throw_damage = "0d0";
  msg_position = 0;

  //todo:if (obj->type==GOLD) {msg("how much?"); get_num(&obj->gold_value);}

  add_to_pack(obj, false);
}

//teleport: Bamf the hero someplace else
void teleport()
{
  struct Room* rm;
  Coord c;

  Screen::DrawChar(player.pos, Level::get_tile(player.pos));
  do {
    rm = rnd_room(); 
    rnd_pos(rm, &c);
  } while (!(step_ok(get_tile_or_monster(c))));
  if (rm != player.room) {
    leave_room(&player.pos); 
    player.pos = c; 
    enter_room(&player.pos);
  }
  else { player.pos = c; look(true);}
  Screen::DrawChar(player.pos, PLAYER);
  //turn off IS_HELD in case teleportation was done while fighting a Flytrap
  if (player.is_held()) { 
      player.set_is_held(false); 
  }
  no_move = 0;
  repeat_cmd_count = 0;
  running = false;
  clear_typeahead_buffer();
  //Teleportation can be a confusing experience (unless you really are a wizard)
  if (!game->hero().is_wizard())
  {
    if (player.is_confused())
        lengthen(unconfuse, rnd(4)+2);
    else 
        fuse(unconfuse, 0, rnd(4)+2);
    player.set_confused(true);
  }
}

//show_map: Print out the map for the wizard
void show_map(bool show_monsters)
{
  int y, x, real;

  wdump();
  clear();
  for (y = 1; y<maxrow; y++) for (x = 0; x<COLS; x++)
  {
    if (!(real = Level::get_flags({x, y})&F_REAL)) standout();
    Screen::DrawChar({x, y}, show_monsters ? get_tile_or_monster({x, y}) : Level::get_tile({x, y}));
    if (!real) standend();
  }
  show_win("---More (level map)---");
  wrestor();
}

int get_num(short *place)
{
  char numbuf[12];

  getinfo(numbuf, 10);
  *place = atoi(numbuf);
  return (*place);
}
