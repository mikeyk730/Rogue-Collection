#pragma once
#include "item.h"

struct Ring : public Item
{
    Ring(int which);
    Ring(int which, int level);

    virtual Item* Clone() const;
};

Item* create_ring();

//ring_on: Put a ring on a hand
bool ring_on();

//ring_eat: How much food does this ring use up?
int ring_eat(int hand);

//ring_num: Print ring bonuses
char *ring_num(Item *obj);
