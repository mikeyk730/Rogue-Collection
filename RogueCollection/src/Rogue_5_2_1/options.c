/*
 * This file has all the code for the option command.  I would rather
 * this command were not necessary, but it is the only way to keep the
 * wolves off of my back.
 *
 * @(#)options.c	4.12 (Berkeley) 3/2/82
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

#define	EQSTR(a, b, c)	(strncmp(a, b, c) == 0)

#define	NUM_OPTS	(sizeof optlist / sizeof (OPTION))

/*
 * description of an option and what to do with it
 */
struct optstruct {
    char	*o_name;	/* option name */
    char	*o_prompt;	/* prompt for interactive entry */
    int		*o_opt;		/* pointer to thing to set */
    int		(*o_putfunc)();	/* function to print value */
    int		(*o_getfunc)();	/* function to get value interactively */
};

typedef struct optstruct	OPTION;

int	put_bool(), get_bool(), put_str(), get_str();

OPTION	optlist[] = {
    {"terse",	 "Terse output: ",
		 (int *) &terse,	put_bool,	get_bool	},
    {"flush",	 "Flush typeahead during battle: ",
		(int *) &fight_flush,	put_bool,	get_bool	},
    {"jump",	 "Show position only at end of run: ",
		(int *) &jump,		put_bool,	get_bool	},
    {"step",	"Do inventories one line at a time: ",
		(int *) &slow_invent,	put_bool,	get_bool	},
    {"askme",	"Ask me about unidentified things: ",
		(int *) &askme,		put_bool,	get_bool	},
    {"passgo",	"Follow turnings in passageways: ",
		(int *) &passgo,	put_bool,	get_bool	},
    {"name",	 "Name: ",
		(int *) whoami,		put_str,	get_str		},
    {"fruit",	 "Fruit: ",
		(int *) fruit,		put_str,	get_str		},
    {"file",	 "Save file: ",
		(int *) file_name,	put_str,	get_str		}
};

/*
 * option:
 *	Print and then set options from the terminal
 */
option()
{
    register OPTION	*op;
    register int	retval;

    wclear(hw);
    /*
     * Display current values of options
     */
    for (op = optlist; op < &optlist[NUM_OPTS]; op++)
    {
	waddstr(hw, op->o_prompt);
	(*op->o_putfunc)(op->o_opt);
	waddch(hw, '\n');
    }
    /*
     * Set values
     */
    wmove(hw, 0, 0);
    for (op = optlist; op < &optlist[NUM_OPTS]; op++)
    {
	waddstr(hw, op->o_prompt);
	if ((retval = (*op->o_getfunc)(op->o_opt, hw)))
	    if (retval == QUIT)
		break;
	    else if (op > optlist) {	/* MINUS */
		wmove(hw, (op - optlist) - 1, 0);
		op -= 2;
	    }
	    else	/* trying to back up beyond the top */
	    {
		putchar('\007');
		wmove(hw, 0, 0);
		op--;
	    }
    }
    /*
     * Switch back to original screen
     */
    mvwaddstr(hw, LINES-1, 0, "--Press space to continue--");
    wrefresh(hw);
    wait_for(' ');
    clearok(curscr, TRUE);
    touchwin(stdscr);
    after = FALSE;
}

/*
 * put_bool
 *	Put out a boolean
 */
put_bool(b)
bool	*b;
{
    waddstr(hw, *b ? "True" : "False");
}

/*
 * put_str:
 *	Put out a string
 */
put_str(str)
char *str;
{
    waddstr(hw, str);
}

/*
 * get_bool:
 *	Allow changing a boolean option and print it out
 */
get_bool(bp, win)
bool *bp;
WINDOW *win;
{
    register int oy, ox;
    register bool op_bad;

    op_bad = TRUE;
    getyx(win, oy, ox);
    waddstr(win, *bp ? "True" : "False");
    while (op_bad)	
    {
	wmove(win, oy, ox);
	wrefresh(win);
	switch (readcharw(win))
	{
	    case 't':
	    case 'T':
		*bp = TRUE;
		op_bad = FALSE;
		break;
	    case 'f':
	    case 'F':
		*bp = FALSE;
		op_bad = FALSE;
		break;
	    case '\n':
	    case '\r':
		op_bad = FALSE;
		break;
	    case '\033':
	    case '\007':
		return QUIT;
	    case '-':
		return MINUS;
	    default:
		mvwaddstr(win, oy, ox + 10, "(T or F)");
	}
    }
    wmove(win, oy, ox);
    waddstr(win, *bp ? "True" : "False");
    waddch(win, '\n');
    return NORM;
}

