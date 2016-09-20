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
			mpos = 0;
		}
		else 
			break;
	}

	switch (obj->type)
	{
	case SCROLL:
	case POTION:
		game->get_class(obj->type).discover(obj->which);
		break;

	case RING:
	case STICK:
		game->get_class(obj->type).discover(obj->which);
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

//create_obj: Wizard command for getting anything he wants
void create_obj()
{
  Item *obj;
  byte ch, bless;
  int limit, option;

  if ((obj = create_item(0,0))==NULL) {msg("can't create anything now"); return;}
  msg("type of item !:%c ?:%c /:%c =:%c ):%c ]:%c ,:%c $:%c ", POTION, SCROLL, STICK,RING, WEAPON, ARMOR, AMULET, FOOD);
  switch (readchar())
  {
  case '!': obj->type = POTION; limit=MAXPOTIONS-1; break;
  case '?': obj->type = SCROLL; limit=MAXSCROLLS-1; break;
  case '/': obj->type = STICK; limit=MAXSTICKS-1; break;
  case '=': obj->type = RING; limit=MAXRINGS-1; break;
  case ')': obj->type = WEAPON; limit=MAXWEAPONS-1; break;
  case ']': obj->type = ARMOR; limit=MAXARMORS-1; break;
  case ',': obj->type = AMULET; limit=0; break;
  default: obj->type = FOOD; limit=1; break;
  }
  mpos = 0;
  msg("which %c do you want? (0-%x)", obj->type, limit);
  option = (isdigit((ch = readchar()))?ch-'0':ch-'a'+10);
  obj->which = (option >= 0 && option <= limit) ? option : 0;
  obj->group = 0;
  obj->count = 1;
  obj->damage = obj->throw_damage = "0d0";
  mpos = 0;
  if (obj->type==WEAPON || obj->type==ARMOR)
  {
    msg("blessing? (+,-,n)");
    bless = readchar();
    mpos = 0;
    if (bless=='-') 
        obj->set_cursed();
    if (obj->type==WEAPON)
    {
      obj->initialize_weapon(obj->which);
      if (bless=='-') 
          obj->hit_plus -= rnd(3)+1;
      if (bless=='+') 
          obj->hit_plus += rnd(3)+1;
    }
    else
    {
      obj->armor_class = get_default_class(obj->which);
      if (bless=='-') 
          obj->armor_class += rnd(3)+1;
      if (bless=='+')
          obj->armor_class -= rnd(3)+1;
    }
  }
  else if (obj->type==RING) switch (obj->which)
  {
  case R_PROTECT: case R_ADDSTR: case R_ADDHIT: case R_ADDDAM:
    msg("blessing? (+,-,n)");
    bless = readchar();
    mpos = 0;
    if (bless=='-') obj->set_cursed();
    obj->ring_level = (bless=='-'?-1:rnd(2)+1);
    break;

  case R_AGGR: case R_TELEPORT:
    obj->set_cursed();
    break;
  }
  else if (obj->type==STICK) fix_stick(obj);
  else if (obj->type==GOLD) {msg("how much?"); get_num(&obj->gold_value);}
  add_pack(obj, false);
}

//teleport: Bamf the hero someplace else
void teleport()
{
  struct Room* rm;
  Coord c;

  mvaddch(player.pos.y, player.pos.x, get_tile(player.pos.y, player.pos.x));
  do {
    rm = rnd_room(); 
    rnd_pos(rm, &c);
  } while (!(step_ok(get_tile_or_monster(c.y, c.x))));
  if (rm != player.room) {
    leave_room(&player.pos); 
    player.pos = c; 
    enter_room(&player.pos);
  }
  else { player.pos = c; look(true);}
  mvaddch(player.pos.y, player.pos.x, PLAYER);
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
    if (!(real = get_flags(y, x)&F_REAL)) standout();
    mvaddch(y, x, show_monsters ? get_tile_or_monster(y, x) : get_tile(y, x));
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
