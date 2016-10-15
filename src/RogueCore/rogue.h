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

#define CTRL(ch)   (ch&0x1f)
#define UNCTRL(ch) (ch|0x40)
#define ESCAPE     (0x1b)

//Things that appear on the screens
#include "../Shared/pc_gfx_charmap.h"

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

#define MAXSTR    80 //maximum length of strings
#define MAXLINES  25 //maximum number of screen lines used
#define MAXCOLS   80 //maximum number of screen columns used

//External variables

//string constants

extern const char *const level_titles[], *const helpcoms[], *const helpobjs[];
extern const char *const flash_msg, *const intense;

//Now all the global variables

extern char prbuf[];
extern Random* g_random;
extern GameState* game;
