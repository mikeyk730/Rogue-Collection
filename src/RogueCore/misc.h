#pragma once
#include "item.h"
#include "util.h"

//tr_name: Print the name of a trap
char *tr_name(byte type);

//look: A quick glance all around the player
void look(bool wakeup);

//find_obj: Find the unclaimed object at y, x
Item* find_obj(Coord p, bool expect_item);

//vowelstr: For printfs: if string starts with a vowel, return "n" for an "an".
const char *vowelstr(const char *str);
std::string vowelstr(const std::string& str);

//is_in_use: See if the object is one of the currently used items
int is_in_use(Item *obj);

//get_dir: Set up the direction co_ordinate for use in various "prefix" commands
int get_dir(Coord* delta);

bool find_dir(byte ch, Coord *cp);

//sign: Return the sign of the number
int sign(int nm);

//spread: Give a spread around a given number (+/- 10%)
int spread(int nm);

//step_ok: Returns true if it is ok to step on ch
int step_ok(int ch);

//get_magic_char: Decide how good an object is and return the correct character for printing.
int get_magic_char(Item *obj);

//help: prints out help screens
void help(const char*const* helpscr);

int distance(Coord a, Coord b);

int equal(Coord a, Coord b);

int offmap(Coord p);

//do_search: Player gropes about him to find hidden things.
bool do_search();

//do_go_down_stairs: He wants to go down a level
bool do_go_down_stairs();

//do_go_up_stairs: He wants to go up a level
bool do_go_up_stairs();

//prompt player for definition of macro
bool do_record_macro();
