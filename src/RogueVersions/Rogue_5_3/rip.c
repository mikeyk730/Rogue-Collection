/*
 * File for the fun ends
 * Death or a total win
 *
 * @(#)rip.c	4.32 (NMT from Berkeley 5.2) 8/25/83
 */

#include <curses.h>
#ifdef	r_attron
#include <term.h>
#endif	r_attron
#include <time.h>
#include <signal.h>
#include <sys/types.h>
//#include <pwd.h>
#include "rogue.h"
#include "score.h"

static char *rip[] = {
"                       __________",
"                      /          \\",
"                     /    REST    \\",
"                    /      IN      \\",
"                   /     PEACE      \\",
"                  /                  \\",
"                  |                  |",
"                  |                  |",
"                  |   killed by a    |",
"                  |                  |",
"                  |       1984       |",
"                 *|     *  *  *      | *",
"         ________)/\\\\_//(\\/(/\\)/\\//\\/|_)_______",
    0
};

/*
 * score:
 *	Figure score and post it.
 */
/* VARARGS2 */
score(amount, flags, monst)
int amount, flags;
char monst;
{
    register SCORE *scp;
    register int i;
    register SCORE *sc2;
    register FILE *outf;
    register int prflags = 0;
    register int (*fp)(), uid;

    static SCORE top_ten[10];
    static char  *reason[] = {
	"killed",
	"quit",
	"A total winner",
    };
    int	endit();

    start_score();

    if (flags != -1)
	endwin();

    if (fd >= 0)
	outf = fdopen(fd, "wb");
    else
	return;

    for (scp = top_ten; scp < &top_ten[10]; scp++)
    {
	scp->sc_score = 0;
	for (i = 0; i < MAXSTR; i++)
	    scp->sc_name[i] = rnd(255);
	scp->sc_flags = RN;
	scp->sc_level = RN;
	scp->sc_monster = RN;
	scp->sc_uid = RN;
    }

    signal(SIGINT, SIG_DFL);
    if (flags != -1
#ifdef WIZARD
	    || wizard
#endif
	)
    {
        mvaddstr(LINES - 1, 0, "[Press return to continue]");
        refresh();
        wait_for('\n');
    }
#ifdef WIZARD
    if (wizard)
	if (strcmp(prbuf, "names") == 0)
	    prflags = 1;
	else if (strcmp(prbuf, "edit") == 0)
	    prflags = 2;
#endif
    encread((char *) top_ten, sizeof top_ten, fd);
    /*
     * Insert her in list if need be
     */
    sc2 = NULL;
    if (!noscore)
    {
	uid = 0; //mdk:getuid();
	for (scp = top_ten; scp < &top_ten[10]; scp++)
	    if (amount > scp->sc_score)
		break;
#ifdef LIMIT_TOPTEN
	    else if (flags != 2 && scp->sc_uid == uid && scp->sc_flags != 2)
		scp = &top_ten[10];	/* only one score per nowin uid */
#endif
	if (scp <= &top_ten[9])
	{
#ifdef LIMIT_TOPTEN
	    if (flags != 2)
		for (sc2 = scp; sc2 < &top_ten[10]; sc2++)
		{
		    if (sc2->sc_uid == uid && sc2->sc_flags != 2)
			break;
		}
	    else
#endif
		sc2 = &top_ten[9];
	    while (sc2 > scp)
	    {
		*sc2 = sc2[-1];
		sc2--;
	    }
	    scp->sc_score = amount;
	    strncpy(scp->sc_name, whoami, MAXSTR);
	    scp->sc_flags = flags;
	    if (flags == 2)
		scp->sc_level = max_level;
	    else
		scp->sc_level = level;
	    scp->sc_monster = monst;
	    scp->sc_uid = uid;
	    sc2 = scp;
	}
    }
    /*
     * Print the list
     */
    clear();//mdk:printf->printw putchar->addch
    printw("\nTop Ten Rogueists:\nRank\tScore\tName\n");
    for (scp = top_ten; scp < &top_ten[10]; scp++)
    {
	if (scp->sc_score) {
        if (sc2 == scp) {
            PC_GFX_COLOR(0x70);
        }
	    printw("%d\t%d\t%s: %s on level %d", scp - top_ten + 1,
		scp->sc_score, scp->sc_name, reason[scp->sc_flags < 3 ? scp->sc_flags : 0],
		scp->sc_level);
	    if (scp->sc_flags == 0)
		printw(" by %s", killname((char) scp->sc_monster, TRUE));
	    if (prflags == 1)
	    {
		struct passwd *pp, *getpwuid();

		//if ((pp = getpwuid(scp->sc_uid)) == NULL)
		    printw(" (%d)", scp->sc_uid);
		//else
		//    printw(" (%s)", pp->pw_name);
		addch('\n');
	    }
	    else if (prflags == 2)
	    {
		fflush(stdout);
		gets(prbuf);
		if (prbuf[0] == 'd')
		{
		    for (sc2 = scp; sc2 < &top_ten[9]; sc2++)
			*sc2 = *(sc2 + 1);
		    top_ten[9].sc_score = 0;
		    for (i = 0; i < MAXSTR; i++)
			top_ten[9].sc_name[i] = rnd(255);
		    top_ten[9].sc_flags = RN;
		    top_ten[9].sc_level = RN;
		    top_ten[9].sc_monster = RN;
		    scp--;
		}
	    }
	    else
		printw(".\n");
        PC_GFX_NOCOLOR(0x70);
	}
	else
	    break;
    }
    fseek(outf, 0L, 0);

    //mdk:
    mvaddstr(LINES - 1, 0, "[Press return to continue]");
    refresh();
    wait_for('\n');

    /*
     * Update the list file
     */
    if (sc2 != NULL)
    {
	if (lock_sc())
	{
	    fp = signal(SIGINT, SIG_IGN);
	    encwrite((char *) top_ten, sizeof top_ten, outf);
	    unlock_sc();
	    signal(SIGINT, fp);
	}
    }
    fclose(outf);
}

