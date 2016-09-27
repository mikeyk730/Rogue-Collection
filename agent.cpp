#include "agent.h"
#include "rings.h"
#include "pack.h"

void Agent::calculate_roll_stats(Agent *the_defender, Item *weapon, bool hurl,
    int* hit_plus, std::string* damage_string, int* damage_plus)
{
    *damage_string = stats.damage;
    *hit_plus = 0;
    *damage_plus = 0;
}

int Agent::calculate_armor() const
{
    return stats.ac;
}

int Agent::armor_for_display() const
{
    int a = calculate_armor();
    return (-((a)-11));
}

int Agent::calculate_strength() const
{
    return stats.m_str;
}

int Agent::calculate_max_strength() const
{
    return stats.m_max_str;
}

void Agent::restore_strength()
{
    stats.m_str = stats.m_max_str;
}

void Agent::adjust_strength(int amt)
{
    stats.m_str += amt;
    if (stats.m_str > 31)
        stats.m_str = 31;
    if (stats.m_str > stats.m_max_str)
        stats.m_max_str = stats.m_str;
}

int Agent::level() const
{
    return stats.level;
}

int Agent::experience() const
{
    return stats.m_exp;
}

void Agent::gain_experience(int exp)
{
    stats.m_exp += exp;
}

std::string Agent::damage_string() const
{
    return stats.damage;
}

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

Coord Agent::position() const
{
    return pos;
}
