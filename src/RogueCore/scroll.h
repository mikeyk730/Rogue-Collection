#pragma once
#include "item.h"

struct ItemCategory;

struct Scroll : public Item
{
    Scroll();

    virtual std::string Name() const override;
    virtual bool IsMagic() const override;
    virtual bool IsEvil() const override;
    virtual int Worth() const override;
    virtual std::string InventoryName() const override;

    virtual ItemCategory* Category() const;

    virtual void Read() = 0;
    virtual ItemCategory& Info() const = 0;
};

bool do_read_scroll();

std::string GenerateScrollName();
void PrintScrollDiscoveries();