#pragma once
#include "item.h"

struct Agent;

struct Potion : public Item
{
    Potion(int which);

    virtual Item* Clone() const;
    virtual std::string Name() const;
    virtual std::string InventoryName() const;
    virtual bool IsEvil() const;
    virtual int Worth() const;
};

Item* create_potion();

//quaff: Quaff a potion from the pack
bool quaff();

//show_invisible: Turn on the ability to see invisible
void show_invisible();

//turn_see: Put on or off seeing monsters on this level
bool turn_see(bool turn_off);
void turn_see_wrapper(int turn_off);

//th_effect: Compute the effect of this potion hitting a monster.
void affect_monster(Item *obj, Agent *monster);
