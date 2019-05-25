#pragma once
#include <string>
#include <list>
#include <coord.h>

struct Monster;
struct Item;
struct Room;

//flags for creatures
const int IS_BLIND  = 0x0001; //creature is blind
const int SEE_MONST = 0x0002; //hero can detect unseen monsters
const int IS_RUN    = 0x0004; //creature is running at the player
const int IS_FOUND  = 0x0008; //creature has been seen (used for objects)
const int IS_INVIS  = 0x0010; //creature is invisible
const int IS_MEAN   = 0x0020; //creature can wake when player enters room
const int IS_GREED  = 0x0040; //creature runs to protect gold
const int IS_HELD   = 0x0080; //creature has been held
const int IS_HUH    = 0x0100; //creature is confused
const int IS_REGEN  = 0x0200; //creature can regenerate
const int CAN_HUH   = 0x0400; //creature can confuse
const int CAN_SEE   = 0x0800; //creature can see invisible creatures
const int IS_CANC   = 0x1000; //creature has special qualities cancelled
const int IS_SLOW   = 0x2000; //creature has been slowed
const int IS_HASTE  = 0x4000; //creature has been hastened
const int IS_FLY    = 0x8000; //creature is of the flying type

//Structure for monsters and player
struct Agent
{
    Agent();
    Agent(const Agent&) = delete;
    virtual ~Agent();

    virtual std::string get_name() = 0;

    virtual void calculate_roll_stats(
        Agent *defender,
        Item *weapon,
        bool hurl,
        int* hit_plus,
        std::string* damage_string,
        int* damage_plus);
    virtual int calculate_armor() const;

    virtual int calculate_strength() const;
    virtual int calculate_max_strength() const;
    void restore_strength();

    int experience() const;
    virtual void gain_experience(int exp);

    int get_hp() const;
    void increase_hp(int n, bool max_bonus, bool second_max_bonus);
    bool decrease_hp(int n, bool can_kill);
    int drain_hp();

    const Coord position() const;
    void set_position(Coord c);
    Room* room() const;
    void set_room(Room* r);

    int level() const;
    std::string damage_string() const;

    bool attack(Agent *defender, Item *weapon, bool hurl);

    void set_as_target_of(Monster* m);

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

    int m_flags = 0;                  //State word
    Stats m_stats;                    //Physical description
private:
    Coord m_position = { 0, 0 };      //Position
    Room *m_room = 0;                 //Current room for thing
public:
    std::list<Item*> m_pack;          //What the thing is carrying

    bool m_invulnerable = false;

private:
    bool is_flag_set(int flag) const;
    void set_flag(int flag, bool enable);

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
