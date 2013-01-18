//global variable initializaton
//@(#)extern.c5.2 (Berkeley) 6/16/82

#include "rogue.h"

#ifdef LOG
int captains_log = FALSE;
#endif LOG

#ifdef SDEBUG
int is_com;
#endif SDEBUG

//revno: current revision level
//verno: current version of a particular rev

int revno = REV;
int verno = VER;

//All this should be low as possible in memory so that we can save the min

//Names of the various weapons
char *w_names[MAXWEAPONS+1] =
{
  "mace",
  "long sword",
  "short bow",
  "arrow",
  "dagger",
  "two handed sword",
  "dart",
  "crossbow",
  "crossbow bolt",
  "spear",
  NULL //fake entry for dragon's breath
};

//Names of armor types
char *a_names[MAXARMORS] =
{
  "leather armor",
  "ring mail",
  "studded leather armor",
  "scale mail",
  "chain mail",
  "splint mail",
  "banded mail",
  "plate mail"
};

//Chance for each armor type
int a_chances[MAXARMORS] =
{
  20,
  35,
  50,
  63,
  75,
  85,
  95,
  100
};

//Armor class for each armor type
int a_class[MAXARMORS] =
{
  8,
  7,
  7,
  6,
  5,
  4,
  4,
  3
};

struct magic_item s_magic[MAXSCROLLS] =
{
  {"monster confusion",   8, 140},
  {"magic mapping",       5, 150},
  {"hold monster",        3, 180},
  {"sleep",               5,   5},
  {"enchant armor",       8, 160},
  {"identify",           27, 100},
  {"scare monster",       4, 200},
  {"food detection",      4,  50},
  {"teleportation",       7, 165},
  {"enchant weapon",     10, 150},
  {"create monster",      5,  75},
  {"remove curse",        8, 105},
  {"aggravate monsters",  4,  20},
  {"blank paper",         1,   5},
  {"vorpalize weapon",    1, 300}
};

struct magic_item p_magic[MAXPOTIONS] =
{
  {"confusion",          8,   5},
  {"paralysis",         10,   5},
  {"poison",             8,   5},
  {"gain strength",     15, 150},
  {"see invisible",      2, 100},
  {"healing",           15, 130},
#ifdef DEMO
  {"advertisement",      6, 130},
#else
  {"monster detection",  6, 130},
#endif DEMO
  {"magic detection",    6, 105},
  {"raise level",        2, 250},
  {"extra healing",      5, 200},
  {"haste self",         4, 190},
  {"restore strength",  14, 130},
  {"blindness",          4,   5},
  {"thirst quenching",   1,   5}
};

struct magic_item r_magic[MAXRINGS] =
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

struct magic_item ws_magic[MAXSTICKS] =
{
  {"light",          12, 250},
  {"striking",        9,  75},
  {"lightning",       3, 330},
  {"fire",            3, 330},
  {"cold",            3, 330},
  {"polymorph",      15, 310},
  {"magic missile",  10, 170},
  {"haste monster",   9,   5},
  {"slow monster",   11, 350},
  {"drain life",      9, 300},
  {"nothing",         1,   5},
  {"teleport away",   5, 340},
  {"teleport to",     5,  50},
  {"cancellation",    5, 280}
};

#ifdef HELP

#define MAXHELPC  65
char *helpcoms[MAXHELPC] =
{
  "F1     list of commands",
  "F2     list of symbols",
  "F3     repeat command",
  "F4     repeat message",
  "F5     rename something",
  "F6     recall what's been discovered",
  "F7     inventory of your possessions",
  "F8     <dir> identify trap type",
  "F9     The Any Key (definable)",
  "Alt F9 defines the Any Key",
  "F10    Supervisor Key (fake dos)",
  "Space  Clear -More- message",
  "\x11\xd9     the Enter Key",
  "\x1b      left",
  "\x19      down",
  "\x18      up",
  "\x1a      right",
  "Home   up & left",
  "PgUp   up & right",
  "End    down & left",
  "PgDn   down & right",
  "Scroll Fast Play mode",
  ".      rest",
  ">      go down a staircase",
  "<      go up a staircase",
  "Esc    cancel command",
  "d      drop object",
  "e      eat food",
  "f      <dir> find something",
  "q      quaff potion",
  "r      read paper",
  "s      search for trap/secret door",
  "t      <dir> throw something",
  "w      wield a weapon",
  "z      <dir> zap with a wand",
  "B      run down & left",
  "H      run left",
  "J      run down",
  "K      run up",
  "L      run right",
  "N      run down & right",
  "U      run up & right",
  "Y      run up & left",
  "W      wear armor",
  "T      take armor off",
  "P      put on ring",
  "Q      quit",
  "R      remove ring",
  "S      save game",
  "^      identify trap",
  "?      help",
  "/      key",
  "+      throw",
  "-      zap",
  "Ctrl t terse message format",
  "Ctrl r repeat message",
  "Del    search for something hidden",
  "Ins    <dir> find something",
  "a      repeat command",
  "c      rename something",
  "i      inventory",
  "v      version number",
  "!      Supervisor Key (fake DOS)",
  "D      list what has been discovered",
  0
};

