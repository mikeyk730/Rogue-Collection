#pragma once
//whatis: What a certain object is
void whatis();

void summon_object();

int get_num(short *place);

//show_map: Print out the map for the wizard
void show_map(bool show_monsters);

void debug_screen();

struct Cheats
{
    Cheats();

    bool detect_monsters() const;
    bool no_dark_rooms() const;
    bool slow_monster() const;
    bool haste_self() const;
    bool invunerability() const;
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
    bool free_level_movement() const;

private:
    //todo: replace with set<string>?
    bool m_detect_monsters = false;
    bool m_see_invisible = false;
    bool m_no_traps = false;
    bool m_no_hidden_doors = false;
    bool m_no_ring_hunger = false;
    bool m_no_hunger = false;
    bool m_no_dark_rooms = false;
};


