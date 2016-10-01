#pragma once
#include <string>

struct Monster;

const char* get_monster_name(char monster);

//randmonster: Pick a monster to show up.  The lower the level, the meaner the monster.
char randmonster(bool wander, int level);

//expadd: Experience to add for this monster's level/hit points
int exp_add(Monster *monster);

//create_wandering_monster: Create a new wandering monster and aim it at the player
void create_wandering_monster();

//wake_monster: What to do when the hero steps next to a monster
Monster *wake_monster(Coord p);

//pick_vorpal_monster: Choose a sort of monster for the enemy of a vorpally enchanted weapon
char pick_vorpal_monster();

void load_monster_cfg(const std::string& filename);
