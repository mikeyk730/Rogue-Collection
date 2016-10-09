#include "curses.h"
#include <ctype.h>
#include "rogue.h"

/*
 * Routines to deal with the pack
 *
 * @(#)pack.c	3.6 (Berkeley) 6/15/81
 */

/*
 * add_pack:
 *	Pick up an object and add it to the pack.  If the argument is non-null
 * use it as the linked_list pointer instead of gettting it off the ground.
 */
add_pack(item, silent)
register struct linked_list *item;
bool silent;
{
    register struct linked_list *ip, *lp;
    register struct object *obj, *op;
    register bool exact, from_floor;

    if (item == NULL)
    {
	from_floor = TRUE;
	if ((item = find_obj(hero.y, hero.x)) == NULL)
	    return;
    }
    else
	from_floor = FALSE;
    obj = (struct object *) ldata(item);
    /*
     * Link it into the pack.  Search the pack for a object of similar type
     * if there isn't one, stuff it at the beginning, if there is, look for one
     * that is exactly the same and just increment the count if there is.
     * it  that.  Food is always put at the beginning for ease of access, but
     * is not ordered so that you can't tell good food from bad.  First check
     * to see if there is something in thr same group and if there is then
     * increment the count.
     */
    if (obj->o_group)
    {
	for (ip = pack; ip != NULL; ip = next(ip))
	{
	    op = (struct object *) ldata(ip);
	    if (op->o_group == obj->o_group)
	    {
		/*
		 * Put it in the pack and notify the user
		 */
		op->o_count++;
		if (from_floor)
		{
		    detach(lvl_obj, item);
		    mvaddch(hero.y, hero.x,
			(roomin(&hero) == NULL ? PASSAGE : FLOOR));
		}
		discard(item);
		item = ip;
		goto picked_up;
	    }
	}
    }
    /*
     * Check if there is room
     */
    if (inpack == MAXPACK-1)
    {
	msg("You can't carry anything else.");
	return;
    }
    /*
     * Check for and deal with scare monster scrolls
     */
    if (obj->o_type == SCROLL && obj->o_which == S_SCARE)
	if (obj->o_flags & ISFOUND)
	{
	    msg("The scroll turns to dust as you pick it up.");
	    detach(lvl_obj, item);
	    mvaddch(hero.y, hero.x, FLOOR);
	    return;
	}
	else
	    obj->o_flags |= ISFOUND;

    inpack++;
    if (from_floor)
    {
	detach(lvl_obj, item);
	mvaddch(hero.y, hero.x, (roomin(&hero) == NULL ? PASSAGE : FLOOR));
    }
    /*
     * Search for an object of the same type
     */
    exact = FALSE;
    for (ip = pack; ip != NULL; ip = next(ip))
    {
	op = (struct object *) ldata(ip);
	if (obj->o_type == op->o_type)
	    break;
    }
    if (ip == NULL)
    {
	/*
	 * Put it at the end of the pack since it is a new type
	 */
	for (ip = pack; ip != NULL; ip = next(ip))
	{
	    op = (struct object *) ldata(ip);
	    if (op->o_type != FOOD)
		break;
	    lp = ip;
	}
    }
    else
    {
	/*
	 * Search for an object which is exactly the same
	 */
	while (ip != NULL && op->o_type == obj->o_type)
	{
	    if (op->o_which == obj->o_which)
	    {
		exact = TRUE;
		break;
	    }
	    lp = ip;
	    if ((ip = next(ip)) == NULL)
		break;
	    op = (struct object *) ldata(ip);
	}
    }
    if (ip == NULL)
    {
	/*
	 * Didn't find an exact match, just stick it here
	 */
	if (pack == NULL)
	    pack = item;
	else
	{
	    lp->l_next = item;
	    item->l_prev = lp;
	    item->l_next = NULL;
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
	    discard(item);
	    item = ip;
	    goto picked_up;
	}
	if ((item->l_prev = prev(ip)) != NULL)
	    item->l_prev->l_next = item;
	else
	    pack = item;
	item->l_next = ip;
	ip->l_prev = item;
    }
picked_up:
    /*
     * Notify the user
     */
    obj = (struct object *) ldata(item);
    if (notify && !silent)
    {
	if (!terse)
	    addmsg("You now have ");
	msg("%s (%c)", inv_name(obj, !terse), pack_char(obj));
    }
    if (obj->o_type == AMULET)
	amulet = TRUE;
}

