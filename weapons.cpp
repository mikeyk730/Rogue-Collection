//Functions for dealing with problems brought about by weapons
//weapons.c   1.4 (AI Design) 12/22/84

#include <stdio.h>

#include "rogue.h"
#include "game_state.h"
#include "monsters.h"
#include "pack.h"
#include "list.h"
#include "weapons.h"
#include "output_interface.h"
#include "io.h"
#include "things.h"
#include "fight.h"
#include "rooms.h"
#include "misc.h"
#include "main.h"
#include "level.h"
#include "mach_dep.h"
#include "armor.h"
#include "hero.h"
#include "pack.h"
#include "monster.h"

//mdk: IS_MISL has no effect
static struct init_weps
{
    char *iw_dam;   //Damage when wielded
    char *iw_hrl;   //Damage when thrown
    char iw_launch; //Launching weapon
    int iw_flags;   //Miscellaneous flags
} init_dam[MAXWEAPONS + 1] =
{
  "2d4", "1d3", NONE,     0,                 //Mace
  "3d4", "1d2", NONE,     0,                 //Long sword
  "1d1", "1d1", NONE,     0,                 //Bow
  "1d1", "2d3", BOW,      IS_MANY | IS_MISL, //Arrow
  "1d6", "1d4", NONE,     IS_MISL,           //Dagger
  "4d4", "1d2", NONE,     0,                 //2h sword
  "1d1", "1d3", NONE,     IS_MANY | IS_MISL, //Dart
  "1d1", "1d1", NONE,     0,                 //Crossbow
  "1d2", "2d5", CROSSBOW, IS_MANY | IS_MISL, //Crossbow bolt
  "2d3", "1d6", NONE,     IS_MISL,           //Spear
  "6d6", "6d6", NONE,     0                  //flame/frost/ice/lightning
};


//Names of the various weapons
const char *weapon_names[MAXWEAPONS + 1] =
{
  "mace",
  "long sword",
  "short bow",
  "arrow",
  "dagger",
  "two handed sword",
  "dart",
  "crossbow",
  "crossbow bolt",
  "spear",
  "bolt"  //flame/frost/ice/lightning
};

Item* create_weapon()
{
    int which = rnd(MAXWEAPONS);
    return new Weapon(which);
}

//throw_projectile: Fire a projectile in a given direction
bool throw_projectile()
{
    Coord delta;
    if (!get_dir(&delta))
        return false;

    Item *obj, *nitem;

    //Get which thing we are hurling
    if ((obj = get_item("throw", WEAPON)) == NULL)
        return false;

    if (!can_drop(obj, true))
        return true;

    //Get rid of the thing.  If it is a non-multiple item object, or if it is the last thing, just drop it.  
    //Otherwise, create a new item with a count of one.
    if (obj->count <= 1) {
        game->hero().pack.remove(obj);
    }
    else
    {
        obj->count--;
        nitem = obj->Clone();
        nitem->count = 1;
        obj = nitem;
    }
    do_motion(obj, delta);
    //AHA! Here it has hit something.  If it is a wall or a door, or if it misses (combat) the monster, put it on the floor
    if (game->level().monster_at(obj->pos) == NULL || !projectile_hit(obj->pos, obj))
        fall(obj, true);

    return true;
}

//do_motion: Do the actual motion on the screen done by an object travelling across the room
void do_motion(Item *obj, Coord delta)
{
    byte under = UNSET;

    //Come fly with us ...
    obj->pos = game->hero().pos;
    for (;;)
    {
        int ch;

        //Erase the old one
        if (under != UNSET && !equal(obj->pos, game->hero().pos) && game->hero().can_see(obj->pos))
            game->screen().mvaddch(obj->pos, under);
        //Get the new position
        obj->pos.y += delta.y;
        obj->pos.x += delta.x;
        //mdk: Originally thrown items would pass through mimics.  With =throws_affect_mimics= they
        //have a chance to hit.
        bool hit_mimics(game->options.throws_affect_mimics());
        if (step_ok(ch = game->level().get_tile_or_monster(obj->pos, hit_mimics)) && ch != DOOR)
        {
            //It hasn't hit anything yet, so display it if alright.
            if (game->hero().can_see(obj->pos))
            {
                //mdk:bugfix: xerox tile was replaced with floor after object passed
                under = game->level().get_tile_or_monster(obj->pos, false);
                //under = game->level().get_tile(obj->pos);

                game->screen().mvaddch(obj->pos, obj->type);
                tick_pause();
            }
            else 
                under = UNSET;
            continue;
        }
        break;
    }
}

