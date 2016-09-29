#pragma once

#include "main.h"
#include "item.h"
#include "agent.h"
#include "rogue.h"
#include "armor.h"

Item::Item(int type, int which, const std::string& name) :
    m_name(name)
{
    initialize(type, which);
}

Item::~Item()
{
}

Coord Item::position() const
{
    return pos;
}

void Item::set_position(Coord p)
{
    pos = p;
}

int Item::get_ring_level() const
{
    return ring_level;
}

int Item::get_charges() const
{
    return charges;
}

void Item::use_charge()
{
    if (--charges < 1)
        charges = 0;
}

void Item::drain_striking()
{
    if (--charges < 0) {
        damage = "0d0";
        hit_plus = 0;
        damage_plus = 0;
        charges = 0;
    }
}

void Item::randomize_damage()
{
    if (rnd(20) == 0) {
        damage = "3d8";
        damage_plus = 9;
    }
    else {
        damage = "2d8";
        damage_plus = 4;
    }
}

int Item::get_armor_class() const
{
    return armor_class;
}

int Item::armor_class_for_display() const
{
    int a = get_armor_class();
    return (-((a)-11));
}

void Item::enchant_armor()
{
    armor_class--;
    remove_curse();
}

void Item::weaken_armor()
{
    armor_class++;
}

int Item::get_gold_value() const
{
    return gold_value;
}

void Item::initialize(int type, int which)
{
    this->type = type;
    this->which = which;
    this->hit_plus = 0;
    this->damage_plus = 0;
    this->damage = "0d0";
    this->throw_damage = "0d0";
    this->armor_class = 11;
    this->gold_value = 0;
    this->ring_level = 0;
    this->charges = 0;
    this->count = 1;
    this->group = 0;
    this->flags = 0;
    this->enemy = 0;
}

bool Item::is_flag_set(short flag) const {
    return (flags & flag) != 0;
}

bool Item::is_known() const
{
    return is_flag_set(IS_KNOW);
}
bool Item::is_cursed() const
{
    return is_flag_set(IS_CURSED);
}
bool Item::did_flash() const
{
    return is_flag_set(DID_FLASH);
}
bool Item::is_projectile() const
{
    return is_flag_set(IS_MISL);
}
bool Item::does_group() const
{
    return is_flag_set(IS_MANY);
}
bool Item::is_revealed() const
{
    return is_flag_set(IS_REVEAL);
}
bool Item::is_found() const
{
    return is_flag_set(IS_FOUND);
}

void Item::remove_curse()
{
    flags &= ~IS_CURSED;
}

void Item::set_known() {
    flags |= IS_KNOW;
}
void Item::set_cursed() {
    flags |= IS_CURSED;
}
void Item::set_revealed() {
    flags |= IS_REVEAL;
}
void Item::set_found() {
    flags |= IS_FOUND;
}
void Item::set_flashed() {
    flags |= DID_FLASH;
}

Amulet::Amulet() : Item(AMULET, 0, "amulet")
{
    this->hit_plus = 0;
    this->damage_plus = 0;
    this->damage = "0d0";
    this->throw_damage = "0d0";
    this->armor_class = 11;
}

Item * Amulet::Clone() const
{
    return new Amulet(*this);
}

Gold::Gold(int value) :
    Item(GOLD, 0, "gold")
{
    gold_value = value;
}

Item * Gold::Clone() const
{
    return new Gold(*this);
}

//is_magic: Returns true if an object radiates magic
bool Item::is_magic()
{
    switch (type)
    {
    case ARMOR:
        return get_armor_class() != get_default_class(which);
    case WEAPON:
        return get_hit_plus() != 0 || get_damage_plus() != 0;
    case POTION: case SCROLL: case STICK: case RING: case AMULET:
        return true;
    }
    return false;
}

int does_item_group(int type)
{
    return (type == POTION || type == SCROLL || type == FOOD || type == GOLD);
}
