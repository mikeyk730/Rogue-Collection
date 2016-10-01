#pragma once
#include "item.h"

struct Scroll : public Item
{
    Scroll(int which);

    virtual Item* Clone() const;
    virtual std::string Name() const;
    virtual std::string InventoryName() const;
    virtual bool IsMagic() const;
    virtual bool IsEvil() const;
    virtual int Worth() const;

    //todo: each scroll should be a class
    void read_monster_confusion();
    void read_magic_mapping();
    void read_hold_monster();
    void read_sleep();
    void read_enchant_armor();
    void read_identify();
    void read_scare_monster();
    void read_food_detection();
    void read_teleportation();
    void read_enchant_weapon();
    void read_create_monster();
    void read_remove_curse();
    void read_aggravate_monsters();
    void read_blank_paper();
    void read_vorpalize_weapon();

};

Item* create_scroll();

bool read_scroll();
int is_scare_monster_scroll(Item* item);
