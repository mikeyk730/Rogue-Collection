/*
 * Various input/output functions
 *
 * @(#)io.c	4.13 (Berkeley) 2/25/82
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981, 1982 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <curses.h>
#include <ctype.h>
#include <string.h>
#include "rogue.h"
#include <stdarg.h>

/*
 * msg:
 *	Display a message at the top of the screen.
 */
static char msgbuf[BUFSIZ];
static int newpos = 0;

msg(char *fmt, ...)
{
    va_list ap;
    /*
     * if the string is "", just clear the line
     */
    if (*fmt == '\0')
    {
	move(0, 0);
	clrtoeol();
	mpos = 0;
	return;
    }
    /*
     * otherwise add to the message and flush it out
     */
    va_start(ap,fmt);
    doadd(fmt, ap);
    va_end(ap);
    endmsg();
}

/*
 * addmsg:
 *	Add things to the current message
 */

addmsg(char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    doadd(fmt, ap);
    va_end(ap);
}

/*
 * endmsg:
 *	Display a new msg (giving him a chance to see the previous one
 *	if it is up there with the --More--)
 */

endmsg()
{
    if (save_msg)
    {
	strncpy(huh, msgbuf, 80);
	huh[79] = 0;
    }

    if (mpos)
    {
	look(FALSE);
	move(0, mpos);
	addstr("--More--");
	refresh();
	wait_for(' ');
    }
    /*
     * All messages should start with uppercase, except ones that
     * start with a pack addressing character
     */
    if (islower(msgbuf[0]) && msgbuf[1] != ')')
	msgbuf[0] = toupper(msgbuf[0]);
    mvaddstr(0, 0, msgbuf);
    clrtoeol();
    mpos = newpos;
    newpos = 0;
    refresh();
}

/*
 * doadd:
 *	Perform an add onto the message buffer
 */

doadd(char *fmt, va_list ap)
{
    vsprintf(&msgbuf[newpos], fmt, ap);
    newpos = strlen(msgbuf);
}

/*
 * step_ok:
 *	Returns true if it is ok to step on ch
 */
step_ok(ch)
{
    switch (ch)
    {
	case ' ':
	case '|':
	case '-':
	    return FALSE;
	default:
	    return (!isalpha(ch));
    }
}

/*
 * readchar:
 *	Flushes stdout so that screen is up to date and then returns
 *	getchar().
 */
readcharw(win)
WINDOW *win;
{
    int ch;

    ch = md_readchar(win);

    if ((ch == 3) || (ch == 0))
    {
	quit(0);
	return(27);
    }

    return(ch);
}

readchar()
{
    return( readcharw(stdscr) );
}

char *
unctrol(ch)
char ch;
{
    return( (char *) unctrl(ch) );
}

/*
 * status:
 *	Display the important stats line.  Keep the cursor where it was.
 */
status()
{
    register int oy, ox, temp;
    static int hpwidth = 0, s_hungry;
    static int s_lvl, s_pur = -1, s_hp, s_ac = 0;
    static str_t s_str;
    static long s_exp = 0;
    static char *state_name[] =
    {
	"", "Hungry", "Weak", "Faint"
    };

    /*
     * If nothing has changed since the last status, don't
     * bother.
     */
    if (s_hp == pstats.s_hpt && s_exp == pstats.s_exp && s_pur == purse
	&& s_ac == (cur_armor != NULL ? cur_armor->o_ac : pstats.s_arm)
	&& s_str == pstats.s_str && s_lvl == level && s_hungry == hungry_state)
	    return;
	
    getyx(stdscr, oy, ox);
    if (s_hp != max_hp)
    {
	temp = s_hp = max_hp;
	for (hpwidth = 0; temp; hpwidth++)
	    temp /= 10;
    }
    move(LINES - 1, 0);
    printw("Level: %d  Gold: %-5d  Hp: %*d(%*d)  Str: %2d(%d)  Ac: %-2d  Exp: %d/%ld  %s",
	    level, purse, hpwidth, pstats.s_hpt, hpwidth, max_hp, pstats.s_str,
	    max_stats.s_str,
	    cur_armor != NULL ? cur_armor->o_ac : pstats.s_arm, pstats.s_lvl,
	    pstats.s_exp, state_name[hungry_state]);

    clrtoeol();
    /*
     * Save old status
     */
    s_lvl = level;
    s_pur = purse;
    s_hp = pstats.s_hpt;
    s_str = pstats.s_str;
    s_exp = pstats.s_exp; 
    s_ac = (cur_armor != NULL ? cur_armor->o_ac : pstats.s_arm);
    s_hungry = hungry_state;
    move(oy, ox);
}

/*
 * wait_for
 *	Sit around until the guy types the right key
 */



wait_for(ch)
register char ch;
{
    w_wait_for(stdscr, ch);
}

w_wait_for(win,ch)
WINDOW *win;
register char ch;
{
    register char c;

    if (ch == '\n')
        while ((c = readcharw(win)) != '\n' && c != '\r')
	    continue;
    else
        while (readcharw(win) != ch)
	    continue;
}

/*
 * show_win:
 *	Function used to display a window and wait before returning
 */
show_win(scr, message)
register WINDOW *scr;
char *message;
{
    mvwaddstr(scr, 0, 0, message);
    touchwin(scr);
    wmove(scr, hero.y, hero.x);
    wrefresh(scr);
    w_wait_for(scr,' ');
    clearok(curscr, TRUE);
    touchwin(stdscr);
}
