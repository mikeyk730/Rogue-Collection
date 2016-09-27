#pragma once
#include "agent.h"

//swing: Returns true if the swing hits
bool swing(int at_lvl, int op_arm, int wplus);

//check_level: Check to see if the guy has gone up a level.
void check_level();

//prname: The print name of a combatant
char *prname(const char *who, bool upper);

//hit: Print a message to indicate a successful hit
void display_hit_msg(const char *er, const char *ee);

//miss: Print a message to indicate a poor swing
void miss(const char *er, const char *ee);

//save_throw: See if a creature save against something
int save_throw(int which, Agent *monster);

//save: See if he saves against various nasty things
int save(int which);

//str_plus: Compute bonus/penalties for strength on the "to hit" roll
int str_plus(unsigned int str);

//add_dam: Compute additional damage done for exceptionally high or low strength
int add_dam(unsigned int str);

//raise_level: The guy just magically went up a level.
void raise_level();

//thunk: A projectile hit or missed a monster
void display_throw_msg(Item *weapon, const char *mname, char *does, char *did);

//remove: Remove a monster from the screen
void remove_monster(Agent *monster, bool waskill);

//is_magic: Returns true if an object radiates magic
bool is_magic(Item *obj);

//killed: Called to put a monster to death
void killed(Agent *monster, bool print);
