//File with various monster functions in it
//monsters.c  1.4 (A.I. Design)       12/14/84

#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

#include "rogue.h"
#include "monsters.h"
#include "daemons.h"
#include "list.h"
#include "main.h"
#include "chase.h"
#include "new_leve.h"
#include "rooms.h"
#include "things.h"
#include "io.h"
#include "misc.h"
#include "daemon.h"
#include "fight.h"
#include "rings.h"
#include "thing.h"
#include "rooms.h"
#include "level.h"


//List of monsters in rough order of vorpalness
static char *lvl_mons = "K BHISOR LCA NYTWFP GMXVJD";
static char *wand_mons = "KEBHISORZ CAQ YTW PUGM VJ ";

#define ___  1
#define XX  10

const int EX_DIVIDES      =  0x0001;
const int EX_SHOOTS_ICE   =  0x0002;
const int EX_SHOOTS_FIRE  =  0x0004;
const int EX_RUSTS_ARMOR  =  0x0008;
const int EX_HOLDS        =  0x0010;
const int EX_STATIONARY   =  0x0020;
const int EX_MIMICS       =  0x0040;
const int EX_CONFUSES     =  0x0080;
const int EX_STEALS_GOLD  =  0x0100;
const int EX_STEALS_MAGIC =  0x0200;
const int EX_DRAINS_STR   =  0x0400;
const int EX_DRAINS_EXP   =  0x0800;
const int EX_DRAINS_MAXHP =  0x1000;
const int EX_HOLD_ATTACKS =  0x2000;
const int EX_SUICIDES     =  0x4000;
const int EX_DROPS_GOLD   =  0x8000;

bool Agent::can_divide() const
{
    return (exflags & EX_DIVIDES) != 0;
}

bool Agent::is_stationary() const {
    return (exflags & EX_STATIONARY) != 0;
}

bool Agent::can_hold() const {
    return (exflags & EX_HOLDS) != 0;
}

bool Agent::hold_attacks() const {
    return (exflags & EX_HOLD_ATTACKS) != 0;
}

bool Agent::shoots_fire() const {
    return (exflags & EX_SHOOTS_FIRE) != 0;
}

bool Agent::immune_to_fire() const {
    return shoots_fire();
}

bool Agent::shoots_ice() const {
    return (exflags & EX_SHOOTS_ICE) != 0;
}

bool Agent::causes_confusion() const {
    return (exflags & EX_CONFUSES) != 0;
}

bool Agent::is_mimic() const {
    return (exflags & EX_MIMICS) != 0;
}

bool Agent::is_disguised() const {
    return is_mimic() && type != disguise;
}

bool Agent::drops_gold() const {
    return (exflags & EX_DROPS_GOLD) != 0;
}

bool Agent::steals_gold() const {
    return (exflags & EX_STEALS_GOLD) != 0;
}

bool Agent::steals_magic() const {
    return (exflags & EX_STEALS_MAGIC) != 0;
}

bool Agent::drains_life() const {
    return (exflags & EX_DRAINS_MAXHP) != 0;
}

bool Agent::drains_exp() const {
    return (exflags & EX_DRAINS_EXP) != 0;
}

bool Agent::drains_strength() const {
    return (exflags & EX_DRAINS_STR) != 0;
}

bool Agent::rusts_armor() const {
    return (exflags & EX_RUSTS_ARMOR) != 0;
}

bool Agent::dies_from_attack() const{
    return (exflags & EX_SUICIDES) != 0;
}

//todo: make configurable
bool Agent::is_monster_confused_this_turn() const {
    return ((is_confused() && rnd(5) != 0) ||
        // Phantoms are slightly confused all of the time, and bats are quite confused all the time
        type == 'P' && rnd(5) == 0 ||
        type == 'B' && rnd(2) == 0);
}

bool Agent::is_flag_set(int flag) const {
    return ((flags & flag) != 0);
}

bool Agent::is_flying() const {
    return is_flag_set(IS_FLY);
}

bool Agent::is_mean() const {
    return is_flag_set(IS_MEAN);
}

bool Agent::regenerates_hp() const {
    return is_flag_set(IS_REGEN);
}

bool Agent::is_greedy() const {
    return is_flag_set(IS_GREED);
}

