#pragma once
//whatis: What a certain object is
void whatis();

void summon_object();

int get_num(short *place);

//show_map: Print out the map for the wizard
void show_map(bool show_monsters);

struct Cheats
{
    bool detects_others() const;

private:
    bool m_detects_monsters = false;
};


