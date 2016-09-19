#pragma once
//Rogue definitions and variable declarations
//rogue.h      1.4 (AI Design) 12/14/84

#include <string>
#include <list>
#include <string.h>
#include "main.h"

struct GameState;

//Options set for PC rogue

#define REV  1
#define VER  48

typedef unsigned char byte;

//Maximum number of different things

#define MAXROOMS     9
#define MAXTHINGS    9
#define MAXOBJ       9
#define MAXPACK      23
#define MAXTRAPS     10
#define AMULETLEVEL  26
#define NUMTHINGS    7  //number of types of things
#define MAXPASS      13 //upper limit on number of passages
#define MAXNAME      20 //Maximum Length of a scroll

//All the fun defines

#define CTRL(ch)        (ch&037)
#ifdef DEBUG
#define debug           msg
#else
#define debug           if(0) msg
#endif

//Things that appear on the screens

#define PASSAGE   (0xb1)
#define DOOR      (0xce)
#define FLOOR     (0xfa)
#define PLAYER    (0x01)
#define TRAP      (0x04)
#define STAIRS    (0xf0)
#define GOLD      (0x0f)
#define POTION    (0xad)
#define SCROLL    (0x0e) //(0x0d)
#define MAGIC     '$'
#define BMAGIC    '+'
#define FOOD      (0x05)
#define STICK     (0xe7)
#define ARMOR     (0xdb) //(0x08)
#define AMULET    (0x0c)
#define RING      (0xf8) //(0x09)
#define WEAPON    (0x18)
#define CALLABLE  -1

bool isfloor(byte c);

#define VWALL   (0xba)
#define HWALL   (0xcd)
#define ULWALL  (0xc9)
#define URWALL  (0xbb)
#define LLWALL  (0xc8)
#define LRWALL  (0xbc)

//Various constants
#define BEAR_TIME    spread(3)
#define SLEEP_TIME   spread(5)
#define HOLD_TIME    spread(2)
#define WANDER_TIME  spread(70)
#define HUH_DURATION spread(20)
#define SEE_DURATION spread(300)
#define HUNGER_TIME  spread(1300)
#define MORE_TIME    150
#define STOMACH_SIZE 2000
#define STARVE_TIME  850
#define ESCAPE       27
#define LEFT         0
#define RIGHT        1
#define BOLT_LENGTH  6
#define LAMP_DIST    3

//Save against things
#define VS_POISON        00
#define VS_LUCK          01
#define VS_MAGIC         03

//Various flag bits

//Flags for level map
#define F_PASS   0x040 //is a passageway
#define F_MAZE   0x020 //have seen this corridor before
#define F_REAL   0x010 //what you see is what you get
#define F_PNUM   0x00f //passage number mask
#define F_TMASK  0x007 //trap number mask

//Trap types
#define T_DOOR   00
#define T_ARROW  01
#define T_SLEEP  02
#define T_BEAR   03
#define T_TELEP  04
#define T_DART   05
#define NTRAPS   6

#define MAXPOTIONS  14
#define MAXSCROLLS  15

//Weapon types
#define MACE        0
#define SWORD       1
#define BOW         2
#define ARROW       3
#define DAGGER      4
#define TWOSWORD    5
#define DART        6
#define CROSSBOW    7
#define BOLT        8
#define SPEAR       9
#define FLAME       10 //fake entry for dragon breath (ick)
#define MAXWEAPONS  10 //this should equal FLAME

//Armor types
#define LEATHER          0
#define RING_MAIL        1
#define STUDDED_LEATHER  2
#define SCALE_MAIL       3
#define CHAIN_MAIL       4
#define SPLINT_MAIL      5
#define BANDED_MAIL      6
#define PLATE_MAIL       7
#define MAXARMORS        8

//Ring types
#define R_PROTECT   0
#define R_ADDSTR    1
#define R_SUSTSTR   2
#define R_SEARCH    3
#define R_SEEINVIS  4
#define R_NOP       5
#define R_AGGR      6
#define R_ADDHIT    7
#define R_ADDDAM    8
#define R_REGEN     9
#define R_DIGEST    10
#define R_TELEPORT  11
#define R_STEALTH   12
#define R_SUSTARM   13
#define MAXRINGS    14

//Rod/Wand/Staff types
#define WS_LIGHT      0
#define WS_HIT        1
#define WS_ELECT      2
#define WS_FIRE       3
#define WS_COLD       4
#define WS_POLYMORPH  5
#define WS_MISSILE    6
#define WS_HASTE_M    7
#define WS_SLOW_M     8
#define WS_DRAIN      9
#define WS_NOP        10
#define WS_TELAWAY    11
#define WS_TELTO      12
#define WS_CANCEL     13
#define MAXSTICKS     14

//Coordinate data type
struct Coord
{
    int x;
    int y;
};

//Stuff about magic items
struct MagicItem
{
  char *name;
  int prob;
  short worth;
};

struct Array
{
  char storage[MAXNAME+1];
};


//Structure describing a fighting being
struct Stats
{
  unsigned int str;   //Strength
  long exp;           //Experience
  int level;          //Level of mastery
  int ac;             //Armor class
  int hp;             //Hit points
  std::string damage; //String describing damage done
  int max_hp;         //Max hit points

  int get_hp() const {
      return hp;
  }

  bool decrease_hp(int n, bool can_kill){
      hp -= n;
      if (!can_kill && hp <= 0)
          hp = 1;
      return hp > 0;
  }

  void increase_hp(int n, bool max_bonus, bool second_max_bonus){
      hp += n;

      if (max_bonus && hp > max_hp) 
          ++max_hp;
      if (second_max_bonus && hp > max_hp + level + 1) 
          ++max_hp;

      if (hp > max_hp) {
          hp = max_hp;
      }
  }

  int drain_hp(){
      hp /= 2;
      return hp;
  }
};



//External variables


extern Coord delta, oldpos;
extern struct Room *oldrp, passages[];
extern struct Stats max_stats;

#define MAXSTR    80 //maximum length of strings
#define MAXLINES  25 //maximum number of screen lines used
#define MAXCOLS   80 //maximum number of screen columns used

//Now all the global variables

extern int maxrow;
extern int LINES, COLS;
extern int bailout;

extern struct Array _guesses[];

extern bool counts_as_turn, again, stop_at_door, fastmode, fast_play_enabled, firstmove, 
  playing, running;
extern int was_trapped;

extern char *flash, *he_man[], *helpcoms[], *helpobjs[],
  huh[], *intense, outbuf[], prbuf[], *release, run_character, 
  take;
extern const char* typeahead;

extern int repeat_cmd_count, iguess, mpos, sleep_timer, no_food, no_move, turns_since_heal;
extern GameState* game;

#define BUFSIZE             128
