#pragma once

#include "agent.h"

struct Monster : public Agent
{
    virtual std::string get_name();

    //create_monster: Pick a new monster and add it to the list
    static Monster* CreateMonster(byte type, Coord *cp, int level);

    //do_chase: Make one thing chase another.  Returns pointer to monster that may have been invalidated.  //todo:nix return value
    Monster* do_chase();

    //attack: The monster attacks the player.  Returns pointer to monster that may have been invalidated.  //todo:nix return value
    Monster* attack_player();
};