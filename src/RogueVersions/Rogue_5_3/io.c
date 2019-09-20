/*
 * Various input/output functions
 *
 * @(#)io.c	4.16 (NMT from Berkeley 5.2) 8/25/83
 */

#include <curses.h>
#include <ctype.h>
#include "rogue.h"

/*
 * msg:
 *	Display a message at the top of the screen.
 */
static char msgbuf[BUFSIZ];
static int newpos = 0;

/* VARARGS1 */
msg(const char* fmt, ...)
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
	return;
    }
    /*
     * otherwise add to the message and flush it out
     */

    va_start(args, fmt);
    doadd(fmt, args);
    va_end(args);
    endmsg();
}

/*
 * addmsg:
 *	Add things to the current message
 */
/* VARARGS1 */
addmsg(const char* fmt, ...)
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
endmsg()
{
    if (save_msg)
	strcpy(huh, msgbuf);
    if (mpos)
    {
	look(FALSE);
	move(0, mpos);
    PC_GFX_COLOR(0x70);
	addstr(MORE_MSG);
    PC_GFX_NOCOLOR(0x70);
	refresh();
	wait_for(' ');
    }
    /*
     * All messages should start with uppercase, except ones that
     * start with a pack addressing character
     */
    if (islower(msgbuf[0]) && msgbuf[1] != ')')
	msgbuf[0] = toupper(msgbuf[0]);
    MDK_LOG("msg: %s\n", msgbuf);
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
step_ok(unsigned int ch)
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
readchar()
{
    register int cnt;
    char c;

    c = getch();
    MDK_LOG("getch: '%c' (%x)\n", c, c);
    //cnt = 0;
    //while (read(0, &c, 1) <= 0)
	//if (cnt++ > 100)	/* if we are getting infinite EOFs */
	//    auto_save();	/* save the game */
    return c;
}

/*
 * unctrl:
 *	Print a readable version of a certain character
 */
//char *
//unctrl(ch)
//char ch;
//{
//    extern char *_unctrl[];		/* Defined in curses library */

//    return _unctrl[ch&0177];
//}

/*
 * status:
 *	Display the important stats line.  Keep the cursor where it was.
 */
status()
{
    int armor;
    register int oy, ox, temp;
    static int hpwidth = 0, s_hungry;
    static int s_lvl, s_pur = -1, s_hp, s_ac = 0;
    static int s_showac = 0;
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
	&& s_str == pstats.s_str && s_lvl == level && s_hungry == hungry_state && s_showac == showac)
	    return;

    getyx(stdscr, oy, ox);
    if (s_hp != max_hp)
    {
	temp = s_hp = max_hp;
	for (hpwidth = 0; temp; hpwidth++)
	    temp /= 10;
    }
    move(LINES - 1, 0);
    PC_GFX_COLOR(0x0e);
    const char* format = showac ? "Level: %d  Gold: %-5d  Hp: %*d(%*d)  Str: %2d(%d)  Ac: %-2d  Exp: %d/%ld  %s" :
        "Level: %d  Gold: %-5d  Hp: %*d(%*d)  Str: %2d(%d)  Arm: %-2d  Exp: %d/%ld  %s";
    armor = cur_armor != NULL ? cur_armor->o_ac : pstats.s_arm;
    if (!showac)
        armor = ARMOR_DISPLAY(armor);
    printw(format,
	    level, purse, hpwidth, pstats.s_hpt, hpwidth, max_hp, pstats.s_str,
	    max_stats.s_str,
	    armor, pstats.s_lvl,
	    pstats.s_exp, state_name[hungry_state]);

    clrtoeol();
    PC_GFX_NOCOLOR(0x0e);
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
    s_showac = showac;
    move(oy, ox);
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
    wait_for(' ');
    clearok(curscr, TRUE);
#ifdef	r_attron
    touchwin(stdscr);
#endif	r_attron
}
