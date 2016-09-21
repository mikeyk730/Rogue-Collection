#pragma once
#include "item.h"

struct Potion : public Item
{
    Potion(int which);

    virtual Item* Clone() const;
};

Item* create_potion();

//quaff: Quaff a potion from the pack
void quaff();

//invis_on: Turn on the ability to see invisible
void invis_on();

//turn_see: Put on or off seeing monsters on this level
bool turn_see(bool turn_off);
void turn_see_wrapper(int turn_off);

//th_effect: Compute the effect of this potion hitting a monster.
void affect_monster(Item *obj, Agent *monster);
int is_bad_potion(Item* obj);
