//Functions for dealing with problems brought about by weapons
//weapons.c   1.4 (AI Design) 12/22/84

#include <stdio.h>
#include <cstring>
#include "rogue.h"
#include "random.h"
#include "game_state.h"
#include "monsters.h"
#include "pack.h"
#include "list.h"
#include "weapons.h"
#include "output_shim.h"
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
#include "text.h"

//mdk: IS_MISL has no effect.  It did in Unix 3.6.3, but I think it was
//correctly made obsolete.
static struct init_weps
{
    const char *iw_dam;   //Damage when wielded
    const char *iw_hrl;   //Damage when thrown
    char iw_launch; //Launching weapon
    int iw_flags;   //Miscellaneous flags
} init_dam[MAXWEAPONS + 1] =
{
  "2d4", "1d3", NO_WEAPON,  0,                 //Mace
  "3d4", "1d2", NO_WEAPON,  0,                 //Long sword
  "1d1", "1d1", NO_WEAPON,  0,                 //Bow
  "1d1", "2d3", BOW,        IS_MANY | IS_MISL, //Arrow
  "1d6", "1d4", NO_WEAPON,  IS_MISL,           //Dagger
  "4d4", "1d2", NO_WEAPON,  0,                 //2h sword
  "1d1", "1d3", NO_WEAPON,  IS_MANY | IS_MISL, //Dart
  "1d1", "1d1", NO_WEAPON,  0,                 //Crossbow
  "1d2", "2d5", CROSSBOW,   IS_MANY | IS_MISL, //Crossbow bolt
  "2d3", "1d6", NO_WEAPON,  IS_MISL,           //Spear
  "6d6", "6d6", NO_WEAPON,  0                  //flame/frost/ice/lightning
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

//do_throw_item: Fire a projectile in a given direction
bool do_throw_item()
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
    if (obj->m_count <= 1) {
        game->hero().m_pack.remove(obj);
    }
    else
    {
        obj->m_count--;
        nitem = obj->Clone();
        nitem->m_count = 1;
        obj = nitem;
    }
    do_motion(obj, delta);
    //AHA! Here it has hit something.  If it is a wall or a door, or if it misses (combat) the monster, put it on the floor
    if (game->level().monster_at(obj->position()) == NULL || !projectile_hit(obj->position(), obj))
        fall(obj, true);

    return true;
}

