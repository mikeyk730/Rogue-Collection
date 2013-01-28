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
