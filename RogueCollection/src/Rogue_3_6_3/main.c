/*
 * @(#)main.c	3.27 (Berkeley) 6/15/81
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

/* Updated by Rogue Central @ coredumpcentral.org on 2012-12-06.
 * Copyright (C) 2012 Rogue Central @ coredumpcentral.org. All Rights Reserved.
 * See README.CDC, LICENSE.CDC, and CHANGES.CDC for more information.
 */

#include "curses.h"
#include <time.h>
#include <signal.h>
#include <limits.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "machdep.h"
#include "rogue.h"

int num_checks = 0;			/* times we've gone over in checkout() */
WINDOW *cw;                              /* Window that the player sees */
WINDOW *hw;                              /* Used for the help command */
WINDOW *mw;                              /* Used to store mosnters */
FILE   *scoreboard = NULL;

main(argc, argv, envp)
char **argv;
char **envp;
{
    char *env;
    struct linked_list *item;
    struct object *obj;
    int lowtime;
    time_t now;

    md_init(MD_STRIP_CTRL_KEYPAD);

    open_score();

    /* 
     * Drop setuid/setgid after opening the scoreboard file. 
     */

    md_normaluser();

    /*
     * check for print-score option
     */
    if (argc == 2 && strcmp(argv[1], "-s") == 0)
    {
	waswizard = TRUE;
	score(0, -1, 0);
	exit(0);
    }
    /*
     * Check to see if he is a wizard
     */
    if (argc >= 2 && argv[1][0] == '\0')
	/*if (strcmp(PASSWD, crypt(md_getpass("Wizard's password: "), "mT")) == 0) Replaced by RRPF with:*/
	if(passwd() == TRUE)
	{
	    wizard = TRUE;
	    argv++;
	    argc--;
	}

    /*
     * get home and options from environment
     */
    strcpy(home, md_gethomedir());
    
    if (strlen(home) > PATH_MAX - strlen("rogue36.sav") - 1)
        *home = 0;
    
    strcpy(file_name, home);
    strcat(file_name, "rogue36.sav");

    if ((env = getenv("ROGUEOPTS")) != NULL)
	parse_opts(env);
    if (env == NULL || whoami[0] == '\0')
	strucpy(whoami, md_getusername(), strlen(md_getusername()));
    if (env == NULL || fruit[0] == '\0')
	strcpy(fruit, "slime-mold");

    if (too_much() && !wizard && !author())
    {
	printf("Sorry, %s, but the system is too loaded now.\n", whoami);
	printf("Try again later.  Meanwhile, why not enjoy a%s %s?\n",
	    vowelstr(fruit), fruit);
	exit(1);
    }

    if (argc == 2)
	if (!restore(argv[1], envp)) /* Note: restore will never return */
	    exit(1);

    time(&now);
    lowtime = (int) now;

    env = getenv("SEED");

    if (env)
        seed = atoi(env);
    else
        seed = 0;

    if (seed > 0)
    {
        waswizard = 1; /* don't save scores if SEED specified */
        dnum = seed;
    }
    else
        dnum = lowtime + md_getpid();

    if (wizard || env)
	printf("Hello %s, welcome to dungeon #%d", whoami, dnum);
    else
	printf("Hello %s, just a moment while I dig the dungeon...", whoami);

    fflush(stdout);
    seed = dnum;
    init_player();			/* Roll up the rogue */
    init_things();			/* Set up probabilities of things */
    init_names();			/* Set up names of scrolls */
    init_colors();			/* Set up colors of potions */
    init_stones();			/* Set up stone settings of rings */
    init_materials();			/* Set up materials of wands */
    initscr();				/* Start up cursor package */
#ifdef USE_PC_GFX
#define init_color_pair(attr) init_pair((attr), (attr) & 0x0f, ((attr) & 0xf0) >> 4)
    start_color();
    init_color(0x00, 0, 0, 0); //black
    init_color(0x01, 0, 0, 667); //blue
    init_color(0x02, 0, 667, 0); //green
    init_color(0x03, 0, 667, 667); //cyan
    init_color(0x04, 667, 0, 0); //red
    init_color(0x05, 667, 0, 667); //magenta
    init_color(0x06, 667, 333, 0); //brown
    init_color(0x07, 667, 667, 667); //grey
    init_color(0x08, 250, 250, 250); //d_grey
    init_color(0x09, 333, 333, 1000); //l_blue
    init_color(0x0a, 333, 1000, 333); //l_green
    init_color(0x0b, 100, 1000, 1000); //l_cyan
    init_color(0x0c, 1000, 333, 333); //l_red
    init_color(0x0d, 1000, 100, 1000); //l_magenta
    init_color(0x0e, 1000, 1000, 100); //yellow
    init_color(0x0f, 1000, 1000, 1000); //white
    init_color_pair(0x01);
    init_color_pair(0x02);
    init_color_pair(0x04);
    init_color_pair(0x05);
    init_color_pair(0x06);
    init_color_pair(0x07);
    init_color_pair(0x09);
    init_color_pair(0x0A);
    init_color_pair(0x0D);
    init_color_pair(0x0E);
    init_color_pair(0x70);
    init_color_pair(0x71);
    init_color_pair(0x72);
    init_color_pair(0x74);
    init_color_pair(0x78);
    init_color_pair(0x7E);
    init_color_pair(0xA0);
    curs_set(0);
#endif


    if (COLS < 70)
    {
	endwin();
	printf("\n\nSorry, %s, but your terminal window has too few columns.\n", whoami);
	printf("Your terminal has %d columns, needs 70.\n",COLS);
	exit(1);
    }
    if (LINES < 22)
    {
	endwin();
	printf("\n\nSorry, %s, but your terminal window has too few lines.\n", whoami);
	printf("Your terminal has %d lines, needs 22.\n",LINES);
	exit(1);
    }
    

    setup();
    /*
     * Set up windows
     */
    cw = newwin(LINES, COLS, 0, 0);
    mw = newwin(LINES, COLS, 0, 0);
    hw = newwin(LINES, COLS, 0, 0);
    keypad(cw,1);
    waswizard = wizard;
    new_level();			/* Draw current level */
    /*
     * Start up daemons and fuses
     */
    start_daemon(doctor, 0, AFTER);
    fuse(swander, 0, WANDERTIME, AFTER);
    start_daemon(stomach, 0, AFTER);
    start_daemon(runners, 0, AFTER);
    /*
     * Give the rogue his weaponry.  First a mace.
     */
    item = new_item(sizeof *obj);
    obj = (struct object *) ldata(item);
    obj->o_type = WEAPON;
    obj->o_which = MACE;
    init_weapon(obj, MACE);
    obj->o_hplus = 1;
    obj->o_dplus = 1;
    obj->o_flags |= ISKNOW;
    add_pack(item, TRUE);
    cur_weapon = obj;
    /*
     * Now a +1 bow
     */
    item = new_item(sizeof *obj);
    obj = (struct object *) ldata(item);
    obj->o_type = WEAPON;
    obj->o_which = BOW;
    init_weapon(obj, BOW);
    obj->o_hplus = 1;
    obj->o_dplus = 0;
    obj->o_flags |= ISKNOW;
    add_pack(item, TRUE);
    /*
     * Now some arrows
     */
    item = new_item(sizeof *obj);
    obj = (struct object *) ldata(item);
    obj->o_type = WEAPON;
    obj->o_which = ARROW;
    init_weapon(obj, ARROW);
    obj->o_count = 25+rnd(15);
    obj->o_hplus = obj->o_dplus = 0;
    obj->o_flags |= ISKNOW;
    add_pack(item, TRUE);
    /*
     * And his suit of armor
     */
    item = new_item(sizeof *obj);
    obj = (struct object *) ldata(item);
    obj->o_type = ARMOR;
    obj->o_which = RING_MAIL;
    obj->o_ac = a_class[RING_MAIL] - 1;
    obj->o_flags |= ISKNOW;
    cur_armor = obj;
    add_pack(item, TRUE);
    /*
     * Give him some food too
     */
    item = new_item(sizeof *obj);
    obj = (struct object *) ldata(item);
    obj->o_type = FOOD;
    obj->o_count = 1;
    obj->o_which = 0;
    add_pack(item, TRUE);
    playit();
}

