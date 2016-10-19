#pragma once
#include "item.h"

struct Food : public Item
{
    Food(int which);

    virtual Item* Clone() const override;
    virtual std::string TypeName() const override;
    virtual std::string InventoryName() const override;
    virtual bool IsMagic() const override;
    virtual bool IsEvil() const override;
    virtual int Worth() const override;
};

Item* CreateFood();