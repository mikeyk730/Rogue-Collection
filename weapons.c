//Functions for dealing with problems brought about by weapons
//weapons.c   1.4 (AI Design) 12/22/84

#include "rogue.h"

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

//missile: Fire a missile in a given direction
missile(int ydelta, int xdelta)
{
  THING *obj, *nitem;

  //Get which thing we are hurling
  if ((obj = get_item("throw", WEAPON))==NULL) return;
  if (!can_drop(obj) || is_current(obj)) return;
  //Get rid of the thing.  If it is a non-multiple item object, or if it is the last thing, just drop it.  Otherwise, create a new item with a count of one.
hack:
  if (obj->o_count<2) {detach(pack, obj); inpack--;}
  else
  {
    //here is a quick hack to check if we can get a new item
    if ((nitem = new_item())==NULL)
    {
      obj->o_count = 1;
      msg("something in your pack explodes!!!");
      goto hack;
    }
    obj->o_count--;
    if (obj->o_group==0) inpack--;
    bcopy(*nitem, *obj);
    nitem->o_count = 1;
    obj = nitem;
  }
  do_motion(obj, ydelta, xdelta);
  //AHA! Here it has hit something.  If it is a wall or a door, or if it misses (combat) the monster, put it on the floor
  if (moat(obj->o_pos.y, obj->o_pos.x)==NULL || !hit_monster(unc(obj->o_pos), obj)) fall(obj, TRUE);
}

//do_motion: Do the actual motion on the screen done by an object travelling across the room
do_motion(THING *obj, int ydelta, int xdelta)
{
  byte under = '@';

  //Come fly with us ...
  bcopy(obj->o_pos, hero);
  for (;;)
  {
    int ch;

    //Erase the old one
    if (under!='@' && !ce(obj->o_pos, hero) && cansee(unc(obj->o_pos))) mvaddch(obj->o_pos.y, obj->o_pos.x, under);
    //Get the new position
    obj->o_pos.y += ydelta;
    obj->o_pos.x += xdelta;
    if (step_ok(ch = winat(obj->o_pos.y, obj->o_pos.x)) && ch!=DOOR)
    {
      //It hasn't hit anything yet, so display it if alright.
      if (cansee(unc(obj->o_pos)))
      {
        under = chat(obj->o_pos.y, obj->o_pos.x);
        mvaddch(obj->o_pos.y, obj->o_pos.x, obj->o_type);
        tick_pause();
      }
      else under = '@';
      continue;
    }
    break;
  }
}

char *short_name(THING *obj)
{
  switch (obj->o_type)
  {
    case WEAPON: return w_names[obj->o_which];
    case ARMOR: return a_names[obj->o_which];
    case FOOD: return "food";
    case POTION: case SCROLL: case AMULET: case STICK: case RING: return index(inv_name(obj, TRUE), ' ')+1;
    default: return "bizzare thing";
  }
}

//fall: Drop an item someplace around here.
fall(THING *obj, bool pr)
{
  static coord fpos;
  int index;

  switch (fallpos(obj, &fpos))
  {
    case 1:
      index = INDEX(fpos.y, fpos.x);
      _level[index] = obj->o_type;
      bcopy(obj->o_pos, fpos);
      if (cansee(fpos.y, fpos.x))
      {
        if ((flat(obj->o_pos.y, obj->o_pos.x)&F_PASS) || (flat(obj->o_pos.y, obj->o_pos.x)&F_MAZE)) standout();
        mvaddch(fpos.y, fpos.x, obj->o_type);
        standend();
        if (moat(fpos.y, fpos.x)!=NULL) moat(fpos.y, fpos.x)->t_oldch = obj->o_type;
      }
      attach(lvl_obj, obj);
      return;

    case 2:
      pr = 0;
  }
  if (pr) msg("the %s vanishes%s.", short_name(obj), noterse(" as it hits the ground"));
  discard(obj);
}

//init_weapon: Set up the initial goodies for a weapon
init_weapon(THING *weap, byte type)
{
  struct init_weps *iwp;

  iwp = &init_dam[type];
  weap->o_damage = iwp->iw_dam;
  weap->o_hurldmg = iwp->iw_hrl;
  weap->o_launch = iwp->iw_launch;
  weap->o_flags = iwp->iw_flags;
  if (weap->o_flags&ISMANY) {weap->o_count = rnd(8)+8; weap->o_group = group++;}
  else weap->o_count = 1;
}

//hit_monster: Does the missile hit the monster?
hit_monster(int y, int x, THING *obj)
{
  static coord mp;
  THING *mo;

  if (mo = moat(y, x)) {mp.y = y; mp.x = x; return fight(&mp, mo->t_type, obj, TRUE);}
  return FALSE;
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
wield()
{
  THING *obj, *oweapon;
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
  if (obj->o_type==ARMOR) {msg("you can't wield armor"); goto bad;}
  if (is_current(obj)) goto bad;
  sp = inv_name(obj, TRUE);
  cur_weapon = obj;
  ifterse2("now wielding %s (%c)", "you are now wielding %s (%c)", sp, pack_char(obj));
}

//fallpos: Pick a random position around the given (y, x) coordinates
fallpos(THING *obj, coord *newpos)
{
  int y, x, cnt = 0, ch;
  THING *onfloor;

  for (y = obj->o_pos.y-1; y<=obj->o_pos.y+1; y++)
  for (x = obj->o_pos.x-1; x<=obj->o_pos.x+1; x++)
  {
    //check to make certain the spot is empty, if it is, put the object there, set it in the level list and re-draw the room if he can see it
    if ((y==hero.y && x==hero.x) || offmap(y,x)) continue;
    if ((ch = chat(y, x))==FLOOR || ch==PASSAGE)
    {
      if (rnd(++cnt)==0) {newpos->y = y; newpos->x = x;}
      continue;
    }
    if (step_ok(ch) && (onfloor = find_obj(y, x)) && onfloor->o_type==obj->o_type && onfloor->o_group && onfloor->o_group==obj->o_group)
    {
      onfloor->o_count += obj->o_count;
      return 2;
    }
  }
  return (cnt!=0);
}

tick_pause()
{
  int otick;
  extern int tick;

  otick = tick;
  while (otick==tick) ;
}
