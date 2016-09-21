#pragma once
#include "item.h"

struct Food : public Item
{
    Food(int which);

    virtual Item* Clone() const;
};

const char* get_inv_name_food(Item* food);

Item* create_food();

//eat: She wants to eat something, so let her try
void eat();