#define MAXHELPO  24
char *helpobjs[MAXHELPO] =
{
  "\xfa: the floor",
  "\x1: the hero",
  "\x5: some food",
  "\xc: the amulet of yendor",
  "\xd: a scroll",
  "\x18: a weapon",
  "\x8: a piece of armor",
  "\xf: some gold",
  "\xe7: a magic staff",
  "\xad: a potion",
  "\x9: a magic ring",
  "\xb2: a passage",

  //make sure in 40 or 80 column none of line draw set connects. this is currently in column 1 for 80

  "\xce: a door",
  "\xc9: an upper left corner",
  "\x04: a trap",
  "\xcd: a horizontal wall",
  "\xbc: a lower right corner",
  "\xc8: a lower left corner",
  "\xba: a vertical wall",
  "\xbb: an upper right corner",
  "\xf0: a stair case",
  "$,+: safe and perilous magic",
  "A-Z: 26 different monsters",
  0
};

#endif HELP

//Names of the various experience levels

char *he_man[] =
{
  "",
  "Guild Novice",
  "Apprentice",
  "Journeyman",
  "Adventurer",
  "Fighter",
  "Warrior",
  "Rogue",
  "Champion",
  "Master Rogue",
  "Warlord",
  "Hero",
  "Guild Master",
  "Dragonlord",
  "Wizard",
  "Rogue Geek",
  "Rogue Addict",
  "Schmendrick",
  "Gunfighter",
  "Time Waster",
  "Bug Chaser"
};

//Lattice C compiler funnies

int maxitems = 0;
int reinit = FALSE;

bool after;                 //True if we want after daemons
bool noscore;               //Was a wizard sometime
bool again;                 //The last command is repeated
bool s_know[MAXSCROLLS];    //Does he know what a scroll does
bool p_know[MAXPOTIONS];    //Does he know what a potion does
bool r_know[MAXRINGS];      //Does he know what a ring does
bool ws_know[MAXSTICKS];    //Does he know what a stick does
bool amulet = FALSE;        //He has the amulet
bool saw_amulet = FALSE;    //He has seen the amulet
//bool askme = TRUE;        //Ask about unidentified things
bool door_stop = FALSE;     //Stop running when we pass a door
bool fastmode = FALSE;      //Run until you see something
bool faststate = FALSE;     //Toggle for find (see above)
//bool fight_flush = TRUE;  //True if toilet input
bool firstmove = FALSE;     //First move after setting door_stop
//bool jump = FALSE;        //Show running as series of jumps
//bool passgo = TRUE;       //Follow passages
bool playing = TRUE;        //True until he quits
bool running = FALSE;       //True if player is running
bool save_msg = TRUE;       //Remember last msg
//bool slow_invent = FALSE; //Inventory one line at a time
bool terse = FALSE;
bool expert = FALSE;

#ifdef ME
int is_me;
#endif

bool was_trapped = FALSE;   //Was a trap sprung

#ifdef WIZARD
bool wizard = FALSE;        //True if allows wizard commands
#endif

int bailout = FALSE;
char take;  //Thing the rogue is taking
char runch; //Direction player is running

//now names are associated with fixed pointers

struct array s_names[MAXSCROLLS];  //Names of the scrolls
char *p_colors[MAXPOTIONS];        //Colors of the potions
char *r_stones[MAXRINGS];          //Stone settings of the rings
char *ws_made[MAXSTICKS];          //What sticks are made of
//char *release;                   //Release number of rogue
char huh[BUFSIZE];                  //The last message printed
char *s_guess[MAXSCROLLS];         //Players guess at what scroll is
char *p_guess[MAXPOTIONS];         //Players guess at what potion is
char *r_guess[MAXRINGS];           //Players guess at what ring is
char *ws_guess[MAXSTICKS];         //Players guess at what wand is

//storage array for guesses

struct array _guesses[MAXSCROLLS+MAXPOTIONS+MAXRINGS+MAXSTICKS];
int iguess = 0;
char *ws_type[MAXSTICKS]; //Is it a wand or a staff

int maxrow;            //Last Line used for map
int max_level;         //Deepest player has gone
int ntraps;            //Number of traps on this level
int dnum;              //Dungeon number
int level = 1;         //What level rogue is on
int purse = 0;         //How much gold the rogue has
int mpos = 0;          //Where cursor is on top line
int no_move = 0;       //Number of turns held in place
int no_command = 0;    //Number of turns asleep
int inpack = 0;        //Number of things in pack
int total = 0;         //Total dynamic memory bytes
int no_food = 0;       //Number of levels without food
int count = 0;         //Number of times to repeat command
int fung_hit = 0;      //Number of time fungi has hit
int quiet = 0;         //Number of quiet turns
int food_left;         //Amount of food in hero's stomach
int group = 2;         //Current group number
int hungry_state = 0;  //How hungry is he
int cksum = CSUM;
long seed;             //Random number seed

