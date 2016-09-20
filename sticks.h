#pragma once
#include "item.h"

//init_materials: Initialize the construction materials for wands and staffs
void init_materials();
std::string get_material(int type);
std::string get_stick_type(int type);
void init_new_stick(Item* stick);
int get_stick_value(int type);
std::string get_stick_name(int type);
std::string get_stick_guess(int type);
void set_stick_guess(int type, const char* value);
const char* get_inv_name_stick(Item* stick);

//fix_stick: Set up a new stick
void fix_stick(Item *cur);

//do_zap: Perform a zap with a wand
void do_zap();

//drain: Do drain hit points from player schtick
void drain();

//fire_bolt: Fire a bolt in a given direction from a specific starting place
bool fire_bolt(Coord *start, Coord *dir, const char *name);

//charge_str: Return an appropriate string for a wand charge
const char *get_charge_string(Item *obj);

int does_know_stick(int type);
void discover_stick(int type);
