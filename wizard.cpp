//Special wizard commands (some of which are also non-wizard commands under strange circumstances)
//wizard.c    1.4 (AI Design) 12/14/84

#include <sstream>
#include <iomanip>
#include <ctype.h>
#include <stdlib.h>

#include "rogue.h"
#include "game_state.h"
#include "daemons.h"
#include "pack.h"
#include "thing.h"
#include "output_interface.h"
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
#include "monster.h"

//whatis: What a certain object is
void whatis()
{
    if (game->hero().pack.empty()) {
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

    msg(obj->inv_name(false));
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

  msg_position = 0;

  //todo:if (obj->type==GOLD) {msg("how much?"); get_num(&obj->gold_value);}

  game->hero().add_to_pack(obj, false);
}

//show_map: Print out the map for the wizard
void show_map(bool show_monsters)
{
  int y, x, real;

  game->screen().wdump();
  game->screen().clear();
  const int COLS = game->screen().columns();
  for (y = 1; y<maxrow; y++) for (x = 0; x<COLS; x++)
  {
    if (!(real = game->level().get_flags({x, y})&F_REAL))
        game->screen().standout();
    game->screen().mvaddch({x, y}, show_monsters ? game->level().get_tile_or_monster({x, y}) : game->level().get_tile({x, y}));
    if (!real) 
        game->screen().standend();
  }
  show_win("---More (level map)---");
  game->screen().wrestor();
}

namespace 
{
    using std::left;
    using std::setw;

    void add_debug_items(const std::list<Item*>& items, bool coord, const char* fmt)
    {
        for (auto it = items.begin(); it != items.end(); ++it)
        {
            Item* item = *it;
            std::ostringstream ss;

            Coord pos = item->position();
            ss  << item->inv_name(false);
            if (coord)
                ss << " at (" << pos.x << "," << pos.y << ")";

            add_line("", fmt, ss.str().c_str());
        }
    }

    void debug_items()
    {
        add_debug_items(game->level().items, true, "  %s");
    }

    void debug_monsters()
    {
        for (auto i = game->level().monsters.begin(); i != game->level().monsters.end(); ++i)
        {
            Monster* monster = *i;
            std::ostringstream ss;
            ss << "  "   << left << setw(14) << monster->get_name() << " ";
            ss << "hp:"  << left << setw(4) << monster->get_hp() << " ";
            ss << "lvl:" << left << setw(3) << monster->level() << " ";
            ss << "amr:" << left << setw(4) << monster->calculate_armor() << " ";
            ss << "exp:" << left << setw(5) << monster->experience() << " ";
            ss << "dmg:" << left << setw(4) << monster->damage_string();
            add_line("", ss.str().c_str(), "");
            if (!monster->pack.empty()) {
                add_debug_items(monster->pack, false, "    * %s");
            }
        }
    }
}

void debug_screen()
{
    add_line("", "Level Items:", "");
    debug_items();
    add_line("", " ", "");
    add_line("", "Level Monsters:", "");
    debug_monsters();
    end_line("");
}

int get_num(short *place)
{
  char numbuf[12];

  getinfo(numbuf, 10);
  *place = atoi(numbuf);
  return (*place);
}

Cheats::Cheats()
{
    //todo: should be empty
    //m_detect_monsters = true;
    //m_see_invisible = true;
    //m_no_traps = true;
    //m_no_hidden_doors = true;
    //m_no_ring_hunger = true;
    //m_no_hunger = true;
    //m_no_dark_rooms = true;
}

bool Cheats::detect_monsters() const
{
    return m_detect_monsters;
}

bool Cheats::no_dark_rooms() const
{
    return m_no_dark_rooms;
}

bool Cheats::see_invisible() const
{
    return m_see_invisible;
}

bool Cheats::no_hunger() const
{
    return m_no_hunger;
}

bool Cheats::no_hidden_doors() const
{
    return m_no_hidden_doors;
}

bool Cheats::no_traps() const
{
    return m_no_traps;
}

bool Cheats::no_ring_hunger() const
{
    return m_no_ring_hunger;
}
