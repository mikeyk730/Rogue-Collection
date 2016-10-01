#pragma once

#include "item.h"

struct Gold : public Item
{
    Gold(int value);

    virtual Item* Clone() const;
    virtual std::string Name() const;
    virtual std::string InventoryName() const;
    virtual bool IsMagic() const;
    virtual bool IsEvil() const;
    virtual int Worth() const;

    int get_gold_value() const;

    short gold_value = 0;
};
