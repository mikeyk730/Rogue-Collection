#pragma once
//Rogue definitions and variable declarations
//rogue.h      1.4 (AI Design) 12/14/84

#include <string>
#include <list>
#include <string.h>

struct Random;
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
#define MISSILE   '*'

//todo: figure out what this does:
#define UNSET       '@'
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
#define F_REAL   0x010 //the level tile is actual (not set for secret doors or traps)
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

#define MAXSTR    80 //maximum length of strings
#define MAXLINES  25 //maximum number of screen lines used
#define MAXCOLS   80 //maximum number of screen columns used

#define ON  0xF1
#define OFF 0xF0

//Coordinate data type
struct Coord
{
    int x;
    int y;
};

Coord north(Coord p);
Coord south(Coord p);
Coord east(Coord p);
Coord west(Coord p);

bool operator==(const Coord& c1, const Coord& c2);
Coord operator+(const Coord& c1, const Coord& c2);

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


//External variables

//string constants

extern const char *const level_titles[], *const helpcoms[], *const helpobjs[];
extern const char *const flash, *const intense;

//Now all the global variables

struct CommandModifiers
{
    //todo: which do i need to serialize?
    bool scroll_lock() const { return m_fast_play_enabled; }
    bool fast_mode() const { return m_fast_mode; }
    
    bool m_fast_mode = false;      //Run until you see something
    bool m_fast_play_enabled = false;     //Toggle for find (see above)
};

extern char prbuf[];
extern Random* g_random;
extern GameState* game;

#define BUFSIZE             128
