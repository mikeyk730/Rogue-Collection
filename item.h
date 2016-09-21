#pragma once
#include "rogue.h"

struct Agent;

//flags for objects
const short IS_CURSED = 0x0001; //object is cursed
const short IS_KNOW = 0x0002; //player knows details about the object
const short DID_FLASH = 0x0004; //has the vorpal weapon flashed
//const short IS_EGO   = 0x0008; //weapon has control of player
const short IS_MISL = 0x0010; //object is a missile type
const short IS_MANY = 0x0020; //object comes in groups
const short IS_REVEAL = 0x0040; //Do you know who the enemy of the object is

//Weapon types
#define MACE        0
#define SWORD       1
#define BOW         2
#define ARROW       3
#define DAGGER      4
#define TWOSWORD    5
#define DART        6
#define CROSSBOW    7
#define BOLT        8
#define SPEAR       9
#define FLAME       10 //fake entry for dragon breath (ick)
#define MAXWEAPONS  10 //this should equal FLAME

//Armor types
#define LEATHER          0
#define RING_MAIL        1
#define STUDDED_LEATHER  2
#define SCALE_MAIL       3
#define CHAIN_MAIL       4
#define SPLINT_MAIL      5
#define BANDED_MAIL      6
#define PLATE_MAIL       7
#define MAXARMORS        8

//Ring types
#define R_PROTECT   0
#define R_ADDSTR    1
#define R_SUSTSTR   2
#define R_SEARCH    3
#define R_SEEINVIS  4
#define R_NOP       5
#define R_AGGR      6
#define R_ADDHIT    7
#define R_ADDDAM    8
#define R_REGEN     9
#define R_DIGEST    10
#define R_TELEPORT  11
#define R_STEALTH   12
#define R_SUSTARM   13
#define MAXRINGS    14

//Rod/Wand/Staff types
#define WS_LIGHT      0
#define WS_HIT        1
#define WS_ELECT      2
#define WS_FIRE       3
#define WS_COLD       4
#define WS_POLYMORPH  5
#define WS_MISSILE    6
#define WS_HASTE_M    7
#define WS_SLOW_M     8
#define WS_DRAIN      9
#define WS_NOP        10
#define WS_TELAWAY    11
#define WS_TELTO      12
#define WS_CANCEL     13
#define MAXSTICKS     14

struct Item
{
protected:
    //Item();
    Item(int type, int which);
public:
    virtual ~Item();

    virtual Item* Clone() const = 0;

//protected:
    int type;                      //What kind of object it is
    Coord pos;                     //Where it lives on the screen
    char launcher;                 //What you need to launch it
    char *damage;                  //Damage if used like sword
    char *throw_damage;            //Damage if thrown
    int count;                     //Count for plural objects
    int which;                     //Which object of a type it is
    int hit_plus;                  //Plusses to hit
    int damage_plus;               //Plusses to damage
    short misc;                    //Item dependent: rmor class, gold value, stick charges, ring level
    short flags;                   //Information about objects
    char enemy;                    //If it is enchanted, who it hates
    int group;                     //Group number for this object
public:

    void set_location(Coord p);

    void initialize(int type, int which);

    bool is_flag_set(short flag) const;

    bool is_known() const;
    bool is_cursed() const;
    bool did_flash() const;
    bool is_missile() const;
    bool does_group() const;
    bool is_revealed() const;
    bool is_found() const;

    void remove_curse();

    void set_known();
    void set_cursed();
    void set_revealed();
    void set_found();
    void set_flashed();

    //weapon-specific functions
    void initialize_weapon(byte type);
    const char* get_inv_name_weapon() const;
    void enchant_weapon();
    void vorpalize();
    bool is_vorpalized() const;
    bool is_vorpalized_against(Agent* monster) const;
    const char* get_vorpalized_name() const;
};

#define charges      misc
#define gold_value   misc
#define armor_class  misc
#define ring_level   misc

struct Amulet : public Item
{
    Amulet();

    virtual Item* Clone() const;
};

struct Gold : public Item
{
    Gold(int value);

    virtual Item* Clone() const;
};

//todo:move into game
#include <list>
extern std::list<Item*> level_items;
