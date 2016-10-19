#pragma once
#include "item.h"

struct Potion : public Item
{
    Potion();

    virtual std::string TypeName() const override;
    virtual std::string InventoryName() const override;
    virtual bool IsMagic() const override;
    virtual bool IsEvil() const override;
    virtual int Worth() const override;

    virtual void Quaff() = 0;
    virtual void AffectMonster(Monster* m);
};

//do_quaff: Quaff a potion from the pack
bool do_quaff();

void affect_monster(Potion *potion, Monster *monster);

//show_invisible: Turn on the ability to see invisible
void show_invisible();

//detect_monsters: Put on or off seeing monsters on this level
bool detect_monsters(bool enable);
void disable_detect_monsters(int disable);
