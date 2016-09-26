#pragma once
#include "item.h"

struct Weapon : public Item
{
    Weapon(int which);
    Weapon(int which, int hit, int damage);

    virtual Item* Clone() const;
};

const char* get_weapon_name(int which);
Item* create_weapon();

//projectile: Fire a projectile in a given direction
void projectile(int ydelta, int xdelta);

//do_motion: Do the actual motion on the screen done by an object travelling across the room
void do_motion(Item *obj, int ydelta, int xdelta);

const char *short_name(Item *obj);

//fall: Drop an item someplace around here.
void fall(Item *obj, bool pr);

//hit_monster: Does the projectile hit the monster?
int hit_monster(Coord p, Item *obj);

//num: Figure out the plus number for armor/weapons
char *num(int n1, int n2, char type);

//wield: Pull out a certain weapon
void wield();

//fallpos: Pick a random position around the given (y, x) coordinates
int fallpos(Item *obj, Coord *newpos);

