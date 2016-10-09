/*
 * Functions for dealing with problems brought about by weapons
 *
 * @(#)weapons.c	3.17 (Berkeley) 6/15/81
 */

#include "curses.h"
#include <ctype.h>
#include "rogue.h"

#define NONE 100

char *w_names[MAXWEAPONS] = {
    "mace",
    "long sword",
    "short bow",
    "arrow",
    "dagger",
    "rock",
    "two handed sword",
    "sling",
    "dart",
    "crossbow",
    "crossbow bolt",
    "spear",
};

static struct init_weps {
    char *iw_dam;
    char *iw_hrl;
    char iw_launch;
    int iw_flags;
} init_dam[MAXWEAPONS] = {
    "2d4", "1d3", NONE, 0,		/* Mace */
    "1d10", "1d2", NONE,0,		/* Long sword */
    "1d1", "1d1", NONE,	0,		/* Bow */
    "1d1", "1d6", BOW,	ISMANY|ISMISL,	/* Arrow */
    "1d6", "1d4", NONE,	ISMISL,		/* Dagger */
    "1d2", "1d4", SLING,ISMANY|ISMISL,	/* Rock */
    "3d6", "1d2", NONE,	0,		/* 2h sword */
    "0d0", "0d0", NONE, 0,		/* Sling */
    "1d1", "1d3", NONE,	ISMANY|ISMISL,	/* Dart */
    "1d1", "1d1", NONE, 0,		/* Crossbow */
    "1d2", "1d10", CROSSBOW, ISMANY|ISMISL,/* Crossbow bolt */
    "1d8", "1d6", NONE, ISMISL,		/* Spear */
};

/*
 * missile:
 *	Fire a missile in a given direction
 */

missile(ydelta, xdelta)
int ydelta, xdelta;
{
    register struct object *obj;
    register struct linked_list *item, *nitem;

    /*
     * Get which thing we are hurling
     */
    if ((item = get_item("throw", WEAPON)) == NULL)
	return;
    obj = (struct object *) ldata(item);
    if (!dropcheck(obj) || is_current(obj))
	return;
    /*
     * Get rid of the thing.  If it is a non-multiple item object, or
     * if it is the last thing, just drop it.  Otherwise, create a new
     * item with a count of one.
     */
    if (obj->o_count < 2)
    {
	detach(pack, item);
	inpack--;
    }
    else
    {
	obj->o_count--;
	if (obj->o_group == 0)
	    inpack--;
	nitem = (struct linked_list *) new_item(sizeof *obj);
	obj = (struct object *) ldata(nitem);
	*obj = *((struct object *) ldata(item));
	obj->o_count = 1;
	item = nitem;
    }
    do_motion(obj, ydelta, xdelta);
    /*
     * AHA! Here it has hit something.  If it is a wall or a door,
     * or if it misses (combat) the mosnter, put it on the floor
     */
    if (!isupper(mvwinch(mw, obj->o_pos.y, obj->o_pos.x))
	|| !hit_monster(unc(obj->o_pos), obj))
	    fall(item, TRUE);
    mvwaddch(cw, hero.y, hero.x, PLAYER);
}

/*
 * do the actual motion on the screen done by an object traveling
 * across the room
 */
do_motion(obj, ydelta, xdelta)
register struct object *obj;
register int ydelta, xdelta;
{
    /*
     * Come fly with us ...
     */
    obj->o_pos = hero;
    for (;;)
    {
	register int ch;

	/*
	 * Erase the old one
	 */
	if (!ce(obj->o_pos, hero) && cansee(unc(obj->o_pos)) &&
	    mvwinch(cw, obj->o_pos.y, obj->o_pos.x) != ' ')
		    mvwaddch(cw, obj->o_pos.y, obj->o_pos.x,
			    show(obj->o_pos.y, obj->o_pos.x));
	/*
	 * Get the new position
	 */
	obj->o_pos.y += ydelta;
	obj->o_pos.x += xdelta;
	if (step_ok(ch = winat(obj->o_pos.y, obj->o_pos.x)) && ch != DOOR)
	{
	    /*
	     * It hasn't hit anything yet, so display it
	     * If it alright.
	     */
	    if (cansee(unc(obj->o_pos)) &&
		mvwinch(cw, obj->o_pos.y, obj->o_pos.x) != ' ')
	    {
		mvwaddch(cw, obj->o_pos.y, obj->o_pos.x, obj->o_type);
		draw(cw);
	    }
	    continue;
	}
	break;
    }
}

