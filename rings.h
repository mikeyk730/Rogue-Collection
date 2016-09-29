#pragma once
#include "item.h"

struct Ring : public Item
{
    Ring(int which);
    Ring(int which, int level);

    virtual Item* Clone() const;
    virtual std::string Name() const;
    virtual std::string InventoryName() const;
    virtual bool IsEvil() const;
    virtual int Worth() const;

    int get_ring_level() const;

private:
    short ring_level = 0;
};

Item* create_ring();

//ring_eat: How much food does this ring use up?
int ring_eat(int hand);

//ring_num: Print ring bonuses
char *ring_num(const Ring *obj);
