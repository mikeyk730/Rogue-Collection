#pragma once
//whatis: What a certain object is
void whatis();

void summon_object();

int get_num(short *place);

//show_map: Print out the map for the wizard
void show_map(bool show_monsters);

struct Cheats
{
    Cheats();

    bool detects_monsters() const;
    bool no_dark_passages() const;
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
    bool sees_invisible() const;

private:
    //todo: replace with set<string>?
    bool m_detects_monsters = false;
    bool m_sees_invisible = false;
    bool m_no_traps = false;
    bool m_no_hidden_doors = false;
    bool m_no_ring_hunger = false;
    bool m_no_hunger = false;
};


