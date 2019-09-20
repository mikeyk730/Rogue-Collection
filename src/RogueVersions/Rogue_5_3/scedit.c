/*
 * score editor
 *
 * @(#)scedit.c	4.2 (NMT from Berkeley 5.2) 8/25/83
 */
#ifndef ROGUE_COLLECTION
# include	<stdio.h>
# include	<pwd.h>
# include	<signal.h>
# include	<ctype.h>
# include	<unctrl.h>

# define	TRUE	1
# define	FALSE	0
# define	bool	char
# define	RN	(((Seed = Seed*11109+13849) >> 16) & 0xffff)

# define	MAXSTR	80

# include	"score.h"

SCORE	Top_ten[10];

char	Buf[BUFSIZ],
	*Reason[] = {
		"killed",
		"quit",
		"A total winner",
	};

int	Seed,
	Inf;

struct passwd	*getpwnam();

main(ac, av)
int	ac;
char	**av; {

	register char	*scorefile;
	register FILE	*outf;

	if (ac == 1)
		scorefile = SCOREFILE;
	else
		scorefile = av[1];
	Seed = getpid();

	if ((Inf = open(scorefile, 2)) < 0) {
		perror(scorefile);
		exit(1);
	}
	s_encread((char *) Top_ten, sizeof Top_ten, Inf);

	while (do_comm())
		continue;

	exit(0);
}

/*
 * do_comm:
 *	Get and execute a command
 */
do_comm() {

	register char		*sp;
	register SCORE		*scp;
	register struct passwd	*pp;
	struct passwd		*getpwuid();
	static FILE		*outf = NULL;
	static bool		written = TRUE;

	printf("\nCommand: ");
	while (isspace(Buf[0] = getchar()) || Buf[0] == '\n')
		continue;
	fgets(&Buf[1], BUFSIZ, stdin);
	Buf[strlen(Buf) - 1] = '\0';
	switch (Buf[0]) {
	  case 'w':
		if (strncmp(Buf, "write", strlen(Buf)))
			goto def;
		lseek(Inf, 0L, 0);
		if (outf == NULL && (outf = fdopen(Inf, "w")) == NULL) {
			perror("fdopen");
			exit(1);
		}
		fseek(outf, 0L, 0);
		if (s_lock_sc())
		{
			register int	(*fp)();

			fp = signal(SIGINT, SIG_IGN);
			s_encwrite((char *) Top_ten, sizeof Top_ten, outf);
			s_unlock_sc();
			signal(SIGINT, fp);
			written = TRUE;
		}
		break;

	  case 'a':
		if (strncmp(Buf, "add", strlen(Buf)))
			goto def;
		add_score();
		written = FALSE;
		break;

	  case 'd':
		if (strncmp(Buf, "delete", strlen(Buf)))
			goto def;
		del_score();
		written = FALSE;
		break;

	  case 'p':
		if (strncmp(Buf, "print", strlen(Buf)))
			goto def;
		printf("\nTop Ten Rogueists:\nRank\tScore\tName\n");
		for (scp = Top_ten; scp < &Top_ten[10]; scp++)
			if (!pr_score(scp, TRUE))
				break;
		break;

	  case 'q':
		if (strncmp(Buf, "quit", strlen(Buf)))
			goto def;
		if (!written) {
			printf("No write\n");
			written = TRUE;
		}
		else
			return FALSE;
		break;

	  default:
def:
		printf("Unkown command: \"%s\"\n", Buf);
	}
	return TRUE;
}

/*
 * add_score:
 *	Add a score to the score file
 */
