/*
 * Routines to deal with the pack
 *
 * @(#)pack.c	4.15 (Berkeley) 4/6/82
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981, 1982 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <curses.h>
#include <ctype.h>
#include "rogue.h"

/*
 * update_mdest:
 *	Called after picking up an object, before discarding it.
 *	If this was the object of something's desire, that monster will
 *	get mad and run at the hero
 */
update_mdest(obj)
register THING *obj;
{
    register THING *mp;

    for (mp = mlist; mp != NULL; mp = next(mp))
	if (mp->t_dest == &obj->o_pos)
     mp->t_dest = &hero;
}

/*
 * add_pack:
 *	Pick up an object and add it to the pack.  If the argument is
 *	non-null use it as the linked_list pointer instead of gettting
 *	it off the ground.
 */
add_pack(obj, silent)
register THING *obj;
bool silent;
{
    register THING *op, *lp = NULL;
    register bool exact, from_floor;
    register char floor;
    int discarded = 0;

    if (obj == NULL)
    {
	from_floor = TRUE;
	if ((obj = find_obj(hero.y, hero.x)) == NULL)
	    return;
    }
    else
	from_floor = FALSE;
    /*
     * Link it into the pack.  Search the pack for a object of similar type
     * if there isn't one, stuff it at the beginning, if there is, look for one
     * that is exactly the same and just increment the count if there is.
     * it  that.  Food is always put at the beginning for ease of access, but
     * is not ordered so that you can't tell good food from bad.  First check
     * to see if there is something in thr same group and if there is then
     * increment the count.
     */
/*    floor = (proom->r_flags & ISGONE) ? PASSAGE : FLOOR; */
    if (proom) floor = (proom->r_flags & ISGONE) ? PASSAGE : FLOOR;
    else floor = FLOOR;
    if (obj->o_group)
    {
	for (op = pack; op != NULL; op = next(op))
	{
	    if (op->o_group == obj->o_group)
	    {
		/*
		 * Put it in the pack and notify the user
		 */
		op->o_count++;
		if (from_floor)
		{
		    detach(lvl_obj, obj);
		    mvaddch(hero.y, hero.x, floor);
		    chat(hero.y, hero.x) = floor;
		}
		update_mdest(obj);
		discard(obj);
		obj = op;
		discarded = 1;
		goto picked_up;
	    }
	}
    }
    /*
     * Check if there is room
     */
    if (inpack == MAXPACK-1)
    {
	msg("you can't carry anything else");
	return;
    }
    /*
     * Check for and deal with scare monster scrolls
     */
    if (obj->o_type == SCROLL && obj->o_which == S_SCARE)
	if (obj->o_flags & ISFOUND)
	{
	    detach(lvl_obj, obj);
	    mvaddch(hero.y, hero.x, floor);
	    chat(hero.y, hero.x) = floor;
	    msg("the scroll turns to dust as you pick it up");
	    return;
	}
	else
	    obj->o_flags |= ISFOUND;

    inpack++;
    if (from_floor)
    {
	detach(lvl_obj, obj);
	mvaddch(hero.y, hero.x, floor);
	chat(hero.y, hero.x) = floor;
    }
    /*
     * Search for an object of the same type
     */
    exact = FALSE;
    for (op = pack; op != NULL; op = next(op))
	if (obj->o_type == op->o_type)
	    break;
    if (op == NULL)
    {
	/*
	 * Put it at the end of the pack since it is a new type
	 */
	for (op = pack; op != NULL; op = next(op))
	{
	    if (op->o_type != FOOD)
		break;
	    lp = op;
	}
    }
    else
    {
	/*
	 * Search for an object which is exactly the same
	 */
	while (op->o_type == obj->o_type)
	{
	    if (op->o_which == obj->o_which)
	    {
		exact = TRUE;
		break;
	    }
	    lp = op;
	    if ((op = next(op)) == NULL)
		break;
	}
    }
    if (op == NULL)
    {
	/*
	 * Didn't find an exact match, just stick it here
	 */
	if (pack == NULL)
	    pack = obj;
	else
	{
	    lp->l_next = obj;
	    obj->l_prev = lp;
	    obj->l_next = NULL;
	}
    }
    else
    {
	/*
	 * If we found an exact match.  If it is a potion, food, or a 
	 * scroll, increase the count, otherwise put it with its clones.
	 */
	if (exact && ISMULT(obj->o_type))
	{
	    op->o_count++;
	    update_mdest(obj);
	    discard(obj);
	    obj = op;
	    discarded = 1;
	    goto picked_up;
	}
	if ((obj->l_prev = prev(op)) != NULL)
	    obj->l_prev->l_next = obj;
	else
	    pack = obj;
	obj->l_next = op;
	op->l_prev = obj;
    }
picked_up:
    /*
     * If this was the object of something's desire, that monster will
     * get mad and run at the hero
     */
    if (!discarded)
        update_mdest(obj);

    if (obj->o_type == AMULET)
	amulet = TRUE;
    /*
     * Notify the user
     */
    if (!silent)
    {
	if (!terse)
	    addmsg("you now have ");
	msg("%s (%c)", inv_name(obj, !terse), pack_char(obj));
    }
}

