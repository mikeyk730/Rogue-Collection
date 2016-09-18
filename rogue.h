//Rogue definitions and variable declarations
//rogue.h      1.4 (AI Design) 12/14/84

#include <string>
#include <list>
#include <string.h>
#include "main.h"

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

//flags for rooms
const short IS_DARK  = 0x0001; //room is dark
const short IS_GONE  = 0x0002; //room is gone (a corridor)
const short IS_MAZE  = 0x0004; //room is a maze

//flags for objects
const short IS_CURSED  = 0x0001; //object is cursed
const short IS_KNOW    = 0x0002; //player knows details about the object
const short DID_FLASH  = 0x0004; //has the vorpal weapon flashed
//const short IS_EGO   = 0x0008; //weapon has control of player
const short IS_MISL    = 0x0010; //object is a missile type
const short IS_MANY    = 0x0020; //object comes in groups
const short IS_REVEAL  = 0x0040; //Do you know who the enemy of the object is

//flags for creatures
const short IS_BLIND   = 0x0001; //creature is blind
const short SEE_MONST  = 0x0002; //hero can detect unseen monsters
const short IS_RUN     = 0x0004; //creature is running at the player
const short IS_FOUND   = 0x0008; //creature has been seen (used for objects)
const short IS_INVIS   = 0x0010; //creature is invisible
const short IS_MEAN    = 0x0020; //creature can wake when player enters room
const short IS_GREED   = 0x0040; //creature runs to protect gold
const short IS_HELD    = 0x0080; //creature has been held
const short IS_HUH     = 0x0100; //creature is confused
const short IS_REGEN   = 0x0200; //creature can regenerate
const short CAN_HUH    = 0x0400; //creature can confuse
const short CAN_SEE    = 0x0800; //creature can see invisible creatures
const short IS_CANC    = 0x1000; //creature has special qualities cancelled
const short IS_SLOW    = 0x2000; //creature has been slowed
const short IS_HASTE   = 0x4000; //creature has been hastened
const short IS_FLY     = (short)0x8000; //creature is of the flying type
const short IS_DIRTY   = (short)0x8000; //dirty bit 
//todo: don't reuse flag.  turn dirty into different variable

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

//Room structure
struct Room
{
  int index;
  Coord pos;       //Upper left corner
  Coord size;      //Size of room
  Coord gold;      //Where the gold is
  int goldval;     //How much the gold is worth
  short flags;     //Info about the room
  int num_exits;   //Number of exits
  Coord exits[12]; //Where the exits are

  bool is_flag_set(short flag) const{
      return (flags & flag) != 0;
  }

  bool is_dark() const{
      return is_flag_set(IS_DARK);
  }
  bool is_maze() const{
      return is_flag_set(IS_MAZE);
  }
  bool is_gone() const{
      return is_flag_set(IS_GONE);
  }
  void set_maze(){
      flags |= IS_MAZE;
  }
  void set_gone(){
      flags |= IS_GONE;
  }
  void set_dark(bool enable){
      if (enable)
          flags |= IS_DARK;
      else
          flags &= ~IS_DARK;
  }
  void reset(){
      goldval = num_exits = flags = 0;
  }
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

struct Agent;
struct Item
{
  int type;                      //What kind of object it is
  Coord pos;                     //Where it lives on the screen
  char launcher;                 //What you need to launch it
  char *damage;                  //Damage if used like sword
  char *throw_damage;            //Damage if thrown
  int count;                     //Count for plural objects
  int which;                     //Which object of a type it is
  int hit_plus;                  //Plusses to hit
  int damage_plus;               //Plusses to damage
  short misc;                    //Item dependent: rmor class, gold value, stick charges, ring level
  short flags;                   //Information about objects
  char enemy;                    //If it is enchanted, who it hates
  int group;                     //Group number for this object
  void initialize(int type, int which);

  bool is_flag_set(short flag) const{
      return (flags & flag) != 0;
  }

  bool is_known() const
  {
      return is_flag_set(IS_KNOW);
  }
  bool is_cursed() const
  {
      return is_flag_set(IS_CURSED);
  }
  bool did_flash() const
  {
      return is_flag_set(DID_FLASH);
  }
  bool is_missile() const
  {
      return is_flag_set(IS_MISL);
  }
  bool does_group() const
  {
      return is_flag_set(IS_MANY);
  }
  bool is_revealed() const
  {
      return is_flag_set(IS_REVEAL);
  }
  bool is_found() const
  {
      return is_flag_set(IS_FOUND);
  }

  void remove_curse()
  {
      flags &= ~IS_CURSED;
  }

