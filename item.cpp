#pragma once

#include "main.h"
#include "item.h"
#include "agent.h"
#include "rogue.h"
#include "armor.h"
#include "weapons.h"

Item::Item(int type, int which)
{
    initialize(type, which);
}

Item::~Item()
{
}

Coord Item::position() const
{
    return m_position;
}

void Item::set_position(Coord p)
{
    m_position = p;
}

int Item::get_charges() const
{
    return m_charges;
}

void Item::use_charge()
{
    if (--m_charges < 1)
        m_charges = 0;
}

void Item::drain_striking()
{
    if (--m_charges < 0) {
        m_damage = "0d0";
        m_hit_plus = 0;
        m_damage_plus = 0;
        m_charges = 0;
    }
}

void Item::randomize_damage()
{
    if (rnd(20) == 0) {
        m_damage = "3d8";
        m_damage_plus = 9;
    }
    else {
        m_damage = "2d8";
        m_damage_plus = 4;
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
    this->m_type = type;
    this->m_which = which;
    this->m_launcher = NONE;
    this->m_hit_plus = 0;
    this->m_damage_plus = 0;
    this->m_damage = "0d0";
    this->m_throw_damage = "0d0";
    this->armor_class = 11;
    this->gold_value = 0;
    this->m_charges = 0;
    this->m_count = 1;
    this->m_group = 0;
    this->m_flags = 0;
    this->enemy = 0;
}

bool Item::is_flag_set(short flag) const {
    return (m_flags & flag) != 0;
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
    m_flags &= ~IS_CURSED;
}

void Item::set_known() {
    m_flags |= IS_KNOW;
}
void Item::set_cursed() {
    m_flags |= IS_CURSED;
}
void Item::set_revealed() {
    m_flags |= IS_REVEAL;
}
void Item::set_found() {
    m_flags |= IS_FOUND;
}
void Item::set_flashed() {
    m_flags |= DID_FLASH;
}

//is_magic: Returns true if an object radiates magic
bool Item::is_magic()
{
    switch (m_type)
    {
    case ARMOR:
        return get_armor_class() != get_default_class(m_which);
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
