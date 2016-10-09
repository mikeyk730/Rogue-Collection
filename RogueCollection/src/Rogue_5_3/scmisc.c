/*
 * copies of several routines needed for scedit
 *
 * @(#)scmisc.c	4.2 (NMT from Berkeley 5.2) 8/25/83
 */

# include	<stdio.h>
# include	<sys/types.h>
# include	<sys/stat.h>

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

extern char	encstr[];

char *lockfile = "/tmp/.roguelock";
char prbuf[MAXSTR];			/* Buffer for sprintfs */

MONST	monsters[] = {
	{ "giant ant" }, { "bat" }, { "centaur" }, { "dragon" },
	{ "floating eye" }, { "violet fungi" }, { "gnome" }, { "hobgoblin" },
	{ "invisible stalker" }, { "jackal" }, { "kobold" }, { "leprechaun" },
	{ "mimic" }, { "nymph" }, { "orc" }, { "purple worm" }, { "quasit" },
	{ "rust monster" }, { "snake" }, { "troll" }, { "umber hulk" },
	{ "vampire" }, { "wraith" }, { "xorn" }, { "yeti" }, { "zombie" }
};

/*
 * s_lock_sc:
 *	lock the score file.  If it takes too long, ask the user if
 *	they care to wait.  Return TRUE if the lock is successful.
 */
s_lock_sc()
{
    register int cnt;
    static struct stat sbuf;
    time_t time();

over:
    close(8);	/* just in case there are no files left */
    if (creat(lockfile, 0000) >= 0)
	return TRUE;
    for (cnt = 0; cnt < 5; cnt++)
    {
	sleep(1);
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
		if (creat(lockfile, 0000) >= 0)
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
}

/*
 * s_unlock_sc:
 *	Unlock the score file
 */
s_unlock_sc()
{
    unlink(lockfile);
}

/*
 * s_encwrite:
 *	Perform an encrypted write
 */
s_encwrite(start, size, outf)
register char *start;
unsigned int size;
register FILE *outf;
{
    register char *ep;

    ep = encstr;

    while (size--)
    {
	putc(*start++ ^ *ep++, outf);
	if (*ep == '\0')
	    ep = encstr;
    }
}

/*
 * s_encread:
 *	Perform an encrypted read
 */
s_encread(start, size, inf)
register char *start;
unsigned int size;
register int inf;
{
    register char *ep;
    register int read_size;

    if ((read_size = read(inf, start, size)) == -1 || read_size == 0)
	return read_size;

    ep = encstr;

    while (size--)
    {
	*start++ ^= *ep++;
	if (*ep == '\0')
	    ep = encstr;
    }
    return read_size;
}

/*
 * s_killname:
 *	Convert a code to a monster name
 */
char *
s_killname(monst, doart)
register char monst;
bool doart;
{
    register char *sp;
    register bool article;

    article = TRUE;
    switch (monst)
    {
	when 'a':
	    sp = "arrow";
	when 'b':
	    sp = "bolt";
	when 'd':
	    sp = "dart";
	when 's':
	    sp = "starvation";
	    article = FALSE;
	otherwise:
	    if (monst >= 'A' && monst <= 'Z')
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
s_vowelstr(str)
register char *str;
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
