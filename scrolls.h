#pragma once
#include "item.h"

struct Scroll : public Item
{
    Scroll(int which);

    virtual Item* Clone() const;
    virtual std::string InventoryName() const;
};

Item* create_scroll();

void read_scroll();
int is_scare_monster_scroll(Item* item);
int is_bad_scroll(Item* item);
