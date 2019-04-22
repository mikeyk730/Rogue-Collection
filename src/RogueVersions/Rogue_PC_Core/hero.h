#pragma once
#include "agent.h"
#include "rogue.h"

struct Monster;
struct Item;
struct Ring;
struct Armor;

#define LEFT         0
#define RIGHT        1
#define LAMP_DIST    3

struct Hero : public Agent
{
    enum HungerState
    {
        Full = 0,
        Hungry = 1,
        Weak = 2,
        Faint = 3
    };

    Hero(const std::string& name);

    //fight: The player attacks the monster.  Returns pointer to monster that may have been invalidated.  //todo:nix return value
    Monster* fight(Coord pos, Item *weapon, bool thrown);

    virtual void calculate_roll_stats(Agent *defender, Item *weapon, bool hurl,
        int* hit_plus, std::string* damage_string, int* damage_plus);
    virtual int calculate_armor() const;

    virtual void gain_experience(int exp);

    virtual int calculate_strength() const;
    virtual int calculate_max_strength() const;
private:
    int calculate_strength_impl(int s) const;

public:

    //init_player: Roll up the rogue
    void init_player();

    void on_new_level();

    virtual std::string get_name();
    void set_name(const std::string& name);

    int get_purse();
    void adjust_purse(int delta);

    bool eat();
    void ingest();
    void digest();
    int get_hungry_state();
    int get_food_left();

    bool put_on_ring();
    bool remove_ring();
    int is_ring_on_hand(int h, int r) const;
    int is_wearing_ring(int r) const;

    bool adjust_strength(int amt);

    //pick_up: Add something to characters pack.
    void pick_up(byte ch);

    //obtain_item: Pick up an object and add it to the pack.  If the argument is non-null use it as the linked_list pointer instead of getting it off the ground.
    void obtain_item(Item *obj, bool silent);
private:
    bool add_to_list(Item** obj, bool from_floor);
public:
    int get_pack_size();
    byte get_pack_index(Item* obj);
    Item* get_random_magic_item() const;
    //inventory: List what is in the pack
    int inventory(int type, const char *lstr);
    Item* get_item_from_inventory(byte user_input, byte *max_valid_char);
    Item* get_item(const std::string& purpose, int type);

    //teleport: Bamf the hero someplace else
    void teleport();

    //can_see_monster: Return true if the hero can see the monster
    bool can_see_monster(Monster *monster);

    //can_see: Returns true if the hero can see a certain coordinate.
    int can_see(Coord p);

    //wield: Pull out a certain weapon
    bool wield();

    void reduce_level();

    //check_level: Check to see if the guy has gone up a level.
    void check_level(bool print);

    //raise_level: The guy just magically went up a level.
    void raise_level(bool print);

    void do_hit(Item* weapon, int thrown, Monster* monster, const char* name);
    void do_miss(Item* weapon, int thrown, Monster* monster, const char* name);

    //pick_up_gold: Add gold to the pack
    void pick_up_gold(int value);

    //true if player currently has amulet
    bool has_amulet();

    //true if player ever had amulet
    bool had_amulet();

    Item* get_current_weapon() const;
    Armor* get_current_armor() const;
    Ring* get_ring(int hand) const;

    void set_current_weapon(Item* item);
    void set_current_armor(Armor* item);
    void set_ring(int hand, Ring* ring);

    //wear_armor: The player wants to wear something, so let him/her put it on.
    bool wear_armor();

    //take_off_armor: Get the armor off of the player's back
    bool take_off_armor();

    //add_haste: Add a haste to the player
    int add_haste(bool is_temporary);

    void set_num_actions(int actions);
    bool decrement_num_actions();

    int get_sleep_turns() const;
    void increase_sleep_turns(int time);
    bool decrement_sleep_turns();

    void set_trap_turns(int turns);
    void reset_trap_turns();
    bool decrement_trap_turns();

    bool is_held_by(Agent* a) const;
    void set_held_by(Agent* a);
    void clear_hold();

    void set_sprung_trap(int type);
    bool sprung_trap() const;
    bool sprung_teleport_trap() const;
    void reset_sprung();

    Coord previous_position() const;
    Room* previous_room() const;
    bool has_moved() const;
    void update_position();

private:
    int m_purse = 0;
    bool m_had_amulet = false;
    HungerState hungry_state = Full;    //How hungry is he
    int food_left = 0;        //Amount of food in hero's stomach
    int m_num_actions = 0;
    std::string m_name;

    Armor* m_armor = 0;       //What a well dresssed rogue wears
    Item* m_weapon = 0;       //Which weapon he is wielding
    Ring* m_rings[2] = {0,0}; //Which rings are being worn

    int m_trap_type = NTRAPS; //The trap that the hero has sprung
    int m_sleep_turns = 0;    //Number of turns to sleep
    int m_trap_turns = 0;     //Number of turns stuck in place
    Agent* m_held_by = 0;     //The agent holding the player

    Room* m_previous_room = 0;
    Coord m_previous_position = { 0, 0 };
};

