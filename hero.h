#pragma once

#include "agent.h"

struct Monster;

struct Hero : public Agent
{
    Hero();

    //fight: The player attacks the monster.  Returns pointer to monster that may have been invalidated.  //todo:nix return value
    Monster* fight(Coord *monster, Item *weapon, bool thrown);

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

    //add_to_pack: Pick up an object and add it to the pack.  If the argument is non-null use it as the linked_list pointer instead of getting it off the ground.
    void add_to_pack(Item *obj, bool silent);
    int get_pack_size();

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
    void check_level();

    //raise_level: The guy just magically went up a level.
    void raise_level();

    void do_hit(Item* weapon, int thrown, Monster* monster, const char* name);
    void do_miss(Item* weapon, int thrown, Monster* monster, const char* name);

    //pick_up_gold: Add gold to the pack
    void pick_up_gold(int value);

    //true if player currently has amulet
    bool has_amulet();

    //true if player ever had amulet
    bool had_amulet();

    Item* get_current_weapon() const;
    Item* get_current_armor() const;
    Item* get_ring(int hand) const;

    void set_current_weapon(Item* item);
    void set_current_armor(Item* item);
    void set_ring(int hand, Item* item);

    //wear_armor: The player wants to wear something, so let him/her put it on.
    bool wear_armor();

    //take_off_armor: Get the armor off of the player's back
    bool take_off_armor();

private:

    int m_purse = 0;
    bool m_had_amulet = false;
    int hungry_state = 0;    //How hungry is he
    int food_left = 0;       //Amount of food in hero's stomach
    std::string m_name;

    Item* cur_armor = 0;       //What a well dresssed rogue wears
    Item* cur_weapon = 0;      //Which weapon he is wielding
    Item* cur_ring[2] = {0,0}; //Which rings are being worn
};

