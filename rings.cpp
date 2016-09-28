//Routines dealing specifically with rings
//rings.c     1.4 (AI Design) 12/13/84

#include <stdio.h>

#include "rogue.h"
#include "agent.h"
#include "item_class.h"
#include "game_state.h"
#include "rings.h"
#include "pack.h"
#include "misc.h"
#include "potions.h"
#include "main.h"
#include "io.h"
#include "things.h"
#include "mach_dep.h"
#include "weapons.h"
#include "hero.h"

char ring_buf[6];

typedef struct { char *st_name; int st_value; } STONE;

static STONE stones[] =
{
  {"agate",           25},
  {"alexandrite",     40},
  {"amethyst",        50},
  {"carnelian",       40},
  {"diamond",        300},
  {"emerald",        300},
  {"germanium",      225},
  {"granite",          5},
  {"garnet",          50},
  {"jade",           150},
  {"kryptonite",     300},
  {"lapis lazuli",    50},
  {"moonstone",       50},
  {"obsidian",        15},
  {"onyx",            60},
  {"opal",           200},
  {"pearl",          220},
  {"peridot",         63},
  {"ruby",           350},
  {"sapphire",       285},
  {"stibotantalite", 200},
  {"tiger eye",       50},
  {"topaz",           60},
  {"turquoise",       70},
  {"taaffeite",      300},
  {"zircon",          80}
};

#define NSTONES (sizeof(stones)/sizeof(STONE))

RingInfo::RingInfo()
{
    m_magic_props =
    {
      {"protection",          9, 400},
      {"add strength",        9, 400},
      {"sustain strength",    5, 280},
      {"searching",          10, 420},
      {"see invisible",      10, 310},
      {"adornment",           1,  10},
      {"aggravate monster",  10,  10},
      {"dexterity",           8, 440},
      {"increase damage",     8, 400},
      {"regeneration",        4, 460},
      {"slow digestion",      9, 240},
      {"teleportation",       5,  30},
      {"stealth",             7, 470},
      {"maintain armor",      5, 380}
    };

    int i, j;
    bool used[NSTONES];

    for (i = 0; i < NSTONES; i++) used[i] = false;
    for (i = 0; i < MAXRINGS; i++)
    {
        do j = rnd(NSTONES); while (used[j]);
        used[j] = true;
        m_identifier.push_back(stones[j].st_name);
        if (i > 0)
            m_magic_props[i].prob += m_magic_props[i - 1].prob;
        m_magic_props[i].worth += stones[j].st_value;
    }

}

Item* create_ring()
{
    int which = pick_one(game->rings().m_magic_props);
    return new Ring(which);
}

std::string get_stone(int type)
{
    return game->rings().get_identifier(type);
}

//ring_on: Put a ring on a hand
void ring_on()
{
    Item *obj;
    if ((obj = get_item("put on", RING)) == NULL) {
        counts_as_turn = false;
        return;
    }
    game->hero().put_on_ring(obj);
}

//ring_off: Take off a ring
void Hero::ring_off()
{
    int ring;
    if (get_ring(LEFT) == NULL && get_ring(RIGHT) == NULL) {
        msg("you aren't wearing any rings");
        counts_as_turn = false;
        return;
    }
    else if (get_ring(LEFT) == NULL)
        ring = RIGHT;
    else if (get_ring(RIGHT) == NULL)
        ring = LEFT;
    else if ((ring = gethand()) < 0) return;
    msg_position = 0;
    Item* obj = game->hero().get_ring(ring);
    if (obj == NULL) { msg("not wearing such a ring"); counts_as_turn = false; return; }

    char packchar = pack_char(obj);
    if (can_drop(obj))
        msg("was wearing %s(%c)", obj->inv_name(true), packchar);
}

//gethand: Which hand is the hero interested in?
int gethand()
{
    int c;

    for (;;)
    {
        msg("left hand or right hand? ");
        if ((c = readchar()) == ESCAPE) { counts_as_turn = false; return -1; }
        msg_position = 0;
        if (c == 'l' || c == 'L') return LEFT;
        else if (c == 'r' || c == 'R') return RIGHT;
        msg("please type L or R");
    }
}

//ring_eat: How much food does this ring use up?
int ring_eat(int hand)
{
    if (game->hero().get_ring(hand) == NULL) return 0;
    switch (game->hero().get_ring(hand)->which)
    {
    case R_REGEN: return 2;
    case R_SUSTSTR: case R_SUSTARM: case R_PROTECT: case R_ADDSTR: case R_STEALTH: return 1;
    case R_SEARCH: return (rnd(5) == 0);
    case R_ADDHIT: case R_ADDDAM: return (rnd(3) == 0);
    case R_DIGEST: return -rnd(2);
    case R_SEEINVIS: return (rnd(5) == 0);
    default: return 0;
    }
}

//ring_num: Print ring bonuses
char *ring_num(Item *obj)
{
    if (!obj->is_known() && !game->hero().is_wizard())
        return "";

    switch (obj->which)
    {
    case R_PROTECT: case R_ADDSTR: case R_ADDDAM: case R_ADDHIT:
        ring_buf[0] = ' ';
        strcpy(&ring_buf[1], num(obj->get_ring_level(), 0, (char)RING));
        break;

    default:
        return "";
    }
    return ring_buf;
}

std::string RingInfo::get_inventory_name(int which, const std::string& bonus) const
{
    char *pb = prbuf;
    std::string stone = get_identifier(which);

    if (is_discovered(which) || game->hero().is_wizard())
        chopmsg(pb, "A%s ring of %s", "A%s ring of %s(%s)", bonus.c_str(), get_name(which).c_str(), stone.c_str());
    else if (!get_guess(which).empty())
        chopmsg(pb, "A ring called %s", "A ring called %s(%s)", get_guess(which).c_str(), stone.c_str());
    else
        sprintf(pb, "A%s %s ring", vowelstr(stone.c_str()), stone.c_str());

    return prbuf;
}

std::string RingInfo::get_inventory_name(Item * obj) const
{
    return get_inventory_name(obj->which, ring_num(obj));
}

std::string RingInfo::get_inventory_name(int which) const
{
    return get_inventory_name(which, "");
}

Ring::Ring(int which) :
    Item(RING, which)
{
    switch (which)
    {
    case R_ADDSTR: case R_PROTECT: case R_ADDHIT: case R_ADDDAM:
        if ((ring_level = rnd(3)) == 0) {
            ring_level = -1;
            set_cursed();
        }
        break;

    case R_AGGR: case R_TELEPORT:
        set_cursed();
        break;
    }
}

Ring::Ring(int which, int level) :
    Item(RING, which)
{
    ring_level = level;
    if (ring_level < 0)
        set_cursed();

    switch (which)
    {
    case R_AGGR: case R_TELEPORT:
        set_cursed();
        break;
    }
}

Item* Ring::Clone() const
{
    return new Ring(*this);
}
