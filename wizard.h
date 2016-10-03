#pragma once
#include <set>

//whatis: What a certain object is
void whatis();

int get_num(short *place);

//show_map: Print out the map for the wizard
void show_map(bool show_monsters);

bool do_reveal_level();

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

    bool detect_monsters() const;
    bool no_dark_rooms() const;
    bool slow_monster() const;
    bool haste_self() const;
    bool invulnerability() const;
    bool no_hunger() const;
    bool no_hidden_doors() const;
    bool no_traps() const;
    bool reveal_items() const;
    bool show_monster_hp() const;
    bool no_ring_hunger() const;
    bool sense_magic() const;
    bool sense_food() const;
    bool see_invisible() const;
    bool show_food_counter() const;
    bool jump_levels() const;

private:
    bool is_enabled(const std::string& power) const;

    std::set<std::string> m_powers;
    bool m_enabled = false;
    bool m_cheated = false;
};


