#pragma once
#include <string>
#include "agent.h"

const char* get_monster_name(char monster);

//randmonster: Pick a monster to show up.  The lower the level, the meaner the monster.
char randmonster(bool wander, int level);

//new_monster: Pick a new monster and add it to the list
void new_monster(Agent *monster, byte type, Coord *cp, int level);

//expadd: Experience to add for this monster's level/hit points
int exp_add(Agent *monster);

//wanderer: Create a new wandering monster and aim it at the player
void wanderer();

//wake_monster: What to do when the hero steps next to a monster
Agent *wake_monster(int y, int x);

//give_pack: Give a pack to a monster if it deserves one
void give_pack(Agent *monster);

//pick_vorpal_monster: Choose a sort of monster for the enemy of a vorpally enchanted weapon
char pick_vorpal_monster();

//monster_at(x,y): returns pointer to monster at coordinate. if no monster there return NULL
Agent *monster_at(int y, int x);

void load_monster_cfg(const std::string& filename);