bool Agent::is_invisible() const {
    return is_flag_set(IS_INVIS);
}

bool Agent::is_confused() const {
    return is_flag_set(IS_HUH);
}

bool Agent::is_held() const {
    return is_flag_set(IS_HELD);
}

bool Agent::is_blind() const {
    return is_flag_set(IS_BLIND);
}

bool Agent::is_fast() const
{
    return is_flag_set(IS_HASTE);
}

bool Agent::is_slow() const {
    return is_flag_set(IS_SLOW);
}

bool Agent::sees_invisible() const
{
    return is_flag_set(CAN_SEE);
}

bool Agent::detects_others() const {
    return is_flag_set(SEE_MONST);
}

bool Agent::is_running() const {
    return is_flag_set(IS_RUN);
}

bool Agent::is_found() const {
    return is_flag_set(IS_FOUND);
}

bool Agent::can_confuse() const {
    return is_flag_set(CAN_HUH);
}

bool Agent::powers_cancelled() const {
    return is_flag_set(IS_CANC);
}




//Array containing information on all the various types of monsters
struct Monster
{
  std::string name;   //What to call the monster
  int carry;          //Probability of carrying something
  short flags;        //Things about the monster
  struct Stats stats; //Initial stats
  int exflags;        //todo, populate elsewhere?
};

struct Monster monsters[26] =
{
  // Name           CARRY                  FLAG       str,  exp,lvl,amr, hpt, dmg
  { "aquator",          0,                 IS_MEAN,  { XX,   20,  5,  2, ___, "0d0/0d0"         }, EX_RUSTS_ARMOR },
  { "bat",              0,                  IS_FLY,  { XX,    1,  1,  3, ___, "1d2"             }, 0 },
  { "centaur",         15,                       0,  { XX,   25,  4,  4, ___, "1d6/1d6"         }, 0 },
  { "dragon",         100,                 IS_MEAN,  { XX, 6800, 10, -1, ___, "1d8/1d8/3d10"    }, EX_SHOOTS_FIRE },
  { "emu",              0,                 IS_MEAN,  { XX,    2,  1,  7, ___, "1d2"             }, 0 },
  { "venus flytrap",    0,                 IS_MEAN,  { XX,   80,  8,  3, ___, "0d1"             }, EX_HOLDS|EX_STATIONARY|EX_HOLD_ATTACKS },
  { "griffin",         20, IS_MEAN|IS_FLY|IS_REGEN,  { XX, 2000, 13,  2, ___, "4d3/3d5/4d3"     }, 0 },
  { "hobgoblin",        0,                 IS_MEAN,  { XX,    3,  1,  5, ___, "1d8"             }, 0 },
  { "ice monster",      0,                 IS_MEAN,  { XX,   15,  1,  9, ___, "1d2"             }, EX_SHOOTS_ICE },
  { "jabberwock",      70,                       0,  { XX, 4000, 15,  6, ___, "2d12/2d4"        }, 0 },
  { "kestral",          0,          IS_MEAN|IS_FLY,  { XX,    1,  1,  7, ___, "1d4"             }, 0 },
  { "leprechaun",       0,                       0,  { XX,   10,  3,  8, ___, "1d2"             }, EX_STEALS_GOLD|EX_DROPS_GOLD|EX_SUICIDES },
  { "medusa",          40,                 IS_MEAN,  { XX,  200,  8,  2, ___, "3d4/3d4/2d5"     }, EX_CONFUSES },
  { "nymph",          100,                       0,  { XX,   37,  3,  9, ___, "0d0"             }, EX_STEALS_MAGIC|EX_SUICIDES },
  { "orc",             15,                IS_GREED,  { XX,    5,  1,  6, ___, "1d8"             }, 0 },
  { "phantom",          0,                IS_INVIS,  { XX,  120,  8,  3, ___, "4d4"             }, 0 },
  { "quagga",          30,                 IS_MEAN,  { XX,   32,  3,  2, ___, "1d2/1d2/1d4"     }, 0 },
  { "rattlesnake",      0,                 IS_MEAN,  { XX,    9,  2,  3, ___, "1d6"             }, EX_DRAINS_STR },
  { "slime",            0,                 IS_MEAN,  { XX,    1,  2,  8, ___, "1d3"             }, EX_DIVIDES },
  { "troll",           50,        IS_REGEN|IS_MEAN,  { XX,  120,  6,  4, ___, "1d8/1d8/2d6"     }, 0 },
  { "ur-vile",          0,                 IS_MEAN,  { XX,  190,  7, -2, ___, "1d3/1d3/1d3/4d6" }, 0 },
  { "vampire",         20,        IS_REGEN|IS_MEAN,  { XX,  350,  8,  1, ___, "1d10"            }, EX_DRAINS_MAXHP }, //todo: confirm these two
  { "wraith",           0,                       0,  { XX,   55,  5,  4, ___, "1d6"             }, EX_DRAINS_EXP },   //todo
  { "xeroc",           30,                       0,  { XX,  100,  7,  7, ___, "3d4"             }, EX_MIMICS },
  { "yeti",            30,                       0,  { XX,   50,  4,  6, ___, "1d6/1d6"         }, 0 },
  { "zombie",           0,                 IS_MEAN,  { XX,    6,  2,  8, ___, "1d8"             }, 0 }
};

