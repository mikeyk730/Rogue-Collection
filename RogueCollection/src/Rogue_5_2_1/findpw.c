/*
 * print out an encrypted password on the standard output
 *
 * @(#)findpw.c	1.1 (Berkeley) 12/20/81
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981, 1982 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <stdio.h>

main()
{
    char buf[80];

    fprintf(stderr, "Password: ");
    fgets(buf, 80, stdin);
    buf[strlen(buf) - 1] = '\0';
    printf("%s\n", xcrypt(buf, "mT"));
}