add_score()
{
	register SCORE		*scp;
	register struct passwd	*pp;
	register int		i;
	static SCORE		new;

	printf("Name: ");
	fgets(new.sc_name, MAXSTR, stdin);
	new.sc_name[strlen(new.sc_name) - 1] = '\0';
	do {
		printf("Reason: ");
		if ((new.sc_flags = getchar()) < '0' || new.sc_flags > '2') {
			for (i = 0; i < 3; i++)
				printf("%d: %s\n", i, Reason[i]);
			ungetc(new.sc_flags, stdin);
		}
		while (getchar() != '\n')
			continue;
	} while (new.sc_flags < '0' || new.sc_flags > '2');
	new.sc_flags -= '0';
	do {
		printf("User: ");
		fgets(Buf, BUFSIZ, stdin);
		Buf[strlen(Buf) - 1] = '\0';
		if ((pp = getpwnam(Buf)) == NULL)
			printf("who (%s)?\n", Buf);
	} while (pp == NULL);
	new.sc_uid = pp->pw_uid;
	do {
		printf("Monster: ");
		if (!isupper(new.sc_monster = getchar())) {
			printf("type A-Z [%s]\n", unctrl(new.sc_monster));
			ungetc(new.sc_monster, stdin);
		}
		while (getchar() != '\n')
			continue;
	} while (!isupper(new.sc_monster));
	printf("Score: ");
	scanf("%d", &new.sc_score);
	printf("Level: ");
	scanf("%d", &new.sc_level);
	while (getchar() != '\n')
		continue;
	pr_score(&new, FALSE);
	printf("Really add it? ");
	if (getchar() != 'y')
		return;
	while (getchar() != '\n')
		continue;
	insert_score(&new);
}

/*
 * pr_score:
 *	Print out a score entry.  Return FALSE if last entry.
 */
pr_score(scp, num)
register SCORE	*scp;
bool		num; {

	register struct passwd	*pp;

	if (scp->sc_score) {
		if (num)
			printf("%d", scp - Top_ten + 1);
		printf("\t%d\t%s: %s on level %d", scp->sc_score, scp->sc_name,
		    Reason[scp->sc_flags], scp->sc_level);
		if (scp->sc_flags == 0)
		    printf(" by %s", s_killname((char) scp->sc_monster, TRUE));
		if ((pp = getpwuid(scp->sc_uid)) == NULL)
			printf(" (%d)", scp->sc_uid);
		else
			printf(" (%s)", pp->pw_name);
		putchar('\n');
	}
	return scp->sc_score;
}

/*
 * insert_score:
 *	Insert a score into the top ten list
 */
insert_score(new)
SCORE	*new; {

	register SCORE	*scp, *sc2;
	register int	flags, uid, amount;

	flags = new->sc_flags;
	uid = new->sc_uid;
	amount = new->sc_score;

	for (scp = Top_ten; scp < &Top_ten[10]; scp++)
		if (amount > scp->sc_score)
			break;
		else if (flags != 2 && scp->sc_uid == uid && scp->sc_flags != 2)
			scp = &Top_ten[10];
	if (scp < &Top_ten[10]) {
		if (flags != 2)
			for (sc2 = scp; sc2 < &Top_ten[10]; sc2++) {
			    if (sc2->sc_uid == uid && sc2->sc_flags != 2)
				break;
			}
		else
			sc2 = &Top_ten[9];
		while (sc2 > scp) {
			*sc2 = sc2[-1];
			sc2--;
		}
		*scp = *new;
		sc2 = scp;
	}
}

/*
 * del_score:
 *	Delete a score from the score list.
 */
del_score() {

	register SCORE	*scp;
	register int	i;
	int		num;

	for (;;) {
		printf("Which score? ");
		fgets(Buf, BUFSIZ, stdin);
		if (Buf[0] == '\n')
			return;
		sscanf(Buf, "%d", &num);
		if (num < 1 || num > 10)
			printf("range is 1-10\n");
		else
			break;
	}
	num--;
	for (scp = &Top_ten[num]; scp < &Top_ten[9]; scp++)
		*scp = scp[1];
	Top_ten[9].sc_score = 0;
	for (i = 0; i < MAXSTR; i++)
	    Top_ten[9].sc_name[i] = RN;
	Top_ten[9].sc_flags = RN;
	Top_ten[9].sc_level = RN;
	Top_ten[9].sc_monster = RN;
	Top_ten[9].sc_uid = RN;
}
#endif
