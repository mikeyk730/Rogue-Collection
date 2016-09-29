#pragma once
#include "item.h"

struct Armor : public Item
{
    Armor(int which);
    Armor(int which, int ac_mod);

    virtual Item* Clone() const;
    virtual std::string InventoryName() const;
};

int get_default_class(int type);
Item* create_armor();

//waste_time: Do nothing but let other things happen
void waste_time();