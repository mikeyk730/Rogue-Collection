#pragma once

#include "agent.h"

struct Monster : public Agent
{
    virtual std::string get_name();

    //create_monster: Pick a new monster and add it to the list
    static Monster* CreateMonster(byte type, Coord *cp, int level);

    //do_chase: Make one thing chase another.  Returns false if monster has been invalidated
    bool do_chase();

    //attack: The monster attacks the player
    bool attack_player();
};