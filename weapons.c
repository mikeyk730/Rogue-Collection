//Functions for dealing with problems brought about by weapons
//weapons.c   1.4 (AI Design) 12/22/84

#include <stdio.h>

#include "rogue.h"
#include "monsters.h"
#include "pack.h"
#include "list.h"
#include "weapons.h"
#include "curses.h"
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

#define NONE 100

static struct init_weps
{
  char *iw_dam;   //Damage when wielded
  char *iw_hrl;   //Damage when thrown
  char iw_launch; //Launching weapon
  int iw_flags;   //Miscellaneous flags
} init_dam[MAXWEAPONS] =
{
  "2d4", "1d3", NONE,     0,             //Mace
  "3d4", "1d2", NONE,     0,             //Long sword
  "1d1", "1d1", NONE,     0,             //Bow
  "1d1", "2d3", BOW,      ISMANY|ISMISL, //Arrow
  "1d6", "1d4", NONE,     ISMISL,        //Dagger
  "4d4", "1d2", NONE,     0,             //2h sword
  "1d1", "1d3", NONE,     ISMANY|ISMISL, //Dart
  "1d1", "1d1", NONE,     0,             //Crossbow
  "1d2", "2d5", CROSSBOW, ISMANY|ISMISL, //Crossbow bolt
  "2d3", "1d6", NONE,     ISMISL         //Spear
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
  "charge" //fake entry for dragon's breath
};

const char* get_weapon_name(int which)
{
  return weapon_names[which];
}

void init_new_weapon(ITEM* weapon)
{
  int k;
  weapon->type = WEAPON;
  weapon->which = rnd(MAXWEAPONS);
  init_weapon(weapon, weapon->which);
  if ((k = rnd(100))<10) {weapon->flags |= ISCURSED; weapon->hit_plus -= rnd(3)+1;}
  else if (k<15) weapon->hit_plus += rnd(3)+1;
}

//missile: Fire a missile in a given direction
void missile(int ydelta, int xdelta)
{
  ITEM *obj, *nitem;

  //Get which thing we are hurling
  if ((obj = get_item("throw", WEAPON))==NULL) return;
  if (!can_drop(obj) || is_current(obj)) return;
  //Get rid of the thing.  If it is a non-multiple item object, or if it is the last thing, just drop it.  Otherwise, create a new item with a count of one.
hack:
  if (obj->count<2) {
    detach_item(&player.pack, obj); 
  }
  else
  {
    //here is a quick hack to check if we can get a new item
    if ((nitem = create_item(0,0))==NULL)
    {
      obj->count = 1;
      msg("something in your pack explodes!!!");
      goto hack;
    }
    obj->count--;
    *nitem = *obj;
    nitem->count = 1;
    obj = nitem;
  }
  do_motion(obj, ydelta, xdelta);
  //AHA! Here it has hit something.  If it is a wall or a door, or if it misses (combat) the monster, put it on the floor
  if (monster_at(obj->pos.y, obj->pos.x)==NULL || !hit_monster(obj->pos.y, obj->pos.x, obj))
    fall(obj, TRUE);
}

//do_motion: Do the actual motion on the screen done by an object travelling across the room
void do_motion(ITEM *obj, int ydelta, int xdelta)
{
  byte under = '@';

  //Come fly with us ...
  obj->pos = player.pos;
  for (;;)
  {
    int ch;

    //Erase the old one
    if (under!='@' && !equal(obj->pos, player.pos) && cansee(obj->pos.y, obj->pos.x))
      mvaddch(obj->pos.y, obj->pos.x, under);
    //Get the new position
    obj->pos.y += ydelta;
    obj->pos.x += xdelta;
    if (step_ok(ch = get_tile_or_monster(obj->pos.y, obj->pos.x)) && ch!=DOOR)
    {
      //It hasn't hit anything yet, so display it if alright.
      if (cansee(obj->pos.y, obj->pos.x))
      {
        under = get_tile(obj->pos.y, obj->pos.x);
        mvaddch(obj->pos.y, obj->pos.x, obj->type);
        tick_pause();
      }
      else under = '@';
      continue;
    }
    break;
  }
}

const char *short_name(ITEM *obj)
{
  switch (obj->type)
  {
  case WEAPON: return get_weapon_name(obj->which);
  case ARMOR: return get_armor_name(obj->which);
  case FOOD: return "food";
  case POTION: case SCROLL: case AMULET: case STICK: case RING: return strchr(inv_name(obj, TRUE), ' ')+1;
  default: return "bizzare thing";
  }
}

