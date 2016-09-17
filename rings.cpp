//Routines dealing specifically with rings
//rings.c     1.4 (AI Design) 12/13/84

#include <stdio.h>

#include "rogue.h"
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
bool r_know[MAXRINGS];      //Does he know what a ring does
char *r_guess[MAXRINGS];           //Players guess at what ring is
const char *r_stones[MAXRINGS];   //Stone settings of the rings

typedef struct {char *st_name; int st_value;} STONE;

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

struct MagicItem r_magic[MAXRINGS] =
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

int does_know_ring(int type)
{
  return r_know[type];
}

void discover_ring(int type)
{
  r_know[type] = TRUE;
}

const char* get_ring_guess(int type)
{
  return r_guess[type];
}

void set_ring_guess(int type, const char* value)
{
  strcpy(r_guess[type], value);
}

int get_ring_value(int type)
{
  return r_magic[type].worth;
}

const char* get_ring_name(int type)
{
  return r_magic[type].name;
}

//init_stones: Initialize the ring stone setting scheme for this time
void init_stones()
{
  int i, j;
  bool used[NSTONES];

  for (i = 0; i<NSTONES; i++) used[i] = FALSE;
  for (i = 0; i<MAXRINGS; i++)
  {
    do j = rnd(NSTONES); while (used[j]);
    used[j] = TRUE;
    r_stones[i] = stones[j].st_name;
    r_know[i] = FALSE;
    r_guess[i] = (char *)&_guesses[iguess++];
    if (i>0) r_magic[i].prob += r_magic[i-1].prob;
    r_magic[i].worth += stones[j].st_value;
  }
}

void init_new_ring(ITEM* ring)
{
  ring->type = RING;
  ring->which = pick_one(r_magic, MAXRINGS);
  switch (ring->which)
  {
  case R_ADDSTR: case R_PROTECT: case R_ADDHIT: case R_ADDDAM:
    if ((ring->ring_level = rnd(3))==0) {
      ring->ring_level = -1; 
      ring->flags |= IS_CURSED;
    }
    break;

  case R_AGGR: case R_TELEPORT:
    ring->flags |= IS_CURSED;
    break;
  }
}

const char* get_stone(int type)
{
  return r_stones[type];
}

//ring_on: Put a ring on a hand
void ring_on()
{
  ITEM *obj;
  int ring = -1;

  if ((obj = get_item("put on", RING))==NULL) goto no_ring;
  //Make certain that it is something that we want to wear
  if (obj->type!=RING) {msg("you can't put that on your finger"); goto no_ring;}
  //find out which hand to put it on
  if (is_current(obj)) goto no_ring;
  if (get_ring(LEFT)==NULL) ring = LEFT;
  if (get_ring(RIGHT)==NULL) ring = RIGHT;
  if (get_ring(LEFT)==NULL && get_ring(RIGHT)==NULL) if ((ring = gethand())<0) goto no_ring;
  if (ring<0) {msg("you already have a ring on each hand"); goto no_ring;}
  set_ring(ring, obj);
  //Calculate the effect it has on the poor guy.
  switch (obj->which)
  {
  case R_ADDSTR: chg_str(obj->ring_level); break;
  case R_SEEINVIS: invis_on(); break;
  case R_AGGR: aggravate(); break;
  }
  msg("%swearing %s (%c)", noterse("you are now "), inv_name(obj, TRUE), pack_char(obj));
  return;
no_ring:
  after = FALSE;
  return;
}

//ring_off: Take off a ring
void ring_off()
{
  int ring;
  ITEM *obj;
  char packchar;

  if (get_ring(LEFT)==NULL && get_ring(RIGHT)==NULL) {msg("you aren't wearing any rings"); after = FALSE; return;}
  else if (get_ring(LEFT)==NULL) ring = RIGHT;
  else if (get_ring(RIGHT)==NULL) ring = LEFT;
  else if ((ring = gethand())<0) return;
  mpos = 0;
  obj = get_ring(ring);
  if (obj==NULL) {msg("not wearing such a ring"); after = FALSE; return;}
  packchar = pack_char(obj);
  if (can_drop(obj)) msg("was wearing %s(%c)", inv_name(obj, TRUE), packchar);
}

//gethand: Which hand is the hero interested in?
int gethand()
{
  int c;

  for (;;)
  {
    msg("left hand or right hand? ");
    if ((c = readchar())==ESCAPE) {after = FALSE; return -1;}
    mpos = 0;
    if (c=='l' || c=='L') return LEFT;
    else if (c=='r' || c=='R') return RIGHT;
    msg("please type L or R");
  }
}

//ring_eat: How much food does this ring use up?
int ring_eat(int hand)
{
  if (get_ring(hand)==NULL) return 0;
  switch (get_ring(hand)->which)
  {
  case R_REGEN: return 2;
  case R_SUSTSTR: case R_SUSTARM: case R_PROTECT: case R_ADDSTR: case R_STEALTH: return 1;
  case R_SEARCH: return (rnd(5)==0);
  case R_ADDHIT: case R_ADDDAM: return (rnd(3)==0);
  case R_DIGEST: return -rnd(2);
  case R_SEEINVIS: return (rnd(5)==0);
  default: return 0;
  }
}

//ring_num: Print ring bonuses
char *ring_num(ITEM *obj)
{
  if (!(obj->flags&IS_KNOW) && !is_wizard()) 
    return "";

  switch (obj->which)
  {
  case R_PROTECT: case R_ADDSTR: case R_ADDDAM: case R_ADDHIT:
    ring_buf[0] = ' ';
    strcpy(&ring_buf[1], num(obj->ring_level, 0, (char)RING));
    break;

  default: 
    return "";
  }
  return ring_buf;
}

int is_ring_on_hand(int hand, int ring)
{
  return (get_ring(hand) != NULL && get_ring(hand)->which == ring);
}

int is_wearing_ring(int ring)
{
  return (is_ring_on_hand(LEFT, ring) || is_ring_on_hand(RIGHT, ring));
}

const char* get_inv_name_ring(ITEM* obj)
{
  char *pb = prbuf;
  int which = obj->which;

  if (does_know_ring(which) || is_wizard())
    chopmsg(pb, "A%s ring of %s", "A%s ring of %s(%s)", ring_num(obj), get_ring_name(which), get_stone(which));
  else if (*get_ring_guess(which)) 
    chopmsg(pb, "A ring called %s", "A ring called %s(%s)", get_ring_guess(which), get_stone(which));
  else 
    sprintf(pb, "A%s %s ring", vowelstr(get_stone(which)), get_stone(which));

  return prbuf;
}
