#pragma once

#include "item.h"

struct Gold : public Item
{
    Gold(int value);

    virtual Item* Clone() const override;
    virtual std::string TypeName() const override;
    virtual std::string InventoryName() const override;
    virtual bool IsMagic() const override;
    virtual bool IsEvil() const override;
    virtual int Worth() const override;

    int get_gold_value() const;

    short gold_value = 0;
};
