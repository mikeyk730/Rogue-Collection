#pragma once
#include "item.h"

#define NONE 100

struct Monster;

struct Weapon : public Item
{
    Weapon(int which);
    Weapon(int which, int hit, int damage);

    virtual Item* Clone() const;
    virtual std::string Name() const;
    virtual std::string InventoryName() const;
    virtual bool IsEvil() const;
    virtual int Worth() const;

    bool zap_vorpalized_weapon(Coord dir);
protected:
    std::string m_name;
};

Item* create_weapon();

//throw_projectile: Fire a projectile in a given direction
bool throw_projectile();

//do_motion: Do the actual motion on the screen done by an object travelling across the m_room
void do_motion(Item *obj, Coord delta);

//fall: Drop an item someplace around here.
void fall(Item *obj, bool pr);

//projectile_hit: Does the projectile hit the monster?  Returns pointer to monster that may have been invalidated.  //todo:nix return value
Monster* projectile_hit(Coord p, Item *obj);

//num: Figure out the plus number for armor/weapons
char *num(int n1, int n2, char type);

//fallpos: Pick a random position around the given (y, x) coordinates
int fallpos(Item *obj, Coord *newpos);

