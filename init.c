//global variable initializaton
//init.c      1.4 (A.I. Design) 12/14/84

#include <stdlib.h>

#include "rogue.h"
#include "thing.h"
#include "init.h"
#include "weapons.h"
#include "misc.h"
#include "pack.h"
#include "main.h"
#include "level.h"

int iguess = 0;

static char *rainbow[] =
{
  "amber",
  "aquamarine",
  "black",
  "blue",
  "brown",
  "clear",
  "crimson",
  "cyan",
  "ecru",
  "gold",
  "green",
  "grey",
  "magenta",
  "orange",
  "pink",
  "plaid",
  "purple",
  "red",
  "silver",
  "tan",
  "tangerine",
  "topaz",
  "turquoise",
  "vermilion",
  "violet",
  "white",
  "yellow"
};

#define NCOLORS (sizeof(rainbow)/sizeof(char *))

static char *c_set = "bcdfghjklmnpqrstvwxyz";
static char *v_set = "aeiou";

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

static char *wood[] =
{
  "avocado wood",
  "balsa",
  "bamboo",
  "banyan",
  "birch",
  "cedar",
  "cherry",
  "cinnibar",
  "cypress",
  "dogwood",
  "driftwood",
  "ebony",
  "elm",
  "eucalyptus",
  "fall",
  "hemlock",
  "holly",
  "ironwood",
  "kukui wood",
  "mahogany",
  "manzanita",
  "maple",
  "oaken",
  "persimmon wood",
  "pecan",
  "pine",
  "poplar",
  "redwood",
  "rosewood",
  "spruce",
  "teak",
  "walnut",
  "zebrawood"
};

#define NWOOD (sizeof(wood)/sizeof(char *))

static char *metal[] =
{
  "aluminum",
  "beryllium",
  "bone",
  "brass",
  "bronze",
  "copper",
  "electrum",
  "gold",
  "iron",
  "lead",
  "magnesium",
  "mercury",
  "nickel",
  "pewter",
  "platinum",
  "steel",
  "silver",
  "silicon",
  "tin",
  "titanium",
  "tungsten",
  "zinc"
};

#define NMETAL (sizeof(metal)/sizeof(char *))

//Declarations for allocated things
char prbuf[MAXSTR];    //Printing buffer used everywhere

//init_player: Roll up the rogue
void init_player()
{
  ITEM *obj;

  bcopy(player.stats, max_stats);
  food_left = HUNGER_TIME;
  //Give the rogue his weaponry.  First a mace.
  obj = create_item(WEAPON, MACE);
  init_weapon(obj, MACE);
  obj->hit_plus = 1;
  obj->damage_plus = 1;
  obj->flags |= ISKNOW;
  obj->count = 1;
  obj->group = 0;
  add_pack(obj, TRUE);
  cur_weapon = obj;
  //Now a +1 bow
  obj = create_item(WEAPON, BOW);
  init_weapon(obj, BOW);
  obj->hit_plus = 1;
  obj->damage_plus = 0;
  obj->count = 1;
  obj->group = 0;
  obj->flags |= ISKNOW;
  add_pack(obj, TRUE);
  //Now some arrows
  obj = create_item(WEAPON, ARROW);
  init_weapon(obj, ARROW);
  obj->count = rnd(15)+25;
  obj->hit_plus = obj->damage_plus = 0;
  obj->flags |= ISKNOW;
  add_pack(obj, TRUE);
  //And his suit of armor
  obj = create_item(ARMOR, RING_MAIL);
  obj->armor_class = a_class[RING_MAIL]-1;
  obj->flags |= ISKNOW;
  obj->count = 1;
  obj->group = 0;
  cur_armor = obj;
  add_pack(obj, TRUE);
  //Give him some food too
  obj = create_item(FOOD, 0);
  obj->count = 1;
  obj->group = 0;
  add_pack(obj, TRUE);
}

//Contains functions for dealing with things like potions and scrolls

//init_colors: Initialize the potion color scheme for this time
void init_colors()
{
  int i, j;
  bool used[NCOLORS];

  for (i = 0; i<NCOLORS; i++) used[i] = FALSE;
  for (i = 0; i<MAXPOTIONS; i++)
  {
    do j = rnd(NCOLORS); while (used[j]);
    used[j] = TRUE;
    p_colors[i] = rainbow[j];
    p_know[i] = FALSE;
    p_guess[i] = (char *)&_guesses[iguess++];
    if (i>0) 
      p_magic[i].prob += p_magic[i-1].prob;
  }
}

//init_names: Generate the names of the various scrolls
void init_names()
{
  int nsyl;
  char *cp, *sp;
  int i, nwords;

  for (i = 0; i<MAXSCROLLS; i++)
  {
    cp = prbuf;
    nwords = rnd(terse?3:4)+2;
    while (nwords--)
    {
      nsyl = rnd(2)+1;
      while (nsyl--)
      {
        sp = getsyl();
        if (&cp[strlen(sp)]>&prbuf[MAXNAME-1]) {nwords = 0; break;}
        while (*sp) *cp++ = *sp++;
      }
      *cp++ = ' ';
    }
    *--cp = '\0';
    //I'm tired of thinking about this one so just in case .....
    prbuf[MAXNAME] = 0;
    s_know[i] = FALSE;
    s_guess[i] = (char *)&_guesses[iguess++];
    strcpy((char*)&s_names[i], prbuf);
    if (i>0) s_magic[i].prob += s_magic[i-1].prob;
  }
}

//getsyl(): generate a random syllable
char* getsyl()
{
  static char _tsyl[4];

  _tsyl[3] = 0;
  _tsyl[2] = rchr(c_set);
  _tsyl[1] = rchr(v_set);
  _tsyl[0] = rchr(c_set);
  return (_tsyl);
}

//rchr(): return random character in given string
char rchr(char *string)
{
  return (string[rnd(strlen(string))]);
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

//init_materials: Initialize the construction materials for wands and staffs
void init_materials()
{
  int i, j;
  char *str;
  bool metused[NMETAL], woodused[NWOOD];

  for (i = 0; i<NWOOD; i++) woodused[i] = FALSE;
  for (i = 0; i<NMETAL; i++) metused[i] = FALSE;
  for (i = 0; i<MAXSTICKS; i++)
  {
    for (;;) if (rnd(2)==0)
    {
      j = rnd(NMETAL);
      if (!metused[j]) {ws_type[i] = "wand"; str = metal[j]; metused[j] = TRUE; break;}
    }
    else
    {
      j = rnd(NWOOD);
      if (!woodused[j]) {ws_type[i] = "staff"; str = wood[j]; woodused[j] = TRUE; break;}
    }
    ws_made[i] = str;
    ws_know[i] = FALSE;
    ws_guess[i] = (char *)&_guesses[iguess++];
    if (i>0) ws_magic[i].prob += ws_magic[i-1].prob;
  }
}
