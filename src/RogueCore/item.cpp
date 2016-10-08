#include "main.h"
#include "item.h"
#include "agent.h"
#include "util.h"
#include "rooms.h"
#include "game_state.h"
#include "hero.h"
#include "rings.h"
#include "armor.h"
#include "level.h"

Item::Item(int type, int which)
{
    m_type = type;
    m_which = which;
    m_launcher = NONE;
    m_hit_plus = 0;
    m_damage_plus = 0;
    m_damage = "0d0";
    m_throw_damage = "0d0";
    m_charges = 0;
    m_count = 1;
    m_group = 0;
    m_flags = 0;
}

Item::~Item()
{
}

//inv_name: Return the name of something as it would appear in an inventory.
std::string Item::inventory_name(const Hero& hero, bool lowercase) const
{
    std::string name = InventoryName();

    if (this == hero.get_current_armor())
        name += " (being worn)";
    if (this == hero.get_current_weapon())
        name += " (weapon in hand)";
    if (this == hero.get_ring(LEFT))
        name += " (on left hand)";
    else if (this == hero.get_ring(RIGHT))
        name += " (on right hand)";

    if (lowercase && isupper(name[0]))
        name[0] = tolower(name[0]);
    else if (!lowercase && islower(name[0]))
        name[0] = toupper(name[0]);

    return name;
}

std::string Item::name() const
{
    return Name();
}

bool Item::is_magic() const
{
    return IsMagic();
}

bool Item::is_evil() const
{
    return IsEvil();
}

int Item::worth() const
{
    return Worth();
}

const Coord Item::position() const
{
    return m_position;
}

void Item::set_position(Coord p)
{
    m_position = p;
}

int Item::charges() const
{
    return m_charges;
}

void Item::use_charge()
{
    if (--m_charges < 1)
        m_charges = 0;
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

std::string Item::throw_damage() const
{
    return m_throw_damage;
}

std::string Item::melee_damage() const
{
    return m_damage;
}

char Item::launcher() const
{
    return m_launcher;
}

Room* Item::room() const
{
    return game->level().get_room_from_position(m_position);
}

int does_item_group(int type)
{
    return (type == POTION || type == SCROLL || type == FOOD || type == GOLD);
}

int Item::hit_plus() const
{
    return m_hit_plus;
}

int Item::damage_plus() const
{
    return m_damage_plus;
}