//do_motion: Do the actual motion on the screen done by an object travelling across the room
void do_motion(Item *obj, Coord delta)
{
    byte under = UNSET;

    //Come fly with us ...
    obj->set_position(game->hero().position());
    for (;;)
    {
        int ch;

        //Erase the old one
        if (under != UNSET && !equal(obj->position(), game->hero().position()) && game->hero().can_see(obj->position())) {
            if ((game->level().is_passage(obj->position()) || game->level().is_maze(obj->position())) && under != PASSAGE && under != ' ')
                game->screen().standout();
            game->screen().add_tile(obj->position(), under);
            game->screen().standend();
        }
        //Get the new position
        obj->set_position(obj->position() + delta);
        //mdk: Originally thrown items would pass through mimics.  With the 'throws_affect_mimics'
        //option, they have a chance to hit.
        bool hit_mimics(game->options.throws_affect_mimics());
        if (step_ok(ch = game->level().get_tile_or_monster(obj->position(), hit_mimics)) && ch != DOOR)
        {
            //It hasn't hit anything yet, so display it if alright.
            if (game->hero().can_see(obj->position()) && game->options.interactive())
            {
                //mdk:bugfix: xerox tile was replaced with floor after object passed
                under = game->screen().mvinch(obj->position());
                //under = game->m_level().get_tile(obj->position());

                if (game->level().use_standout(obj->position(), obj->m_type))
                    game->screen().standout();
                game->screen().add_tile(obj->position(), obj->m_type);
                game->screen().standend();
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

        obj->set_position(fpos);
        game->level().set_tile(obj->position(), obj->m_type);
        game->level().items.push_front(obj);
        //mdk:bugfix: don't set tile beneath unless in sight
        if (game->level().monster_at(fpos) && game->hero().can_see(fpos))
        //if (game->level().monster_at(fpos))
            game->level().monster_at(fpos)->set_tile_beneath(obj->m_type);

        //mdk:bugfix: prevent a fallen item from appearing on top of a monster
        //if (game->hero().can_see(fpos))
        if (game->hero().can_see(fpos) && location_is_empty)
        {
            if (game->level().use_standout(fpos, obj->m_type))
                game->screen().standout();
            game->screen().add_tile(fpos, obj->m_type);
            game->screen().standend();
        }
        return;
    }
    case 2:
        pr = 0;
    }
    if (pr)
        msg(get_text(text_item_vanishes), obj->name().c_str());
    delete obj;
}

void Weapon::initialize_weapon(byte type)
{
    static int group = 2;

    init_weps* defaults = &init_dam[type];
    this->m_damage = defaults->iw_dam;
    this->m_throw_damage = defaults->iw_hrl;
    this->m_launcher = defaults->iw_launch;
    this->m_flags = defaults->iw_flags;
    if (this->does_group()) {
        this->m_count = rnd(8) + 8;
        this->m_group = group++;
    }
    else
        this->m_count = 1;
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

    for (int y = obj->position().y - 1; y <= obj->position().y + 1; y++)
    {
        for (int x = obj->position().x - 1; x <= obj->position().x + 1; x++)
        {
            Coord pos = { x, y };
            //check to make certain the spot is empty, if it is, put the object there, set it in the
            //level list and re-draw the room if he can see it
            if (pos == game->hero().position() || offmap(pos))
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
                if (floor_item && floor_item->m_type == obj->m_type && floor_item->m_group && floor_item->m_group == obj->m_group)
                {
                    floor_item->m_count += obj->m_count;
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

    if (this->m_count > 1)
        sprintf(pb, "%d ", this->m_count);
    else
        sprintf(pb, "A%s ", vowelstr(TypeName().c_str()));
    pb = &prbuf[strlen(prbuf)];
    if (this->is_known() || game->wizard().reveal_items())
        sprintf(pb, "%s %s", num(this->m_hit_plus, this->m_damage_plus, WEAPON), TypeName().c_str());
    else
        sprintf(pb, "%s", TypeName().c_str());
    if (this->m_count > 1) strcat(pb, "s");
    if (this->is_vorpalized() && (this->is_revealed() || game->wizard().reveal_items()))
    {
        strcat(pb, " of ");
        strcat(pb, get_vorpalized_name().c_str());
        strcat(pb, " slaying");
    }

    return prbuf;
}

bool Weapon::IsMagic() const
{
    return hit_plus() != 0 || damage_plus() != 0;
}

bool Weapon::vorpalize()
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
        msg("your %s vanishes in a puff of smoke", TypeName().c_str());
        game->hero().set_current_weapon(0);
        game->hero().m_pack.remove(this);
        delete this; //careful not to do anything afterwards
        return false;
    }

    enemy = pick_vorpal_monster();
    m_hit_plus++;
    m_damage_plus++;
    m_charges = 1;
    msg(flash_msg, TypeName().c_str(), short_msgs() ? "" : intense);

    //Sometimes this is a mixed blessing ...
    if (game->options.act_like_v1_1()) {
        if (rnd(20) == 0) {
            set_cursed();
            if (!save(VS_MAGIC)) {
                set_revealed();
                msg("you feel a sudden desire to kill %ss.", get_vorpalized_name().c_str());
                return true;
            }
        }
    }
    return false;
}

Weapon::Weapon(int which) :
    Item(WEAPON, which),
    m_name(weapon_names[which])
{
    initialize_weapon(which);

    int k;
    if ((k = rnd(100)) < 10) {
        set_cursed();
        m_hit_plus -= rnd(3) + 1;
    }
    else if (k < 15)
        m_hit_plus += rnd(3) + 1;
}

Weapon::Weapon(int which, int hit, int damage) :
    Item(WEAPON, which),
    m_name(weapon_names[which])
{
    initialize_weapon(which);

    m_hit_plus = hit;
    m_damage_plus = damage;
    if (m_hit_plus < 0 || m_damage_plus < 0)
        set_cursed();
}

Item * Weapon::Clone() const
{
    return new Weapon(*this);
}

std::string Weapon::TypeName() const
{
    return m_name;
}

bool Weapon::IsEvil() const
{
    return (hit_plus() < 0 || damage_plus() < 0);
}

int Weapon::Worth() const
{
    int worth = 0;
    switch (m_which)
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
    worth *= 3 * (hit_plus() + damage_plus()) + m_count;
    return worth;
}

void Weapon::enchant_weapon()
{
    this->remove_curse();
    if (rnd(2) == 0)
        this->m_hit_plus++;
    else
        this->m_damage_plus++;
    msg(get_text(text_enchant_weapon), TypeName().c_str());
}

bool Weapon::is_vorpalized() const
{
    return enemy != 0;
}

bool Weapon::is_vorpalized_against(Monster* monster) const
{
    if (!monster)
        return false;
    return enemy == monster->m_type;
}

std::string Weapon::get_vorpalized_name() const
{
    return get_monster_name(enemy);
}

bool Weapon::did_flash() const
{
    return m_flashed;
}

void Weapon::set_flashed()
{
    m_flashed = true;
}