/*
 * endit:
 *	Exit the program abnormally.
 */

void
endit(int p)
{
    fatal("Ok, if you want to exit that badly, I'll have to allow it\n");
}

/*
 * fatal:
 *	Exit the program, printing a message.
 */

void
fatal(char *s)
{
    clear();
    move(LINES-2, 0);
    printw("%s", s);
    draw(stdscr);
    endwin();
    exit(0);
}

/*
 * rnd:
 *	Pick a very random number.
 */

int
rnd(int range)
{
    return range == 0 ? 0 : abs(RN) % range;
}

/*
 * roll:
 *	roll a number of dice
 */

int
roll(int number, int sides)
{
    int dtotal = 0;

    while(number--)
	dtotal += rnd(sides)+1;
    return dtotal;
}
/*
 * handle stop and start signals
 */

void
tstp(int p)
{
#ifdef SIGTSTP
    signal(SIGTSTP, SIG_IGN);
#endif
    mvcur(0, COLS - 1, LINES - 1, 0);
    endwin();
    fflush(stdout);
#ifdef SIGTSTP
    signal(SIGTSTP, SIG_DFL);
    kill(0, SIGTSTP);
    signal(SIGTSTP, tstp);
#endif
    crmode();
    noecho();
    clearok(curscr, TRUE);
    touchwin(cw);
    draw(cw);
    flush_type();	/* flush input */
}

