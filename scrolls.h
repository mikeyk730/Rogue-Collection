#pragma once
#include "item.h"

struct Scroll : public Item
{
    Scroll(int which);

    virtual Item* Clone() const;
    virtual std::string Name() const;
    virtual std::string InventoryName() const;
    virtual bool IsEvil() const;
    virtual int Worth() const;
};

Item* create_scroll();

bool read_scroll();
int is_scare_monster_scroll(Item* item);
