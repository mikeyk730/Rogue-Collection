/*
 * Various installation dependent routines
 *
 * @(#)mach_dep.c	4.37 (Berkeley) 05/23/83
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

/*
 * The various tuneable defines are:
 *
 *	SCOREFILE	Where/if the score file should live.
 *	ALLSCORES	Score file is top ten scores, not top ten
 *			players.  This is only useful when only a few
 *			people will be playing; otherwise the score file
 *			gets hogged by just a few people.
 *	NUMSCORES	Number of scores in the score file (default 10).
 *	NUMNAME		String version of NUMSCORES (first character
 *			should be capitalized) (default "Ten").
 *	MAXLOAD		What (if any) the maximum load average should be
 *			when people are playing.  Since it is divided
 *			by 10, to specify a load limit of 4.0, MAXLOAD
 *			should be "40".	 If defined, then
 *	LOADAV		Should it use it's own routine to get
 *			the load average?
 *	NAMELIST	If so, where does the system namelist
 *			hide?
 *	MAXUSERS	What (if any) the maximum user count should be
 *	        	when people are playing.  If defined, then
 *	UCOUNT		Should it use it's own routine to count
 *			users?
 *	UTMP		If so, where does the user list hide?
 *	CHECKTIME	How often/if it should check during the game
 *			for high load average.
 */

#include <curses.h>
#include "extern.h"
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>

#include <fcntl.h>

static char scorefile[PATH_MAX] = "rogue54.scr";
static char lockfile[PATH_MAX] = "rogue54.lck";

# ifndef NUMSCORES
#	define	NUMSCORES	10
#	define	NUMNAME		"Ten"
# endif

unsigned int numscores = NUMSCORES;
char *Numname = NUMNAME;

# ifdef ALLSCORES
bool allscore = TRUE;
# else  /* ALLSCORES */
bool allscore = FALSE;
# endif /* ALLSCORES */

#ifdef CHECKTIME
static int num_checks;		/* times we've gone over in checkout() */
#endif /* CHECKTIME */

/*
 * init_check:
 *	Check out too see if it is proper to play the game now
 */

init_check()
{
#if defined(MAXLOAD) || defined(MAXUSERS)
    if (too_much())
    {
	printf("Sorry, %s, but the system is too loaded now.\n", whoami);
	printf("Try again later.  Meanwhile, why not enjoy a%s %s?\n",
	    vowelstr(fruit), fruit);
	if (author())
	    printf("However, since you're a good guy, it's up to you\n");
	else
	    exit(1);
    }
#endif
}

/*
 * open_score:
 *	Open up the score file for future use, and then
 *	setuid(getuid()) in case we are running setuid.
 */

open_score()
{
    char *homedir = md_getroguedir();

    if (homedir == NULL)
        homedir = "";

#ifdef SCOREFILE
    strcpy(scorefile, homedir);
    if (*scorefile)
        strcat(scorefile,"/");
    strcat(scorefile, "rogue54.scr");
    strcpy(lockfile, homedir);
    if (*lockfile)
        strcat(lockfile, "/");
    strcat(lockfile, "rogue54.lck");
    fd = open(scorefile, O_RDWR | O_CREAT, 0666);
#else
    fd = -1;
#endif
    md_normaluser();
}

/*
 * setup:
 *	Get starting setup for all games
 */

setup()
{
#ifdef CHECKTIME
    int  checkout();
#endif

#ifdef SIGHUP
    signal(SIGHUP, auto_save);
#endif
#ifndef DUMP
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
    signal(SIGTERM, auto_save);
#endif

    signal(SIGINT, quit);
#ifndef DUMP
#ifdef SIGQUIT
    signal(SIGQUIT, endit);
#endif
#endif
#ifdef CHECKTIME
    signal(SIGALRM, checkout);
    alarm(CHECKTIME * 60);
    num_checks = 0;
#endif
    raw();				/* Raw mode */
    noecho();				/* Echo off */
    keypad(stdscr,1);
#ifdef TIOCGLTC
    getltchars();			/* get the local tty chars */
#endif
}

/*
 * getltchars:
 *	Get the local tty chars for later use
 */

getltchars()
{
#ifdef TIOCGLTC
    ioctl(1, TIOCGLTC, &ltc);
    got_ltc = TRUE;
    orig_dsusp = ltc.t_dsuspc;
    ltc.t_dsuspc = ltc.t_suspc;
    ioctl(1, TIOCSLTC, &ltc);
#endif
}

/* 
 * resetltchars: 
 *      Reset the local tty chars to original values. 
 */ 
void 
resetltchars(void) 
{ 
#ifdef TIOCGLTC 
    if (got_ltc) { 
        ltc.t_dsuspc = orig_dsusp; 
        ioctl(1, TIOCSLTC, &ltc); 
    } 
#endif 
} 
  
/* 
 * playltchars: 
 *      Set local tty chars to the values we use when playing. 
 */ 
void 
playltchars(void) 
{ 
#ifdef TIOCGLTC 
    if (got_ltc) { 
        ltc.t_dsuspc = ltc.t_suspc; 
        ioctl(1, TIOCSLTC, &ltc); 
    } 
#endif 
} 

/*
 * start_score:
 *	Start the scoring sequence
 */

start_score()
{
#ifdef CHECKTIME
    signal(SIGALRM, SIG_IGN);
#endif
}

/*
 * is_symlink:
 *	See if the file has a symbolic link
 */
bool
is_symlink(char *sp)
{
#ifdef S_IFLNK
    struct stat sbuf2;

    if (lstat(sp, &sbuf2) < 0)
	return FALSE;
    else
	return ((sbuf2.st_mode & S_IFMT) != S_IFREG);
#else
    return FALSE;
#endif
}

