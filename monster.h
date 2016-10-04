#pragma once

#include "agent.h"

const int EX_DIVIDES       =   0x0001; //can divide into two (slime)
const int EX_SHOOTS_ICE    =   0x0002; //can shoot frost to freeze player (ice monster)
const int EX_SHOOTS_FIRE   =   0x0004; //can shoot a bolt of fire (dragon)
const int EX_RUSTS_ARMOR   =   0x0008; //has attack that reduces armor (aquator)
const int EX_HOLDS         =   0x0010; //attack causes the player to be held (flytrap)
const int EX_STATIONARY    =   0x0020; //does not move (venus flytrap)
const int EX_MIMICS        =   0x0040; //disguises itself as an item (xerox)
const int EX_CONFUSES      =   0x0080; //can confuse the opponent on sight (medusa)
const int EX_STEALS_GOLD   =   0x0100; //has attack that steals gold (lephrachaun)
const int EX_STEALS_MAGIC  =   0x0200; //has attack that steals magic item (nymph)
const int EX_DRAINS_STR    =   0x0400; //has attack that drains strength (rattlesnake)
const int EX_DRAINS_EXP    =   0x0800; //has attack that drains experience (wraith)
const int EX_DRAINS_MAXHP  =   0x1000; //had attack that drains max hp (vampire)
const int EX_HOLD_ATTACKS  =   0x2000; //increases attack strength during battle (flytrap)
const int EX_SUICIDES      =   0x4000; //dies after a successful attack (nymph and leprachaun)
const int EX_DROPS_GOLD    =   0x8000; //drops gold on death (leprachaun)
const int EX_GUARDS_GOLD   =  0x10000; //guards gold rather than picking it up (orc)
const int EX_SLOW_WHEN_FAR =  0X20000; //moves slowly when far away from the player (slime)
const int EX_FIRE_IMMUNITY =  0X40000; //not injured by flames (dragon)
const int EX_UNFREEZES     =  0x80000; //attack unfreezes the player faster (ice monster)
const int EX_NO_MISS_MSGS  = 0x100000; //don't display a msg when an attack misses (ice monster)

//todo:separate flag for slowness (slime)

struct Monster : public Agent
{
    //create_monster: Pick a new monster and add it to the list
    static Monster* CreateMonster(byte type, Coord *cp, int level);

    virtual std::string get_name();

    //special features
    bool is_stationary() const;
    bool can_hold() const;
    bool hold_attacks() const;
    bool can_divide() const;
    bool shoots_fire() const;
    bool immune_to_fire() const;
    bool shoots_ice() const;
    bool unfreezes_player() const;
    bool causes_confusion() const;
    bool is_mimic() const;
    bool is_disguised() const;
    bool steals_gold() const;
    bool drops_gold() const;
    bool steals_magic() const;
    bool drains_life() const;
    bool drains_exp() const;
    bool drains_strength() const;
    bool rusts_armor() const;
    bool dies_from_attack() const;
    bool guards_gold() const;
    bool slow_when_far() const;
    bool no_miss_msgs() const;

    bool is_monster_confused_this_turn() const;

    //do_chase: Make one thing chase another.  Returns pointer to monster that may have been invalidated.  //todo:nix return value
    Monster* do_chase();
    void do_screen_update(Coord);

    //attack: The monster attacks the player.  Returns pointer to monster that may have been invalidated.  //todo:nix return value
    Monster* attack_player();

    int get_carry_probability() const;

    void set_disguise(); //choose a random disguise
    void reveal_disguise();

    //chase: Find the spot for the chaser(er) to move closer to the chasee(ee). Returns true if we want to keep on chasing later. false if we reach the goal.
    void chase(Coord *chasee_pos, Coord* next_position);

    //slime specific
    void set_dirty(bool enable);
    bool is_dirty();

    //obtain_target: find the proper destination for the monster
    void obtain_target();

    //start_run: Set a monster running after something
    void start_run(bool obtain = true);
    void start_run(Agent* a);

    void hold();

    //give_pack: Give a pack to a monster if it deserves one
    void give_pack();

    void set_destination(Agent* a);
    void set_destination(Item* i);
    void set_destination(Coord* d);
    bool is_going_to(Coord c);
    Coord* destination() const;

    bool has_tile_beneath() const;
    byte tile_beneath() const;
    void invalidate_tile_beneath();
    void set_tile_beneath(byte c);
    void reload_tile_beneath();
    void render();

    char m_type = 0;                  //What it is
    int m_ex_flags = 0;               //Extra state;
private:
    Coord *m_destination = 0;         //Where it is running to
public:
    char m_turn = 0;                  //If slowed, is it a turn to move
    byte m_tile_beneath = 0;          //Tile that is underneath the monster
    byte m_disguise = 0;              //What mimic looks like
    int m_confused_chance = 0;        //Monster is confused 1 in every =m_confused_chance= turns
    int m_dirty = 0;
    int m_flytrap_count = 0;
};
