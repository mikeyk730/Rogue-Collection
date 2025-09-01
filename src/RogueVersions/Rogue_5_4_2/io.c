/*
 * Various input/output functions
 *
 * @(#)io.c	4.32 (Berkeley) 02/05/99
 */

#include <stdarg.h>
#include <curses.h>
#include <ctype.h>
#include <string.h>
#include "rogue.h"

/*
 * msg:
 *	Display a message at the top of the screen.
 */
#define MAXMSG	(NUMCOLS - sizeof "--More--")

static char msgbuf[2*MAXMSG+1];
static int newpos = 0;

/* VARARGS1 */
int
msg(const char *fmt, ...)
{
    va_list args;

    /*
     * if the string is "", just clear the line
     */
    if (*fmt == '\0')
    {
	move(0, 0);
	clrtoeol();
	mpos = 0;
	return ~ESCAPE;
    }
    /*
     * otherwise add to the message and flush it out
     */
    va_start(args, fmt);
    doadd(fmt, args);
    va_end(args);
    return endmsg();
}

/*
 * addmsg:
 *	Add things to the current message
 */
/* VARARGS1 */
void
addmsg(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    doadd(fmt, args);
    va_end(args);
}

/*
 * endmsg:
 *	Display a new msg (giving him a chance to see the previous one
 *	if it is up there with the --More--)
 */
int
endmsg(void)
{
    int ch;

    if (save_msg)
	strcpy(huh, msgbuf);
    if (mpos)
    {
	look(FALSE);
    PC_GFX_COLOR(0x70);
    mvaddstr(0, mpos, MORE_MSG);
    PC_GFX_NOCOLOR(0x70);
    refresh();
	if (!msg_esc)
	    wait_for(stdscr, ' ');
	else
	{
	    while ((ch = readchar()) != ' ')
		if (ch == ESCAPE)
		{
		    msgbuf[0] = '\0';
		    mpos = 0;
		    newpos = 0;
		    msgbuf[0] = '\0';
		    return ESCAPE;
		}
	}
    }
    /*
     * All messages should start with uppercase, except ones that
     * start with a pack addressing character
     */
    if (islower((int)msgbuf[0]) && !lower_msg && msgbuf[1] != ')')
	msgbuf[0] = (char) toupper(msgbuf[0]);
    mvaddstr(0, 0, msgbuf);
    MDK_LOG("msg: %s\n", msgbuf);
    clrtoeol();
    mpos = newpos;
    newpos = 0;
    msgbuf[0] = '\0';
    refresh();
    return ~ESCAPE;
}

/*
 * doadd:
 *	Perform an add onto the message buffer
 */
void
doadd(const char *fmt, va_list args)
{
    static char buf[MAXSTR];

    /*
     * Do the printf into buf
     */
    vsprintf(buf, fmt, args);
    if (strlen(buf) + newpos >= MAXMSG)
        endmsg();
    strcat(msgbuf, buf);
    newpos = (int) strlen(msgbuf);
}

/*
 * step_ok:
 *	Returns true if it is ok to step on ch
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
	    return FALSE;
	default:
	    return (!isalpha(ch));
    }
}

/*
 * readchar:
 *	Reads and returns a character, checking for gross input errors
 */

int
readchar(void)
{
    int ch;

    ch = md_readchar(stdscr);

#ifndef ROGUE_COLLECTION
    if (ch == 3)
    {
		quit(0);
        return(27);
    }
#endif

    MDK_LOG("input: '%c' (%x)\n", ch, ch);
    return(ch);
}

int
wreadchar(WINDOW *win)
{
    int ch;

    ch = md_readchar(win);

#ifndef ROGUE_COLLECTION
    if (ch == 3)
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
status(void)
{
    int oy, ox, temp;
    const char* fmt;
    int armor;
    static int hpwidth = 0;
    static int s_hungry = 0;
    static int s_lvl = 0;
    static int s_pur = -1;
    static int s_hp = 0;
    static int s_arm = 0;
    static int s_showac = 0;
    static int s_str = 0;
    static int s_exp = 0;
    static char *state_name[] =
    {
	"", "Hungry", "Weak", "Faint"
    };

    /*
     * If nothing has changed since the last status, don't
     * bother.
     */
    temp = (cur_armor != NULL ? cur_armor->o_arm : pstats.s_arm);
    if (s_hp == pstats.s_hpt && s_exp == pstats.s_exp && s_pur == purse
	&& s_arm == temp && s_str == pstats.s_str && s_lvl == level
	&& s_hungry == hungry_state && s_showac == showac
	&& !stat_msg
	)
	    return;

    s_arm = temp;

    getyx(stdscr, oy, ox);
    if (s_hp != max_hp)
    {
	temp = max_hp;
	s_hp = max_hp;
	for (hpwidth = 0; temp; hpwidth++)
	    temp /= 10;
    }

    /*
     * Save current status
     */
    s_lvl = level;
    s_pur = purse;
    s_hp = pstats.s_hpt;
    s_str = pstats.s_str;
    s_exp = pstats.s_exp;
    s_hungry = hungry_state;
    s_showac = showac;

    fmt = showac ? "Level: %d  Gold: %-5d  Hp: %*d(%*d)  Str: %2d(%d)  Ac: %-2d   Exp: %d/%d  %s" :
        "Level: %d  Gold: %-5d  Hp: %*d(%*d)  Str: %2d(%d)  Arm: %-2d  Exp: %d/%d  %s";
    armor = showac ? s_arm : ARMOR_DISPLAY(s_arm);

    if (stat_msg)
    {
        move(0, 0);
        msg(fmt,
        level, purse, hpwidth, pstats.s_hpt, hpwidth, max_hp, pstats.s_str,
        max_stats.s_str, armor, pstats.s_lvl, pstats.s_exp,
        state_name[hungry_state]);
    }
    else
    {
	move(STATLINE, 0);
        PC_GFX_COLOR(0x0e);
        printw(fmt,
	    level, purse, hpwidth, pstats.s_hpt, hpwidth, max_hp, pstats.s_str,
	    max_stats.s_str, armor, pstats.s_lvl, pstats.s_exp,
	    state_name[hungry_state]);
        PC_GFX_NOCOLOR(0x0e);
    }

    clrtoeol();
    move(oy, ox);
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
        while ((c = wreadchar(win)) != '\n' && c != '\r')
	    continue;
    else
        while (wreadchar(win) != ch)
	    continue;
}

/*
 * show_win:
 *	Function used to display a window and wait before returning
 */
void
show_win(const char *message)
{
    WINDOW *win;

    win = hw;
    wmove(win, 0, 0);
    waddstr(win, message);
    touchwin(win);
    wrefresh(win);
    wait_for(win, ' ');
    clearok(curscr, TRUE);
    touchwin(stdscr);
}