//fall: Drop an item someplace around here.
void fall(Item *obj, bool pr)
{
    static Coord fpos;

    switch (fallpos(obj, &fpos))
    {
    case 1:
    {
        bool location_is_empty(game->level().is_floor_or_passage(fpos));

        obj->pos = fpos;
        game->level().set_tile(obj->pos, obj->type);
        game->level().items.push_front(obj);
        if (game->level().monster_at(fpos))
            game->level().monster_at(fpos)->set_tile_beneath(obj->type);

        //mdk:bugfix: prevent a fallen item from appearing on top of a monster
        //if (game->hero().can_see(fpos))
        if (game->hero().can_see(fpos) && location_is_empty)
        {
            if ((game->level().get_flags(obj->pos)&F_PASS) || (game->level().get_flags(obj->pos)&F_MAZE))
                game->screen().standout();
            game->screen().mvaddch(fpos, obj->type);
            game->screen().standend();
        }
        return;
    }
    case 2:
        pr = 0;
    }
    if (pr)
        msg("the %s vanishes%s.", obj->Name().c_str(), noterse(" as it hits the ground"));
    delete obj;
}

int Item::get_hit_plus() const
{
    return hit_plus;
}

int Item::get_damage_plus() const
{
    return damage_plus;
}

void Item::initialize_weapon(byte type)
{
    static int group = 2;

    init_weps* defaults = &init_dam[type];
    this->damage = defaults->iw_dam;
    this->throw_damage = defaults->iw_hrl;
    this->m_launcher = defaults->iw_launch;
    this->flags = defaults->iw_flags;
    if (this->does_group()) {
        this->count = rnd(8) + 8;
        this->group = group++;
    }
    else
        this->count = 1;
}

//projectile_hit: Does the projectile hit the monster?
Monster* projectile_hit(Coord p, Item *obj)  //todo: definite memory issues here.  projectile_hit is expeccted to delete, but gets stack variables too
{
    return game->hero().fight(p, obj, true);
}

//num: Figure out the plus number for armor/weapons
char *num(int n1, int n2, char type)
{
    static char numbuf[10];

    sprintf(numbuf, "%s%d", n1 < 0 ? "" : "+", n1);
    if (type == WEAPON)
        sprintf(&numbuf[strlen(numbuf)], ",%s%d", n2 < 0 ? "" : "+", n2);
    return numbuf;
}

//fallpos: Pick a random position around the given (y, x) coordinates
int fallpos(Item *obj, Coord *newpos)
{
   int cnt = 0;

    for (int y = obj->pos.y - 1; y <= obj->pos.y + 1; y++)
    {
        for (int x = obj->pos.x - 1; x <= obj->pos.x + 1; x++)
        {
            Coord pos = { x, y };
            //check to make certain the spot is empty, if it is, put the object there, set it in the 
            //level list and re-draw the room if he can see it
            if (pos == game->hero().pos || offmap(pos)) 
                continue;

            int ch = game->level().get_tile(pos);
            if (ch == FLOOR || ch == PASSAGE)
            {
                if (rnd(++cnt) == 0) { 
                    newpos->y = y; 
                    newpos->x = x; 
                }
                continue;
            }

            if (step_ok(ch)) {
                Item* floor_item = find_obj(pos, false);
                if (floor_item && floor_item->type == obj->type && floor_item->group && floor_item->group == obj->group)
                {
                    floor_item->count += obj->count;
                    return 2;
                }
            }
        }
    }
    return (cnt != 0);
}

