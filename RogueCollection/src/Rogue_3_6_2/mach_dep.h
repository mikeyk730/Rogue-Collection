/*
 * machine dependicies
 *
 * %G% (Berkeley) %W%
 */
/*
 * where scorefile should live
 */
#ifdef __DJGPP__
#define SCOREFILE "C:\\GAMES\\ROGUE36\\ROGUE36.SCR"
#else
#define SCOREFILE	"/usr/local/games/rogue36/rogue36.scr"
#endif

/*
 * Variables for checking to make sure the system isn't too loaded
 * for people to play
 */

#define AUTHORUID        0
#ifndef __DJGPP__
#define	MAXUSERS	25	/* max number of users for this game */
#define	MAXLOAD		40	/* 10 * max 15 minute load average */
#undef  MAXLOAD                 /* load average check is non-portable */
#endif

#if defined(MAXUSERS) || defined(MAXLOAD)
#define	CHECKTIME	15	/* number of minutes between load checks */
				/* if not defined checks are only on startup */
#endif

#ifdef MAXLOAD
#define	LOADAV			/* defined if rogue should provide loadav() */

#ifdef LOADAV
#define	NAMELIST	"/unix"	/* where the system namelist lives */
#endif
#endif

#ifdef MAXUSERS
#define	UCOUNT			/* defined if rogue should provide ucount() */

#ifdef UCOUNT
#define UTMP	"/etc/utmp"	/* where utmp file lives */
#endif
#endif
