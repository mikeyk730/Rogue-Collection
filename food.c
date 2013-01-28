#include "rogue.h"
#include "food.h"
#include "pack.h"
#include "misc.h"
#include "main.h"
#include "fight.h"
#include "io.h"
#include "thing.h"
#include "list.h"

void init_new_food(ITEM* food)
{
  no_food = 0;
  food->type = FOOD;
  if (rnd(10)!=0) food->which = 0; else food->which = 1;
}

//eat: She wants to eat something, so let her try
void eat()
{
  ITEM *obj;

  if ((obj = get_item("eat", FOOD))==NULL) 
    return;
  if (obj->type!=FOOD) {
    msg("ugh, you would get ill if you ate that"); 
    return;
  }
  if (--obj->count<1) {
    detach_item(&player.pack, obj); 
    discard_item(obj);
  }
  if (food_left<0)
    food_left = 0;
  if (food_left>(STOMACH_SIZE-20))
    no_command += 2+rnd(5);
  if ((food_left += HUNGER_TIME-200+rnd(400))>STOMACH_SIZE)
    food_left = STOMACH_SIZE;
  hungry_state = 0;
  if (obj==cur_weapon) 
    cur_weapon = NULL;
  if (obj->which==1)
    msg("my, that was a yummy %s", fruit);
  else if (rnd(100)>70) {
    player.stats.exp++; 
    msg("yuk, this food tastes awful");
    check_level();
  }
  else 
    msg("yum, that tasted good");
  if (no_command) 
    msg("You feel bloated and fall asleep");
}