/*
 * inventory:
 *	List what is in the pack
 */
inventory(list, type)
THING *list;
int type;
{
    register char ch;
    register int n_objs;
    char inv_temp[MAXSTR];

    n_objs = 0;
    for (ch = 'a'; list != NULL; ch++, list = next(list))
    {
	if (type && type != list->o_type && !(type == CALLABLE &&
	    (list->o_type == SCROLL || list->o_type == POTION ||
	     list->o_type == RING || list->o_type == STICK)))
		continue;
	n_objs++;
	sprintf(inv_temp, "%c) %%s", ch);
	add_line(inv_temp, inv_name(list, FALSE));
    }
    if (n_objs == 0)
    {
	if (terse)
	    msg(type == 0 ? "empty handed" :
			    "nothing appropriate");
	else
	    msg(type == 0 ? "you are empty handed" :
			    "you don't have anything appropriate");
	return FALSE;
    }
    end_line();
    return TRUE;
}

/*
 * pick_up:
 *	Add something to characters pack.
 */
pick_up(ch)
char ch;
{
    register THING *obj, *mp;

    switch (ch)
    {
	case GOLD:
	    if ((obj = find_obj(hero.y, hero.x)) == NULL)
		return;
	    money(obj->o_goldval);
	    detach(lvl_obj, obj);
	    update_mdest(obj);
	    discard(obj);
	    proom->r_goldval = 0;
	    break;
	default:
#ifdef WIZARD
	    debug("Where did you pick a '%s' up???", unctrol(ch));
#endif
	case ARMOR:
	case POTION:
	case FOOD:
	case WEAPON:
	case SCROLL:	
	case AMULET:
	case RING:
	case STICK:
	    add_pack(NULL, FALSE);
	    break;
    }
}

/*
 * picky_inven:
 *	Allow player to inventory a single item
 */
picky_inven()
{
    register THING *obj;
    register char ch, mch;

    if (pack == NULL)
	msg("you aren't carrying anything");
    else if (next(pack) == NULL)
	msg("a) %s", inv_name(pack, FALSE));
    else
    {
	msg(terse ? "item: " : "which item do you wish to inventory: ");
	mpos = 0;
	if ((mch = readchar()) == ESCAPE)
	{
	    msg("");
	    return;
	}
	for (ch = 'a', obj = pack; obj != NULL; obj = next(obj), ch++)
	    if (ch == mch)
	    {
		msg("%c) %s",ch,inv_name(obj, FALSE));
		return;
	    }
	if (!terse)
	    msg("'%s' not in pack", unctrol(mch));
	msg("range is 'a' to '%c'", --ch);
    }
}

/*
 * get_item:
 *	Pick something out of a pack for a purpose
 */
THING *
get_item(purpose, type)
char *purpose;
int type;
{
    register THING *obj;
    register char ch, och;

    if (pack == NULL)
	msg("you aren't carrying anything");
    else
    {
	for (;;)
	{
	    if (!terse)
		addmsg("which object do you want to ");
	    addmsg(purpose);
	    if (terse)
		addmsg(" what");
	    msg("? (* for list): ");
	    ch = readchar();
	    mpos = 0;
	    /*
	     * Give the poor player a chance to abort the command
	     */
	    if (ch == ESCAPE || ch == CTRL('G'))
	    {
		after = FALSE;
		msg("");
		return NULL;
	    }
	    if (ch == '*')
	    {
		mpos = 0;
		if (inventory(pack, type) == 0)
		{
		    after = FALSE;
		    return NULL;
		}
		continue;
	    }
	    for (obj = pack, och = 'a'; obj != NULL; obj = next(obj), och++)
		if (ch == och)
		    break;
	    if (obj == NULL)
	    {
		msg("please specify a letter between 'a' and '%c'", och-1);
		continue;
	    }
	    else 
		return obj;
	}
    }
    return NULL;
}

/*
 * pack_char:
 *	Return which character would address a pack object
 */
pack_char(obj)
register THING *obj;
{
    register THING *item;
    register char c;

    c = 'a';
    for (item = pack; item != NULL; item = next(item))
	if (item == obj)
	    return c;
	else
	    c++;
    return '?';
}

/*
 * money:
 *	Add or subtract gold from the pack
 */
money(value)
register int value;
{
    register char floor;

    floor = (proom->r_flags & ISGONE) ? PASSAGE : FLOOR;
    purse += value;
    mvaddch(hero.y, hero.x, floor);
    chat(hero.y, hero.x) = floor;
    if (value > 0)
    {
	if (!terse)
	    addmsg("you found ");
	msg("%d gold pieces", value);
    }
}
