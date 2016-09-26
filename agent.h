#pragma once
#include <list>
#include "rogue.h"

struct Item;

//flags for creatures
const short IS_BLIND = 0x0001; //creature is blind
const short SEE_MONST = 0x0002; //hero can detect unseen monsters
const short IS_RUN = 0x0004; //creature is running at the player
const short IS_FOUND = 0x0008; //creature has been seen (used for objects)
const short IS_INVIS = 0x0010; //creature is invisible
const short IS_MEAN = 0x0020; //creature can wake when player enters room
const short IS_GREED = 0x0040; //creature runs to protect gold
const short IS_HELD = 0x0080; //creature has been held
const short IS_HUH = 0x0100; //creature is confused
const short IS_REGEN = 0x0200; //creature can regenerate
const short CAN_HUH = 0x0400; //creature can confuse
const short CAN_SEE = 0x0800; //creature can see invisible creatures
const short IS_CANC = 0x1000; //creature has special qualities cancelled
const short IS_SLOW = 0x2000; //creature has been slowed
const short IS_HASTE = 0x4000; //creature has been hastened
const short IS_FLY = (short)0x8000; //creature is of the flying type

//Structure for monsters and player
struct Agent
{
    //Structure describing a fighting being
    struct Stats
    {
        unsigned int str;   //Strength
        long exp;           //Experience
        int level;          //Level of mastery
        int ac;             //Armor class
        int hp;             //Hit points
        std::string damage; //String describing damage done
        int max_hp;         //Max hit points
        unsigned int max_str;        //Max strength
    };

    Coord pos = { 0, 0 };             //Position
    char turn = 0;                    //If slowed, is it a turn to move
    char type = 0;                    //What it is
    byte disguise = 0;                //What mimic looks like
    byte oldch = 0;                   //Character that was where it was
    Coord *dest = 0;                  //Where it is running to
    short flags = 0;                  //State word
    int exflags = 0;                  //More state;
    int value = 0;                    //
    Stats stats;                      //Physical description
    Room *room = 0;                   //Current room for thing
    bool invunerable = false;
    std::list<Item*> pack;            //What the thing is carrying

    int get_hp() const;
    void increase_hp(int n, bool max_bonus, bool second_max_bonus);
    bool decrease_hp(int n, bool can_kill);
    int drain_hp();

    void adjust_strength(int amt);
    void restore_strength();

    Coord position() const;

private:
    bool is_flag_set(short flag) const;
    void set_flag(short flag, bool enable);

public:
    const char* get_monster_name() const;
    int get_monster_carry_prob() const;

    //special features
    bool is_monster_confused_this_turn() const;
    bool is_stationary() const;
    bool can_hold() const;
    bool hold_attacks() const;
    bool can_divide() const;
    bool shoots_fire() const;
    bool immune_to_fire() const;
    bool shoots_ice() const;
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

    bool is_flying() const;
    bool is_mean() const;
    bool regenerates_hp() const;
    bool is_greedy() const;
    bool is_invisible() const;
    bool is_confused() const;
    bool is_held() const;
    bool is_blind() const;
    bool is_fast() const;
    bool is_slow() const;
    bool sees_invisible() const;
    bool detects_others() const;
    bool is_running() const;
    bool is_found() const;
    bool can_confuse() const;
    bool powers_cancelled() const;

    void set_invisible(bool enable);
    void set_found(bool enable);
    void set_confused(bool enable);
    void set_running(bool enable);
    void set_is_held(bool enable);
    void set_is_slow(bool enable);
    void set_is_fast(bool enable);
    void set_can_confuse(bool enable);
    void set_cancelled(bool enable);
    void set_blind(bool enable);
    void set_sees_invisible(bool enable);
    void set_detects_others(bool enable);
    void set_is_mean(bool enable);

    void reveal_disguise();

    //slime specific
    void set_dirty(bool enable);
    bool is_dirty();

    //do_chase: Make one thing chase another.  Returns false if monster has been invalidated
    bool do_chase();

    //chase: Find the spot for the chaser(er) to move closer to the chasee(ee). Returns true if we want to keep on chasing later. false if we reach the goal.
    void chase(Coord *chasee_pos);

    //start_run: Set a monster running after something
    void start_run();

    //find_dest: find the proper destination for the monster
    Coord *find_dest();
    void set_destination();

    bool in_same_room_as(Agent* other);

    bool is_seeking(Item* obj);
    bool in_same_room_as(Item* obj);



};
