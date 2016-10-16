/*
 * Read and execute the user commands
 *
 * @(#)command.c	4.31 (Berkeley) 4/6/82
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981, 1982 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

/* Updated by Rogue Central @ coredumpcentral.org on 2012-12-06 and again on
 * 2013-12-31.
 * Copyright (C) 2012-2013 Rogue Central @ coredumpcentral.org. 
 * All Rights Reserved.
 * See README.CDC, LICENSE.CDC, and CHANGES.CDC for more information.
 */

#include <stdlib.h>
#include <curses.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "rogue.h"

shint countch, direction, newcount = FALSE;

/*
 * command:
 *	Process the user commands
 */
command()
{
    register BYTE ch;
    register int ntimes = 1;			/* Number of player moves */
    BYTE *unctrol();

    if (on(player, ISHASTE))
	ntimes++;
    /*
     * Let the daemons start up
     */
    do_daemons(BEFORE);
    do_fuses(BEFORE);
    while (ntimes--)
    {
	/*
	 * these are illegal things for the player to be, so if any are
	 * set, someone's been poking in memeory
	 */
	if (on(player, ISSLOW|ISCANC|ISGREED|ISINVIS|ISMEAN|ISREGEN))
	    auto_save(-1);

	look(TRUE);
	if (!running)
	    door_stop = FALSE;
	status();
	lastscore = purse;
	move(hero.y, hero.x);
	if (!((running || count) && jump))
	    refresh();			/* Draw screen */
	take = 0;
	after = TRUE;
	/*
	 * Read command or continue run
	 */
#ifdef WIZARD
	if (wizard)
	    noscore = TRUE;
#endif
	if (!no_command)
	{
	    if (running) ch = runch;
	    else if (count) ch = countch;
	    else
	    {
		ch = readchar();
		if (mpos != 0 && !running)	/* Erase message if its there */
		    msg("");
	    }
	}
	else
	    ch = '.';
	if (no_command)
	{
	    if (--no_command == 0)
	    {
		player.t_flags |= ISRUN;
		msg("you can move again");
	    }
	}
	else
	{
	    /*
	     * check for prefixes
	     */
	    if (isdigit(ch))
	    {
		count = 0;
		newcount = TRUE;
		while (isdigit(ch))
		{
		    count = count * 10 + (ch - '0');
		    ch = readchar();
		}
		countch = ch;
		/*
		 * turn off count for commands which don't make sense
		 * to repeat
		 */
		switch (ch) {
		    case 'h': case 'j': case 'k': case 'l':
		    case 'y': case 'u': case 'b': case 'n':
		    case 'H': case 'J': case 'K': case 'L':
		    case 'Y': case 'U': case 'B': case 'N':
		    case 'q': case 'r': case 's': case 'f':
		    case 't': case 'C': case 'I': case '.':
		    case 'z':
#ifdef WIZARD
		    case CTRL('D'): case CTRL('U'):
#endif
			break;
		    default:
			count = 0;
		}
	    }
	    switch (ch)
	    {
		case 'f':
		    if (!on(player, ISBLIND))
		    {
			door_stop = TRUE;
			firstmove = TRUE;
		    }
		    if (count && !newcount)
			ch = direction;
		    else
			ch = readchar();
		    switch (ch)
		    {
			case 'h': case 'j': case 'k': case 'l':
			case 'y': case 'u': case 'b': case 'n':
			    ch = toupper(ch);
		    }
		    direction = ch;
	    }
	    newcount = FALSE;
	    /*
	     * execute a command
	     */
	    if (count && !running)
		count--;
	    switch (ch)
	    {
		case '!' : SHELL_CMD;
		when 'h' : do_move(0, -1);
		when 'j' : do_move(1, 0);
		when 'k' : do_move(-1, 0);
		when 'l' : do_move(0, 1);
		when 'y' : do_move(-1, -1);
		when 'u' : do_move(-1, 1);
		when 'b' : do_move(1, -1);
		when 'n' : do_move(1, 1);
		when 'H' : do_run('h');
		when 'J' : do_run('j');
		when 'K' : do_run('k');
		when 'L' : do_run('l');
		when 'Y' : do_run('y');
		when 'U' : do_run('u');
		when 'B' : do_run('b');
		when 'N' : do_run('n');
		when 't':
		    if (!get_dir())
			after = FALSE;
		    else
			missile(delta.y, delta.x);
		when 'Q' : after = FALSE; quit(-1);
		when 'i' : after = FALSE; inventory(pack, 0);
		when 'I' : after = FALSE; picky_inven();
		when 'd' : drop();
		when 'q' : quaff();
		when 'r' : read_scroll();
		when 'e' : eat();
		when 'w' : wield();
		when 'W' : wear();
		when 'T' : take_off();
		when 'P' : ring_on();
		when 'R' : ring_off();
		when 'o' : option(); after = FALSE;
		when 'c' : call(); after = FALSE;
		when '>' : after = FALSE; d_level();
		when '<' : after = FALSE; u_level();
		when '?' : after = FALSE; help();
		when '/' : after = FALSE; identify();
		when 's' : search();
		when 'z':
		    if (get_dir())
			do_zap();
		    else
			after = FALSE;
		when 'D': after = FALSE; discovered();
		when CTRL('R') : after = FALSE; msg(huh);
		when CTRL('L') :
		    after = FALSE;
		    clearok(curscr,TRUE);
		    wrefresh(curscr);
		when 'v' :
		    after = FALSE;
		    msg("rogue version %s. (mctesq was here)", release);
		when 'S' : 
		    after = FALSE;
		    if (save_game())
		    {
			move(LINES-1, 0); 
			clrtoeol();
			refresh();
			endwin();
			exit(0);
		    }
		when '.' : ;			/* Rest command */
		when ' ' : after = FALSE;	/* "Legal" illegal command */
		when '^' :
		    after = FALSE;
		    if (get_dir()) {
			delta.y += hero.y;
			delta.x += hero.x;
			if (chat(delta.y, delta.x) != TRAP)
			    msg("no trap there");
			else
			    msg(tr_name(flat(delta.y, delta.x) & F_TMASK));
		    }
#ifdef WIZARD
		when CTRL('P') :
		    after = FALSE;
		    if (wizard)
		    {
			wizard = FALSE;
			turn_see(TRUE);
			msg("not wizard any more");
		    }
		    else
		    {
			if (wizard = passwd())
			{
			    noscore = TRUE;
			    turn_see(FALSE);
			    msg("you are suddenly as smart as Ken Arnold in dungeon #%d", dnum);
			}
			else
			    msg("sorry");
		    }
#endif
		when ESCAPE :	/* Escape */
		    door_stop = FALSE;
		    count = 0;
		    after = FALSE;
		otherwise :
		    after = FALSE;
#ifdef WIZARD
		    if (wizard) switch (ch)
		    {
			    /* Some Wizard keys commented out & changed
			     * to make the game functional in
			     * various terminals -RRPF
			     */ 
			case '@' : msg("@ %d,%d", hero.y, hero.x);
			when 'C' : create_obj();
			when CTRL('I') : inventory(lvl_obj, 0);
			when CTRL('W') : whatis(FALSE);
			when CTRL('D') : level++; new_level();
			when CTRL('A') : if (level > 1) level--; new_level();
			when CTRL('F') : show_map();
			when CTRL('T') : teleport();
			when CTRL('E') : msg("food left: %d", food_left);
			when '$' : msg("%d things in your pack", inpack);
			/* This one was changed for consistency
			 * to match RRPF's Rogue 3.6's documentation */
			/*when CTRL('K') : add_pass();*/
			when CTRL('C') : add_pass();
			when CTRL('X') : turn_see(on(player, SEEMONST));
			when '~' :
			{
			    register THING *item;

			    if ((item = get_item("charge", STICK)) != NULL)
				item->o_charges = 10000;
			}
			/*when CTRL('H') :*/
			when CTRL('G') :
			{
			    register int i;
			    register THING *obj;

			    for (i = 0; i < 9; i++)
				raise_level();
			    /*
			     * Give the rogue a sword (+1,+1)
			     */
			    obj = new_item();
			    obj->o_type = WEAPON;
			    obj->o_which = TWOSWORD;
			    init_weapon(obj, SWORD);
			    obj->o_hplus = 1;
			    obj->o_dplus = 1;
			    obj->o_count = 1;
			    obj->o_group = 0;
			    add_pack(obj, TRUE);
			    cur_weapon = obj;
			    /*
			     * And his suit of armor
			     */
			    obj = new_item();
			    obj->o_type = ARMOR;
			    obj->o_which = PLATE_MAIL;
			    obj->o_ac = -5;
			    obj->o_flags |= ISKNOW;
			    obj->o_count = 1;
			    obj->o_group = 0;
			    cur_armor = obj;
			    add_pack(obj, TRUE);
			}
			otherwise :
			    illcom(ch);
		    }
		    else
#endif
			illcom(ch);
	    }
	    /*
	     * turn off flags if no longer needed
	     */
	    if (!running)
		door_stop = FALSE;
	}
	/*
	 * If he ran into something to take, let him pick it up.
	 */
	if (take != 0)
	    pick_up(take);
	if (!running)
	    door_stop = FALSE;
	if (!after)
	    ntimes++;
    }
    do_daemons(AFTER);
    do_fuses(AFTER);
    if (ISRING(LEFT, R_SEARCH))
	search();
    else if (ISRING(LEFT, R_TELEPORT) && rnd(50) == 0)
	teleport();
    if (ISRING(RIGHT, R_SEARCH))
	search();
    else if (ISRING(RIGHT, R_TELEPORT) && rnd(50) == 0)
	teleport();
}

