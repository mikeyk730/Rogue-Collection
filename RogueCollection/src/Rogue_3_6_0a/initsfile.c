#include <curses.h>
#include <time.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <pwd.h>
#include "mach_dep.h"
#include "rogue.h"
#include "vers.c"

struct h_list helpstr[1];

main()
{
    static struct sc_ent {
	int sc_score;
	char sc_name[80];
	int sc_flags;
	int sc_level;
	int sc_uid;
	char sc_monster;
    } top_ten[NUMTOP];
    register struct sc_ent *scp;
    register int i;
    register struct sc_ent *sc2;
    register FILE *outf;
    register char *killer;
    register int prflags = 0;
    register int fd;
	int oldsize;

	printf("Oldsize? ");
	scanf("%d", &oldsize);
	printf("Converting %s from %d entries to %d entries\n", SCOREFILE,
	  oldsize, NUMTOP);
    if ((fd = open(SCOREFILE, 2)) < 0) {
		printf("Cannot open %s\n", SCOREFILE);
		exit(1);
	}
    outf = fdopen(fd, "w");
    for (scp = top_ten; scp < &top_ten[NUMTOP]; scp++)
    {
	scp->sc_score = 0;
	for (i = 0; i < 80; i++)
	    scp->sc_name[i] = '\0';
	scp->sc_flags = RN;
	scp->sc_level = RN;
	scp->sc_monster = RN;
	scp->sc_uid = RN;
    }
    encread((char *) top_ten, sizeof(struct sc_ent)*oldsize, fd);
    fseek(outf, 0L, 0);
    /*
     * Update the list file
     */
    encwrite((char *) top_ten, sizeof top_ten, outf);
    fclose(outf);
}


/*
 * perform an encrypted write
 */
encwrite(start, size, outf)
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
 * perform an encrypted read
 */
encread(start, size, inf)
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
