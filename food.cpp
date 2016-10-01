#include <stdio.h>

#include "food.h"
#include "rogue.h"
#include "game_state.h"
#include "main.h"
#include "misc.h"

Food::Food(int which) :
    Item(FOOD, which)
{
}

Item * Food::Clone() const
{
    return new Food(*this);
}

std::string Food::Name() const
{
    return "food";
}

Item* Food::CreateFood()
{
    game->no_food = 0;
    int which = (rnd(10) != 0) ? 0 : 1;
    return new Food(which);
}

std::string Food::InventoryName() const
{
    std::string fruit = game->get_environment("fruit");

    char *pb = prbuf;

    if (m_which == 1)
        if (m_count == 1)
            sprintf(pb, "A%s %s", vowelstr(fruit.c_str()), fruit.c_str());
        else sprintf(pb, "%d %ss", m_count, fruit.c_str());
    else if (m_count == 1)
        strcpy(pb, "Some food");
    else sprintf(pb, "%d rations of food", m_count);

    return prbuf;
}

bool Food::IsMagic() const
{
    return false;
}

bool Food::IsEvil() const
{
    return false;
}

int Food::Worth() const
{
    return 2 * m_count;
}
