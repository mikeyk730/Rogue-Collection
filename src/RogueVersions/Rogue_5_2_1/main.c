/*
 * #     #
 * #    #   #    #  #   ##  #    #   # 
 *		         #
 *
 * @(#)main.c	4.26 (Berkeley) 2/4/82
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981, 1982 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 *
 */

/* Updated by Rogue Central @ coredumpcentral.org on 2012-12-06.
 * Copyright (C) 2012 Rogue Central @ coredumpcentral.org. All Rights Reserved.
 * See README.CDC, LICENSE.CDC, and CHANGES.CDC for more information.
 */

#include <stdlib.h>
#include <curses.h>
#include <signal.h>
#include <limits.h>
#include <string.h>
#include "rogue.h"

/*
 * main:
 *	The main program, of course
 */
GAME_MAIN(argc, argv, envp)
char **argv;
char **envp;
{
    register char *env;
    int lowtime;

    md_init();

#ifndef DUMP
#ifdef SIGQUIT
    signal(SIGQUIT, exit);
#endif
    signal(SIGILL, exit);
#ifdef SIGTRAP
    signal(SIGTRAP, exit);
#endif
#ifdef SIGIOT
    signal(SIGIOT, exit);
#endif
#ifdef SIGEMT
    signal(SIGEMT, exit);
#endif
    signal(SIGFPE, exit);
#ifdef SIGBUS
    signal(SIGBUS, exit);
#endif
    signal(SIGSEGV, exit);
#ifdef SIGSYS
    signal(SIGSYS, exit);
#endif
#endif

#ifdef WIZARD
    /*
     * Check to see if he is a wizard
     */
    if (argc >= 2 && argv[1][0] == '\0')
	/* Line below commented out and replaced -RRPF */
	 /* if (strcmp(PASSWD, xcrypt(md_getpass("Wizard's password: "), "mT")) == 0) */
	if(passwd() == TRUE)
	{
	    wizard = TRUE;
	    player.t_flags |= SEEMONST;
	    argv++;
	    argc--;
	}
#endif

    /*
     * get home and options from environment
     */
    strncpy(home, md_gethomedir(), PATH_MAX);
    strcpy(file_name, home);
    /* Line below needs slash (added by RRPF) */
    strcat(file_name, "/rogue52.sav");

    if ((env = getenv("ROGUEOPTS")) != NULL)
	parse_opts(env);
    if (env == NULL || whoami[0] == '\0')
	strucpy(whoami, md_getusername(md_getuid()), strlen(md_getusername(md_getuid())));
    if (env == NULL || fruit[0] == '\0')
	strcpy(fruit, "slime-mold");

    /*
     * check for print-score option
     */
    open_score();
    if (argc == 2 && strcmp(argv[1], "-s") == 0)
    {
	noscore = TRUE;
	score(0, -1);
	exit(0);
    }
    init_check();			/* check for legal startup */
    if (argc == 2)
	if (!restore(argv[1], envp))	/* Note: restore will never return */
	{
		endwin();
	    exit(1);
	}
    lowtime = (int) time(NULL);

#ifdef WIZARD
    noscore = wizard;
#endif
    if (getenv("SEED") != NULL)
    {
	dnum = atoi(getenv("SEED"));
	noscore = TRUE;
    }
    else
	dnum = lowtime + getpid();
#ifdef WIZARD
    if (wizard)
	printf("Hello %s, welcome to dungeon #%d", whoami, dnum);
    else
#endif
	printf("Hello %s, just a moment while I dig the dungeon...\n\n",whoami);
    fflush(stdout);
    seed = dnum;

    init_player();			/* Set up initial player stats */
    init_things();			/* Set up probabilities of things */
    init_names();			/* Set up names of scrolls */
    init_colors();			/* Set up colors of potions */
    init_stones();			/* Set up stone settings of rings */
    init_materials();			/* Set up materials of wands */

    initscr();				/* Start up cursor package */
#ifdef USE_PC_GFX
    setup_colors();
#endif

    if (COLS < 70) 
    { 
	printf("\n\nSorry, but your terminal window has too few columns.\n");
        printf("Your terminal has %d columns, needs 70.\n",COLS); 
        endwin(); 
        exit(1); 
	} 

    if (LINES < 22) 
    { 
        printf("\n\nSorry, but your terminal window has too few lines.\n"); 
        printf("Your terminal has %d lines, needs 22.\n",LINES); 
        endwin(); 
        exit(1); 
    } 
 
    if ((whoami == NULL) || (*whoami == '\0') || (strcmp(whoami,"dosuser")==0))
    {
        echo();
        mvaddstr(23,2,"Rogue's Name? ");
        wgetnstr(stdscr,whoami,MAXSTR);
        noecho();
    }

    if ((whoami == NULL) || (*whoami == '\0'))
        strcpy(whoami,"Rodney");

    setup();

    /*
     * Set up windows
     */
    hw = newwin(LINES, COLS, 0, 0);
    keypad(stdscr,1);
    new_level();			/* Draw current level */
    /*
     * Start up daemons and fuses
     */
    daemon(doctor, 0, AFTER);
    fuse(swander, 0, WANDERTIME, AFTER);
    daemon(stomach, 0, AFTER);
    daemon(runners, 0, AFTER);
    playit();
}

