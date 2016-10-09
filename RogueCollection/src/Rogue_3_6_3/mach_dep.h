/*
 * machine dependicies
 *
 * %G% (Berkeley) %W%
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

/*
 * where scorefile should live
 */

#include <limits.h>
#include <stdlib.h>

/*
 * Variables for checking to make sure the system isn't too loaded
 * for people to play
 */

#define AUTHORUID        0
#define	MAXUSERS	25	/* max number of users for this game */
#define	MAXLOAD		40	/* 10 * max 15 minute load average */
#define	CHECKTIME	15	/* number of minutes between load checks */

#ifndef PATH_MAX
#define PATH_MAX _MAX_PATH
#endif

#ifdef _WIN32
#define fstat _fstat
#define stat _stat
#define open _open
#define getpid _getpid
#define fdopen _fdopen
#define unlink _unlink
#ifndef __MINGW32__
#define fileno _fileno
#endif
#endif

extern char *md_getusername();
extern char *md_gethomedir();
extern void md_flushinp();
extern char *md_getshell();
extern char *md_gethostname();
extern void md_dobinaryio();
extern char *md_getpass();
extern void md_init();
extern char *xcrypt();
extern char *md_getroguedir();