//fall: Drop an item someplace around here.
void fall(ITEM *obj, bool pr)
{
  static Coord fpos;

  switch (fallpos(obj, &fpos))
  {
  case 1:
    set_tile(fpos.y, fpos.x, obj->type);
    obj->pos = fpos;
    if (cansee(fpos.y, fpos.x))
    {
      if ((get_flags(obj->pos.y, obj->pos.x)&F_PASS) || (get_flags(obj->pos.y, obj->pos.x)&F_MAZE)) standout();
      mvaddch(fpos.y, fpos.x, obj->type);
      standend();
      if (monster_at(fpos.y, fpos.x)!=NULL) monster_at(fpos.y, fpos.x)->oldch = obj->type;
    }
    attach_item(&lvl_obj, obj);
    return;

  case 2:
    pr = 0;
  }
  if (pr) msg("the %s vanishes%s.", short_name(obj), noterse(" as it hits the ground"));
  discard_item(obj);
}

//init_weapon: Set up the initial goodies for a weapon
void init_weapon(ITEM *weap, byte type)
{
  static int group = 2;
  struct init_weps *iwp;

  iwp = &init_dam[type];
  weap->damage = iwp->iw_dam;
  weap->throw_damage = iwp->iw_hrl;
  weap->launcher = iwp->iw_launch;
  weap->flags = iwp->iw_flags;
  if (weap->flags&ISMANY) {weap->count = rnd(8)+8; weap->group = group++;}
  else weap->count = 1;
}

//hit_monster: Does the missile hit the monster?
int hit_monster(int y, int x, ITEM *obj)
{
  static Coord mp;
  AGENT *monster = monster_at(y, x);
  if (!monster)  return FALSE;

  mp.y = y;
  mp.x = x; 
  return fight(&mp, monster->type, obj, TRUE);
 }

//num: Figure out the plus number for armor/weapons
char *num(int n1, int n2, char type)
{
  static char numbuf[10];

  sprintf(numbuf, "%s%d", n1<0?"":"+", n1);
  if (type==WEAPON) sprintf(&numbuf[strlen(numbuf)], ",%s%d", n2<0?"":"+", n2);
  return numbuf;
}

//wield: Pull out a certain weapon
void wield()
{
  ITEM *obj, *oweapon;
  char *sp;

  oweapon = cur_weapon;
  if (!can_drop(cur_weapon)) {cur_weapon = oweapon; return;}
  cur_weapon = oweapon;
  if ((obj = get_item("wield", WEAPON))==NULL)
  {
bad:
    after = FALSE;
    return;
  }
  if (obj->type==ARMOR) {msg("you can't wield armor"); goto bad;}
  if (is_current(obj)) goto bad;
  sp = inv_name(obj, TRUE);
  cur_weapon = obj;
  ifterse("now wielding %s (%c)", "you are now wielding %s (%c)", sp, pack_char(obj));
}

//fallpos: Pick a random position around the given (y, x) coordinates
int fallpos(ITEM *obj, Coord *newpos)
{
  int y, x, cnt = 0, ch;
  ITEM *onfloor;

  for (y = obj->pos.y-1; y<=obj->pos.y+1; y++)
    for (x = obj->pos.x-1; x<=obj->pos.x+1; x++)
    {
      //check to make certain the spot is empty, if it is, put the object there, set it in the level list and re-draw the room if he can see it
      if ((y==player.pos.y && x==player.pos.x) || offmap(y,x)) continue;
      if ((ch = get_tile(y, x))==FLOOR || ch==PASSAGE)
      {
        if (rnd(++cnt)==0) {newpos->y = y; newpos->x = x;}
        continue;
      }
      if (step_ok(ch) && (onfloor = find_obj(y, x)) && onfloor->type==obj->type && onfloor->group && onfloor->group==obj->group)
      {
        onfloor->count += obj->count;
        return 2;
      }
    }
    return (cnt!=0);
}

const char* get_inv_name_weapon(ITEM* weapon)
{
  char *pb = prbuf;
  int which = weapon->which;

  if (weapon->count>1) 
    sprintf(pb, "%d ", weapon->count);
  else
    sprintf(pb, "A%s ", vowelstr(get_weapon_name(which)));
  pb = &prbuf[strlen(prbuf)];
  if (weapon->flags&ISKNOW || is_wizard()) 
    sprintf(pb, "%s %s", num(weapon->hit_plus, weapon->damage_plus, WEAPON), get_weapon_name(which));
  else
    sprintf(pb, "%s", get_weapon_name(which));
  if (weapon->count>1) strcat(pb, "s");
  if (weapon->enemy && (weapon->flags&ISREVEAL || is_wizard()))
  {
    strcat(pb, " of ");
    strcat(pb, get_monster_name(weapon->enemy));
    strcat(pb, " slaying");
  }

  return prbuf;
}