/*
 * death:
 *	Do something really fun when he dies
 */
death(monst)
register char monst;
{
    register char **dp = rip, *killer;
    register struct tm *lt;
    time_t date;
    char buf[MAXSTR];
    struct tm *localtime();

    signal(SIGINT, SIG_IGN);
    purse -= purse / 10;
    signal(SIGINT, leave);
    time(&date);
    lt = localtime(&date);
    clear();
    move(8, 0);
    PC_GFX_COLOR(0x06);
    while (*dp)
	printw("%s\n", *dp++);
    PC_GFX_NOCOLOR(0x06);
    mvaddstr(14, 28-((strlen(whoami)+1)/2), whoami);
    sprintf(buf, "%d Au", purse);
    mvaddstr(15, 28-((strlen(buf)+1)/2), buf);
    killer = killname(monst, FALSE);
    mvaddstr(17, 28-((strlen(killer)+1)/2), killer);
    if (monst == 's')
	mvaddrawch(16, 32, ' ');
    else
	mvaddstr(16, 33, vowelstr(killer));
    sprintf(prbuf, "%4d", 1900 + lt->tm_year);
    mvaddstr(18, 26, prbuf);
    PC_GFX_TOMBSTONE();
    move(LINES-1, 0);
    refresh();
    //mvprintw(0,0,"Doing score\n");
    //refresh();
    score(purse, 0, monst);
    EXIT(0);
}

/*
 * total_winner:
 *	Code for a winner
 */
