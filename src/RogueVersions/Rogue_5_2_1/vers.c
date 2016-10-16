/*
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981, 1982 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

/* 
 * Version number.  Whenever a new version number is desired, use sccs
 * to get vers.c.  encstr is declared here to force it to be loaded
 * before the version number, and therefore not to be written in saved
 * games.
 */

char *release = "5.2";
char encstr[] = "\211g\321_-\251b\324\237;\255\263\214g\"\327\224.,\252|9\265=\357+\343;\311]\341`\251\b\231)\266Y\325\251";
char version[] = "@(#)vers.c	5.2 (Berkeley) 4/11/82";
