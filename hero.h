#pragma once

struct Hero
{
    Hero();

    //init_player: Roll up the rogue
    void init_player();

    std::string get_name();
    void set_name(const char* name);

    int get_purse();
    void adjust_purse(int delta);

    void ingest();
    void digest();
    int get_hungry_state();
    int get_food_left();

    void toggle_wizard();
    bool is_wizard() const;
    bool did_cheat() const;

private:

    int m_purse = 0;
    bool m_wizard = 0;
    bool m_cheated = 0;
    int hungry_state = 0;    //How hungry is he
    int food_left = 0;       //Amount of food in hero's stomach
};