void
setup()
{
#ifdef SIGHUP
    signal(SIGHUP, auto_save);
#endif
    signal(SIGILL, auto_save);
#ifdef SIGTRAP
    signal(SIGTRAP, auto_save);
#endif
#ifdef SIGIOT
    signal(SIGIOT, auto_save);
#endif
#ifdef SIGEMT
    signal(SIGEMT, auto_save);
#endif
    signal(SIGFPE, auto_save);
#ifdef SIGBUS
    signal(SIGBUS, auto_save);
#endif
    signal(SIGSEGV, auto_save);
#ifdef SIGSYS
    signal(SIGSYS, auto_save);
#endif
#ifdef SIGPIPE
    signal(SIGPIPE, auto_save);
#endif
    signal(SIGTERM, auto_save);
    signal(SIGINT, quit);
#ifdef SIGQUIT
    signal(SIGQUIT, endit);
#endif
#ifdef SIGTSTP
    signal(SIGTSTP, tstp);
#endif

    if (!author())
    {
#ifdef SIGALRM
	signal(SIGALRM, checkout);
	alarm(CHECKTIME * 60);
#endif
	num_checks = 0;
    }

    crmode();				/* Cbreak mode */
    noecho();				/* Echo off */
}

/*
 * playit:
 *	The main loop of the program.  Loop until the game is over,
 * refreshing things and looking at the proper times.
 */

void
playit()
{
    char *opts;

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
    endit(-1);
}

/*
 * see if the system is being used too much for this game
 */
int
too_much()
{
    double avec[3];

    if (md_loadav(avec) == 0)
    	return (avec[2] > (MAXLOAD / 10.0));
    else
        return (md_ucount() > MAXUSERS);
}

/*
 * see if a user is an author of the program
 */
int
author()
{
    switch (md_getuid())
    {
	case AUTHORUID:
	    return TRUE;
	default:
	    return FALSE;
    }
}

void
checkout(int p)
{
    static char *msgs[] = {
	"The load is too high to be playing.  Please leave in %d minutes",
	"Please save your game.  You have %d minutes",
	"Last warning.  You have %d minutes to leave",
    };
    int checktime;
#ifdef SIGALRM
    signal(SIGALRM, checkout);
#endif
    if (too_much())
    {
	if (num_checks >= 3)
	    fatal("Sorry.  You took to long.  You are dead\n");
	checktime = CHECKTIME / (num_checks + 1);
	if (num_checks < 3)
		chmsg(msgs[num_checks++], checktime);
#ifdef SIGALRM
	alarm(checktime * 60);
#endif
    }
    else
    {
	if (num_checks)
	{
	    chmsg("The load has dropped back down.  You have a reprieve.");
	    num_checks = 0;
	}
#ifdef SIGALRM
	alarm(CHECKTIME * 60);
#endif
    }
}

/*
 * checkout()'s version of msg.  If we are in the middle of a shell, do a
 * printf instead of a msg to avoid the refresh.
 */
void
chmsg(char *fmt, ...)
{
    va_list args;

    if (in_shell)
    {
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
	putchar('\n');
	fflush(stdout);
    }
    else
    {
        va_start(args, fmt);
        doadd(fmt, args);
        va_end(args);
	endmsg();
    }
}
