#pragma once

#include "item.h"

struct Amulet : public Item
{
    Amulet();

    virtual Item* Clone() const;
    virtual std::string Name() const;
    virtual std::string InventoryName() const;
    virtual bool IsMagic() const;
    virtual bool IsEvil() const;
    virtual int Worth() const;
};
