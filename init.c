//global variable initializaton
//init.c      1.4 (A.I. Design) 12/14/84

#include "rogue.h"

THING *_things;
int *_t_alloc;

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

long *e_levels; //Pointer to array of experience level
char *tbuf;     //Temp buffer used in fighting
char *msgbuf;   //Message buffer for msg()
char *prbuf;    //Printing buffer used everywhere
char *end_mem;  //Pointer to end of memory
char *startmem; //Pointer to the start of static memory
char *end_sb;   //Pointer to the end of static base
char *ring_buf; //Buffer used by ring code

//Declarations for data space that must be saved and restored exactly

byte *_level;
byte *_flags;

//init_player: Roll up the rogue
init_player()
{
  THING *obj;

  bcopy(pstats, max_stats);
  food_left = HUNGERTIME;
  //initialize things
  setmem(_things, MAXITEMS*sizeof(THING), 0);
  setmem(_t_alloc, MAXITEMS*sizeof(int), 0);
  //Give the rogue his weaponry.  First a mace.
  obj = new_item();
  obj->o_type = WEAPON;
  obj->o_which = MACE;
  init_weapon(obj, MACE);
  obj->o_hplus = 1;
  obj->o_dplus = 1;
  obj->o_flags |= ISKNOW;
  obj->o_count = 1;
  obj->o_group = 0;
  add_pack(obj, TRUE);
  cur_weapon = obj;
  //Now a +1 bow
  obj = new_item();
  obj->o_type = WEAPON;
  obj->o_which = BOW;
  init_weapon(obj, BOW);
  obj->o_hplus = 1;
  obj->o_dplus = 0;
  obj->o_count = 1;
  obj->o_group = 0;
  obj->o_flags |= ISKNOW;
  add_pack(obj, TRUE);
  //Now some arrows
  obj = new_item();
  obj->o_type = WEAPON;
  obj->o_which = ARROW;
  init_weapon(obj, ARROW);
  obj->o_count = rnd(15)+25;
  obj->o_hplus = obj->o_dplus = 0;
  obj->o_flags |= ISKNOW;
  add_pack(obj, TRUE);
  //And his suit of armor
  obj = new_item();
  obj->o_type = ARMOR;
  obj->o_which = RING_MAIL;
  obj->o_ac = a_class[RING_MAIL]-1;
  obj->o_flags |= ISKNOW;
  obj->o_count = 1;
  obj->o_group = 0;
  cur_armor = obj;
  add_pack(obj, TRUE);
  //Give him some food too
  obj = new_item();
  obj->o_type = FOOD;
  obj->o_count = 1;
  obj->o_which = 0;
  obj->o_group = 0;
  add_pack(obj, TRUE);
}

//Contains functions for dealing with things like potions and scrolls

//init_things: Initialize the probabilities for types of things
init_things()
{
  struct magic_item *mp;

  for (mp = &things[1]; mp<=&things[NUMTHINGS-1]; mp++) mp->mi_prob += (mp-1)->mi_prob;
}

//init_colors: Initialize the potion color scheme for this time
init_colors()
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
    if (i>0) p_magic[i].mi_prob += p_magic[i-1].mi_prob;
  }
}

//init_names: Generate the names of the various scrolls
init_names()
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
    strcpy(&s_names[i], prbuf);
    if (i>0) s_magic[i].mi_prob += s_magic[i-1].mi_prob;
  }
}

//getsyl(): generate a random syllable
getsyl()
{
  static char _tsyl[4];

  _tsyl[3] = 0;
  _tsyl[2] = rchr(c_set);
  _tsyl[1] = rchr(v_set);
  _tsyl[0] = rchr(c_set);
  return (_tsyl);
}

//rchr(): return random character in given string
rchr(char *string)
{
  return (string[rnd(strlen(string))]);
}

//init_stones: Initialize the ring stone setting scheme for this time
init_stones()
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
    if (i>0) r_magic[i].mi_prob += r_magic[i-1].mi_prob;
    r_magic[i].mi_worth += stones[j].st_value;
  }
}

//init_materials: Initialize the construction materials for wands and staffs
init_materials()
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
    if (i>0) ws_magic[i].mi_prob += ws_magic[i-1].mi_prob;
  }
}

//init_ds(): Allocate things data space
init_ds(int clrflag)
{
  long *ep;

  end_sb = _flags = newmem((MAXLINES-3)*MAXCOLS);
  _level = newmem((MAXLINES-3)*MAXCOLS);
  _things = (THING *)newmem(sizeof(THING)*MAXITEMS);
  _t_alloc = (int *)newmem(MAXITEMS*sizeof(int));
  startmem = tbuf = newmem(MAXSTR);
  msgbuf = newmem(BUFSIZ);
  prbuf = newmem(MAXSTR);
  ring_buf = newmem(6);
  e_levels = (long *)newmem(20*sizeof(long));
  for (ep = e_levels+1, *e_levels = 10L; ep<e_levels+19; ep++) *ep = *(ep-1)<<1;
  *ep = 0L;
}
