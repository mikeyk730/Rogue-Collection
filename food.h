#pragma once
#include "item.h"

struct Food : public Item
{
    Food(int which);

    virtual Item* Clone() const;
};

Item* create_food();

//eat: She wants to eat something, so let her try
void eat();
