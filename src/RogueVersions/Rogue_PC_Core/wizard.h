#pragma once
#include <set>

//whatis: What a certain object is
void whatis();

//add_pass: Add the passages to the current window (wizard command)
void add_pass();

//show_map: Print out the map for the wizard
void show_map(bool show_monsters);

bool do_raise_level();
bool do_toggle_detect();
bool do_msg_position();
bool do_msg_pack_count();
bool do_charge_stick();
bool do_teleport();
bool do_advance_level();
bool do_decrease_level();
bool do_add_passages();
bool do_msg_food();
bool do_reveal_all();
bool do_add_goods();
bool do_toggle_powers();
bool do_toggle_wizard();


void debug_screen();

struct Cheats
{
    Cheats();

    void toggle();
    bool enabled() const;
    bool did_cheat() const;

    void add_powers(const std::string& powers);
    void toggle_powers(const std::string& powers);

    void apply_powers();
    void on_new_level();

    bool detect_monsters() const;
    bool no_dark_rooms() const;
    bool haste_self() const;
    bool invulnerability() const;
    bool no_hunger() const;
    bool no_hidden_doors() const;
    bool no_traps() const;
    //bool slow_monster() const;
    bool reveal_items() const;
    //bool show_monster_hp() const;
    bool no_ring_hunger() const;
    //bool sense_magic() const;
    //bool sense_food() const;
    bool see_invisible() const;
    bool show_food_counter() const;
    bool jump_levels() const;
    bool see_all() const;

private:
    bool is_enabled(const std::string& power) const;

    std::set<std::string> m_powers;
    bool m_enabled = false;
    bool m_cheated = false;
};


