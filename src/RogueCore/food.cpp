#include <sstream>
#include <stdio.h>

#include "food.h"
#include "rogue.h"
#include "game_state.h"
#include "main.h"
#include "misc.h"

Item* Food::CreateFood()
{
    game->no_food = 0;
    int which = (rnd(10) != 0) ? 0 : 1;
    return new Food(which);
}

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

std::string Food::InventoryName() const
{
    std::string fruit = game->get_environment("fruit");
    std::ostringstream ss;

    if (m_which == 1)
        if (m_count == 1)
            ss << "A" << vowelstr(fruit) << " " << fruit;
        else
            ss << m_count << " " << fruit << "s";
    else
        if (m_count == 1)
            ss << "Some food";
        else
            ss << m_count << " rations of food";

    return ss.str();
}
