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
 * Variables for checking to make sure the system isn't too loaded
 * for people to play
 */

#define AUTHORUID        0
#define	MAXUSERS	25	/* max number of users for this game */
#define	MAXLOAD		40	/* 10 * max 15 minute load average */
#define	CHECKTIME	15	/* number of minutes between load checks */