#if defined(MAXLOAD) || defined(MAXUSERS)
/*
 * too_much:
 *	See if the system is being used too much for this game
 */
bool
too_much()
{
#ifdef MAXLOAD
    double avec[3];
#else
    int cnt;
#endif

#ifdef MAXLOAD
    loadav(avec);
    if (avec[1] > (MAXLOAD / 10.0))
	return TRUE;
#endif
#ifdef MAXUSERS
    if (ucount() > MAXUSERS)
	return TRUE;
#endif
    return FALSE;
}

/*
 * author:
 *	See if a user is an author of the program
 */
bool
author()
{
#ifdef MASTER
    if (wizard)
	return TRUE;
#endif
    switch (md_getuid())
    {
	case -1:
	    return TRUE;
	default:
	    return FALSE;
    }
}
#endif

#ifdef CHECKTIME
/*
 * checkout:
 *	Check each CHECKTIME seconds to see if the load is too high
 */

checkout(int sig)
{
    static char *msgs[] = {
	"The load is too high to be playing.  Please leave in %0.1f minutes",
	"Please save your game.  You have %0.1f minutes",
	"Last warning.  You have %0.1f minutes to leave",
    };
    int checktime;

    signal(SIGALRM, checkout);
    if (too_much())
    {
	if (author())
	{
	    num_checks = 1;
	    chmsg("The load is rather high, O exaulted one");
	}
	else if (num_checks++ == 3)
	    fatal("Sorry.  You took too long.  You are dead\n");
	checktime = (CHECKTIME * 60) / num_checks;
	alarm(checktime);
	chmsg(msgs[num_checks - 1], ((double) checktime / 60.0));
    }
    else
    {
	if (num_checks)
	{
	    num_checks = 0;
	    chmsg("The load has dropped back down.  You have a reprieve");
	}
	alarm(CHECKTIME * 60);
    }
}

/*
 * chmsg:
 *	checkout()'s version of msg.  If we are in the middle of a
 *	shell, do a printf instead of a msg to a the refresh.
 */
/* VARARGS1 */

chmsg(char *fmt, int arg)
{
    if (!in_shell)
	msg(fmt, arg);
    else
    {
	printf(fmt, arg);
	putchar('\n');
	fflush(stdout);
    }
}
#endif

#ifdef LOADAV
/*
 * loadav:
 *	Looking up load average in core (for system where the loadav()
 *	system call isn't defined
 */

#include <nlist.h>

struct nlist avenrun = {
    "_avenrun"
};


loadav(double *avg)
{
    int kmem;

    if ((kmem = open("/dev/kmem", 0)) < 0)
	goto bad;
    nlist(NAMELIST, &avenrun);
    if (avenrun.n_type == 0)
    {
	close(kmem);
bad:
	avg[0] = 0.0;
	avg[1] = 0.0;
	avg[2] = 0.0;
	return;
    }

    lseek(kmem, (long) avenrun.n_value, 0);
    read(kmem, (char *) avg, 3 * sizeof (double));
    close(kmem);
}
#endif

#ifdef UCOUNT
/*
 * ucount:
 *	count number of users on the system
 */
#include <utmp.h>

struct utmp buf;

int
ucount()
{
    struct utmp *up;
    FILE *utmp;
    int count;

    if ((utmp = fopen(UTMP, "r")) == NULL)
	return 0;

    up = &buf;
    count = 0;

    while (fread(up, 1, sizeof (*up), utmp) > 0)
	if (buf.ut_name[0] != '\0')
	    count++;
    fclose(utmp);
    return count;
}
#endif

/*
 * lock_sc:
 *	lock the score file.  If it takes too long, ask the user if
 *	they care to wait.  Return TRUE if the lock is successful.
 */
static int lfd = -1;
bool
lock_sc()
{
#ifdef SCOREFILE
    int cnt;
    static struct stat sbuf;

over:
    if ((lfd=md_creat(lockfile, 0000)) >= 0)
	return TRUE;
    for (cnt = 0; cnt < 5; cnt++)
    {
	md_sleep(1);
	if ((lfd=md_creat(lockfile, 0000)) >= 0)
	    return TRUE;
    }
    if (stat(lockfile, &sbuf) < 0)
    {
	lfd=md_creat(lockfile, 0000);
	return TRUE;
    }
    if (time(NULL) - sbuf.st_mtime > 10)
    {
	if (md_unlink(lockfile) < 0)
	    return FALSE;
	goto over;
    }
    else
    {
	printf("The score file is very busy.  Do you want to wait longer\n");
	printf("for it to become free so your score can get posted?\n");
	printf("If so, type \"y\"\n");
	fgets(prbuf, MAXSTR, stdin);
	if (prbuf[0] == 'y')
	    for (;;)
	    {
		if ((lfd=md_creat(lockfile, 0000)) >= 0)
		    return TRUE;
		if (stat(lockfile, &sbuf) < 0)
		{
		    lfd=md_creat(lockfile, 0000);
		    return TRUE;
		}
		if (time(NULL) - sbuf.st_mtime > 10)
		{
		    if (md_unlink(lockfile) < 0)
			return FALSE;
		}
		md_sleep(1);
	    }
	else
	    return FALSE;
    }
#endif
}

/*
 * unlock_sc:
 *	Unlock the score file
 */

unlock_sc()
{
#ifdef SCOREFILE
    if (lfd != -1)
        close(lfd);
    lfd = -1;
    md_unlink(lockfile);
#endif
}

/*
 * flush_type:
 *	Flush typeahead for traps, etc.
 */

flush_type()
{
    flushinp();
}