/*
 * endit:
 *	Exit the program abnormally.
 */
void
endit(int a)
{
    fatal("Ok, if you want to exit that badly, I'll have to allow it\n");
}

/*
 * fatal:
 *	Exit the program, printing a message.
 */
fatal(s)
char *s;
{
    clear();
    move(LINES-2, 0);
    printw("%s", s);
    refresh();
    endwin();
    (void) exit(0);
}

/*
 * rnd:
 *	Pick a very random number.
 */
rnd(range)
register int range;
{
    return range == 0 ? 0 : abs((int) RN) % range;
}

/*
 * roll:
 *	Roll a number of dice
 */
roll(number, sides)
register int number, sides;
{
    register int dtotal = 0;

    while (number--)
	dtotal += rnd(sides)+1;
    return dtotal;
}

/*
 * tstp:
 *	Handle stop and start signals
 */
void
tstp(int a)
{
    register int y, x;
    register int oy, ox;

    getyx(curscr, oy, ox);
    mvcur(0, COLS - 1, LINES - 1, 0);
    endwin();
    clearok(curscr, TRUE);
    fflush(stdout);
#ifdef SIGTSTP
    signal(SIGTSTP, SIG_DFL);
    kill(0, SIGTSTP);
    signal(SIGTSTP, tstp);
#endif
    crmode();
    noecho();
    clearok(curscr, TRUE);
    wrefresh(curscr);
    getyx(curscr, y, x);
    mvcur(y, x, oy, ox);
    fflush(stdout);
    move(oy, ox);
    //curscr->_cury = oy;
    //curscr->_curx = ox;
}

/*
 * playit:
 *	The main loop of the program.  Loop until the game is over,
 *	refreshing things and looking at the proper times.
 */
playit()
{
    register char *opts;

    /*
     * set up defaults for slow terminals
     */

    if (baudrate() < 1200)
    {
        terse = TRUE;
        jump = TRUE;
    }

    /*
     * parse environment declaration of options
     */
    if ((opts = getenv("ROGUEOPTS")) != NULL)
	parse_opts(opts);


    oldpos = hero;
    oldrp = roomin(&hero);
    while (playing)
	command();			/* Command execution */
    endit(0);
}

/*
 * quit:
 *	Have player make certain, then exit.
 */
void
quit(int a)
{
    register int oy, ox;

    /*
     * Reset the signal in case we got here via an interrupt
     */

    if (signal(SIGINT, quit) != quit)
	mpos = 0;
    getyx(curscr, oy, ox);
    msg("really quit?");
    if (readchar() == 'y')
    {
	signal(SIGINT, leave);
	clear();
	mvprintw(LINES - 2, 0, "You quit with %d gold pieces", purse);
	move(LINES - 1, 0);
	refresh();
	score(purse, 1);
	exit(0);
    }
    else
    {
	move(0, 0);
	clrtoeol();
	status();
	move(oy, ox);
	refresh();
	mpos = 0;
	count = 0;
    }
}

/*
 * leave:
 *	Leave quickly, but curteously
 */
void
leave(int sig)
{
/*
    if (!_endwin)
    {*/
	mvcur(0, COLS - 1, LINES - 1, 0);
	endwin();
    /* } */
    putchar('\n');
    exit(0);
}

/*
 * shell:
 *	Let him escape for a while
 */
shell()
{
    /*
     * Set the terminal back to original mode
     */
    move(LINES-1, 0);
    refresh();
    endwin();
    putchar('\n');
    putchar('\n');
    in_shell = TRUE;
    after = FALSE;

    md_shellescape();

    noecho();
    crmode();
    in_shell = FALSE;
    clearok(stdscr, TRUE);
    touchwin(stdscr);
}
