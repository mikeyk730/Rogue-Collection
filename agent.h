#pragma once
#include <list>
#include "rogue.h"

struct Item;
struct Room;

//m_flags for creatures
const short IS_BLIND = 0x0001; //creature is blind
const short SEE_MONST = 0x0002; //hero can detect unseen monsters
const short IS_RUN = 0x0004; //creature is running at the player
const short IS_FOUND = 0x0008; //creature has been seen (used for objects)
const short IS_INVIS = 0x0010; //creature is invisible
const short IS_MEAN = 0x0020; //creature can wake when player enters m_room
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
    Agent();
    Agent(const Agent&) = delete;

    virtual std::string get_name() = 0;

    virtual void calculate_roll_stats(Agent *defender, Item *weapon, bool hurl,
        int* hit_plus, std::string* damage_string, int* damage_plus);
    virtual int calculate_armor() const;
    int armor_for_display() const;

    virtual int calculate_strength() const;
    virtual int calculate_max_strength() const;
    void restore_strength();
    void adjust_strength(int amt);
    
    int experience() const;
    virtual void gain_experience(int exp);

    int get_hp() const;
    void increase_hp(int n, bool max_bonus, bool second_max_bonus);
    bool decrease_hp(int n, bool can_kill);
    int drain_hp();

    Coord position() const;
    int level() const;
    std::string damage_string() const;

    bool attack(Agent *defender, Item *weapon, bool hurl);

    //Structure describing a fighting being
    struct Stats
    {
        unsigned int m_str;     //Strength
        long m_exp;             //Experience
        int m_level;            //Level of mastery
        int m_ac;               //Armor class
        int m_hp;               //Hit points
        std::string m_damage;   //String describing damage done
        int m_max_hp;           //Max hit points
        unsigned int m_max_str; //Max strength
    };

    Coord m_position = { 0, 0 };      //Position
    short m_flags = 0;                //State word
    Stats m_stats;                    //Physical description
    Room *m_room = 0;                 //Current m_room for thing
    std::list<Item*> m_pack;          //What the thing is carrying
    
    bool m_invulnerable = false;

private:
    bool is_flag_set(short flag) const;
    void set_flag(short flag, bool enable);

public:
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

    bool in_same_room_as(Agent* other);
    bool in_same_room_as(Item* obj);
};
