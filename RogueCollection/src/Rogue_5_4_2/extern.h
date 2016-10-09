/*
 * Defines for things used in mach_dep.c
 *
 * @(#)extern.h	4.35 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */


/*
 * Don't change the constants, since they are used for sizes in many
 * places in the program.
 */

#ifndef _WIN32
#include <sys/ioctl.h>
#endif
#include <stdlib.h>

#undef SIGTSTP

#if defined(_WIN32) && !defined(__MINGW32__)
#ifdef _PATH_MAX
#define PATH_MAX _PATH_MAX
#endif
#ifdef _MAX_PATH
#define PATH_MAX _MAX_PATH
#endif
#endif
#include <stdlib.h>
#define MAXSTR		1024	/* maximum length of strings */
#define MAXLINES	32	/* maximum number of screen lines used */
#define MAXCOLS		80	/* maximum number of screen columns used */

#define RN		(((seed = seed*11109+13849) >> 16) & 0xffff)
#ifdef CTRL
#undef CTRL
#endif
#define CTRL(c)		(c & 037)

/*
 * Now all the global variables
 */

extern bool	got_ltc, in_shell, wizard;

extern char	fruit[], orig_dsusp, prbuf[], whoami[];

extern int	fd;

#ifdef TIOCGLTC
extern struct ltchars	ltc;
#endif /* TIOCGLTC */

/*
 * Function types
 */

#include <stdlib.h>

void    auto_save(int);
int	come_down();
int	doctor();
int	end_line();
void    endit(int sig);
int	fatal();
int	getltchars();
int	land();
void    leave(int);
int	my_exit();
int	nohaste();
int	playit();
void    playltchars(void);
int	print_disc(char);
void    quit(int);
void    resetltchars(void);
int	rollwand();
int	runners();
int	set_order();
int	sight();
int	stomach();
int	swander();
int	tstp();
int	unconfuse();
int	unsee();
int	visuals();

char	add_line(char *fmt, char *arg);

char	*killname(char monst, bool doart);
char	*nothing(char type);
char	*type_name(int type);

#ifdef CHECKTIME
int	checkout();
#endif

char *md_getusername();
char *md_getroguedir();
char *md_crypt();
char *md_getpass();
char *md_gethomedir();
