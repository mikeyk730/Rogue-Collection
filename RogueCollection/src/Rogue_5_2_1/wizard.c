
/*
 * Special wizard commands (some of which are also non-wizard commands
 * under strange circumstances)
 *
 * @(#)wizard.c	4.14 (Berkeley) 1/26/82
 */

/* Updated by Rogue Central @ coredumpcentral.org on 2012-12-06.
 * Copyright (C) 2012 Rogue Central @ coredumpcentral.org. All Rights Reserved.
 * See README.CDC, LICENSE.CDC, and CHANGES.CDC for more information.
 */

#include <curses.h>
#include <ctype.h>
#include <string.h>
#include "rogue.h"

/*
 * whatis:
 *	What a certin object is
 */
whatis(insist)
bool insist;
{
    register THING *obj;

    if (pack == NULL)
    {
	msg("You don't have anything in your pack to identify");
	return;
    }

    for (;;)
	if ((obj = get_item("identify", 0)) == NULL && insist)
	    msg("You must identify something");
	else
	    break;

    if (!insist && obj == NULL)
	return;

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

#ifdef WIZARD
/*
 * create_obj:
 *	Wizard command for getting anything he wants
 */
create_obj()
{
    register THING *obj;
    register char ch, bless;

    obj = new_item();
    msg("type of item: ");
    obj->o_type = readchar();
    mpos = 0;
    msg("which %c do you want? (0-f)", obj->o_type);
    obj->o_which = (isdigit((ch = readchar())) ? ch - '0' : ch - 'a' + 10);
    obj->o_group = 0;
    obj->o_count = 1;
    mpos = 0;
    if (obj->o_type == WEAPON || obj->o_type == ARMOR)
    {
	msg("blessing? (+,-,n)");
	bless = readchar();
	mpos = 0;
	if (bless == '-')
	    obj->o_flags |= ISCURSED;
	if (obj->o_type == WEAPON)
	{
	    init_weapon(obj, obj->o_which);
	    if (bless == '-')
		obj->o_hplus -= rnd(3)+1;
	    if (bless == '+')
		obj->o_hplus += rnd(3)+1;
	}
	else
	{
	    obj->o_ac = a_class[obj->o_which];
	    if (bless == '-')
		obj->o_ac += rnd(3)+1;
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
		msg("blessing? (+,-,n)");
		bless = readchar();
		mpos = 0;
		if (bless == '-')
		    obj->o_flags |= ISCURSED;
		obj->o_ac = (bless == '-' ? -1 : rnd(2) + 1);
	    when R_AGGR:
	    case R_TELEPORT:
		obj->o_flags |= ISCURSED;
	}
    else if (obj->o_type == STICK)
	fix_stick(obj);
    else if (obj->o_type == GOLD)
    {
	msg("how much?");
	get_num(&obj->o_goldval, stdscr);
    }
    add_pack(obj, FALSE);
}
#endif

/*
 * telport:
 *	Bamf the hero someplace else
 */
teleport()
{
    register int rm;
    coord c;

    mvaddch(hero.y, hero.x, chat(hero.y, hero.x));
    do
    {
	rm = rnd_room();
	rnd_pos(&rooms[rm], &c);
    } until (step_ok(winat(c.y, c.x)));
    if (&rooms[rm] != proom)
    {
	leave_room(&hero);
	hero = c;
	enter_room(&hero);
    }
    else
    {
	hero = c;
	look(TRUE);
    }
    mvaddch(hero.y, hero.x, PLAYER);
    /*
     * turn off ISHELD in case teleportation was done while fighting
     * a Fungi
     */
    if (on(player, ISHELD)) {
	player.t_flags &= ~ISHELD;
	fung_hit = 0;
	strcpy(monsters['F'-'A'].m_stats.s_dmg, "000d0");
    }
    no_move = 0;
    count = 0;
    running = FALSE;
    flush_type();
    return rm;
}

#ifdef WIZARD
/*
 * passwd:
 *	See if user knows password
 */
passwd()
{
    /*register char *sp, c;
    char buf[MAXSTR], *xcrypt();
    Only below needed - RRPF*/
    char c;

    msg("wizard's Password:");
    /* Commented out by RRPF
     * and replaced with routine below
     mpos = 0;
    sp = buf;
    while ((c = readchar()) != '\n' && c != '\r' && c != ESCAPE)
	if (c == md_killchar())
	    sp = buf;
	else if (c == md_erasechar() && sp > buf)
	    sp--;
	else
	    *sp++ = c;
    if (sp == buf)
	return FALSE;
    *sp = '\0';
    return (strcmp(PASSWD, xcrypt(buf, "mT")) == 0);*/
    c = readchar();
    return (tolower(c) == 'y');
}

/*
 * show_map:
 *	Print out the map for the wizard
 */
show_map()
{
    register int y, x, real;

    wclear(hw);
    for (y = 1; y < LINES - 1; y++)
	for (x = 0; x < COLS; x++)
	{
	    if (!(real = flat(y, x) & F_REAL))
		wstandout(hw);
	    wmove(hw, y, x);
	    waddch(hw, chat(y, x));
	    if (!real)
		wstandend(hw);
	}
    show_win(hw, "---More (level map)---");
}
#endif
