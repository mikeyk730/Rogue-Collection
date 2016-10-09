/*
 * print out an encrypted password on the standard output
 *
 * @(#)findpw.c	1.2 (NMT from Berkeley 5.2) 8/25/83
 */
#include <stdio.h>

main()
{
    char buf[80];

    fprintf(stderr, "Password: ");
    fgets(buf, 80, stdin);
    buf[strlen(buf) - 1] = '\0';
    printf("%s\n", crypt(buf, "mT"));
}
