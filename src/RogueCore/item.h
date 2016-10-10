#pragma once
#include <string>
#include "util.h"

struct Monster;
struct Hero;
struct Room;
struct ItemClass;
struct ItemCategory;

//flags for objects
const short IS_CURSED = 0x0001; //object is cursed
const short IS_KNOW = 0x0002; //player knows details about the object
//const short IS_EGO   = 0x0008; //weapon has control of player
const short IS_MISL = 0x0010; //object is a projectile type
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
#define MAXWEAPONS  10 

#define MAGIC_BOLT  10  //weapons not accessible to the player

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

#define NO_WEAPON     -1

struct Item
{
    static Item* CreateItem();
protected:
    Item(int type, int which);
    Item(const Item&) = default;
public:
    virtual Item* Clone() const = 0;
    virtual ~Item();

    //Return the name of something as it would appear in an inventory.
    std::string inventory_name(const Hero& hero, bool lowercase) const;
    std::string name() const;
    bool is_magic() const;
    bool is_evil() const;
    int worth() const;

    const Coord position() const;
    void set_position(Coord p);

    int hit_plus() const;
    int damage_plus() const;
    std::string melee_damage() const;
    std::string throw_damage() const;
    char launcher() const;

    int charges() const;
    void use_charge();

    bool does_group() const;

    void set_cursed();
    bool is_cursed() const;
    void remove_curse();

    void call_it();
    void discover();
    void set_known();
    bool is_known() const;

    bool is_found() const;
    void set_found();
    bool is_revealed() const;
    void set_revealed();

    Room* room() const;
    ItemClass* item_class() const;

    void set_as_target_of(Monster* m);

    int m_type;                      //What kind of object it is
    int m_which;                     //Which object of a type it is
    int m_count;                     //Count for plural objects
protected:
    Coord m_position;                //Where it lives on the screen
    char m_launcher;                 //What you need to launch it
    std::string m_damage;            //Damage if used like sword
    std::string m_throw_damage;      //Damage if thrown
    int m_hit_plus;                  //Plusses to hit
    int m_damage_plus;               //Plusses to damage
    short m_charges;                 //How many zaps the stick or weapon has
    short m_flags;                   //Information about objects
public:
    int m_group;                     //Group number for this object

    virtual ItemCategory* Category() const { return 0; }
    
private:
    virtual std::string InventoryName() const = 0;
    virtual std::string Name() const = 0;
    virtual bool IsMagic() const = 0;
    virtual bool IsEvil() const = 0;
    virtual int Worth() const = 0;

    bool is_flag_set(short flag) const;
};

int does_item_group(int type);