/*
 * illcom:
 *	What to do with an illegal command
 */
illcom(ch)
BYTE ch;
{
    save_msg = FALSE;
    count = 0;
    msg("illegal command '%s'", unctrol(ch));
    save_msg = TRUE;
}

/*
 * search:
 *	Player gropes about him to find hidden things.
 */
search()
{
    register int y, x;
    register BYTE *fp;
    register int ey, ex;

    if (on(player, ISBLIND))
	return;
    ey = hero.y + 1;
    ex = hero.x + 1;
    for (y = hero.y - 1; y <= ey; y++) 
	for (x = hero.x - 1; x <= ex; x++)
	{
	    if (y == hero.y && x == hero.x)
		continue;
	    fp = &flat(y, x);
	    if (!(*fp & F_REAL))
		switch (chat(y, x))
		{
		    case VWALL:
		    case HWALL:
			if (rnd(5) != 0)
			    break;
			chat(y, x) = DOOR;
			*fp |= F_REAL;
			count = running = FALSE;
			break;
		    case FLOOR:
			if (rnd(2) != 0)
			    break;
			chat(y, x) = TRAP;
			*fp |= F_REAL;
			count = running = FALSE;
			msg("%s%s", terse ? "" : "you found ", tr_name(*fp & F_TMASK));
			break;
		}
	}
}

