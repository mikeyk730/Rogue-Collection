#pragma once

#include "item.h"
#include "agent.h"

void Item::initialize(int type, int which)
{
    this->type = type;
    this->which = which;
    this->hit_plus = 0;
    this->damage_plus = 0;
    this->damage = "0d0";
    this->throw_damage = "0d0";
    this->armor_class = 11;
    this->count = 1;
    this->group = 0;
    this->flags = 0;
    this->enemy = 0;
}

bool Item::is_flag_set(short flag) const{
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
bool Item::is_missile() const
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

void Item::set_known(){
    flags |= IS_KNOW;
}
void Item::set_cursed(){
    flags |= IS_CURSED;
}
void Item::set_revealed(){
    flags |= IS_REVEAL;
}
void Item::set_found(){
    flags |= IS_FOUND;
}
void Item::set_flashed(){
    flags |= DID_FLASH;
}
