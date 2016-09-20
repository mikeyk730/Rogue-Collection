//Function(s) for dealing with potions
//potions.c   1.4 (AI Design) 2/12/84

#include <algorithm>
#include <stdio.h>

#include "rogue.h"
#include "item_class.h"
#include "game_state.h"
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
#include "hero.h"

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

PotionInfo::PotionInfo()
{
	m_magic_props =
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

	int i, j;
	bool used[NCOLORS];

	for (i = 0; i<NCOLORS; i++) used[i] = false;
	for (i = 0; i<MAXPOTIONS; i++)
	{
		do j = rnd(NCOLORS); while (used[j]);
		used[j] = true;
		m_identifier.push_back(rainbow[j]);
		if (i>0)
			m_magic_props[i].prob += m_magic_props[i - 1].prob;
	}

}

PotionInfo* s_potion_info; //todo: mem leaking this for now

int does_know_potion(int type)
{
	return s_potion_info->is_discovered(type);
}

void discover_potion(int type)
{
	s_potion_info->discover(type);
}

int get_potion_value(int type)
{
	return s_potion_info->m_magic_props[type].worth;
}

std::string get_potion_name(int type)
{
	return s_potion_info->m_magic_props[type].name;
}

std::string get_potion_guess(int type)
{
	return s_potion_info->get_guess(type);
}

void set_potion_guess(int type, const char* value)
{
	s_potion_info->set_guess(type, value);
}

//init_colors: Initialize the potion color scheme for this time
void init_colors()
{
	s_potion_info = new PotionInfo();
}

std::string get_color(int type)
{
	return s_potion_info->get_identifier(type);
}

void init_new_potion(Item* potion)
{
  potion->type = POTION;
  potion->which = pick_one(s_potion_info->m_magic_props);
}

void quaff_confusion()
{
  s_potion_info->discover(P_CONFUSE);
  if (!player.is_confused())
  {
    if (player.is_confused()) 
        lengthen(unconfuse, rnd(8)+HUH_DURATION);
    else 
        fuse(unconfuse, 0, rnd(8)+HUH_DURATION);
    player.set_confused(true);
    msg("wait, what's going on? Huh? What? Who?");
  }
}

void quaff_paralysis()
{
  s_potion_info->discover(P_PARALYZE);
  sleep_timer = HOLD_TIME;
  player.set_running(false);
  msg("you can't move");
}

void quaff_poison()
{
  char *sick = "you feel %s sick.";

  s_potion_info->discover(P_POISON);
  if (!is_wearing_ring(R_SUSTSTR)) {chg_str(-(rnd(3)+1)); msg(sick, "very");}
  else msg(sick, "momentarily");
}

void quaff_gain_strength()
{
  s_potion_info->discover(P_STRENGTH);
  chg_str(1);
  msg("you feel stronger. What bulging muscles!");
}

void quaff_see_invisible()
{
  if (!player.sees_invisible()) {
      fuse(unsee, 0, SEE_DURATION); 
      look(false);
      invis_on();
  }
  sight();
  msg("this potion tastes like %s juice", game->get_environment("fruit").c_str());
}

void quaff_healing()
{
  s_potion_info->discover(P_HEALING);
  player.stats.increase_hp(roll(player.stats.level, 4), true, false);
  sight();
  msg("you begin to feel better");
}

void quaff_monster_detection()
{
  fuse(turn_see_wrapper, true, HUH_DURATION);
  if (level_monsters.empty()) 
      msg("you have a strange feeling%s.", noterse(" for a moment"));
  else {
	  if (turn_see(false))
		  s_potion_info->discover(P_MFIND);
      msg("");
  }
}

void quaff_magic_detection()
{
  //Potion of magic detection.  Find everything interesting on the level and show him where they are. 
  //Also give hints as to whether he would want to use the object.
  if (!level_items.empty())
  {
    Agent *monster;
    bool show;

    show = false;
    for (auto it = level_items.begin(); it != level_items.end(); ++it)
    {
      Item* item = *it;
      if (is_magic(item))
      {
        show = true;
        mvaddch(item->pos.y, item->pos.x, goodch(item));
		s_potion_info->discover(P_TFIND);
      }
    }
    for (auto it = level_monsters.begin(); it != level_monsters.end(); ++it){
        monster = *it;
      for (auto it = monster->pack.begin(); it != monster->pack.end(); ++it)
      {
        if (is_magic(*it))
        {
          show = true;
          mvaddch(monster->pos.y, monster->pos.x, MAGIC);
		  s_potion_info->discover(P_TFIND);
        }
      }
    }
    if (show) {msg("You sense the presence of magic."); return;}
  }
  msg("you have a strange feeling for a moment%s.", noterse(", then it passes"));
}

void quaff_raise_level()
{
  s_potion_info->discover(P_RAISE);
  msg("you suddenly feel much more skillful");
  raise_level();
}

void quaff_extra_healing()
{
  s_potion_info->discover(P_XHEAL);
  player.stats.increase_hp(roll(player.stats.level, 8), true, true);
  sight();
  msg("you begin to feel much better");
}

