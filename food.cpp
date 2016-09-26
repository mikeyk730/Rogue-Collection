#include <stdio.h>

#include "rogue.h"
#include "game_state.h"
#include "food.h"
#include "pack.h"
#include "misc.h"
#include "main.h"
#include "fight.h"
#include "io.h"
#include "thing.h"
#include "list.h"
#include "hero.h"

Item* create_food()
{
    no_food = 0;
    int which = (rnd(10) != 0) ? 0 : 1;
    return new Food(which);
}

//eat: She wants to eat something, so let her try
void eat()
{
  Item *obj;

  if ((obj = get_item("eat", FOOD))==NULL) 
    return;
  if (obj->type!=FOOD) {
    msg("ugh, you would get ill if you ate that"); 
    return;
  }
  if (--obj->count<1) {
    game->hero().pack.remove(obj);
    delete obj;
  }
  game->hero().ingest();
  if (obj==get_current_weapon()) 
    set_current_weapon(NULL);
  if (obj->which==1)
    msg("my, that was a yummy %s", game->get_environment("fruit").c_str());
  else if (rnd(100)>70) {
    game->hero().stats.exp++; 
    msg("yuk, this food tastes awful");
    check_level();
  }
  else 
    msg("yum, that tasted good");
  if (sleep_timer) 
    msg("You feel bloated and fall asleep");
}

const char* get_inv_name_food(Item* obj)
{
    std::string fruit = game->get_environment("fruit");

    char *pb = prbuf;
    int which = obj->which;

    if (which == 1)
    if (obj->count == 1)
        sprintf(pb, "A%s %s", vowelstr(fruit.c_str()), fruit.c_str());
    else sprintf(pb, "%d %ss", obj->count, fruit.c_str());
    else if (obj->count == 1)
        strcpy(pb, "Some food");
    else sprintf(pb, "%d rations of food", obj->count);

    return prbuf;
}

Food::Food(int which) :
    Item(FOOD, which)
{ }

Item * Food::Clone() const
{
    return new Food(*this);
}
