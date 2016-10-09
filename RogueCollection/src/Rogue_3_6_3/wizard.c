
/*
 * Special wizard commands (some of which are also non-wizard commands
 * under strange circumstances)
 *
 * @(#)wizard.c	3.8 (Berkeley) 6/3/81
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include "curses.h"
#include <ctype.h>
#include <string.h>
#include "rogue.h"

/*
 * whatis:
 *	What a certin object is
 */

whatis()
{
    register struct object *obj;
    register struct linked_list *item;

    if ((item = get_item("identify", 0)) == NULL)
	return;
    obj = (struct object *) ldata(item);
    switch (obj->o_type)
    {
        case SCROLL:
	    s_know[obj->o_which] = TRUE;
	    if (s_guess[obj->o_which])
	    {
		free(s_guess[obj->o_which]);
		s_guess[obj->o_which] = NULL;
	    }
        when POTION:
	    p_know[obj->o_which] = TRUE;
	    if (p_guess[obj->o_which])
	    {
		free(p_guess[obj->o_which]);
		p_guess[obj->o_which] = NULL;
	    }
	when STICK:
	    ws_know[obj->o_which] = TRUE;
	    obj->o_flags |= ISKNOW;
	    if (ws_guess[obj->o_which])
	    {
		free(ws_guess[obj->o_which]);
		ws_guess[obj->o_which] = NULL;
	    }
        when WEAPON:
        case ARMOR:
	    obj->o_flags |= ISKNOW;
        when RING:
	    r_know[obj->o_which] = TRUE;
	    obj->o_flags |= ISKNOW;
	    if (r_guess[obj->o_which])
	    {
		free(r_guess[obj->o_which]);
		r_guess[obj->o_which] = NULL;
	    }
    }
    msg(inv_name(obj, FALSE));
}

/*
 * create_obj:
 *	Wizard command for getting anything he wants
 */

create_obj()
{
    register struct linked_list *item;
    register struct object *obj;
    register char ch, bless;

    item = new_item(sizeof *obj);
    obj = (struct object *) ldata(item);
    msg("Type of item: ");
    obj->o_type = readchar(cw);
    mpos = 0;
    msg("Which %c do you want? (0-f)", obj->o_type);
    obj->o_which = (isdigit((ch = readchar(cw))) ? ch - '0' : ch - 'a' + 10);
    obj->o_group = 0;
    obj->o_count = 1;
    mpos = 0;
    if (obj->o_type == WEAPON || obj->o_type == ARMOR)
    {
	msg("Blessing? (+,-,n)");
	bless = readchar(cw);
	mpos = 0;
	if (obj->o_type == WEAPON)
	{
	    init_weapon(obj, obj->o_which);
	    if (bless == '-') {
		obj->o_hplus -= rnd(3)+1;
		obj->o_flags |= ISCURSED;
	    }
	    if (bless == '+')
		obj->o_hplus += rnd(3)+1;
	}
	else
	{
	    obj->o_ac = a_class[obj->o_which];
	    if (bless == '-') {
		obj->o_ac += rnd(3)+1;
		obj->o_flags |= ISCURSED;
	    }
	    if (bless == '+')
		obj->o_ac -= rnd(3)+1;
	}
    }
    else if (obj->o_type == RING)
	switch (obj->o_which)
	{
	    case R_PROTECT:
	    case R_ADDSTR:
	    case R_ADDHIT:
	    case R_ADDDAM:
		msg("Blessing? (+,-,n)");
		bless = readchar(cw);
		mpos = 0;
		if (bless == '-')
		    obj->o_flags |= ISCURSED;
		obj->o_ac = (bless == '-' ? -1 : rnd(2) + 1);
	}
    else if (obj->o_type == STICK)
	fix_stick(obj);
    add_pack(item, FALSE);
}

/*
 * telport:
 *	Bamf the hero someplace else
 */

teleport()
{
    register int rm;
    coord c;

    c = hero;
    mvwaddch(cw, hero.y, hero.x, mvwinch(stdscr, hero.y, hero.x));
    do
    {
	rm = rnd_room();
	rnd_pos(&rooms[rm], &hero);
    } until(winat(hero.y, hero.x) == FLOOR);
    light(&c);
    light(&hero);
    mvwaddch(cw, hero.y, hero.x, PLAYER);
    /*
     * turn off ISHELD in case teleportation was done while fighting
     * a Fungi
     */
    if (on(player, ISHELD)) {
	player.t_flags &= ~ISHELD;
	fung_hit = 0;
	strcpy(monsters['F'-'A'].m_stats.s_dmg, "000d0");
    }
    count = 0;
    running = FALSE;
    flush_type();		/* flush typeahead */
    return rm;
}

/*
 * passwd:
 *	see if user knows password
 */

passwd()
{
    register char *sp, c;
    char buf[80], *xcrypt();

    msg("Wizard's Password:");
    mpos = 0;
    sp = buf;
    while ((c = readchar(cw)) != '\n' && c != '\r' && c != '\033')
	if (c == md_killchar())
	    sp = buf;
	else if (c == md_erasechar() && sp > buf)
	    sp--;
	else
	    *sp++ = c;
    if (sp == buf)
	return FALSE;
    *sp = '\0';
    return (strcmp(PASSWD, xcrypt(buf, "mT")) == 0);
}
