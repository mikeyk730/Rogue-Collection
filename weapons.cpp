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
#include "chase.h"
#include "misc.h"
#include "main.h"
#include "level.h"
#include "thing.h"
#include "mach_dep.h"
#include "armor.h"
#include "hero.h"
#include "pack.h"

#define NONE 100

static struct init_weps
{
  char *iw_dam;   //Damage when wielded
  char *iw_hrl;   //Damage when thrown
  char iw_launch; //Launching weapon
  int iw_flags;   //Miscellaneous flags
} init_dam[MAXWEAPONS+1] =
{
  "2d4", "1d3", NONE,     0,               //Mace
  "3d4", "1d2", NONE,     0,               //Long sword
  "1d1", "1d1", NONE,     0,               //Bow
  "1d1", "2d3", BOW,      IS_MANY|IS_MISL, //Arrow
  "1d6", "1d4", NONE,     IS_MISL,         //Dagger
  "4d4", "1d2", NONE,     0,               //2h sword
  "1d1", "1d3", NONE,     IS_MANY|IS_MISL, //Dart
  "1d1", "1d1", NONE,     0,               //Crossbow
  "1d2", "2d5", CROSSBOW, IS_MANY|IS_MISL, //Crossbow bolt
  "2d3", "1d6", NONE,     IS_MISL,         //Spear
  "6d6", "6d6", NONE,     0                //Dragon flame (not accessible to player) //todo: check ice monster
};


//Names of the various weapons
const char *weapon_names[MAXWEAPONS+1] =
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
  "bolt"  // dragon flame/ice monster frost (not accessible to player)
};

const char* get_weapon_name(int which)
{
  return weapon_names[which];
}

Item* create_weapon()
{
    int which = rnd(MAXWEAPONS);
    return new Weapon(which);
}

//throw_projectile: Fire a projectile in a given direction
void throw_projectile(int ydelta, int xdelta)
{
    Item *obj, *nitem;

    //Get which thing we are hurling
    if ((obj = get_item("throw", WEAPON)) == NULL) 
        return;
    if (!can_drop(obj) || is_current(obj)) 
        return;

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
    do_motion(obj, ydelta, xdelta);
    //AHA! Here it has hit something.  If it is a wall or a door, or if it misses (combat) the monster, put it on the floor
    if (game->level().monster_at(obj->pos) == NULL || !projectile_hit(obj->pos, obj))
        fall(obj, true);
}

//do_motion: Do the actual motion on the screen done by an object travelling across the room
void do_motion(Item *obj, int ydelta, int xdelta)
{
  byte under = MDK;

  //Come fly with us ...
  obj->pos = game->hero().pos;
  for (;;)
  {
    int ch;

    //Erase the old one
    if (under != MDK && !equal(obj->pos, game->hero().pos) && game->hero().can_see(obj->pos))
      game->screen().mvaddch(obj->pos, under);
    //Get the new position
    obj->pos.y += ydelta;
    obj->pos.x += xdelta;
    if (step_ok(ch = game->level().get_tile_or_monster(obj->pos)) && ch!=DOOR)
    {
      //It hasn't hit anything yet, so display it if alright.
        if (game->hero().can_see(obj->pos))
      {
        under = game->level().get_tile(obj->pos);
        game->screen().mvaddch(obj->pos, obj->type);
        tick_pause();
      }
      else under = MDK;
      continue;
    }
    break;
  }
}

const char *short_name(Item *obj)
{
  switch (obj->type)
  {
  case WEAPON: return get_weapon_name(obj->which);
  case ARMOR: return get_armor_name(obj->which);
  case FOOD: return "food";
  case POTION: case SCROLL: case AMULET: case STICK: case RING: return strchr(inv_name(obj, true), ' ')+1;
  default: return "bizzare thing";
  }
}

