#pragma once
#include "item.h"

struct Monster;

struct Stick : public Item
{
    Stick(int which);

    virtual Item* Clone() const;
    virtual std::string Name() const;
    virtual std::string InventoryName() const;
    virtual bool IsEvil() const;
    virtual int Worth() const;
};

Item* create_stick();


//do_zap: Perform a zap with a wand
bool do_zap();

//drain: Do drain hit points from player schtick
void drain();

//fire_bolt: Fire a bolt in a given direction from a specific starting place.  Returns pointer to monster that may have been invalidated.  //todo:nix return value
Monster* fire_bolt(Coord *start, Coord *dir, const std::string& name);

//charge_str: Return an appropriate string for a wand charge
const char *get_charge_string(const Item *obj);
