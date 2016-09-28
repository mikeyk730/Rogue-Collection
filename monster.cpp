#include "monster.h"
#include "main.h"
#include "monsters.h"
#include "item.h"

bool Monster::can_divide() const
{
    return (exflags & EX_DIVIDES) != 0;
}

bool Monster::is_stationary() const {
    return (exflags & EX_STATIONARY) != 0;
}

bool Monster::can_hold() const {
    return (exflags & EX_HOLDS) != 0;
}

bool Monster::hold_attacks() const {
    return (exflags & EX_HOLD_ATTACKS) != 0;
}

bool Monster::shoots_fire() const {
    return (exflags & EX_SHOOTS_FIRE) != 0;
}

bool Monster::immune_to_fire() const {
    return shoots_fire();
}

bool Monster::shoots_ice() const {
    return (exflags & EX_SHOOTS_ICE) != 0;
}

bool Monster::causes_confusion() const {
    return (exflags & EX_CONFUSES) != 0;
}

bool Monster::is_mimic() const {
    return (exflags & EX_MIMICS) != 0;
}

bool Monster::is_disguised() const {
    return is_mimic() && type != disguise;
}

bool Monster::drops_gold() const {
    return (exflags & EX_DROPS_GOLD) != 0;
}

bool Monster::steals_gold() const {
    return (exflags & EX_STEALS_GOLD) != 0;
}

bool Monster::steals_magic() const {
    return (exflags & EX_STEALS_MAGIC) != 0;
}

bool Monster::drains_life() const {
    return (exflags & EX_DRAINS_MAXHP) != 0;
}

bool Monster::drains_exp() const {
    return (exflags & EX_DRAINS_EXP) != 0;
}

bool Monster::drains_strength() const {
    return (exflags & EX_DRAINS_STR) != 0;
}

bool Monster::rusts_armor() const {
    return (exflags & EX_RUSTS_ARMOR) != 0;
}

bool Monster::dies_from_attack() const {
    return (exflags & EX_SUICIDES) != 0;
}

std::string Monster::get_name()
{
    return get_monster_name(type);
}

//todo: make configurable
bool Monster::is_monster_confused_this_turn() const {
    return ((is_confused() && rnd(5) != 0) ||
        // Phantoms are slightly confused all of the time, and bats are quite confused all the time
        type == 'P' && rnd(5) == 0 ||
        type == 'B' && rnd(2) == 0);
}

void Monster::reveal_disguise() {
    disguise = type;
}

void Monster::set_dirty(bool enable) {
    value = enable ? 1 : 0;
}

bool Monster::is_dirty() {
    return value == 1;
}

//start_run: Set a monster running after something or stop it from running (for when it dies)
void Monster::start_run()
{
    //Start the beastie running
    set_running(true);
    set_is_held(false);
    set_destination();
}

void Monster::set_destination()
{
    dest = find_dest();
}

bool Monster::is_seeking(Item * obj)
{
    return dest == &obj->pos;
}