total_winner()
{
    register THING *obj;
    register int worth;
    register char c;
    register int oldpurse;

    clear();
    standout();
    addstr("                                                               \n");
    addstr("  @   @               @   @           @          @@@  @     @  \n");
    addstr("  @   @               @@ @@           @           @   @     @  \n");
    addstr("  @   @  @@@  @   @   @ @ @  @@@   @@@@  @@@      @  @@@    @  \n");
    addstr("   @@@@ @   @ @   @   @   @     @ @   @ @   @     @   @     @  \n");
    addstr("      @ @   @ @   @   @   @  @@@@ @   @ @@@@@     @   @     @  \n");
    addstr("  @   @ @   @ @  @@   @   @ @   @ @   @ @         @   @  @     \n");
    addstr("   @@@   @@@   @@ @   @   @  @@@@  @@@@  @@@     @@@   @@   @  \n");
    addstr("                                                               \n");
    addstr("     Congratulations, you have made it to the light of day!    \n");
    standend();
    addstr("\nYou have joined the elite ranks of those who have escaped the\n");
    addstr("Dungeons of Doom alive.  You journey home and sell all your loot at\n");
    addstr("a great profit and are admitted to the fighters guild.\n");
    mvaddstr(LINES - 1, 0, "--Press space to continue--");
    refresh();
    wait_for(' ');
    clear();
    mvaddstr(0, 0, "   Worth  Item");
    oldpurse = purse;
    for (c = 'a', obj = pack; obj != NULL; c++, obj = next(obj))
    {
	switch (obj->o_type)
	{
	    when FOOD:
		worth = 2 * obj->o_count;
	    when WEAPON:
		switch (obj->o_which)
		{
		    when MACE: worth = 8;
		    when SWORD: worth = 15;
		    when CROSSBOW: worth = 30;
		    when ARROW: worth = 1;
		    when DAGGER: worth = 2;
		    when TWOSWORD: worth = 75;
		    when DART: worth = 1;
		    when BOW: worth = 15;
		    when BOLT: worth = 1;
		    when SPEAR: worth = 5;
		}
		worth *= 3 * (obj->o_hplus + obj->o_dplus) + obj->o_count;
		obj->o_flags |= ISKNOW;
	    when ARMOR:
		switch (obj->o_which)
		{
		    when LEATHER: worth = 20;
		    when RING_MAIL: worth = 25;
		    when STUDDED_LEATHER: worth = 20;
		    when SCALE_MAIL: worth = 30;
		    when CHAIN_MAIL: worth = 75;
		    when SPLINT_MAIL: worth = 80;
		    when BANDED_MAIL: worth = 90;
		    when PLATE_MAIL: worth = 150;
		}
		worth += (9 - obj->o_ac) * 100;
		worth += (10 * (a_class[obj->o_which] - obj->o_ac));
		obj->o_flags |= ISKNOW;
	    when SCROLL:
		worth = s_magic[obj->o_which].mi_worth;
		worth *= obj->o_count;
		if (!s_know[obj->o_which])
		    worth /= 2;
		s_know[obj->o_which] = TRUE;
	    when POTION:
		worth = p_magic[obj->o_which].mi_worth;
		worth *= obj->o_count;
		if (!p_know[obj->o_which])
		    worth /= 2;
		p_know[obj->o_which] = TRUE;
	    when RING:
		worth = r_magic[obj->o_which].mi_worth;
		if (obj->o_which == R_ADDSTR || obj->o_which == R_ADDDAM ||
		    obj->o_which == R_PROTECT || obj->o_which == R_ADDHIT)
			if (obj->o_ac > 0)
			    worth += obj->o_ac * 100;
			else
			    worth = 10;
		if (!(obj->o_flags & ISKNOW))
		    worth /= 2;
		obj->o_flags |= ISKNOW;
		r_know[obj->o_which] = TRUE;
	    when STICK:
		worth = ws_magic[obj->o_which].mi_worth;
		worth += 20 * obj->o_charges;
		if (!(obj->o_flags & ISKNOW))
		    worth /= 2;
		obj->o_flags |= ISKNOW;
		ws_know[obj->o_which] = TRUE;
	    when AMULET:
		worth = 1000;
	}
	if (worth < 0)
	    worth = 0;
	mvprintw(c - 'a' + 1, 0, "%c) %5d  %s", c, worth, inv_name(obj, FALSE));
	purse += worth;
    }
    mvprintw(c - 'a' + 1, 0,"   %5d  Gold Pieces          ", oldpurse);
    refresh();
    score(purse, 2);
    EXIT(0);
}

/*
 * killname:
 *	Convert a code to a monster name
 */
char *
killname(monst, doart)
register char monst;
bool doart;
{
    register char *sp;
    register bool article;

    sp = prbuf;
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
	sprintf(prbuf, "a%s ", vowelstr(sp));
    else
	prbuf[0] = '\0';
    strcat(prbuf, sp);
    return prbuf;
}
