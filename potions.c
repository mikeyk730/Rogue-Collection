//Function(s) for dealing with potions
//potions.c   1.4 (AI Design) 2/12/84

#include "rogue.h"
#include "daemons.h"
#include "daemon.h"
#include "potions.h"
#include "pack.h"
#include "curses.h"
#include "io.h"
#include "list.h"
#include "chase.h"
#include "misc.h"
#include "main.h"
#include "fight.h"
#include "rings.h"
#include "thing.h"
#include "monsters.h"
#include "things.h"

#define P_CONFUSE   0
#define P_PARALYZE  1
#define P_POISON    2
#define P_STRENGTH  3
#define P_SEEINVIS  4
#define P_HEALING   5
#define P_MFIND     6
#define P_TFIND     7
#define P_RAISE     8
#define P_XHEAL     9
#define P_HASTE     10
#define P_RESTORE   11
#define P_BLIND     12
#define P_NOP       13

bool p_know[MAXPOTIONS];    //Does he know what a potion does
const char *p_colors[MAXPOTIONS];  //Colors of the potions

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

struct MagicItem p_magic[MAXPOTIONS] =
{
  {"confusion",          8,   5},
  {"paralysis",         10,   5},
  {"poison",             8,   5},
  {"gain strength",     15, 150},
  {"see invisible",      2, 100},
  {"healing",           15, 130},
  {"monster detection",  6, 130},
  {"magic detection",    6, 105},
  {"raise level",        2, 250},
  {"extra healing",      5, 200},
  {"haste self",         4, 190},
  {"restore strength",  14, 130},
  {"blindness",          4,   5},
  {"thirst quenching",   1,   5}
};

int does_know_potion(int type)
{
  return p_know[type];
}

void discover_potion(int type)
{
  p_know[type] = TRUE;
}

int get_potion_value(int type)
{
  return p_magic[type].worth;
}

const char* get_potion_name(int type)
{
  return p_magic[type].name;
}

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

const char* get_color(int type)
{
  return p_colors[type];
}

void init_new_potion(ITEM* potion)
{
  potion->type = POTION;
  potion->which = pick_one(p_magic, MAXPOTIONS);
}

void quaff_confusion()
{
  p_know[P_CONFUSE] = TRUE;
  if (!on(player, ISHUH))
  {
    if (on(player, ISHUH)) lengthen(unconfuse, rnd(8)+HUH_DURATION);
    else fuse(unconfuse, 0, rnd(8)+HUH_DURATION);
    player.flags |= ISHUH;
    msg("wait, what's going on? Huh? What? Who?");
  }
}

void quaff_paralysis()
{
  p_know[P_PARALYZE] = TRUE;
  no_command = HOLD_TIME;
  player.flags &= ~ISRUN;
  msg("you can't move");
}

void quaff_poison()
{
  char *sick = "you feel %s sick.";

  p_know[P_POISON] = TRUE;
  if (!is_wearing_ring(R_SUSTSTR)) {chg_str(-(rnd(3)+1)); msg(sick, "very");}
  else msg(sick, "momentarily");
}

void quaff_gain_strength()
{
  p_know[P_STRENGTH] = TRUE;
  chg_str(1);
  msg("you feel stronger. What bulging muscles!");
}

void quaff_see_invisible()
{
  if (!on(player, CANSEE)) {fuse(unsee, 0, SEE_DURATION); look(FALSE); invis_on();}
  sight();
  msg("this potion tastes like %s juice", fruit);
}

void quaff_healing()
{
  p_know[P_HEALING] = TRUE;
  if ((player.stats.hp += roll(player.stats.level, 4))>player.stats.max_hp) player.stats.hp = ++player.stats.max_hp;
  sight();
  msg("you begin to feel better");
}

void quaff_monster_detection()
{
  fuse(turn_see, TRUE, HUH_DURATION);
  if (mlist==NULL) msg("you have a strange feeling%s.", noterse(" for a moment"));
  else {p_know[P_MFIND] |= turn_see(FALSE); msg("");}
}

void quaff_magic_detection()
{
  AGENT *th;

  //Potion of magic detection.  Find everything interesting on the level and show him where they are.  Also give hints as to whether he would want to use the object.
  if (lvl_obj!=NULL)
  {
    ITEM *tp;
    bool show;

    show = FALSE;
    for (tp = lvl_obj; tp!=NULL; tp = next(tp))
    {
      if (is_magic(tp))
      {
        show = TRUE;
        mvaddch(tp->pos.y, tp->pos.x, goodch(tp));
        p_know[P_TFIND] = TRUE;
      }
    }
    for (th = mlist; th!=NULL; th = next(th))
    {
      for (tp = th->pack; tp!=NULL; tp = next(tp))
      {
        if (is_magic(tp))
        {
          show = TRUE;
          mvaddch(th->pos.y, th->pos.x, MAGIC);
          p_know[P_TFIND] = TRUE;
        }
      }
    }
    if (show) {msg("You sense the presence of magic."); return;}
  }
  msg("you have a strange feeling for a moment%s.", noterse(", then it passes"));
}

void quaff_raise_level()
{
  p_know[P_RAISE] = TRUE;
  msg("you suddenly feel much more skillful");
  raise_level();
}

