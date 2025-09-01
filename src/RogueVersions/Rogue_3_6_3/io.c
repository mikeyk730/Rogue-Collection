/*
 * Various input/output functions
 *
 * @(#)io.c	3.10 (Berkeley) 6/15/81
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <stdlib.h>
#include "curses.h"
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include "machdep.h"
#include "rogue.h"

/*
 * msg:
 *	Display a message at the top of the screen.
 */

static char msgbuf[BUFSIZ];
static int newpos = 0;

/*VARARGS1*/
void
msg(char *fmt, ...)
{
    va_list ap;
    /*
     * if the string is "", just clear the line
     */
    if (*fmt == '\0')
    {
	wmove(cw, 0, 0);
	wclrtoeol(cw);
	mpos = 0;
	return;
    }
    /*
     * otherwise add to the message and flush it out
     */
    va_start(ap, fmt);
    doadd(fmt, ap);
    va_end(ap);
    endmsg();
}

/*
 * add things to the current message
 */
void
addmsg(char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    doadd(fmt, ap);
    va_end(ap);
}

/*
 * Display a new msg (giving him a chance to see the previous one if it
 * is up there with the --More--)
 */
void
endmsg()
{
    strncpy(huh, msgbuf, 80);
    huh[79] = 0;

    if (mpos)
    {
	wmove(cw, 0, mpos);
    PC_GFX_COLOR(cw, 0x70);
    waddstr(cw, MORE_MSG);
    PC_GFX_NOCOLOR(cw, 0x70);
	draw(cw);
	wait_for(cw,' ');
    }
    mvwaddstr(cw, 0, 0, msgbuf);
    MDK_LOG("msg: %s\n", msgbuf);
    wclrtoeol(cw);
    mpos = newpos;
    newpos = 0;
    draw(cw);
}

void
doadd(char *fmt, va_list ap)
{
    vsprintf(&msgbuf[newpos], fmt, ap);
    newpos = (int) strlen(msgbuf);
}

/*
 * step_ok:
 *	returns true if it is ok to step on ch
 */

int
step_ok(int ch)
{
    switch (ch)
    {
	case ' ':
	case VWALL:
	case HWALL:
	PC_GFX_WALL_CASES
	case SECRETDOOR:
	    return FALSE;
	default:
	    return (!isalpha(ch));
    }
}

/*
 * readchar:
 *	flushes stdout so that screen is up to date and then returns
 *	getchar.
 */

int
readchar(WINDOW *win)
{
    int ch;

    ch = md_readchar(win);

#ifndef ROGUE_COLLECTION
    if ((ch == 3) || (ch == 0))
    {
	quit(0);
        return(27);
    }
#endif

    MDK_LOG("input: '%c' (%x)\n", ch, ch);
    return(ch);
}

/*
 * status:
 *	Display the important stats line.  Keep the cursor where it was.
 */

void
status()
{
    int oy, ox, temp;
    char *pb;
    static char buf[80];
    static int hpwidth = 0, s_hungry = -1;
    static int s_lvl = -1, s_pur, s_hp = -1, s_str, s_add, s_ac = 0;
    static long s_exp = 0;
    static int s_showac = 0;

    /*
     * If nothing has changed since the last status, don't
     * bother.
     */
    if (s_hp == pstats.s_hpt && s_exp == pstats.s_exp && s_pur == purse
	&& s_ac == (cur_armor != NULL ? cur_armor->o_ac : pstats.s_arm) && s_showac == showac
	&& s_str == pstats.s_str.st_str && s_add == pstats.s_str.st_add
	&& s_lvl == level && s_hungry == hungry_state)
	    return;

    getyx(cw, oy, ox);
    if (s_hp != max_hp)
    {
	temp = s_hp = max_hp;
	for (hpwidth = 0; temp; hpwidth++)
	    temp /= 10;
    }
    sprintf(buf, "Level: %d  Gold: %-5d  Hp: %*d(%*d)  Str: %-2d",
	level, purse, hpwidth, pstats.s_hpt, hpwidth, max_hp,
	pstats.s_str.st_str);
    if (pstats.s_str.st_add != 0)
    {
	pb = &buf[strlen(buf)];
	sprintf(pb, "/%d", pstats.s_str.st_add);
    }
    pb = &buf[strlen(buf)];
    if (showac) {
        sprintf(pb, "  Ac: %-2d  Exp: %d/%d",
            cur_armor != NULL ? cur_armor->o_ac : pstats.s_arm, pstats.s_lvl,
            pstats.s_exp);
    }
    else {
        sprintf(pb, "  Arm: %-2d Exp: %d/%d",
            ARMOR_DISPLAY(cur_armor != NULL ? cur_armor->o_ac : pstats.s_arm), pstats.s_lvl,
            pstats.s_exp);
    }
    /*
     * Save old status
     */
    s_lvl = level;
    s_pur = purse;
    s_hp = pstats.s_hpt;
    s_str = pstats.s_str.st_str;
    s_add = pstats.s_str.st_add;
    s_exp = pstats.s_exp;
    s_ac = (cur_armor != NULL ? cur_armor->o_ac : pstats.s_arm);
    s_showac = showac;
    PC_GFX_COLOR(cw, 0x0e);
    mvwaddstr(cw, LINES - 1, 0, buf);
    switch (hungry_state)
    {
	case 0: ;
	when 1:
	    waddstr(cw, "  Hungry");
	when 2:
	    waddstr(cw, "  Weak");
	when 3:
	    waddstr(cw, "  Fainting");
    }
    PC_GFX_NOCOLOR(cw, 0x0e);
    wclrtoeol(cw);
    s_hungry = hungry_state;
    wmove(cw, oy, ox);
}

/*
 * wait_for
 *	Sit around until the guy types the right key
 */

void
wait_for(WINDOW *win, int ch)
{
    int c;

    if (ch == '\n')
        while ((c = readchar(win)) != '\n' && c != '\r')
	    continue;
    else
        while (readchar(win) != ch)
	    continue;
}

/*
 * show_win:
 *	function used to display a window and wait before returning
 */

void
show_win(WINDOW *scr, char *message)
{
    mvwaddstr(scr, 0, 0, message);
    touchwin(scr);
    wmove(scr, hero.y, hero.x);
    draw(scr);
    wait_for(scr,' ');
    clearok(cw, TRUE);
    touchwin(cw);
}

void
flush_type()
{
	flushinp();
}