/*
 * help:
 *	Give single character help, or the whole mess if he wants it
 */
help()
{
    register const struct h_list *strp = helpstr;
    register BYTE helpch;
    register int cnt;

    msg("character you want help for (* for all): ");
    helpch = readchar();
    mpos = 0;
    /*
     * If its not a *, print the right help string
     * or an error if he typed a funny character.
     */
    if (helpch != '*')
    {
	move(0, 0);
	while (strp->h_ch)
	{
	    if (strp->h_ch == helpch)
	    {
		msg("%s%s", unctrol(strp->h_ch), strp->h_desc);
		break;
	    }
	    strp++;
	}
	if (strp->h_ch != helpch)
	    msg("unknown character '%s'", unctrol(helpch));
	return;
    }
    /*
     * Here we print help for everything.
     * Then wait before we return to command mode
     */
    wclear(hw);
    cnt = 0;
    while (strp->h_ch)
    {
	mvwaddstr(hw, cnt % 23, cnt > 22 ? 40 : 0, unctrol(strp->h_ch));
	waddstr(hw, strp->h_desc);
	cnt++;
	strp++;
    }
    wmove(hw, LINES-1, 0);
    wprintw(hw, "--Press space to continue--");
    wrefresh(hw);
    w_wait_for(hw,' ');
    wmove(stdscr, 0, 0);
    wclrtoeol(stdscr);
    touchwin(stdscr);
    clearok(stdscr, TRUE);
    refresh();
}

