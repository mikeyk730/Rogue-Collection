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

#include <limits.h>
#include <curses.h>
#include "rogue.h"
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef SCOREFILE
static char lockfile[PATH_MAX]  = "rogue52.lck";
static char scorefile[PATH_MAX] = "rogue52.scr";
#endif

#ifdef CHECKTIME
static int num_checks;		/* times we've gone over in checkout() */
#endif 

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

int
directory_exists(char *dirname)
{
    struct stat sb;

    if (stat(dirname, &sb) == 0) /* path exists */
        return (S_ISDIR (sb.st_mode));

    return(0);
}


char *
roguehome()
{
    static char path[1024];
    char *end,*home;

    if ( (home = getenv("ROGUEHOME")) != NULL)
    {
        if (*home)
        {
            strncpy(path, home, PATH_MAX - 20);

            end = &path[strlen(path)-1];


            while( (end >= path) && ((*end == '/') || (*end == '\\')))
                *end-- = '\0';
        
            if (directory_exists(path))
                return(path);
        }
    }


    if (directory_exists("/var/games/roguelike"))
        return("/var/games/roguelike");
    if (directory_exists("/var/lib/roguelike"))
        return("/var/lib/roguelike");
    if (directory_exists("/var/roguelike"))
        return("/var/roguelike");
    if (directory_exists("/usr/games/lib"))
        return("/usr/games/lib");
    if (directory_exists("/games/roguelik"))
        return("/games/roguelik");

    return(NULL);
}


/*
 * open_score:
 *	Open up the score file for future use, and then
 *	setuid(getuid()) in case we are running setuid.
 */
open_score()
{
    char *homedir = roguehome();

    if (homedir == NULL)
        homedir = "";

    strcpy(scorefile, homedir);
    if (*scorefile)
	strcat(scorefile,"/");
    strcat(scorefile, "rogue52.scr");
    strcpy(lockfile, homedir);
    if (*lockfile)
        strcat(lockfile,"/");
    strcat(lockfile, "rogue52.lck");
#ifdef SCOREFILE
    fd = open(scorefile, O_RDWR | O_CREAT, 0666 );
#else
    fd = -1;
#endif
   
    setuid(getuid());
    setgid(getgid());
}

/*
 * setup:
 *	Get starting setup for all games
 */
setup()
{
    void  auto_save(), quit(), endit(), tstp();
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

    signal(SIGHUP, auto_save);
#ifndef DUMP
    signal(SIGILL, auto_save);
    signal(SIGTRAP, auto_save);
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
    signal(SIGQUIT, endit);
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
#ifdef CHECKTIME
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

#if defined(MAXLOAD) || defined(MAXUSERS)
/*
 * too_much:
 *	See if the system is being used too much for this game
 */
too_much()
{
#ifdef MAXLOAD
    double avec[3];
#else
    register int cnt;
#endif

#ifdef MAXLOAD
    loadav(avec);
    return (avec[1] > (MAXLOAD / 10.0));
#else
    return (ucount() > MAXUSERS);
#endif
}

/*
 * author:
 *	See if a user is an author of the program
 */
author()
{
#ifdef WIZARD
    if (wizard)
	return TRUE;
#endif
    switch (getuid())
    {
	case 162:
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
checkout()
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
	    fatal("Sorry.  You took to long.  You are dead\n");
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
#endif

#ifdef LOADAV

#include <nlist.h>

struct nlist avenrun = {
    "_avenrun"
};

/*
 * loadav:
 *	Looking up load average in core (for system where the loadav()
 *	system call isn't defined
 */
loadav(avg)
register double *avg;
{
    register int kmem;

    if ((kmem = open("/dev/kmem", 0)) < 0)
	goto bad;
    nlist(NAMELIST, &avenrun);
    if (avenrun.n_type == 0)
    {
bad:
	avg[0] = avg[1] = avg[2] = 0.0;
	return;
    }

    lseek(kmem, (long) avenrun.n_value, 0);
    read(kmem, (char *) avg, 3 * sizeof (double));
}
#endif

#ifdef UCOUNT
/*
 * ucount:
 *	Count number of users on the system
 */
#include <utmp.h>

struct utmp buf;

ucount()
{
    register struct utmp *up;
    register FILE *utmp;
    register int count;

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
lock_sc()
{
#ifdef SCOREFILE
    register int cnt;
    static struct stat sbuf;
    time_t time();

over:
    if (creat(lockfile, 0000) > 0)
	return TRUE;
    for (cnt = 0; cnt < 5; cnt++)
    {
	sleep(1);
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
	if (unlink(lockfile) < 0)
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
		    if (unlink(lockfile) < 0)
			return FALSE;
		}
		sleep(1);
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
    unlink(lockfile);
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