/*
 * inventory:
 *	list what is in the pack
 */
inventory(list, type)
struct linked_list *list;
int type;
{
    register struct object *obj;
    register char ch;
    register int n_objs;
    char inv_temp[80];

    n_objs = 0;
    for (ch = 'a'; list != NULL; ch++, list = next(list))
    {
	obj = (struct object *) ldata(list);
	if (type && type != obj->o_type && !(type == CALLABLE &&
	    (obj->o_type == SCROLL || obj->o_type == POTION ||
	     obj->o_type == RING || obj->o_type == STICK)))
		continue;
	switch (n_objs++)
	{
	    /*
	     * For the first thing in the inventory, just save the string
	     * in case there is only one.
	     */
	    case 0:
		sprintf(inv_temp, "%c) %s", ch, inv_name(obj, FALSE));
		break;
	    /*
	     * If there is more than one, clear the screen, print the
	     * saved message and fall through to ...
	     */
	    case 1:
		if (slow_invent)
		    msg(inv_temp);
		else
		{
		    wclear(hw);
		    waddstr(hw, inv_temp);
		    waddch(hw, '\n');
		}
	    /*
	     * Print the line for this object
	     */
	    default:
		if (slow_invent)
		    msg("%c) %s", ch, inv_name(obj, FALSE));
		else
		    wprintw(hw, "%c) %s\n", ch, inv_name(obj, FALSE));
	}
    }
    if (n_objs == 0)
    {
	if (terse)
	    msg(type == 0 ? "Empty handed." :
			    "Nothing appropriate");
	else
	    msg(type == 0 ? "You are empty handed." :
			    "You don't have anything appropriate");
	return FALSE;
    }
    if (n_objs == 1)
    {
	msg(inv_temp);
	return TRUE;
    }
    if (!slow_invent)
    {
	mvwaddstr(hw, LINES-1, 0, "--Press space to continue--");
	draw(hw);
	wait_for(' ');
	clearok(cw, TRUE);
	touchwin(cw);
    }
    return TRUE;
}

/*
 * pick_up:
 *	Add something to characters pack.
 */
pick_up(ch)
char ch;
{
    switch(ch)
    {
	case GOLD:
	    money();
	    break;
	default:
	    debug("Where did you pick that up???");
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
    register struct linked_list *item;
    register char ch, mch;

    if (pack == NULL)
	msg("You aren't carrying anything");
    else if (next(pack) == NULL)
	msg("a) %s", inv_name((struct object *) ldata(pack), FALSE));
    else
    {
	msg(terse ? "Item: " : "Which item do you wish to inventory: ");
	mpos = 0;
	if ((mch = readchar()) == ESCAPE)
	{
	    msg("");
	    return;
	}
	for (ch = 'a', item = pack; item != NULL; item = next(item), ch++)
	    if (ch == mch)
	    {
		msg("%c) %s",ch,inv_name((struct object *) ldata(item), FALSE));
		return;
	    }
	if (!terse)
	    msg("'%s' not in pack", unctrl(mch));
	msg("Range is 'a' to '%c'", --ch);
    }
}

/*
 * get_item:
 *	pick something out of a pack for a purpose
 */
struct linked_list *
get_item(purpose, type)
char *purpose;
int type;
{
    register struct linked_list *obj;
    register char ch, och;

    if (pack == NULL)
	msg("You aren't carrying anything.");
    else
    {
	for (;;)
	{
	    if (!terse)
		addmsg("Which object do you want to ");
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
		msg("Please specify a letter between 'a' and '%c'", och-1);
		continue;
	    }
	    else 
		return obj;
	}
    }
    return NULL;
}

pack_char(obj)
register struct object *obj;
{
    register struct linked_list *item;
    register char c;

    c = 'a';
    for (item = pack; item != NULL; item = next(item))
	if ((struct object *) ldata(item) == obj)
	    return c;
	else
	    c++;
    return 'z';
}
