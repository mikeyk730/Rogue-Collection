//global variable initializaton
//@(#)extern.c5.2 (Berkeley) 6/16/82

#include "rogue.h"

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
  "\xe: a scroll",
  "\x18: a weapon",
  "\xdb: a piece of armor",
  "\xf: some gold",
  "\xe7: a magic staff",
  "\xad: a potion",
  "\xf8: a magic ring",
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

bool counts_as_turn;        //True if we want after daemons
bool again;                 //The last command is repeated
bool door_stop = false;     //Stop running when we pass a door
bool fastmode = false;      //Run until you see something
bool faststate = false;     //Toggle for find (see above)
bool firstmove = false;     //First move after setting door_stop
bool playing = true;        //True until he quits
bool running = false;       //True if player is running

int was_trapped = 0;   //Was a trap sprung

int bailout = false;
char take;  //Thing the rogue is taking
char runch; //Direction player is running

int iguess = 0;

char prbuf[MAXSTR];    //Printing buffer used everywhere
char huh[BUFSIZE];                  //The last message printed

//storage array for guesses
struct Array _guesses[MAXSCROLLS+MAXPOTIONS+MAXRINGS+MAXSTICKS];

int maxrow;            //Last Line used for map
int mpos = 0;          //Where cursor is on top line
int no_move = 0;       //Number of turns held in place
int sleep_timer = 0;    //Number of turns asleep
int total_items = 0;   //Total dynamic memory bytes
int no_food = 0;       //Number of levels without food
int count = 0;         //Number of times to repeat command
int flytrap_hit = 0;   //Number of time flytrap has hit
int quiet = 0;         //Number of quiet turns

long seed = 0;         //Random number seed

Coord oldpos; //Position before last look() call
Coord delta;  //Change indicated to get_dir()

struct Room *oldrp; //Roomin(&oldpos)

struct Stats max_stats = {16, 0, 1, 10, 12, "1d4", 12}; //The maximum for the player

Agent player; //The rogue
std::list<Item*> level_items; //List of objects on this level
std::list<Agent*> level_monsters; //List of monsters on the level

//Common strings
char *typeahead = "";
char *intense = " of intense white light";
char *flash = "your %s gives off a flash%s";
