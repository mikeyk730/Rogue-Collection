#include <string.h>

#include "rogue.h"
#include "hero.h"
#include "rip.h"
#include "main.h"
#include "misc.h"
#include "pack.h"
#include "armor.h"
#include "weapons.h"
#include "io.h"
#include "thing.h"
#include "rings.h"

static char s_name[23];
static int s_purse = 0;
static int s_wizard = 0;
static int s_cheated = 0;
static int hungry_state = 0;   //How hungry is he
static int food_left = 0;       //Amount of food in hero's stomach

const char* get_name()
{
  return s_name;
}

void set_name(const char* name)
{
  strcpy(s_name, name);
}

int get_purse()
{
  return s_purse;
}

void adjust_purse(int delta)
{
  s_purse += delta;
  if (s_purse < 0) 
    s_purse = 0;
}

void set_wizard(int enable)
{
  s_wizard = enable;
  if (enable)
    s_cheated = TRUE;
}

int is_wizard()
{
  return s_wizard;
}

int did_cheat()
{
  return s_cheated;
}

void ingest()
{
  if (food_left<0)
    food_left = 0;
  if (food_left>(STOMACH_SIZE-20))
    no_command += 2+rnd(5);
  if ((food_left += HUNGER_TIME-200+rnd(400))>STOMACH_SIZE)
    food_left = STOMACH_SIZE;
  hungry_state = 0;
}

void digest()
{
  int oldfood, deltafood;

  if (food_left<=0)
  {
    if (food_left--<-STARVE_TIME) death('s');
    //the hero is fainting
    if (no_command || rnd(5)!=0) return;
    no_command += rnd(8)+4;
    player.flags &= ~ISRUN;
    running = FALSE;
    count = 0;
    hungry_state = 3;
    msg("%syou faint from lack of food", noterse("you feel very weak. "));
  }
  else
  {
    oldfood = food_left;
    //If you are in 40 column mode use food twice as fast (e.g. 3-(80/40) = 1, 3-(40/40) = 2 : pretty gross huh?)
    deltafood = ring_eat(LEFT)+ring_eat(RIGHT)+1;
    if (in_small_screen_mode()) deltafood *= 2;
    food_left -= deltafood;
    if (food_left<MORE_TIME && oldfood>=MORE_TIME) {hungry_state = 2; msg("you are starting to feel weak");}
    else if (food_left<2*MORE_TIME && oldfood>=2*MORE_TIME) {hungry_state = 1; msg("you are starting to get hungry");}
  }
}

//init_player: Roll up the rogue
void init_player()
{
  ITEM *obj;

  player.stats = max_stats;
  food_left = HUNGER_TIME;

  //Give the rogue his weaponry.  First a mace.
  obj = create_item(WEAPON, MACE);
  init_weapon(obj, MACE);
  obj->hit_plus = 1;
  obj->damage_plus = 1;
  obj->flags |= ISKNOW;
  add_pack(obj, TRUE);
  set_current_weapon(obj);

  //Now a +1 bow
  obj = create_item(WEAPON, BOW);
  init_weapon(obj, BOW);
  obj->hit_plus = 1;
  obj->damage_plus = 0;
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
  obj->armor_class = get_default_class(RING_MAIL)-1;
  obj->flags |= ISKNOW;
  set_current_armor(obj);
  add_pack(obj, TRUE);

  //Give him some food too
  obj = create_item(FOOD, 0);
  add_pack(obj, TRUE);
}

int get_hungry_state()
{
  return hungry_state;
}

int get_food_left()
{
    return food_left;
}
