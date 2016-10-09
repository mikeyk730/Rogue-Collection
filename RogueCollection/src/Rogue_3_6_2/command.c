/*
 * Read and execute the user commands
 *
 * @(#)command.c	3.45 (Berkeley) 6/15/81
 */

#include "curses.h"
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include "rogue.h"

/*
 * command:
 *	Process the user commands
 */

char countch = FALSE, direction = FALSE, newcount = FALSE;

command()
{
    register char ch;
    register int ntimes = 1;			/* Number of player moves */

    if (on(player, ISHASTE)) ntimes++;
    /*
     * Let the daemons start up
     */
    do_daemons(BEFORE);
    do_fuses(BEFORE);
    while (ntimes--)
    {
	look(TRUE);
	if (!running)
	    door_stop = FALSE;
	status();
	lastscore = purse;
	wmove(cw, hero.y, hero.x);
	if (!((running || count) && jump))
	    draw(cw);			/* Draw screen */
	take = 0;
	after = TRUE;
	/*
	 * Read command or continue run
	 */
	if (wizard)
	    waswizard = TRUE;
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
	else ch = ' ';
	if (no_command)
	{
	    if (--no_command == 0)
		msg("You can move again.");
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
		    case 't': case 'C': case 'I': case ' ':
		    case 'z': case 'p':
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
		case '!' : shell();
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
		when 'o' : option();
		when 'c' : call();
		when '>' : after = FALSE; d_level();
		when '<' : after = FALSE; u_level();
		when '?' : after = FALSE; help();
		when '/' : after = FALSE; identify();
		when 's' : search();
		when 'z' : do_zap(FALSE);
		when 'p':
		    if (get_dir())
			do_zap(TRUE);
		    else
			after = FALSE;
		when 'v' : msg("Rogue version %s. (mctesq was here)", release);
		when CTRL('L') : after = FALSE; clearok(curscr,TRUE);draw(curscr);
		when CTRL('R') : after = FALSE; msg(huh);
		when 'S' : 
		    after = FALSE;
		    if (save_game())
		    {
			wmove(cw, LINES-1, 0); 
			wclrtoeol(cw);
			draw(cw);
			endwin();
			exit(0);
		    }
		when ' ' : ;			/* Rest command */
		when CTRL('P') :
		    after = FALSE;
		    if (wizard)
		    {
			wizard = FALSE;
			msg("Not wizard any more");
		    }
		    else
		    {
			if (wizard = passwd())
			{
			    msg("You are suddenly as smart as Ken Arnold in dungeon #%d", dnum);
 			    wizard = TRUE;
			    waswizard = TRUE;
			}
			else
			    msg("Sorry");
		    }
		when ESCAPE :	/* Escape */
		    door_stop = FALSE;
		    count = 0;
		    after = FALSE;
		otherwise :
		    after = FALSE;
		    if (wizard) switch (ch)
		    {
			case '@' : msg("@ %d,%d", hero.y, hero.x);
			when 'C' : create_obj();
			when CTRL('I') : inventory(lvl_obj, 0);
			when CTRL('W') : whatis();
			when CTRL('D') : level++; new_level();
			when CTRL('U') : level--; new_level();
			when CTRL('F') : show_win(stdscr, "--More (level map)--");
			when CTRL('X') : show_win(mw, "--More (monsters)--");
			when CTRL('T') : teleport();
			when CTRL('E') : msg("food left: %d", food_left);
			when CTRL('A') : msg("%d things in your pack", inpack);
			when CTRL('C') : add_pass();
			when CTRL('N') :
			{
			    register struct linked_list *item;

			    if ((item = get_item("charge", STICK)) != NULL)
				((struct object *) ldata(item))->o_charges = 10000;
			}
			when CTRL('H') :
			{
			    register int i;
			    register struct linked_list *item;
			    register struct object *obj;

			    for (i = 0; i < 9; i++)
				raise_level();
			    /*
			     * Give the rogue a sword (+1,+1)
			     */
			    item = new_item(sizeof *obj);
			    obj = (struct object *) ldata(item);
			    obj->o_type = WEAPON;
			    obj->o_which = TWOSWORD;
			    init_weapon(obj, SWORD);
			    obj->o_hplus = 1;
			    obj->o_dplus = 1;
			    add_pack(item, TRUE);
			    cur_weapon = obj;
			    /*
			     * And his suit of armor
			     */
			    item = new_item(sizeof *obj);
			    obj = (struct object *) ldata(item);
			    obj->o_type = ARMOR;
			    obj->o_which = PLATE_MAIL;
			    obj->o_ac = -5;
			    obj->o_flags |= ISKNOW;
			    cur_armor = obj;
			    add_pack(item, TRUE);
			}
			otherwise :
			    msg("Illegal command '%s'.", unctrl(ch));
			    count = 0;
		    }
		    else
		    {
			msg("Illegal command '%s'.", unctrl(ch));
			count = 0;
		    }
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
    }
    /*
     * Kick off the rest if the daemons and fuses
     */
    if (after)
    {
	look(FALSE);
	do_daemons(AFTER);
	do_fuses(AFTER);
	if (ISRING(LEFT, R_SEARCH))
	    search();
	else if (ISRING(LEFT, R_TELEPORT) && rnd(100) < 2)
	    teleport();
	if (ISRING(RIGHT, R_SEARCH))
	    search();
	else if (ISRING(RIGHT, R_TELEPORT) && rnd(100) < 2)
	    teleport();
    }
}

/*
 * quit:
 *	Have player make certain, then exit.
 */

void
quit(int p)
{
    /*
     * Reset the signal in case we got here via an interrupt
     */
    if (signal(SIGINT, quit) != quit)
	mpos = 0;
    msg("Really quit?");
    draw(cw);
    if (readchar() == 'y')
    {
	clear();
	move(LINES-1, 0);
	draw(stdscr);
	score(purse, 1);
	exit(0);
    }
    else
    {
	signal(SIGINT, quit);
	wmove(cw, 0, 0);
	wclrtoeol(cw);
	status();
	draw(cw);
	mpos = 0;
	count = 0;
    }
}

/*
 * search:
 *	Player gropes about him to find hidden things.
 */

search()
{
    register int x, y;
    register char ch;

    /*
     * Look all around the hero, if there is something hidden there,
     * give him a chance to find it.  If its found, display it.
     */
    if (on(player, ISBLIND))
	return;
    for (x = hero.x - 1; x <= hero.x + 1; x++)
	for (y = hero.y - 1; y <= hero.y + 1; y++)
	{
	    ch = winat(y, x);
	    switch (ch)
	    {
		case SECRETDOOR:
		    if (rnd(100) < 20) {
			mvaddch(y, x, DOOR);
			count = 0;
		    }
		    break;
		case TRAP:
		{
		    register struct trap *tp;

		    if (mvwinch(cw, y, x) == TRAP)
			break;
		    if (rnd(100) > 50)
			break;
		    tp = trap_at(y, x);
		    tp->tr_flags |= ISFOUND;
		    mvwaddch(cw, y, x, TRAP);
		    count = 0;
		    running = FALSE;
		    msg(tr_name(tp->tr_type));
		}
	    }
	}
}

/*
 * help:
 *	Give single character help, or the whole mess if he wants it
 */

help()
{
    register struct h_list *strp = helpstr;
    register char helpch;
    register int cnt;

    msg("Character you want help for (* for all): ");
    helpch = readchar();
    mpos = 0;
    /*
     * If its not a *, print the right help string
     * or an error if he typed a funny character.
     */
    if (helpch != '*')
    {
	wmove(cw, 0, 0);
	while (strp->h_ch)
	{
	    if (strp->h_ch == helpch)
	    {
		msg("%s%s", unctrl(strp->h_ch), strp->h_desc);
		break;
	    }
	    strp++;
	}
	if (strp->h_ch != helpch)
	    msg("Unknown character '%s'", unctrl(helpch));
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
	mvwaddstr(hw, cnt % 23, cnt > 22 ? 40 : 0, unctrl(strp->h_ch));
	waddstr(hw, strp->h_desc);
	cnt++;
	strp++;
    }
    wmove(hw, LINES-1, 0);
    wprintw(hw, "--Press space to continue--");
    draw(hw);
    wait_for(' ');
    wclear(hw);
    draw(hw);
    wmove(cw, 0, 0);
    wclrtoeol(cw);
    status();
    touchwin(cw);
}

/*
 * identify:
 *	Tell the player what a certain thing is.
 */

identify()
{
    register char ch, *str;

    msg("What do you want identified? ");
    ch = readchar();
    mpos = 0;
    if (ch == ESCAPE)
    {
	msg("");
	return;
    }
    if (isalpha(ch) && isupper(ch))
	str = monsters[ch-'A'].m_name;
    else switch(ch)
    {
	case '|':
	case '-':
	    str = "wall of a room";
	when GOLD: str = "gold";
	when STAIRS : str = "passage leading down";
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
	when AMULET: str = "The Amulet of Yendor";
	when RING: str = "ring";
	when STICK: str = "wand or staff";
	otherwise: str = "unknown character";
    }
    msg("'%s' : %s", unctrl(ch), str);
}

/*
 * d_level:
 *	He wants to go down a level
 */

d_level()
{
    if (winat(hero.y, hero.x) != STAIRS)
	msg("I see no way down.");
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
    if (winat(hero.y, hero.x) == STAIRS)
    {
	if (amulet)
	{
	    level--;
	    if (level == 0)
		total_winner();
	    new_level();
	    msg("You feel a wrenching sensation in your gut.");
	    return;
	}
    }
    msg("I see no way up.");
}

/*
 * Let him escape for a while
 */

shell()
{
    register int pid;
    register char *sh;
    int ret_status;

    /*
     * Set the terminal back to original mode
     */
    sh = getenv("SHELL");
    wclear(hw);
    wmove(hw, LINES-1, 0);
    draw(hw);
    endwin();
    in_shell = TRUE;
    fflush(stdout);
    /*
     * Fork and do a shell
     */
    while((pid = fork()) < 0)
	sleep(1);
    if (pid == 0)
    {
	/*
	 * Set back to original user, just in case
	 */
	setuid(getuid());
	setgid(getgid());
	execl(sh == NULL ? "/bin/sh" : sh, "shell", "-i", 0);
	perror("No shelly");
	exit(-1);
    }
    else
    {
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	while (wait(&ret_status) != pid)
	    continue;
	signal(SIGINT, endit);
	signal(SIGQUIT, endit);
	printf("\n[Press return to continue]");
	noecho();
	crmode();
	in_shell = FALSE;
	wait_for('\n');
	clearok(cw, TRUE);
	touchwin(cw);
    }
}

/*
 * allow a user to call a potion, scroll, or ring something
 */
call()
{
    register struct object *obj;
    register struct linked_list *item;
    register char **guess, *elsewise;
    register bool *know;

    item = get_item("call", CALLABLE);
    /*
     * Make certain that it is somethings that we want to wear
     */
    if (item == NULL)
	return;
    obj = (struct object *) ldata(item);
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
	    msg("You can't call that anything");
	    return;
    }
    if (know[obj->o_which])
    {
	msg("That has already been identified");
	return;
    }
    if (terse)
	addmsg("C");
    else
	addmsg("Was c");
    msg("alled \"%s\"", elsewise);
    if (terse)
	msg("Call it: ");
    else
	msg("What do you want to call it? ");
    if (guess[obj->o_which] != NULL)
	free(guess[obj->o_which]);
    strcpy(prbuf, elsewise);
    if (get_str(prbuf, cw) == NORM)
    {
	guess[obj->o_which] = malloc((unsigned int) strlen(prbuf) + 1);
	strcpy(guess[obj->o_which], prbuf);
    }
}
