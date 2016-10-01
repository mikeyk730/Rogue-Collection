#pragma once
#include "item.h"

struct Monster;

struct Stick : public Item
{
    Stick(int which);

    virtual Item* Clone() const;
    virtual std::string Name() const;
    virtual std::string InventoryName() const;
    virtual bool IsMagic() const;
    virtual bool IsEvil() const;
    virtual int Worth() const;

    bool zap_light(Coord dir);
    bool zap_striking(Coord dir);
    bool zap_lightning(Coord dir);
    bool zap_fire(Coord dir);
    bool zap_cold(Coord dir);
    bool zap_polymorph(Coord dir);
    bool zap_magic_missile(Coord dir);
    bool zap_haste_monster(Coord dir);
    bool zap_slow_monster(Coord dir);
    bool zap_drain_life(Coord dir);
    bool zap_nothing(Coord dir);
    bool zap_teleport_away(Coord dir);
    bool zap_teleport_to(Coord dir);
    bool zap_cancellation(Coord dir);

    void drain_striking();
    void set_striking_damage();
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