void quaff_extra_healing()
{
  p_know[P_XHEAL] = TRUE;
  if ((player.stats.hp += roll(player.stats.level, 8))>player.stats.max_hp)
  {
    if (player.stats.hp>player.stats.max_hp+player.stats.level+1) ++player.stats.max_hp;
    player.stats.hp = ++player.stats.max_hp;
  }
  sight();
  msg("you begin to feel much better");
}

void quaff_haste_self()
{
  p_know[P_HASTE] = TRUE;
  if (add_haste(TRUE)) msg("you feel yourself moving much faster");
}

void quaff_restore_strength()
{
  if (is_ring_on_hand(LEFT, R_ADDSTR)) 
    add_str(&player.stats.str, -cur_ring[LEFT]->ring_level);
  if (is_ring_on_hand(RIGHT, R_ADDSTR)) 
    add_str(&player.stats.str, -cur_ring[RIGHT]->ring_level);
  if (player.stats.str < max_stats.str) 
    player.stats.str = max_stats.str;
  if (is_ring_on_hand(LEFT, R_ADDSTR)) 
    add_str(&player.stats.str, cur_ring[LEFT]->ring_level);
  if (is_ring_on_hand(RIGHT, R_ADDSTR)) 
    add_str(&player.stats.str, cur_ring[RIGHT]->ring_level);
  msg("%syou feel warm all over", noterse("hey, this tastes great.  It makes "));
}

void quaff_blindness()
{
  p_know[P_BLIND] = TRUE;
  if (!on(player, ISBLIND))
  {
    player.flags |= ISBLIND;
    fuse(sight, 0, SEE_DURATION);
    look(FALSE);
  }
  msg("a cloak of darkness falls around you");
}

void quaff_thirst_quenching()
{
  msg("this potion tastes extremely dull");
}

void(*potion_functions[MAXPOTIONS])() = {
  quaff_confusion,       
  quaff_paralysis,       
  quaff_poison,          
  quaff_gain_strength,   
  quaff_see_invisible,   
  quaff_healing,         
  quaff_monster_detection,
  quaff_magic_detection, 
  quaff_raise_level,     
  quaff_extra_healing,   
  quaff_haste_self,      
  quaff_restore_strength,
  quaff_blindness,       
  quaff_thirst_quenching
};

//quaff: Quaff a potion from the pack
void quaff()
{
  ITEM *obj;

  if ((obj = get_item("quaff", POTION))==NULL) return;
  //Make certain that it is something that we want to drink
  if (obj->type!=POTION) {msg("yuk! Why would you want to drink that?"); return;}
  if (obj==cur_weapon) cur_weapon = NULL;

  //Calculate the effect it has on the poor guy.
  potion_functions[obj->which]();

  status();
  call_it(p_know[obj->which], &p_guess[obj->which]);

  //Throw the item away
  if (obj->count>1) obj->count--;
  else {
    detach_item(&player.pack, obj); 
    discard_item(obj);
  }
}

//invis_on: Turn on the ability to see invisible
void invis_on()
{
  AGENT *th;

  player.flags |= CANSEE;
  for (th = mlist; th!=NULL; th = next(th)) if (on(*th, ISINVIS) && can_see_monst(th))
  {
    mvaddch(th->pos.y, th->pos.x, th->disguise);
  }
}

//turn_see: Put on or off seeing monsters on this level
bool turn_see(bool turn_off)
{
  AGENT *mp;
  bool can_see, add_new;
  byte was_there;

  add_new = FALSE;
  for (mp = mlist; mp!=NULL; mp = next(mp))
  {
    move(mp->pos.y, mp->pos.x);
    can_see = (can_see_monst(mp) || (was_there = curch())==mp->type);
    if (turn_off)
    {
      if (!can_see_monst(mp) && mp->oldch!='@') addch(mp->oldch);
    }
    else
    {
      if (!can_see) {standout(); mp->oldch = was_there;}
      addch(mp->type);
      if (!can_see) {standend(); add_new++;}
    }
  }
  player.flags |= SEEMONST;
  if (turn_off) player.flags &= ~SEEMONST;
  return add_new;
}

//th_effect: Compute the effect of this potion hitting a monster.
void th_effect(ITEM *obj, AGENT *tp)
{
  switch (obj->which)
  {
  case P_CONFUSE: case P_BLIND:
    tp->flags |= ISHUH;
    msg("the %s appears confused", get_monster_name(tp->type));
    break;

  case P_PARALYZE:
    tp->flags &= ~ISRUN;
    tp->flags |= ISHELD;
    break;

  case P_HEALING: case P_XHEAL:
    if ((tp->stats.hp += rnd(8))>tp->stats.max_hp) tp->stats.hp = ++tp->stats.max_hp;
    break;

  case P_RAISE:
    tp->stats.hp += 8;
    tp->stats.max_hp += 8;
    tp->stats.level++;
    break;

  case P_HASTE:
    tp->flags |= ISHASTE;
    break;
  }
  msg("the flask shatters.");
}

int is_bad_potion(ITEM* obj)
{
  return obj && obj->type == POTION &&
    (obj->which == P_CONFUSE || obj->which == P_PARALYZE || obj->which == P_POISON || obj->which == P_BLIND);
}