/*
 * identify:
 *	Tell the player what a certain thing is.
 */
identify()
{
    register BYTE ch;
    register const BYTE *str;

    msg("what do you want identified? ");
    ch = readchar();
    mpos = 0;
    if (ch == ESCAPE)
    {
	msg("");
	return;
    }
    if (isupper(ch))
	str = monsters[ch-'A'].m_name;
    else switch (ch)
    {
	case VWALL:
	case HWALL:
	    str = "wall of a room";
	when GOLD: str = "gold";
	when STAIRS : str = "a staircase";
	when DOOR: str = "door";
	when FLOOR: str = "room floor";
	when PLAYER: str = "you";
	when PASSAGE: str = "passage";
	when TRAP: str = "trap";
	when POTION: str = "potion";
	when SCROLL: str = "scroll";
	when FOOD: str = "food";
	when WEAPON: str = "weapon";
	when ' ' : str = "solid rock";
	when ARMOR: str = "armor";
	when AMULET: str = "the Amulet of Yendor";
	when RING: str = "ring";
	when STICK: str = "wand or staff";
	otherwise: str = "unknown character";
    }
    msg("'%s': %s", unctrol(ch), str);
}

/*
 * d_level:
 *	He wants to go down a level
 */
d_level()
{
    if (chat(hero.y, hero.x) != STAIRS)
	msg("I see no way down");
    else
    {
	level++;
	new_level();
    }
}

/*
 * u_level:
 *	He wants to go up a level
 */
u_level()
{
    if (chat(hero.y, hero.x) == STAIRS)
	if (amulet)
	{
	    level--;
	    if (level == 0)
		total_winner();
	    new_level();
	    msg("you feel a wrenching sensation in your gut");
	}
	else
	    msg("your way is magically blocked");
    else
	msg("I see no way up");
}

/*
 * call:
 *	Allow a user to call a potion, scroll, or ring something
 */
call()
{
    register THING *obj;
    register BYTE **guess;
    const BYTE *elsewise;
    register bool *know;

    obj = get_item("call", CALLABLE);
    /*
     * Make certain that it is somethings that we want to wear
     */
    if (obj == NULL)
	return;
    switch (obj->o_type)
    {
	case RING:
	    guess = r_guess;
	    know = r_know;
	    elsewise = (r_guess[obj->o_which] != NULL ?
			r_guess[obj->o_which] : r_stones[obj->o_which]);
	when POTION:
	    guess = p_guess;
	    know = p_know;
	    elsewise = (p_guess[obj->o_which] != NULL ?
			p_guess[obj->o_which] : p_colors[obj->o_which]);
	when SCROLL:
	    guess = s_guess;
	    know = s_know;
	    elsewise = (s_guess[obj->o_which] != NULL ?
			s_guess[obj->o_which] : s_names[obj->o_which]);
	when STICK:
	    guess = ws_guess;
	    know = ws_know;
	    elsewise = (ws_guess[obj->o_which] != NULL ?
			ws_guess[obj->o_which] : ws_made[obj->o_which]);
	otherwise:
	    msg("you can't call that anything");
	    return;
    }
    if (know[obj->o_which])
    {
	msg("that has already been identified");
	return;
    }
    if (!terse)
	addmsg("Was ");
    msg("called \"%s\"", elsewise);
    if (terse)
	msg("call it: ");
    else
	msg("what do you want to call it? ");
    if (guess[obj->o_which] != NULL)
	free(guess[obj->o_which]);
    strcpy(prbuf, elsewise);
    if (get_str(prbuf, stdscr) == NORM)
    {
	guess[obj->o_which] = malloc((unsigned int) strlen(prbuf) + 1);
	strcpy(guess[obj->o_which], prbuf);
    }
    CLEAR_MSG;
}
