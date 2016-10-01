#include <sstream>
#include <algorithm>
#include "agent.h"
#include "main.h"
#include "fight.h"
#include "game_state.h"
#include "hero.h"
#include "level.h"
#include "item.h"

bool Agent::is_flag_set(short flag) const {
    return ((m_flags & flag) != 0);
}

void Agent::set_flag(short flag, bool enable) {
    if (enable)
        m_flags |= flag;
    else
        m_flags &= ~flag;
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

Agent::Agent()
{
}

void Agent::calculate_roll_stats(Agent *defender, Item *weapon, bool hurl,
    int* hit_plus, std::string* damage_string, int* damage_plus)
{
    *damage_string = m_stats.m_damage;
    *hit_plus = 0;
    *damage_plus = 0;
}

int Agent::calculate_armor() const
{
    return m_stats.m_ac;
}

int Agent::armor_for_display() const
{
    int a = calculate_armor();
    return (-((a)-11));
}

int Agent::calculate_strength() const
{
    return m_stats.m_str;
}

int Agent::calculate_max_strength() const
{
    return m_stats.m_max_str;
}

void Agent::restore_strength()
{
    m_stats.m_str = m_stats.m_max_str;
}

void Agent::adjust_strength(int amt)
{
    m_stats.m_str += amt;
    if (m_stats.m_str > 31)
        m_stats.m_str = 31;
    if (m_stats.m_str > m_stats.m_max_str)
        m_stats.m_max_str = m_stats.m_str;
}

int Agent::level() const
{
    return m_stats.m_level;
}

int Agent::experience() const
{
    return m_stats.m_exp;
}

void Agent::gain_experience(int exp)
{
    m_stats.m_exp += exp;
}

std::string Agent::damage_string() const
{
    return m_stats.m_damage;
}

int Agent::get_hp() const {
    return m_stats.m_hp;
}

bool Agent::decrease_hp(int n, bool can_kill) {
    if (m_invulnerable)
        return true;

    m_stats.m_hp -= n;
    if (!can_kill && m_stats.m_hp <= 0)
        m_stats.m_hp = 1;
    return m_stats.m_hp > 0;
}

void Agent::increase_hp(int n, bool max_bonus, bool second_max_bonus) {
    m_stats.m_hp += n;

    if (max_bonus && m_stats.m_hp > m_stats.m_max_hp)
        ++m_stats.m_max_hp;
    if (second_max_bonus && m_stats.m_hp > m_stats.m_max_hp + m_stats.m_level + 1)
        ++m_stats.m_max_hp;

    if (m_stats.m_hp > m_stats.m_max_hp) {
        m_stats.m_hp = m_stats.m_max_hp;
    }
}

int Agent::drain_hp() {
    m_stats.m_hp /= 2;
    return m_stats.m_hp;
}

Coord Agent::position() const
{
    return m_position;
}

bool Agent::attack(Agent *defender, Item *weapon, bool hurl)
{
    std::string damage_string;
    int hplus;
    int dplus;
    calculate_roll_stats(defender, weapon, hurl, &hplus, &damage_string, &dplus);

    //If the creature being attacked is not running (asleep or held) then the attacker gets a plus four bonus to hit.
    //mdk: the hero never has IS_RUN set, so Monsters always get this +4.  Is this intended?
    if (!defender->is_running())
        hplus += 4;

    int defender_armor = defender->calculate_armor();

    std::ostringstream ss;
    ss << get_name() << "[hp=" << get_hp() << "] " << damage_string << " "<<hplus<<","<<dplus<<" attack on "
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
        if (attempt_swing(m_stats.m_level, defender_armor, hplus + str_plus(calculate_strength())))
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

bool Agent::in_same_room_as(Agent* other)
{
    return m_room == other->m_room;
}

bool Agent::in_same_room_as(Item * obj)
{
    return m_room == obj->get_room();
}