  void set_known(){
      flags |= IS_KNOW;
  }
  void set_cursed(){
      flags |= IS_CURSED;
  }
  void set_revealed(){
      flags |= IS_REVEAL;
  }
  void set_found(){
      flags |= IS_FOUND;
  }
  void set_flashed(){
      flags |= DID_FLASH;
  }

  //gold-specific functions
  void initialize_gold(int value, Coord location);

  //weapon-specific functions
  void initialize_weapon(byte type);
  const char* get_inv_name_weapon() const;
  void enchant_weapon();
  void vorpalize();
  bool is_vorpalized() const;
  bool is_vorpalized_against(Agent* monster) const;
  const char* get_vorpalized_name() const;
};

#define charges      misc
#define gold_value   misc
#define armor_class  misc
#define ring_level   misc

//Structure for monsters and player
struct Agent
{
  Coord pos;                    //Position
  char turn;                    //If slowed, is it a turn to move
  char type;                    //What it is
  byte disguise;                //What mimic looks like
  byte oldch;                   //Character that was where it was
  Coord *dest;                  //Where it is running to
  short flags;                  //State word
  int exflags;                  //More state;
  int value;                    //
  Stats stats;                  //Physical description
  Room *room;                   //Current room for thing
  std::list<Item*> pack;        //What the thing is carrying

private:
  bool is_flag_set(short flag) const;
  void set_flag(short flag, bool enable);

public:
  const char* get_monster_name() const;
  int get_monster_carry_prob() const;

  //special features
  bool is_monster_confused_this_turn() const;
  bool is_stationary() const;
  bool can_hold() const;
  bool hold_attacks() const;
  bool can_divide() const;
  bool shoots_fire() const;
  bool immune_to_fire() const;
  bool shoots_ice() const;
  bool causes_confusion() const;
  bool is_mimic() const;
  bool is_disguised() const;
  bool steals_gold() const;
  bool drops_gold() const;
  bool steals_magic() const;
  bool drains_life() const;
  bool drains_exp() const;
  bool drains_strength() const;
  bool rusts_armor() const;
  bool dies_from_attack() const;

  bool is_flying() const;
  bool is_mean() const;
  bool regenerates_hp() const;
  bool is_greedy() const;
  bool is_invisible() const;
  bool is_confused() const;
  bool is_held() const;
  bool is_blind() const;
  bool is_fast() const;
  bool is_slow() const;
  bool sees_invisible() const;
  bool detects_others() const;
  bool is_running() const;
  bool is_found() const;
  bool can_confuse() const;
  bool powers_cancelled() const;

  void set_invisible(bool enable){
      set_flag(IS_INVIS, enable);
  }
  void set_found(bool enable){
      set_flag(IS_FOUND, enable);
  }
  void set_confused(bool enable){
      set_flag(IS_HUH, enable);
  }
  void set_running(bool enable){
      set_flag(IS_RUN, enable);
  }
  void set_is_held(bool enable){
      set_flag(IS_HELD, enable);
  }
  void set_is_slow(bool enable){
      set_flag(IS_SLOW, enable);
  }
  void set_is_fast(bool enable){
      set_flag(IS_HASTE, enable);
  }
  void set_can_confuse(bool enable){
      set_flag(CAN_HUH, enable);
  }
  void set_cancelled(bool enable){
      set_flag(IS_CANC, enable);
  }
  void set_blind(bool enable){
      set_flag(IS_BLIND, enable);
  }
  void set_sees_invisible(bool enable){
      set_flag(CAN_SEE, enable);
  }
  void set_detects_others(bool enable){
      set_flag(SEE_MONST, enable);
  }
  void set_is_mean(bool enable){
      set_flag(IS_MEAN, enable);
  }

  void reveal_disguise(){
      disguise = type;
  }

  //slime specific
  void set_dirty(bool enable){
      value = enable ? 1 : 0;
  }
  bool is_dirty(){
      return value == 1;
  }
};

//External variables

extern std::list<Item*> level_items;
extern std::list<Agent*> level_monsters;
extern Agent player;
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

extern char s_menu[], s_score[], s_save[], s_screen[], s_levels[], s_monstercfg[];

extern struct Array _guesses[];

extern bool counts_as_turn, again, door_stop, fastmode, faststate, firstmove, 
  playing, running;
extern int was_trapped;

extern char file_name[], fruit[], *flash, *he_man[], *helpcoms[], *helpobjs[],
  huh[], macro[], *intense, outbuf[], prbuf[], *release, runch, 
  *typeahead, take, whoami[];

extern int repeat_cmd_count, iguess, mpos, sleep_timer, no_food, no_move, turns_since_heal;

extern long seed;

#define BUFSIZE             128
