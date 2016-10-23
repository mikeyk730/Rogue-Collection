/*
 * Various installation dependent routines
 *
 * @(#)mach_dep.c	4.23 (Berkeley) 5/19/82
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981, 1982 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

/*
 * The various tuneable defines are:
 *
 *	SCOREFILE	Where/if the score file should live.
 *	MAXLOAD		What (if any) the maximum load average should be
 *			when people are playing.  If defined, then
 *	LOADAV		Should rogue define it's own routine to
 *			get the load average?
 *	NAMELIST	If so, where does the system namelist hide?
 *	MAXUSERS	What (if any) the maximum user count should be
 *			when people are playing.  If defined, then
 *	UCOUNT		Should rogue define it's own routine to
 *			count users?
 *	UTMP		If so, where does the user list hide?
 *	CHECKTIME	How often/if rogue should check during the game
 *			for high load average.
 */

/* Updated by Rogue Central @ coredumpcentral.org on 2014-05-27.
 * Copyright (C) 2012 Rogue Central @ coredumpcentral.org. All Rights Reserved.
 * See README.CDC, LICENSE.CDC, and CHANGES.CDC for more information.
 */

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <curses.h>
#include <time.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "rogue.h"


int num_checks;		/* times we've gone over in checkout() */

#ifdef SCOREFILE
#ifdef LOCKFILE
static char *lockfile = LOCKFILE;
#endif
#endif

/*
 * init_check:
 *	Check out too see if it is proper to play the game now
 */
init_check()
{
    if (too_much())
    {
	printf("Sorry, %s, but the system is too loaded now.\n", whoami);
	printf("Try again later.  Meanwhile, why not enjoy a%s %s?\n",
	    vowelstr(fruit), fruit);
	/*This code removed by RRPF
	 * if (author())
	    printf("However, since you're a good guy, it's up to you\n");
	else*/
	    exit(1);
    }
}

/*
 * open_score:
 *	Open up the score file for future use, and then
 *	setuid(getuid()) in case we are running setuid.
 */
open_score()
{
#ifdef SCOREFILE
    fd = open(SCOREFILE, O_RDWR | O_CREAT, 0666 );
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
    void  auto_save(), endit(), tstp();
#ifdef CHECKTIME
    int  checkout();
#endif

    /*
     * make sure that large terminals don't overflow the bounds
     * of the program
     */
    if (LINES > MAXLINES)
	LINES = MAXLINES;
    if (COLS > MAXCOLS)
	COLS = MAXCOLS;

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
    crmode();				/* Cbreak mode */
    noecho();				/* Echo off */
}

/*
 * start_score:
 *	Start the scoring sequence
 */
start_score()
{
#ifdef SIGALRM
    signal(SIGALRM, SIG_IGN);
#endif
}

/*
 * issymlink:
 *	See if the file has a symbolic link
 */
issymlink(sp)
char *sp;
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

/*
 * too_much:
 *	See if the system is being used too much for this game
 */
too_much()
{
#ifdef MAXLOAD
    double avec[3];
    
    if (md_getloadavg(avec) == 0)
        if (avec[2] > (MAXLOAD / 10.0))
	    return(1);
#endif
#ifdef MAXUSERS
    if (md_ucount() > MAXUSERS)
	return(1) ;
#endif
    return(0);
}

/*
 * author:
 *	See if a user is an author of the program
 */
/* Function removed by RRPF 
author()
{
#ifdef WIZARD
    if (wizard)
	return TRUE;
#endif
    switch (md_getuid())
    {
	case 0:
	    return TRUE;
	default:
	    return FALSE;
    }
}*/

/*
 * checkout:
 *	Check each CHECKTIME seconds to see if the load is too high
 */
void
checkout(int s)
{
    static char *msgs[] = {
	"The load is too high to be playing.  Please leave in %0.1f minutes",
	"Please save your game.  You have %0.1f minutes",
	"Last warning.  You have %0.1f minutes to leave",
    };
    int checktime = 0;

#ifdef SIGALRM
    signal(SIGALRM, checkout);
#endif

    if (too_much())
    {
	/*This code removed for RRPF
	 * if (author())
	{
	    num_checks = 1;
	    chmsg("The load is rather high, O exaulted one");
	}
	else if (num_checks++ == 3)*/
	    fatal("Sorry.  You took to long.  You are dead\n");

#ifdef CHECKTIME
	checktime = (CHECKTIME * 60) / num_checks;
#endif
#ifdef SIGALRM
	alarm(checktime);
#endif

	chmsg(msgs[num_checks - 1], ((double) checktime / 60.0));
    }
    else
    {
	if (num_checks)
	{
	    num_checks = 0;
	    chmsg("The load has dropped back down.  You have a reprieve");
	}
#ifdef CHECKTIME
#ifdef SIGALRM
	alarm(CHECKTIME * 60);
#endif
#endif
    }
}

/*
 * chmsg:
 *	checkout()'s version of msg.  If we are in the middle of a
 *	shell, do a printf instead of a msg to avoid the refresh.
 */
chmsg(fmt, arg)
char *fmt;
int arg;
{
    if (in_shell)
    {
	printf(fmt, arg);
	putchar('\n');
	fflush(stdout);
    }
    else
	msg(fmt, arg);
}

/*
 * lock_sc:
 *	lock the score file.  If it takes too long, ask the user if
 *	they care to wait.  Return TRUE if the lock is successful.
 */
lock_sc()
{
#ifdef SCOREFILE
#ifdef LOCKFILE
    register int cnt;
    static struct stat sbuf;

over:
    if (creat(lockfile, 0000) > 0)
     	return TRUE;
    for (cnt = 0; cnt < 5; cnt++)
    {
	md_sleep(1);
	if (creat(lockfile, 0000) > 0)
	    return TRUE;
    }
    if (stat(lockfile, &sbuf) < 0)
    {
	creat(lockfile, 0000);
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
		if (creat(lockfile, 0000) > 0)
		    return TRUE;
		if (stat(lockfile, &sbuf) < 0)
		{
		    creat(lockfile, 0000);
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
#endif
}

/*
 * unlock_sc:
 *	Unlock the score file
 */
unlock_sc()
{
#ifdef SCOREFILE
#ifdef LOCKFILE
    md_unlink(lockfile);
#endif
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
