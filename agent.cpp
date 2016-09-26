#include "agent.h"
#include "rings.h"
#include "pack.h"

void Agent::set_invisible(bool enable){
    set_flag(IS_INVIS, enable);
}
void Agent::set_found(bool enable){
    set_flag(IS_FOUND, enable);
}
void Agent::set_confused(bool enable){
    set_flag(IS_HUH, enable);
}
void Agent::set_running(bool enable){
    set_flag(IS_RUN, enable);
}
void Agent::set_is_held(bool enable){
    set_flag(IS_HELD, enable);
}
void Agent::set_is_slow(bool enable){
    set_flag(IS_SLOW, enable);
}
void Agent::set_is_fast(bool enable){
    set_flag(IS_HASTE, enable);
}
void Agent::set_can_confuse(bool enable){
    set_flag(CAN_HUH, enable);
}
void Agent::set_cancelled(bool enable){
    set_flag(IS_CANC, enable);
}
void Agent::set_blind(bool enable){
    set_flag(IS_BLIND, enable);
}
void Agent::set_sees_invisible(bool enable){
    set_flag(CAN_SEE, enable);
}
void Agent::set_detects_others(bool enable){
    set_flag(SEE_MONST, enable);
}
void Agent::set_is_mean(bool enable){
    set_flag(IS_MEAN, enable);
}

void Agent::reveal_disguise(){
    disguise = type;
}

//slime specific
void Agent::set_dirty(bool enable){
    value = enable ? 1 : 0;
}
bool Agent::is_dirty(){
    return value == 1;
}

int Agent::get_hp() const {
    return stats.hp;
}

bool Agent::decrease_hp(int n, bool can_kill){
    if (invunerable)
        return true;

    stats.hp -= n;
    if (!can_kill && stats.hp <= 0)
        stats.hp = 1;
    return stats.hp > 0;
}

void Agent::increase_hp(int n, bool max_bonus, bool second_max_bonus){
    stats.hp += n;

    if (max_bonus && stats.hp > stats.max_hp)
        ++stats.max_hp;
    if (second_max_bonus && stats.hp > stats.max_hp + stats.level + 1)
        ++stats.max_hp;

    if (stats.hp > stats.max_hp) {
        stats.hp = stats.max_hp;
    }
}

int Agent::drain_hp(){
    stats.hp /= 2;
    return stats.hp;
}

//add_str: Perform the actual add, checking upper and lower bound
void add_str(unsigned int *sp, int amt)
{
    if ((*sp += amt)<3) *sp = 3;
    else if (*sp>31) *sp = 31;
}

void Agent::adjust_strength(int amt)
{
    unsigned int comp;

    if (amt == 0) return;
    add_str(&stats.str, amt);
    comp = stats.str;
    if (is_ring_on_hand(LEFT, R_ADDSTR))
        add_str(&comp, -get_ring(LEFT)->get_ring_level());
    if (is_ring_on_hand(RIGHT, R_ADDSTR))
        add_str(&comp, -get_ring(RIGHT)->get_ring_level());
    if (comp > stats.max_str)
        stats.max_str = comp;
}

void Agent::restore_strength()
{
    if (is_ring_on_hand(LEFT, R_ADDSTR))
        add_str(&stats.str, -get_ring(LEFT)->get_ring_level());
    if (is_ring_on_hand(RIGHT, R_ADDSTR))
        add_str(&stats.str, -get_ring(RIGHT)->get_ring_level());
    if (stats.str < stats.max_str)
        stats.str = stats.max_str;
    if (is_ring_on_hand(LEFT, R_ADDSTR))
        add_str(&stats.str, get_ring(LEFT)->get_ring_level());
    if (is_ring_on_hand(RIGHT, R_ADDSTR))
        add_str(&stats.str, get_ring(RIGHT)->get_ring_level());
}

Coord Agent::position() const
{
    return pos;
}