void quaff_haste_self()
{
  s_potion_info->discover(P_HASTE);
  if (add_haste(true)) msg("you feel yourself moving much faster");
}

void quaff_restore_strength()
{
  if (is_ring_on_hand(LEFT, R_ADDSTR)) 
    add_str(&player.stats.str, -get_ring(LEFT)->ring_level);
  if (is_ring_on_hand(RIGHT, R_ADDSTR)) 
    add_str(&player.stats.str, -get_ring(RIGHT)->ring_level);
  if (player.stats.str < max_stats.str) 
    player.stats.str = max_stats.str;
  if (is_ring_on_hand(LEFT, R_ADDSTR)) 
    add_str(&player.stats.str, get_ring(LEFT)->ring_level);
  if (is_ring_on_hand(RIGHT, R_ADDSTR)) 
    add_str(&player.stats.str, get_ring(RIGHT)->ring_level);
  msg("%syou feel warm all over", noterse("hey, this tastes great.  It makes "));
}

void quaff_blindness()
{
  s_potion_info->discover(P_BLIND);
  if (!player.is_blind())
  {
    player.set_blind(true);
    fuse(sight, 0, SEE_DURATION);
    look(false);
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
  Item *obj;

  if ((obj = get_item("quaff", POTION))==NULL) return;
  //Make certain that it is something that we want to drink
  if (obj->type!=POTION) {msg("yuk! Why would you want to drink that?"); return;}
  if (obj==get_current_weapon()) 
    set_current_weapon(NULL);

  //Calculate the effect it has on the poor guy.
  potion_functions[obj->which]();

  status();
  s_potion_info->call_it2(obj->which);

  //Throw the item away
  if (obj->count>1) obj->count--;
  else {
    detach_item(player.pack, obj); 
    discard_item(obj);
  }
}

//invis_on: Turn on the ability to see invisible
void invis_on()
{
    player.set_sees_invisible(true);
    std::for_each(level_monsters.begin(), level_monsters.end(), [](Agent *monster){
        if (monster->is_invisible() && can_see_monster(monster))
        {
            mvaddch(monster->pos.y, monster->pos.x, monster->disguise);
        }
    });
}

void turn_see_wrapper(int turn_off)
{
    turn_see(turn_off != 0);
}

//turn_see: Put on or off seeing monsters on this level
bool turn_see(bool turn_off)
{
    bool add_new = false;

    std::for_each(level_monsters.begin(), level_monsters.end(), [turn_off, &add_new](Agent* monster){

        bool can_see;
        byte was_there;

        move(monster->pos.y, monster->pos.x);
        can_see = (can_see_monster(monster) || (was_there = curch()) == monster->type);
        if (turn_off)
        {
            if (!can_see_monster(monster) && monster->oldch != MDK) addch(monster->oldch);
        }
        else
        {
            if (!can_see) { standout(); monster->oldch = was_there; }
            addch(monster->type);
            if (!can_see) { standend(); add_new++; }
        }

    });

    player.set_detects_others(!turn_off);
    return add_new;
}

//th_effect: Compute the effect of this potion hitting a monster.
void affect_monster(Item *potion, Agent *monster)
{
  msg("the flask shatters.");

  switch (potion->which)
  {
  case P_CONFUSE: case P_BLIND:
    monster->set_confused(true);
    msg("the %s appears confused", monster->get_monster_name());
    break;

  case P_PARALYZE:
    monster->set_running(false);
    monster->set_is_held(true);
    break;

  case P_HEALING: case P_XHEAL:
      monster->stats.increase_hp(rnd(8), true, false);
    break;

  case P_RAISE:
    monster->stats.max_hp += 8;
    monster->stats.increase_hp(8, false, false);
    monster->stats.level++;
    break;

  case P_HASTE:
    monster->set_is_fast(true);
    break;
  }
}

int is_bad_potion(Item* obj)
{
  return obj && obj->type == POTION &&
    (obj->which == P_CONFUSE || obj->which == P_PARALYZE || obj->which == P_POISON || obj->which == P_BLIND);
}

const char* get_inv_name_potion(Item* obj)
{
  char *pb = prbuf;
  int which = obj->which;
  std::string color = get_color(which);

  if (obj->count==1) {
    strcpy(pb, "A potion ");
    pb = &prbuf[9];
  }
  else {
    sprintf(pb, "%d potions ", obj->count); 
    pb = &pb[strlen(prbuf)];
  }
  if (does_know_potion(which) || game->hero().is_wizard()) {
    chopmsg(pb, "of %s", "of %s(%s)", get_potion_name(which).c_str(), color.c_str());
  }
  else if (!get_potion_guess(which).empty()) {
    chopmsg(pb, "called %s", "called %s(%s)", get_potion_guess(which).c_str(), color.c_str());
  }
  else if (obj->count==1) 
    sprintf(prbuf, "A%s %s potion", vowelstr(color.c_str()), color.c_str());
  else sprintf(prbuf, "%d %s potions", obj->count, color.c_str());

  return prbuf;
}