std::string Weapon::InventoryName() const
{
    char *pb = prbuf;

    if (this->count > 1)
        sprintf(pb, "%d ", this->count);
    else
        sprintf(pb, "A%s ", vowelstr(Name().c_str()));
    pb = &prbuf[strlen(prbuf)];
    if (this->is_known() || game->wizard().reveal_items())
        sprintf(pb, "%s %s", num(this->hit_plus, this->damage_plus, WEAPON), Name().c_str());
    else
        sprintf(pb, "%s", Name().c_str());
    if (this->count > 1) strcat(pb, "s");
    if (this->is_vorpalized() && (this->is_revealed() || game->wizard().reveal_items()))
    {
        strcat(pb, " of ");
        strcat(pb, get_vorpalized_name().c_str());
        strcat(pb, " slaying");
    }

    return prbuf;
}

void Item::enchant_weapon()
{
    this->remove_curse();
    if (rnd(2) == 0)
        this->hit_plus++;
    else
        this->damage_plus++;
    ifterse("your %s glows blue", "your %s glows blue for a moment", Name().c_str());
}

bool Item::is_vorpalized() const
{
    return enemy != 0;
}

bool Item::is_vorpalized_against(Monster* monster) const
{
    if (!monster)
        return false;
    return enemy == monster->type;
}

std::string Item::get_vorpalized_name() const
{
    return get_monster_name(enemy);
}

std::string Item::get_throw_damage() const
{
    return throw_damage;
}

std::string Item::get_damage() const
{
    return damage;
}

char Item::launcher() const
{
    return m_launcher;
}

Room* Item::get_room()
{
    return get_room_from_position(pos);
}

void Item::vorpalize()
{
    //Extra Vorpal Enchant Weapon
    //    Give weapon +1,+1
    //    Is extremely vorpal against one certain type of monster
    //    Against this type of enemy the weapon gets:
    //        +4,+4
    //        The ability to zap one such monster into oblivion

    //You aren't allowed to doubly vorpalize a weapon.
    if (is_vorpalized())
    {
        msg("your %s vanishes in a puff of smoke", Name().c_str());
        game->hero().set_current_weapon(0);
        game->hero().pack.remove(this);
        delete this; //careful not to do anything afterwards
        return;
    }

    enemy = pick_vorpal_monster();
    hit_plus++;
    damage_plus++;
    charges = 1;
    msg(flash, Name().c_str(), short_msgs() ? "" : intense);
}

Weapon::Weapon(int which) :
    Item(WEAPON, which),
    m_name(weapon_names[which])
{
    initialize_weapon(which);

    int k;
    if ((k = rnd(100)) < 10) {
        set_cursed();
        hit_plus -= rnd(3) + 1;
    }
    else if (k < 15)
        hit_plus += rnd(3) + 1;
}

Weapon::Weapon(int which, int hit, int damage) :
    Item(WEAPON, which),
    m_name(weapon_names[which])
{
    initialize_weapon(which);

    hit_plus = hit;
    damage_plus = damage;
    if (hit_plus < 0 || damage_plus < 0)
        set_cursed();
}

Item * Weapon::Clone() const
{
    return new Weapon(*this);
}

std::string Weapon::Name() const
{
    return m_name;
}

bool Weapon::IsEvil() const
{
    return (get_hit_plus() < 0 || get_damage_plus() < 0);
}

int Weapon::Worth() const
{
    int worth = 0;
    switch (which)
    {
    case MACE: worth = 8; break;
    case SWORD: worth = 15; break;
    case CROSSBOW: worth = 30; break;
    case ARROW: worth = 1; break;
    case DAGGER: worth = 2; break;
    case TWOSWORD: worth = 75; break;
    case DART: worth = 1; break;
    case BOW: worth = 15; break;
    case BOLT: worth = 1; break;
    case SPEAR: worth = 5; break;
    }
    worth *= 3 * (get_hit_plus() + get_damage_plus()) + count;
    return worth;
}