/*
 * fall:
 *	Drop an item someplace around here.
 */

fall(item, pr)
register struct linked_list *item;
bool pr;
{
    register struct object *obj;
    register struct room *rp;
    static coord fpos;

    obj = (struct object *) ldata(item);
    if (fallpos(&obj->o_pos, &fpos, TRUE))
    {
	mvaddch(fpos.y, fpos.x, obj->o_type);
	obj->o_pos = fpos;
	if ((rp = roomin(&hero)) != NULL && !(rp->r_flags & ISDARK))
	{
	    light(&hero);
	    mvwaddch(cw, hero.y, hero.x, PLAYER);
	}
	attach(lvl_obj, item);
	return;
    }
    if (pr)
	msg("Your %s vanishes as it hits the ground.", w_names[obj->o_which]);
    discard(item);
}

/*
 * init_weapon:
 *	Set up the initial goodies for a weapon
 */

init_weapon(weap, type)
register struct object *weap;
char type;
{
    register struct init_weps *iwp;

    iwp = &init_dam[type];
    strcpy(weap->o_damage,iwp->iw_dam);
    strcpy(weap->o_hurldmg,iwp->iw_hrl);
    weap->o_launch = iwp->iw_launch;
    weap->o_flags = iwp->iw_flags;
    if (weap->o_flags & ISMANY)
    {
	weap->o_count = rnd(8) + 8;
	weap->o_group = newgrp();
    }
    else
	weap->o_count = 1;
}

/*
 * Does the missile hit the monster
 */

hit_monster(y, x, obj)
register int y, x;
struct object *obj;
{
    static coord mp;

    mp.y = y;
    mp.x = x;
    return fight(&mp, winat(y, x), obj, TRUE);
}

/*
 * num:
 *	Figure out the plus number for armor/weapons
 */

char *
num(n1, n2)
register int n1, n2;
{
    static char numbuf[80];

    if (n1 == 0 && n2 == 0)
	return "+0";
    if (n2 == 0)
	sprintf(numbuf, "%s%d", n1 < 0 ? "" : "+", n1);
    else
        sprintf(numbuf, "%s%d,%s%d",
				n1 < 0 ? "" : "+", n1, n2 < 0 ? "" : "+", n2);
    return numbuf;
}    

/*
 * wield:
 *	Pull out a certain weapon
 */

wield()
{
    register struct linked_list *item;
    register struct object *obj, *oweapon;

    oweapon = cur_weapon;
    if (!dropcheck(cur_weapon))
    {
	cur_weapon = oweapon;
	return;
    }
    cur_weapon = oweapon;
    if ((item = get_item("wield", WEAPON)) == NULL)
    {
bad:
	after = FALSE;
	return;
    }

    obj = (struct object *) ldata(item);
    if (obj->o_type == ARMOR)
    {
	msg("You can't wield armor");
	goto bad;
    }
    if (is_current(obj))
        goto bad;

    if (terse)
	addmsg("W");
    else
	addmsg("You are now w");
    msg("ielding %s", inv_name(obj, TRUE));
    cur_weapon = obj;
}

/*
 * pick a random position around the give (y, x) coordinates
 */
fallpos(pos, newpos, passages)
register coord *pos, *newpos;
register bool passages;
{
    register int y, x, cnt, ch;

    cnt = 0;
    for (y = pos->y - 1; y <= pos->y + 1; y++)
	for (x = pos->x - 1; x <= pos->x + 1; x++)
	{
	    /*
	     * check to make certain the spot is empty, if it is,
	     * put the object there, set it in the level list
	     * and re-draw the room if he can see it
	     */
	    if (y == hero.y && x == hero.x)
		continue;
	    if (((ch = winat(y, x)) == FLOOR || (passages && ch == PASSAGE))
					&& rnd(++cnt) == 0)
	    {
		newpos->y = y;
		newpos->x = x;
	    }
	}
    return (cnt != 0);
}