//fall: Drop an item someplace around here.
void fall(Item *obj, bool pr)
{
  static Coord fpos;

  switch (fallpos(obj, &fpos))
  {
  case 1:
    game->level().set_tile(fpos, obj->type);
    obj->pos = fpos;
    if (game->hero().can_see(fpos))
    {
      if ((game->level().get_flags(obj->pos)&F_PASS) || (game->level().get_flags(obj->pos)&F_MAZE))
          game->screen().standout();
      game->screen().mvaddch(fpos, obj->type);
      game->screen().standend();
      if (game->level().monster_at(fpos)!=NULL) 
          game->level().monster_at(fpos)->oldch = obj->type;
    }
    game->level().items.push_front(obj);
    return;

  case 2:
    pr = 0;
  }
  if (pr)
      msg("the %s vanishes%s.", short_name(obj), noterse(" as it hits the ground"));
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
int projectile_hit(Coord p, Item *obj)  //todo: definite memory issues here.  projectile_hit is expeccted to delete, but gets stack variables too
{
  static Coord mp;
  Agent *monster = game->level().monster_at(p);
  if (!monster)  return false;

  mp.y = p.y;
  mp.x = p.x; 
  return game->hero().fight(&mp, obj, true);
 }

//num: Figure out the plus number for armor/weapons
char *num(int n1, int n2, char type)
{
  static char numbuf[10];

  sprintf(numbuf, "%s%d", n1<0?"":"+", n1);
  if (type==WEAPON) 
      sprintf(&numbuf[strlen(numbuf)], ",%s%d", n2<0?"":"+", n2);
  return numbuf;
}

//wield: Pull out a certain weapon
void Hero::wield()
{
  Item *obj;
  char *sp;

  if (!can_drop(get_current_weapon())) {
    return;
  }
  obj = get_item("wield", WEAPON);
  if (!obj || is_current(obj) || obj->type==ARMOR)
  {
    if (obj && obj->type==ARMOR) 
      msg("you can't wield armor"); 
    counts_as_turn = false;
    return;
  }

  sp = inv_name(obj, true);
  set_current_weapon(obj);
  ifterse("now wielding %s (%c)", "you are now wielding %s (%c)", sp, pack_char(obj));
}

//fallpos: Pick a random position around the given (y, x) coordinates
int fallpos(Item *obj, Coord *newpos)
{
  int y, x, cnt = 0, ch;
  Item *onfloor;

  for (y = obj->pos.y-1; y<=obj->pos.y+1; y++)
    for (x = obj->pos.x-1; x<=obj->pos.x+1; x++)
    {
      //check to make certain the spot is empty, if it is, put the object there, set it in the level list and re-draw the room if he can see it
      if ((y==game->hero().pos.y && x==game->hero().pos.x) || offmap({x,y})) continue;
      if ((ch = game->level().get_tile({x, y}))==FLOOR || ch==PASSAGE)
      {
        if (rnd(++cnt)==0) {newpos->y = y; newpos->x = x;}
        continue;
      }
      if (step_ok(ch) && (onfloor = find_obj({x,y})) && onfloor->type==obj->type && onfloor->group && onfloor->group==obj->group)
      {
        onfloor->count += obj->count;
        return 2;
      }
    }
    return (cnt!=0);
}

const char* Item::get_inv_name_weapon() const
{
  char *pb = prbuf;
  int which = this->which;

  if (this->count>1) 
    sprintf(pb, "%d ", this->count);
  else
    sprintf(pb, "A%s ", vowelstr(get_weapon_name(which)));
  pb = &prbuf[strlen(prbuf)];
  if (this->is_known() || game->hero().is_wizard()) 
    sprintf(pb, "%s %s", num(this->hit_plus, this->damage_plus, WEAPON), get_weapon_name(which));
  else
    sprintf(pb, "%s", get_weapon_name(which));
  if (this->count>1) strcat(pb, "s");
  if (this->is_vorpalized() && (this->is_revealed() || game->hero().is_wizard()))
  {
    strcat(pb, " of ");
    strcat(pb, this->get_vorpalized_name());
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
    ifterse("your %s glows blue", "your %s glows blue for a moment", get_weapon_name(this->which));
}

bool Item::is_vorpalized() const
{
    return enemy != 0;
}

bool Item::is_vorpalized_against(Agent* monster) const
{
    if (!monster)
        return false;
    return enemy == monster->type;
}

const char* Item::get_vorpalized_name() const
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

void Item::set_name(const std::string & name)
{
    m_name = name;
}

std::string Item::name() const
{
    return m_name;
}

Room * Item::get_room()
{
    return get_room_from_position(&pos);
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
        msg("your %s vanishes in a puff of smoke", get_weapon_name(which));
        set_current_weapon(0);
        game->hero().pack.remove(this);
        delete this; //careful not to do anything afterwards
        return;
    }

    enemy = pick_vorpal_monster();
    hit_plus++;
    damage_plus++;
    charges = 1;
    msg(flash, get_weapon_name(which), short_msgs() ? "" : intense);
}

Weapon::Weapon(int which) :
    Item(WEAPON, which)
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
    Item(WEAPON, which)
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