/*
 * get_str:
 *	Set a string option
 */
#define MAXINP	50	/* max string to read from terminal or environment */

get_str(opt, win)
register char *opt;
WINDOW *win;
{
    register char *sp;
    register int c, oy, ox;
    char buf[MAXSTR];

    getyx(win, oy, ox);
    wrefresh(win);
    /*
     * loop reading in the string, and put it in a temporary buffer
     */
    for (sp = buf; (c = readcharw(win)) != '\n' && c != '\r' && c != '\033';
	wclrtoeol(win), wrefresh(win))
    {
	if (c == -1)
	    continue;
	else if (c == md_erasechar())	/* process erase character */
	{
	    if (sp > buf)
	    {
		register int i;

		sp--;
		for (i = strlen(unctrol(*sp)); i; i--)
		    waddch(win, '\b');
	    }
	    continue;
	}
	else if (c == md_killchar())     /* process kill character */
	{
	    sp = buf;
	    wmove(win, oy, ox);
	    continue;
	}
	else if (sp == buf)
	{
	    if (c == '-' && win != stdscr)
		break;
	    else if (c == '~')
	    {
		strcpy(buf, home);
		waddstr(win, home);
		sp += strlen(home);
		continue;
	    }
	}
	if (sp >= &buf[MAXINP] || !(isprint(c) || c == ' '))
	    putchar(CTRL('G'));
	else
	{
	    *sp++ = c;
	    waddstr(win, unctrol(c));
	}
    }
    *sp = '\0';
    if (sp > buf)	/* only change option if something has been typed */
	strucpy(opt, buf, strlen(buf));
    wmove(win, oy, ox);
    waddstr(win, opt);
    waddch(win, '\n');
    wrefresh(win);
    if (win == stdscr)
	mpos += sp - buf;
    if (c == '-')
	return MINUS;
    else if (c == '\033' || c == '\007')
	return QUIT;
    else
	return NORM;
}

#ifdef WIZARD
/*
 * get_num:
 *	Get a numeric option
 */
get_num(opt, win)
short *opt;
WINDOW *win;
{
    register int i;
    char buf[MAXSTR];

    if ((i = get_str(buf, win)) == NORM)
	*opt = atoi(buf);
    return i;
}
#endif

/*
 * parse_opts:
 *	Parse options from string, usually taken from the environment.
 *	The string is a series of comma seperated values, with booleans
 *	being stated as "name" (true) or "noname" (false), and strings
 *	being "name=....", with the string being defined up to a comma
 *	or the end of the entire option string.
 */
parse_opts(str)
register char *str;
{
    register char *sp;
    register OPTION *op;
    register int len;

    while (*str)
    {
	/*
	 * Get option name
	 */
	for (sp = str; isalpha(*sp); sp++)
	    continue;
	len = sp - str;
	/*
	 * Look it up and deal with it
	 */
	for (op = optlist; op < &optlist[NUM_OPTS]; op++)
	    if (EQSTR(str, op->o_name, len))
	    {
		if (op->o_putfunc == put_bool)	/* if option is a boolean */
		    *(bool *)op->o_opt = TRUE;
		else				/* string option */
		{
		    register char *start;
		    /*
		     * Skip to start of string value
		     */
		    for (str = sp + 1; *str == '='; str++)
			continue;
		    if (*str == '~')
		    {
			strcpy((char *) op->o_opt, home);
			start = (char *) op->o_opt + strlen(home);
			while (*++str == '/')
			    continue;
		    }
		    else
			start = (char *) op->o_opt;
		    /*
		     * Skip to end of string value
		     */
		    for (sp = str + 1; *sp && *sp != ','; sp++)
			continue;
		    strucpy(start, str, sp - str);
		}
		break;
	    }
	    /*
	     * check for "noname" for booleans
	     */
	    else if (op->o_putfunc == put_bool
	      && EQSTR(str, "no", 2) && EQSTR(str + 2, op->o_name, len - 2))
	    {
		*(bool *)op->o_opt = FALSE;
		break;
	    }

	/*
	 * skip to start of next option name
	 */
	while (*sp && !isalpha(*sp))
	    sp++;
	str = sp;
    }
}

/*
 * strucpy:
 *	Copy string using unctrol for things
 */
strucpy(s1, s2, len)
register char *s1, *s2;
register int len;
{
    if (len > MAXINP)
	len = MAXINP;
    while (len--)
    {
	if (isprint(*s2) || *s2 == ' ')
	    *s1++ = *s2;
	s2++;
    }
    *s1 = '\0';
}
