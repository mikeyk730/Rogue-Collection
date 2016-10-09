/*
 * copies of several routines needed for score
 *
 * @(#)smisc.c	4.7 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

# include	<stdio.h>
# include	<sys/types.h>
# include	<sys/stat.h>
# include	<ctype.h>

# define	TRUE		1
# define	FALSE		0
# define	MAXSTR		80
# define	bool		char
# define	when		break;case
# define	otherwise	break;default

typedef struct {
	char	*m_name;
} MONST;

char	*s_vowelstr();

extern char	encstr[], frob;

char *lockfile = "/tmp/.fredlock";

char prbuf[MAXSTR];			/* buffer for sprintfs */

#ifdef MDK
static
#endif
MONST	monsters[] = {
	{ "aquator" }, { "bat" }, { "centaur" }, { "dragon" }, { "emu" },
	{ "venus flytrap" }, { "griffin" }, { "hobgoblin" }, { "ice monster" },
	{ "jabberwock" }, { "kobold" }, { "leprechaun" }, { "medusa" },
	{ "nymph" }, { "orc" }, { "phantom" }, { "quasit" }, { "rattlesnake" },
	{ "snake" }, { "troll" }, { "ur-vile" }, { "vampire" }, { "wraith" },
	{ "xeroc" }, { "yeti" }, { "zombie" }
};

/*
 * s_lock_sc:
 *	lock the score file.  If it takes too long, ask the user if
 *	they care to wait.  Return TRUE if the lock is successful.
 */
bool
s_lock_sc()
{
    int cnt;
    static struct stat sbuf;
    time_t time();

over:
    close(8);	/* just in case there are no files left */
    if (creat(lockfile, 0000) >= 0)
	return TRUE;
    for (cnt = 0; cnt < 5; cnt++)
    {
	md_sleep(1);
	if (creat(lockfile, 0000) >= 0)
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
		if (creat(lockfile, 0000) >= 0)
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
}

/*
 * s_unlock_sc:
 *	Unlock the score file
 */

s_unlock_sc()
{
    md_unlink(lockfile);
}

/*
 * s_encwrite:
 *	Perform an encrypted write
 */

s_encwrite(char *start, unsigned int size, FILE *outf)
{
    char *e1, *e2, fb;
    int temp;
    extern char statlist[];

    e1 = encstr;
    e2 = statlist;
    fb = frob;

    while (size--)
    {
	putc(*start++ ^ *e1 ^ *e2 ^ fb, outf);
	temp = *e1++;
	fb += temp * *e2++;
	if (*e1 == '\0')
	    e1 = encstr;
	if (*e2 == '\0')
	    e2 = statlist;
    }
}

/*
 * s_encread:
 *	Perform an encrypted read
 */

s_encread(char *start, unsigned int size, int inf)
{
    char *e1, *e2, fb;
    int temp;
    int read_size;
    extern char statlist[];

    fb = frob;

    if ((read_size = read(inf, start, size)) == 0 || read_size == -1)
	return;

    e1 = encstr;
    e2 = statlist;

    while (size--)
    {
	*start++ ^= *e1 ^ *e2 ^ fb;
	temp = *e1++;
	fb += temp * *e2++;
	if (*e1 == '\0')
	    e1 = encstr;
	if (*e2 == '\0')
	    e2 = statlist;
    }
}

/*
 * s_killname:
 *	Convert a code to a monster name
 */
char *
s_killname(char monst, bool doart)
{
    char *sp;
    bool article;

    article = TRUE;
    switch (monst)
    {
	case 'a':
	    sp = "arrow";
	when 'b':
	    sp = "bolt";
	when 'd':
	    sp = "dart";
	when 's':
	    sp = "starvation";
	    article = FALSE;
	when 'h':
	    sp = "hypothermia";
	    article = FALSE;
	otherwise:
	    if (isupper(monst))
		sp = monsters[monst-'A'].m_name;
	    else
	    {
		sp = "God";
		article = FALSE;
	    }
    }
    if (doart && article)
	sprintf(prbuf, "a%s ", s_vowelstr(sp));
    else
	prbuf[0] = '\0';
    strcat(prbuf, sp);
    return prbuf;
}

/*
 * s_vowelstr:
 *      For printfs: if string starts with a vowel, return "n" for an
 *	"an".
 */
char *
s_vowelstr(char *str)
{
    switch (*str)
    {
	case 'a': case 'A':
	case 'e': case 'E':
	case 'i': case 'I':
	case 'o': case 'O':
	case 'u': case 'U':
	    return "n";
	default:
	    return "";
    }
}