char f_damage[10];

#undef ___
#undef XX

//todo: validation
void load_monster_cfg(const char* filename)
{
    std::ifstream file(filename, std::ios::in);
    std::string line;
    while (std::getline(file, line)){
        std::istringstream ss(line);

        char type;
        Monster m;
        ss >> type;
        ss >> m.name;
        ss >> m.carry;
        ss >> std::hex >> m.flags;
        ss >> std::dec >> m.stats.str >> m.stats.exp >> m.stats.level >> m.stats.ac >> m.stats.hp;
        ss >> m.stats.damage;
        ss >> std::hex >> m.exflags;

        std::replace(m.name.begin(), m.name.end(), '_', ' ');

        monsters[type - 'A'] = m;
        //printf("%c %s %d %x %d %d %d %d %d %s %x\n", type, m.name.c_str(), m.carry, m.flags, m.stats.str, m.stats.exp, m.stats.level, m.stats.ac, m.stats.hp, m.stats.damage.c_str(), m.exflags);
    }
}

const char* get_monster_name(char monster)
{
  return monsters[monster-'A'].name.c_str();
}

const char* Agent::get_monster_name() const
{
    return ::get_monster_name(type);
}

int Agent::get_monster_carry_prob() const
{
    return monsters[type - 'A'].carry;
}


//randmonster: Pick a monster to show up.  The lower the level, the meaner the monster.
char randmonster(bool wander, int level)
{
  int d;
  char *mons;

  mons = wander?wand_mons:lvl_mons;
  do
  {
    int r10 = rnd(5)+rnd(6);

    d = level+(r10-5);
    if (d<1) d = rnd(5)+1;
    if (d>26) d = rnd(5)+22;
  } while (mons[--d]==' ');
  return mons[d];
}

void set_xeroc_disguise(AGENT* X)
{
  switch (rnd(get_level() >= AMULETLEVEL ? 9 : 8))
  {
  case 0: X->disguise = GOLD; break;
  case 1: X->disguise = POTION; break;
  case 2: X->disguise = SCROLL; break;
  case 3: X->disguise = STAIRS; break;
  case 4: X->disguise = WEAPON; break;
  case 5: X->disguise = ARMOR; break;
  case 6: X->disguise = RING; break;
  case 7: X->disguise = STICK; break;
  case 8: X->disguise = AMULET; break;
  }
}

//new_monster: Pick a new monster and add it to the list
void new_monster(AGENT *monster, byte type, Coord *position, int level)
{
  int level_add = (level <= AMULETLEVEL) ? 0 : level-AMULETLEVEL;
  const struct Monster* defaults;
  
  defaults = &monsters[type-'A'];
  attach_agent(&mlist, monster);

  monster->type = type;
  monster->disguise = type;
  monster->pos = *position;
  monster->oldch = '@';
  monster->room = roomin(position);
  monster->flags = defaults->flags;
  monster->exflags = defaults->exflags;
  monster->stats = defaults->stats;
  monster->stats.level += level_add;
  monster->stats.hp = monster->stats.max_hp = roll(monster->stats.level, 8);
  monster->stats.ac -= level_add;
  monster->stats.exp += level_add*10 + exp_add(monster);
  monster->turn = true;
  monster->pack = NULL;

  //todo: remove F,X checks
  if (type=='F') 
    monster->stats.damage = f_damage;
  if (monster->is_mimic()) 
    set_xeroc_disguise(monster);

  if (is_wearing_ring(R_AGGR)) 
    start_run(monster);
}

