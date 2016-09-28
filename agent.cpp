#include <sstream>
#include <algorithm>
#include "agent.h"
#include "main.h"
#include "fight.h"
#include "game_state.h"
#include "hero.h"
#include "level.h"

bool Agent::is_flag_set(short flag) const {
    return ((flags & flag) != 0);
}

void Agent::set_flag(short flag, bool enable) {
    if (enable)
        flags |= flag;
    else
        flags &= ~flag;
}

bool Agent::is_flying() const {
    return is_flag_set(IS_FLY);
}

bool Agent::is_mean() const {
    return is_flag_set(IS_MEAN);
}

bool Agent::regenerates_hp() const {
    return is_flag_set(IS_REGEN);
}

bool Agent::is_greedy() const {
    return is_flag_set(IS_GREED);
}

bool Agent::is_invisible() const {
    return is_flag_set(IS_INVIS);
}

bool Agent::is_confused() const {
    return is_flag_set(IS_HUH);
}

bool Agent::is_held() const {
    return is_flag_set(IS_HELD);
}

bool Agent::is_blind() const {
    return is_flag_set(IS_BLIND);
}

bool Agent::is_fast() const
{
    return is_flag_set(IS_HASTE);
}

bool Agent::is_slow() const {
    return is_flag_set(IS_SLOW);
}

bool Agent::sees_invisible() const
{
    return game->wizard().see_invisible() ||
        is_flag_set(CAN_SEE);
}

bool Agent::detects_others() const {
    return game->wizard().detect_monsters() ||
        is_flag_set(SEE_MONST);
}

bool Agent::is_running() const {
    return is_flag_set(IS_RUN);
}

bool Agent::is_found() const {
    return is_flag_set(IS_FOUND);
}

bool Agent::can_confuse() const {
    return is_flag_set(CAN_HUH);
}

bool Agent::powers_cancelled() const {
    return is_flag_set(IS_CANC);
}

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

void Agent::set_invisible(bool enable) {
    set_flag(IS_INVIS, enable);
}

void Agent::set_found(bool enable) {
    set_flag(IS_FOUND, enable);
}

void Agent::set_confused(bool enable) {
    set_flag(IS_HUH, enable);
}

void Agent::set_running(bool enable) {
    set_flag(IS_RUN, enable);
}

void Agent::set_is_held(bool enable) {
    set_flag(IS_HELD, enable);
}

void Agent::set_is_slow(bool enable) {
    set_flag(IS_SLOW, enable);
}

void Agent::set_is_fast(bool enable) {
    set_flag(IS_HASTE, enable);
}

void Agent::set_can_confuse(bool enable) {
    set_flag(CAN_HUH, enable);
}

void Agent::set_cancelled(bool enable) {
    set_flag(IS_CANC, enable);
}

void Agent::set_blind(bool enable) {
    set_flag(IS_BLIND, enable);
}

void Agent::set_sees_invisible(bool enable) {
    set_flag(CAN_SEE, enable);
}

void Agent::set_detects_others(bool enable) {
    set_flag(SEE_MONST, enable);
}

void Agent::set_is_mean(bool enable) {
    set_flag(IS_MEAN, enable);
}

int Agent::get_hp() const {
    return stats.hp;
}

bool Agent::decrease_hp(int n, bool can_kill) {
    if (invunerable)
        return true;

    stats.hp -= n;
    if (!can_kill && stats.hp <= 0)
        stats.hp = 1;
    return stats.hp > 0;
}

void Agent::increase_hp(int n, bool max_bonus, bool second_max_bonus) {
    stats.hp += n;

    if (max_bonus && stats.hp > stats.max_hp)
        ++stats.max_hp;
    if (second_max_bonus && stats.hp > stats.max_hp + stats.level + 1)
        ++stats.max_hp;

    if (stats.hp > stats.max_hp) {
        stats.hp = stats.max_hp;
    }
}

int Agent::drain_hp() {
    stats.hp /= 2;
    return stats.hp;
}

Coord Agent::position() const
{
    return pos;
}

bool Agent::attack(Agent *defender, Item *weapon, bool hurl)
{
    std::string damage_string;
    int hplus;
    int dplus;
    calculate_roll_stats(defender, weapon, hurl, &hplus, &damage_string, &dplus);

    //If the creature being attacked is not running (asleep or held) then the attacker gets a plus four bonus to hit.
    if (!defender->is_running())
        hplus += 4;

    int defender_armor = defender->calculate_armor();

    std::ostringstream ss;
    ss << get_name() << "[hp=" << get_hp() << "] " << damage_string << " attack on "
        << defender->get_name() << "[hp=" << defender->get_hp() << "]";
    game->log("battle", ss.str());

    bool did_hit = false;
    const char* cp = damage_string.c_str();
    for (;;)
    {
        int ndice = atoi(cp);
        if ((cp = strchr(cp, 'd')) == NULL)
            break;
        int nsides = atoi(++cp);
        if (attempt_swing(stats.level, defender_armor, hplus + str_plus(calculate_strength())))
        {
            did_hit = true;

            int r = roll(ndice, nsides);
            int str_bonus = add_dam(calculate_strength());
            int damage = dplus + r + str_bonus;

            bool half_damage(defender == &game->hero() && max_level() == 1); //make it easier on level one
            if (half_damage) {
                damage = (damage + 1) / 2;
            }
            damage = std::max(0, damage);
            defender->decrease_hp(damage, true);

            std::ostringstream ss;
            ss << "damage=" << damage << " => " << defender->get_name() << "[hp=" << defender->get_hp() << "]: ("
                << damage_string << "=" << r << " + dplus=" << dplus << " + str_plus=" << str_bonus << ")"
                << (half_damage ? "/2" : "");
            game->log("battle", ss.str());
        }
        if ((cp = strchr(cp, '/')) == NULL) break;
        cp++;
    }
    return did_hit;
}
