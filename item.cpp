#pragma once

#include "main.h"
#include "item.h"
#include "agent.h"
#include "rogue.h"
#include "rooms.h"

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

void Item::initialize(int type, int which)
{
    this->m_type = type;
    this->m_which = which;
    this->m_launcher = NONE;
    this->m_hit_plus = 0;
    this->m_damage_plus = 0;
    this->m_damage = "0d0";
    this->m_throw_damage = "0d0";
    this->m_charges = 0;
    this->m_count = 1;
    this->m_group = 0;
    this->m_flags = 0;
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

std::string Item::get_throw_damage() const
{
    return m_throw_damage;
}

std::string Item::get_damage() const
{
    return m_damage;
}

char Item::launcher() const
{
    return m_launcher;
}

Room* Item::get_room()
{
    return get_room_from_position(m_position);
}

int does_item_group(int type)
{
    return (type == POTION || type == SCROLL || type == FOOD || type == GOLD);
}

int Item::get_hit_plus() const
{
    return m_hit_plus;
}

int Item::get_damage_plus() const
{
    return m_damage_plus;
}
