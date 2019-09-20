
/*
 * Special wizard commands (some of which are also non-wizard commands
 * under strange circumstances)
 *
 * @(#)wizard.c	4.16 (NMT from Berkeley 5.2) 8/25/83
 */

#include <curses.h>
#include <ctype.h>
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
        when SCROLL:
	    s_know[obj->o_which] = TRUE;
	    if (s_guess[obj->o_which])
	    {
		cfree(s_guess[obj->o_which]);
		s_guess[obj->o_which] = NULL;
	    }
        when POTION:
	    p_know[obj->o_which] = TRUE;
	    if (p_guess[obj->o_which])
	    {
		cfree(p_guess[obj->o_which]);
		p_guess[obj->o_which] = NULL;
	    }
	when STICK:
	    ws_know[obj->o_which] = TRUE;
	    obj->o_flags |= ISKNOW;
	    if (ws_guess[obj->o_which])
	    {
		cfree(ws_guess[obj->o_which]);
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
		cfree(r_guess[obj->o_which]);
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
    ch = readchar();
    obj->o_type = PC_GFX_TRANSLATE(ch);
    mpos = 0;
    msg("which %c do you want? (0-f)", ch);
    if (ch != '*')
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
	msg("how much? ");
	get_num(&obj->o_goldval, stdscr);
    CLEAR_MSG;
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

    mvaddrawch(hero.y, hero.x, chat(hero.y, hero.x));
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
    mvaddrawch(hero.y, hero.x, PLAYER);
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
    if (fight_flush) flush_type();
}

#ifdef WIZARD
/*
 * passwd:
 *	See if user knows password
 */
passwd()
{
    register char *sp, c;
    char buf[MAXSTR], *crypt();

    msg("wizard's Password:");
    mpos = 0;
#ifdef ROGUE_COLLECTION
    return (readchar() == 'y');
#endif
    sp = buf;
    while ((c = getchar()) != '\n' && c != '\r' && c != ESCAPE)
#ifndef r_attron
	if (c == _tty.sg_kill)
#else	r_attron
	if (c == killchar())
#endif	r_attron
	    sp = buf;
#ifndef r_attron
	else if (c == _tty.sg_erase && sp > buf)
#else	r_attron
	else if ((c == erasechar()) && sp > buf)
#endif	r_attron
	    sp--;
	else
	    *sp++ = c;
    if (sp == buf)
	return FALSE;
    *sp = '\0';
    return (strcmp("plus5",buf) == 0);
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
	    waddrawch(hw, chat(y, x));
	    if (!real)
		wstandend(hw);
	}
    show_win(hw, "---More (level map)---");
}
#endif