int hit_mul = P_DAMAGE;
int goodchk = 1;
char *_whoami;

//WINDOW *hw; //Used as a scratch window

coord oldpos; //Position before last look() call
coord delta;  //Change indicated to get_dir()

THING *cur_armor;   //What a well dresssed rogue wears
THING *cur_ring[2]; //Which rings are being worn
THING *cur_weapon;  //Which weapon he is wielding

struct room *oldrp; //Roomin(&oldpos)
struct room rooms[MAXROOMS]; //One for each room -- A level
//One for each passage
struct room passages[MAXPASS] =
{
  { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
  { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
  { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
  { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
  { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
  { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
  { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
  { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
  { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
  { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
  { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
  { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 }
};

#define INIT_STATS {16, 0, 1, 10, 12, "1d4", 12}

struct stats max_stats = INIT_STATS; //The maximum for the player

THING player; //The rogue
THING *lvl_obj = NULL; //List of objects on this level
THING *mlist = NULL; //List of monsters on the level

#define ___  1
#define XX  10

struct monster monsters[26] =
{
  // Name           CARRY                  FLAG   str,  exp,lvl,amr, hpt, dmg
  { "aquator",          0,               ISMEAN, { XX,   20,  5,  2, ___, "0d0/0d0"         } },
  { "bat",              0,                ISFLY, { XX,    1,  1,  3, ___, "1d2"             } },
  { "centaur",         15,                    0, { XX,   25,  4,  4, ___, "1d6/1d6"         } },
  { "dragon",         100,               ISMEAN, { XX, 6800, 10, -1, ___, "1d8/1d8/3d10"    } },
  { "emu",              0,               ISMEAN, { XX,    2,  1,  7, ___, "1d2"             } },
  //NOTE: the damage is %%% so that xstr won't merge this string with others, since it is written on in the program
  { "venus flytrap",    0,               ISMEAN, { XX,   80,  8,  3, ___, "%%%d0"           } },
  { "griffin",         20, ISMEAN|ISFLY|ISREGEN, { XX, 2000, 13,  2, ___, "4d3/3d5/4d3"     } },
  { "hobgoblin",        0,               ISMEAN, { XX,    3,  1,  5, ___, "1d8"             } },
  { "ice monster",      0,               ISMEAN, { XX,   15,  1,  9, ___, "1d2"             } },
  { "jabberwock",      70,                    0, { XX, 4000, 15,  6, ___, "2d12/2d4"        } },
  { "kestral",          0,         ISMEAN|ISFLY, { XX,    1,  1,  7, ___, "1d4"             } },
  { "leprechaun", ISGREED,                    0, { XX,   10,  3,  8, ___, "1d2"             } },
  { "medusa",          40,               ISMEAN, { XX,  200,  8,  2, ___, "3d4/3d4/2d5"     } },
  { "nymph",          100,                    0, { XX,   37,  3,  9, ___, "0d0"             } },
  { "orc",             15,              ISGREED, { XX,    5,  1,  6, ___, "1d8"             } },
  { "phantom",          0,              ISINVIS, { XX,  120,  8,  3, ___, "4d4"             } },
  { "quagga",          30,               ISMEAN, { XX,   32,  3,  2, ___, "1d2/1d2/1d4"     } },
  { "rattlesnake",      0,               ISMEAN, { XX,    9,  2,  3, ___, "1d6"             } },
  { "slime",            0,               ISMEAN, { XX,    1,  2,  8, ___, "1d3"             } },
  { "troll",           50,       ISREGEN|ISMEAN, { XX,  120,  6,  4, ___, "1d8/1d8/2d6"     } },
  { "ur-vile",          0,               ISMEAN, { XX,  190,  7, -2, ___, "1d3/1d3/1d3/4d6" } },
  { "vampire",         20,       ISREGEN|ISMEAN, { XX,  350,  8,  1, ___, "1d10"            } },
  { "wraith",           0,                    0, { XX,   55,  5,  4, ___, "1d6"             } },
  { "xeroc",           30,                    0, { XX,  100,  7,  7, ___, "3d4"             } },
  { "yeti",            30,                    0, { XX,   50,  4,  6, ___, "1d6/1d6"         } },
  { "zombie",           0,               ISMEAN, { XX,    6,  2,  8, ___, "1d8"             } }
};

char f_damage[10];

#undef ___
#undef XX

struct magic_item things[NUMTHINGS] =
{
  {0, 27 }, //potion
  {0, 30 }, //scroll
  {0, 17 }, //food
  {0,  8 }, //weapon
  {0,  8 }, //armor
  {0,  5 }, //ring
  {0,  5 }  //stick
};

//Common strings

char nullstr[] = "";
char *typeahead = nullstr;

char *intense = " of intense white light";
char *flash = "your %s gives off a flash%s";
char *it = "it";
char *you = "you";
char *no_mem = "Not enough Memory";
char *smsg = "\r\n*** Stack Overflow ***\r\n$";
