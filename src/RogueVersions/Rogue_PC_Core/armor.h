#pragma once
#include "item.h"

struct Armor : public Item
{
    Armor(int which);
    Armor(int which, int ac_mod);

    virtual Item* Clone() const override;
    virtual std::string TypeName() const override;
    virtual std::string InventoryName() const override;
    virtual bool IsMagic() const override;
    virtual bool IsEvil() const override;
    virtual int Worth() const override;

    int armor_class() const;
    void enchant_armor();
    void weaken_armor();

    static int for_display(int ac);
private:
    short m_armor_class;
};

int get_default_class(int type);
Item* create_armor();

//waste_time: Do nothing but let other things happen
void waste_time();