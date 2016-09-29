#include <stdio.h>

#include "rogue.h"
#include "game_state.h"
#include "food.h"
#include "pack.h"
#include "misc.h"
#include "main.h"
#include "fight.h"
#include "io.h"
#include "list.h"
#include "hero.h"

Item* create_food()
{
    game->no_food = 0;
    int which = (rnd(10) != 0) ? 0 : 1;
    return new Food(which);
}

void eat()
{
    Item* obj = get_item("eat", FOOD);
    if (obj == nullptr)
        return;
    game->hero().eat(obj);
}

const char* Item::get_inv_name_food()
{
    std::string fruit = game->get_environment("fruit");

    char *pb = prbuf;

    if (which == 1)
        if (this->count == 1)
            sprintf(pb, "A%s %s", vowelstr(fruit.c_str()), fruit.c_str());
        else sprintf(pb, "%d %ss", this->count, fruit.c_str());
    else if (this->count == 1)
        strcpy(pb, "Some food");
    else sprintf(pb, "%d rations of food", this->count);

    return prbuf;
}

Food::Food(int which) :
    Item(FOOD, which, "food")
{ 
}

Item * Food::Clone() const
{
    return new Food(*this);
}
