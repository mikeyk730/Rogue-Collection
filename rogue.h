//Rogue definitions and variable declarations
//rogue.h      1.4 (AI Design) 12/14/84

#include <string.h>

//Options set for PC rogue

#define REV  1
#define VER  48

#define bcopy(a,b)  memcpy(&(a),&(b),sizeof(b))
typedef unsigned char byte;
typedef unsigned char bool;

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
#define MAXITEMS     83 //Maximum number of randomly generated things

//All the fun defines

#define next(ptr)       (*ptr).l_next
#define prev(ptr)       (*ptr).l_prev
#define on(thing,flag)  (((thing).flags&(flag))!=0)
#define CTRL(ch)        (ch&037)
#define GOLDCALC        (rnd(50+10*level)+2)
#define isfloor(c)      ((c)==FLOOR || (c)==PASSAGE)
#define isgone(rp)      (((rp)->flags&ISGONE) && ((rp)->flags&ISMAZE)==0)

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

//flags for rooms
#define ISDARK  0x0001 //room is dark
#define ISGONE  0x0002 //room is gone (a corridor)
#define ISMAZE  0x0004 //room is a maze

//flags for objects
#define ISCURSED  0x0001 //object is cursed
#define ISKNOW    0x0002 //player knows details about the object
#define DIDFLASH  0x0004 //has the vorpal weapon flashed
#define ISEGO     0x0008 //weapon has control of player
#define ISMISL    0x0010 //object is a missile type
#define ISMANY    0x0020 //object comes in groups
#define ISREVEAL  0x0040 //Do you know who the enemy of the object is

//flags for creatures
#define ISBLIND   0x0001 //creature is blind
#define SEEMONST  0x0002 //hero can detect unseen monsters
#define ISRUN     0x0004 //creature is running at the player
#define ISFOUND   0x0008 //creature has been seen (used for objects)
#define ISINVIS   0x0010 //creature is invisible
#define ISMEAN    0x0020 //creature can wake when player enters room
#define ISGREED   0x0040 //creature runs to protect gold
#define ISHELD    0x0080 //creature has been held
#define ISHUH     0x0100 //creature is confused
#define ISREGEN   0x0200 //creature can regenerate
#define CANHUH    0x0400 //creature can confuse
#define CANSEE    0x0800 //creature can see invisible creatures
#define ISCANC    0x1000 //creature has special qualities cancelled
#define ISSLOW    0x2000 //creature has been slowed
#define ISHASTE   0x4000 //creature has been hastened
#define ISFLY     0x8000 //creature is of the flying type

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
typedef struct
{
  int x;
  int y;
} Coord;

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

//Room structure
struct Room
{
  Coord pos;       //Upper left corner
  Coord size;      //Size of room
  Coord gold;      //Where the gold is
  int goldval;     //How much the gold is worth
  short flags;     //Info about the room
  int num_exits;   //Number of exits
  Coord exits[12]; //Where the exits are
};

//Structure describing a fighting being
struct Stats
{
  unsigned int str;   //Strength
  long exp;           //Experience
  int level;          //Level of mastery
  int armor_class;    //Armor class
  int hp;             //Hit points
  char *damage;       //String describing damage done
  int max_hp;         //Max hit points
};

struct Item
{
  struct Item *l_next, *l_prev; //Next pointer in link
  int type;                      //What kind of object it is
  Coord pos;                     //Where it lives on the screen
  char launcher;                 //What you need to launch it
  char *damage;                  //Damage if used like sword
  char *throw_damage;            //Damage if thrown
  int count;                     //Count for plural objects
  int which;                     //Which object of a type it is
  int hit_plus;                  //Plusses to hit
  int damage_plus;               //Plusses to damage
  short armor_class;             //Armor class
  short flags;                   //Information about objects
  char enemy;                    //If it is enchanted, who it hates
  int group;                     //Group number for this object
};
typedef struct Item ITEM;

#define charges      armor_class
#define gold_value   armor_class

//Structure for monsters and player
struct Agent
{
  struct Agent *l_next, *l_prev; //Next pointer in link
  Coord pos;                    //Position
  char turn;                    //If slowed, is it a turn to move
  char type;                    //What it is
  byte disguise;                //What mimic looks like
  byte oldch;                   //Character that was where it was
  Coord *dest;                  //Where it is running to
  short flags;                  //State word
  struct Stats stats;           //Physical description
  struct Room *room;            //Current room for thing
  struct Item *pack;            //What the thing is carrying
};

typedef struct Agent AGENT;

//External variables

extern ITEM *cur_armor, *cur_ring[2], *cur_weapon, *lvl_obj;
extern AGENT *mlist, player;
extern Coord delta, oldpos;
extern struct Room *oldrp, passages[], rooms[];
extern struct Stats max_stats;
extern struct MagicItem p_magic[], r_magic[], s_magic[], ws_magic[];

#define MAXSTR    80 //maximum length of strings
#define MAXLINES  25 //maximum number of screen lines used
#define MAXCOLS   80 //maximum number of screen columns used

//Now all the global variables

extern int maxrow;
extern int LINES, COLS;
extern int bailout;

extern char *l_menu, *l_name, *l_fruit, *l_score, *l_save, *l_macro, *l_drive;
extern char s_menu[], s_name[], s_fruit[], s_score[], s_save[], s_macro[];
extern char s_screen[];

extern struct Array s_names[], _guesses[];
extern char *s_guess[], *p_guess[], *r_guess[], *ws_guess[];
extern char f_damage[];

extern bool after, again, door_stop, expert, fastmode, faststate, firstmove, in_shell, 
  noscore, playing, running, save_msg, saw_amulet, terse, was_trapped, wizard;

extern bool p_know[], r_know[], s_know[], ws_know[];

extern char *a_names[], file_name[], fruit[], *flash, *he_man[], *helpcoms[], *helpobjs[],
  huh[], macro[], *intense, outbuf[], *p_colors[], prbuf[], *r_stones[], *release, runch, 
  *typeahead, take, whoami[], *ws_made[], *ws_type[];

extern int a_chances[], a_class[], count, food_left, flytrap_hit, fd, hungry_state,
  lastscore, level, max_level, mpos, no_command, no_food, no_move, ntraps, purse, quiet, total_items;

extern long seed;

#define TRUE                1
#define FALSE               0
#define BUFSIZE             128
