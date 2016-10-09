/*
 * Various input/output functions
 *
 * @(#)io.c	3.10 (Berkeley) 6/15/81
 */

#include "curses.h"
#include <ctype.h>
#include <stdarg.h>
#include "rogue.h"

/*
 * msg:
 *	Display a message at the top of the screen.
 */

static char msgbuf[BUFSIZ];
static int newpos = 0;

/*VARARGS1*/
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
endmsg()
{
    strcpy(huh, msgbuf);
    if (mpos)
    {
	wmove(cw, 0, mpos);
	waddstr(cw, "--More--");
	draw(cw);
	wait_for(' ');
    }
    mvwaddstr(cw, 0, 0, msgbuf);
    wclrtoeol(cw);
    mpos = newpos;
    newpos = 0;
    draw(cw);
}

doadd(char *fmt, va_list ap)
{
    vsprintf(&msgbuf[newpos], fmt, ap);
    newpos = strlen(msgbuf);
}

/*
 * step_ok:
 *	returns true if it is ok to step on ch
 */

step_ok(ch)
{
    switch (ch)
    {
	case ' ':
	case '|':
	case '-':
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

readchar()
{
    return( wgetch(cw) );
}

/*
 * status:
 *	Display the important stats line.  Keep the cursor where it was.
 */

status()
{
    register int oy, ox, temp;
    register char *pb;
    static char buf[80];
    static int hpwidth = 0, s_hungry = -1;
    static int s_lvl = -1, s_pur, s_hp = -1, s_str, s_add, s_ac = 0;
    static long s_exp = 0;

    /*
     * If nothing has changed since the last status, don't
     * bother.
     */
    if (s_hp == pstats.s_hpt && s_exp == pstats.s_exp && s_pur == purse
	&& s_ac == (cur_armor != NULL ? cur_armor->o_ac : pstats.s_arm)
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
    sprintf(pb, "  Ac: %-2d  Exp: %d/%ld",
	cur_armor != NULL ? cur_armor->o_ac : pstats.s_arm, pstats.s_lvl,
	pstats.s_exp);
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
    wclrtoeol(cw);
    s_hungry = hungry_state;
    wmove(cw, oy, ox);
}

/*
 * wait_for
 *	Sit around until the guy types the right key
 */

wait_for(ch)
register char ch;
{
    register char c;

    if (ch == '\n')
        while ((c = readchar()) != '\n' && c != '\r')
	    continue;
    else
        while (readchar() != ch)
	    continue;
}

/*
 * show_win:
 *	function used to display a window and wait before returning
 */

show_win(scr, message)
register WINDOW *scr;
char *message;
{
    mvwaddstr(scr, 0, 0, message);
    touchwin(scr);
    wmove(scr, hero.y, hero.x);
    draw(scr);
    wait_for(' ');
    clearok(cw, TRUE);
    touchwin(cw);
}

flush_type()
{
	flushinp();
}
