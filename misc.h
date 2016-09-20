#pragma once
#include "item.h"

//tr_name: Print the name of a trap
char *tr_name(byte type);

//look: A quick glance all around the player
void look(bool wakeup);

//find_obj: Find the unclaimed object at y, x
Item *find_obj(int y, int x);

//add_haste: Add a haste to the player
int add_haste(bool potion);

//aggravate: Aggravate all the monsters on this level
void aggravate();

//vowelstr: For printfs: if string starts with a vowel, return "n" for an "an".
const char *vowelstr(const char *str);

//is_current: See if the object is one of the currently used items
int is_current(Item *obj);

//get_dir: Set up the direction co_ordinate for use in various "prefix" commands
int get_dir();

bool find_dir(byte ch, Coord *cp);

//sign: Return the sign of the number
int sign(int nm);

//spread: Give a spread around a given number (+/- 10%)
int spread(int nm);

//call_it: Call an object something after use.
void call_it(bool know, char **guess);

//step_ok: Returns true if it is ok to step on ch
int step_ok(int ch);

//goodch: Decide how good an object is and return the correct character for printing.
int goodch(Item *obj);

//help: prints out help screens
void help(const char*const* helpscr);

int distance(Coord a, Coord b);

int equal(Coord a, Coord b);

int offmap(int y, int x);

byte get_tile_or_monster(int y, int x);

//search: Player gropes about him to find hidden things.
void search();

//go_down_stairs: He wants to go down a level
void go_down_stairs();

//go_up_stairs: He wants to go up a level
void go_up_stairs();

//call: Allow a user to call a potion, scroll, or ring something
void call();

//prompt player for definition of macro
void record_macro();
