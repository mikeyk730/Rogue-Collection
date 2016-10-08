#pragma once
//Rogue definitions and variable declarations
//rogue.h      1.4 (AI Design) 12/14/84

struct Random;
struct GameState;

//Options set for PC rogue

#define REV  1
#define VER  48

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

#define CTRL(ch)  (ch&037)
#define ESCAPE    (0x1b)

//Things that appear on the screens

#define PASSAGE   (0xb1)
#define DOOR      (0xce)
#define FLOOR     (0xfa)
#define PLAYER    (0x01)
#define TRAP      (0x04)
#define STAIRS    (0xf0)
#define GOLD      (0x0f)
#define POTION    (0xad)
#define SCROLL    (0x0e) //originally (0x0d)
#define FOOD      (0x05)
#define STICK     (0xe7)
#define ARMOR     (0xdb) //originally (0x08)
#define AMULET    (0x0c)
#define RING      (0xf8) //originally (0x09)
#define WEAPON    (0x18)
#define VWALL     (0xba)
#define HWALL     (0xcd)
#define ULWALL    (0xc9)
#define URWALL    (0xbb)
#define LLWALL    (0xc8)
#define LRWALL    (0xbc)
#define MISSILE   '*'
#define MAGIC     '$'
#define BMAGIC    '+'


#define UNSET     '@'

//Various constants
#define SLEEP_TIME   spread(5)
#define WANDER_TIME  spread(70)
#define HUH_DURATION spread(20)
#define SEE_DURATION spread(300)
#define BOLT_LENGTH  6

//Save against things
#define VS_POISON        00
#define VS_LUCK          01
#define VS_MAGIC         03

//Various flag bits

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

//External variables

//string constants

extern const char *const level_titles[], *const helpcoms[], *const helpobjs[];
extern const char *const flash, *const intense;

//Now all the global variables

extern char prbuf[];
extern Random* g_random;
extern GameState* game;
