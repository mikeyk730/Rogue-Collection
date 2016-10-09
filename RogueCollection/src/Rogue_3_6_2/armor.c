/*
 * This file contains misc functions for dealing with armor
 * @(#)armor.c	3.9 (Berkeley) 6/15/81
 */

#include "curses.h"
#include "rogue.h"

/*
 * wear:
 *	The player wants to wear something, so let him/her put it on.
 */

wear()
{
    register struct linked_list *item;
    register struct object *obj;

    if (cur_armor != NULL)
    {
	addmsg("You are already wearing some");
	if (!terse)
	    addmsg(".  You'll have to take it off first");
	endmsg();
	after = FALSE;
	return;
    }
    if ((item = get_item("wear", ARMOR)) == NULL)
	return;
    obj = (struct object *) ldata(item);
    if (obj->o_type != ARMOR)
    {
	msg("You can't wear that.");
	return;
    }
    waste_time();
    if (!terse)
	addmsg("You are now w");
    else
	addmsg("W");
    msg("earing %s.", a_names[obj->o_which]);
    cur_armor = obj;
    obj->o_flags |= ISKNOW;
}

/*
 * take_off:
 *	Get the armor off of the players back
 */

take_off()
{
    register struct object *obj;

    if ((obj = cur_armor) == NULL)
    {
	if (terse)
		msg("Not wearing armor");
	else
		msg("You aren't wearing any armor");
	return;
    }
    if (!dropcheck(cur_armor))
	return;
    cur_armor = NULL;
    if (terse)
	addmsg("Was");
    else
	addmsg("You used to be ");
    msg(" wearing %c) %s", pack_char(obj), inv_name(obj, TRUE));
}

/*
 * waste_time:
 *	Do nothing but let other things happen
 */

waste_time()
{
    do_daemons(BEFORE);
    do_fuses(BEFORE);
    do_daemons(AFTER);
    do_fuses(AFTER);
}
