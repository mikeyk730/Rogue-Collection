#pragma once
#include <vector>
#include "item.h"

//init_things: Initialize the probabilities for types of things
void init_things();

void chopmsg(char *s, char *shmsg, char *lnmsg, ...);

//drop: Put something down
void drop();

//can_drop: Do special checks for dropping or unweilding|unwearing|unringing
bool can_drop(Item *op);

//new_thing: Return a new thing
Item* create_item();

//pick_one: Pick an item out of a list of nitems possible magic items
int pick_one(std::vector<MagicItem> magic);
int pick_one(struct MagicItem *magic, int nitems);

//discovered: list what the player has discovered in this game of a certain type
void discovered();

//print_disc: Print what we've discovered of type 'type'
void print_disc(byte type);

//set_order: Set up order for list

void set_order(short *order, int numthings);

//add_line: Add a line to the list of discoveries
int add_line(const char *use, const char *fmt, const char *arg);

//end_line: End the list of lines
int end_line(char *use);

//nothing: Set up prbuf so that message for "nothing found" is there
char *nothing(byte type);