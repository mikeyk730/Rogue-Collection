#pragma once
#include "item.h"

struct Armor : public Item
{
    Armor(int which);
    Armor(int which, int ac_mod);

    virtual Item* Clone() const;
};

int get_default_class(int type);
Item* create_armor();

const char* get_armor_name(int type);

//wear: The player wants to wear something, so let him/her put it on.
void wear();

//waste_time: Do nothing but let other things happen
void waste_time();