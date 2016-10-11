#pragma once
#include <vector>
#include "item.h"

//Stuff about magic items
struct MagicItem
{
    char *name;
    int prob;
    short worth;
};

Item* CreateScroll();
Item* SummonScroll(int i);
int NumScrollTypes();

Item* CreatePotion();
Item* SummonPotion(int i);
int NumPotionTypes();

Item* CreateStick();
Item* SummonStick(int i);
int NumStickTypes();

void LoadScrolls(const std::string& filename);
void LoadPotions(const std::string& filename);
void LoadSticks(const std::string& filename);
void LoadRings(const std::string& filename);
void LoadWeapons(const std::string& filename);
void LoadArmor(const std::string& filename);

//init_things: Initialize the probabilities for types of things
void init_things();

void chopmsg(char *s, char *shmsg, char *lnmsg, ...);

//do_drop: Put something down
bool do_drop();

//can_drop: Do special checks for dropping or unweilding|unwearing|unringing
bool can_drop(Item *op, bool unequip);

//pick_one: Pick an item out of a list of nitems possible magic items
int pick_one(std::vector<MagicItem> magic);
int pick_one(struct MagicItem *magic, int nitems);

//do_discovered: list what the player has discovered in this game of a certain type
bool do_discovered();

//print_disc: Print what we've discovered of type 'type'
void print_disc(byte type);

//set_order: Set up order for list

void set_order(short *order, int numthings);

//add_line: Add a line to the list of discoveries
int add_line(const char *use, const char *fmt, const char *arg);

//end_line: End the list of lines
int end_line(const char *use);

//nothing: Set up prbuf so that message for "nothing found" is there
char *nothing(byte type);