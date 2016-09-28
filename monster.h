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

    int get_carry_probability() const;

    void reveal_disguise();

    //chase: Find the spot for the chaser(er) to move closer to the chasee(ee). Returns true if we want to keep on chasing later. false if we reach the goal.
    void chase(Coord *chasee_pos);

    //slime specific
    void set_dirty(bool enable);
    bool is_dirty();

    //find_dest: find the proper destination for the monster
    Coord *find_dest();
    void set_destination();

    //start_run: Set a monster running after something
    void start_run();

    //give_pack: Give a pack to a monster if it deserves one
    void give_pack();
};