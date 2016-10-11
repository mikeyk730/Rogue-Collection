#pragma once
#include "item.h"

struct ItemCategory;

struct Scroll : public Item
{
    Scroll();

    virtual std::string TypeName() const override;
    virtual bool IsMagic() const override;
    virtual bool IsEvil() const override;
    virtual int Worth() const override;
    virtual std::string InventoryName() const override;

    virtual void Read() = 0;
};

bool do_read_scroll();

std::string GenerateScrollName();
void PrintScrollDiscoveries();