//f_restor(): restor initial damage string for flytraps
void f_restor()
{
    //todo:clean this up
  const struct Monster *monster = &monsters['F'-'A'];
  flytrap_hit = 0;
  strcpy(f_damage, monster->stats.damage.c_str());
}

//expadd: Experience to add for this monster's level/hit points
int exp_add(AGENT *monster)
{
  int divisor = (monster->stats.level == 1) ? 8 : 6;
  int value = monster->stats.max_hp / divisor;

  if (monster->stats.level>9) 
    value *= 20;
  else if (monster->stats.level>6) 
    value *= 4;
  
  return value;
}

//wanderer: Create a new wandering monster and aim it at the player
void wanderer()
{
  struct Room *room;
  AGENT *monster;
  Coord cp;

  //can we allocate a new monster
  if ((monster = create_agent())==NULL) return;
  do
  {
    room = rnd_room();
    if (room==player.room) continue;
    rnd_pos(room, &cp);
  } while (!(room!=player.room && step_ok(get_tile_or_monster(cp.y, cp.x))));
  new_monster(monster, randmonster(true, get_level()), &cp, get_level());
  if (bailout) debug("wanderer bailout");
  //debug("started a wandering %s", monsters[tp->type-'A'].m_name);
  start_run(monster);
}

//wake_monster: What to do when the hero steps next to a monster
AGENT *wake_monster(int y, int x)
{
  AGENT *monster;
  struct Room *room;
  int dst;

  if ((monster = monster_at(y, x))==NULL) return monster;
  //Every time he sees mean monster, it might start chasing him
  if (!monster->is_running() && rnd(3)!=0 && monster->is_mean() && !monster->is_held() && !is_wearing_ring(R_STEALTH))
  {
    monster->dest = &player.pos;
    monster->flags |= IS_RUN;
  }
  if (monster->causes_confusion() && !player.is_blind() && !monster->is_found() && !monster->powers_cancelled() && monster->is_running())
  {
    room = player.room;
    dst = DISTANCE(y, x, player.pos.y, player.pos.x);
    if ((room!=NULL && !(room->flags&IS_DARK)) || dst<LAMP_DIST)
    {
      monster->flags |= IS_FOUND;
      if (!save(VS_MAGIC))
      {
        if (player.is_confused()) lengthen(unconfuse, rnd(20)+HUH_DURATION);
        else fuse(unconfuse, 0, rnd(20)+HUH_DURATION);
        player.flags |= IS_HUH;
        msg("the %s's gaze has confused you", monster->get_monster_name());
      }
    }
  }
  //Let greedy ones guard gold
  if (monster->is_greedy() && !monster->is_running())
  {
    monster->flags = monster->flags|IS_RUN;
    if (player.room->goldval) 
        monster->dest = &player.room->gold;
    else 
        monster->dest = &player.pos;
  }
  return monster;
}

//give_pack: Give a pack to a monster if it deserves one
void give_pack(AGENT *monster)
{
  if (rnd(100) < monster->get_monster_carry_prob()) 
    attach_item(&monster->pack, new_item());
}

//pick_mons: Choose a sort of monster for the enemy of a vorpally enchanted weapon
char pick_monster()
{
  char *cp;
  
  do {
    cp = lvl_mons+strlen(lvl_mons);
    while (--cp>=lvl_mons && rnd(10));
    if (cp<lvl_mons) return 'M';
  } while (*cp == ' ');

  return *cp;
}

//monster_at(x,y): returns pointer to monster at coordinate. if no monster there return NULL
AGENT *monster_at(int y, int x)
{
  AGENT *monster;
  for (monster = mlist; monster!=NULL; monster = next(monster)) 
    if (monster->pos.x == x && monster->pos.y == y) 
      return monster;
  return NULL;
}
