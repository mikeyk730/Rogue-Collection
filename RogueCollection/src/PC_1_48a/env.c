/*
 * Env.c:   routines to set up environment
 *          Jon Lane  -  10/31/83
 */


#define ERROR   -1
#define NULL	0
#define MATCH    0
#define MAXEP	 8
#define FOREVER	 1

char l_name[] = "name";
char l_save[] = "savefile";
char l_score[] = "scorefile";
char l_macro[] = "macro";
char l_fruit[] = "fruit";
char l_drive[] = "drive";
char l_menu [] = "menu";
char l_screen[]   = "screen";

char whoami[] = "Rodney\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
char s_score[]  =  "rogue.scr\0\0\0\0\0";
char s_save[]   =   "rogue.sav\0\0\0\0\0";
char macro[]    =   "v\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
char fruit[]    =  "Slime Mold\0\0\0\0\0\0\0\0\0\0\0\0\0";
char s_drive[]  =  "?";
char s_menu[]   =  "on\0";
char s_screen[]    =  "\0w fast";

struct environment {
    char *e_label;
    char *e_string;
    int  strlen;
} element[MAXEP] = {
	 l_name,	whoami,		23,
	 l_score,	s_score,	14,
	 l_save,	s_save,		14,
	 l_macro,	macro,		40,
	 l_fruit,	fruit,		23,
	 l_drive,	s_drive,	 1,
	 l_menu,	s_menu,		 3,
	 l_screen,	s_screen,	 7,
} ;


static int fd;
static int ch;
static int pstate;
static char blabel[11], bstring[25];
static char *plabel, *pstring;
/*
 *  setenv: read in environment from a file
 *
 *        envfile - name of file that contains data to be
 *                  put in the environment
 *
 *        STATUS  - setenv return -1 if the file does not
 *                  exist or if there is not enough memory
 *                  to expand the environment
 *
 */
setenv(envfile)
    char *envfile;
{
	register char pc;

	one_tick();	/* if he tries to disable the clock */
    if ((fd = open(envfile,0)) < 0)
	{
        return(ERROR);
	}

	while ( FOREVER )
	{
		/*
		 * Look for another label
		 */
		pstate = 0;
		plabel = blabel;
		pstring = bstring;

		/*
		 * Skip white space, this is the only state (pstate == 0)
		 * where eof will not be aborted
		 */
		while (isspace(peekc()))
			;
		if (ch == 0) {
				close(fd);
				return(NULL);
		}
		pstate = 3;
		/*
		 * Skip comments.
		 */
		if (ch == '#') {
			while (peekc() != '\n')
				;
			continue;
		}
		pstate = 1;
		/*
		 * start of label found
		 */
		*plabel = ch;
		while ((pc = peekc()) != '=' && pc != '-')
			if (!isspace(*plabel) || !isspace(ch))
				*(++plabel) = ch;
		if (!isspace(*plabel))
			plabel++;
		*plabel = 0;

		/*
		 * Looking for corresponding string
		 */
		while (isspace(peekc()))
			;
		
		/*
		 * Start of string found
		 */
		pstate = 2;
		*pstring = ch;
		while (peekc() != '\n')
			if (!isspace(*pstring) || !isspace(ch))
				*(++pstring) = ch;
		if (!isspace(*pstring))
			pstring++;
		*pstring = 0;
		lcase(blabel);
		putenv(blabel,bstring);
		/* printf("env: found (%s) = (%s)\n",blabel,bstring); */
	}		
	/*
	 * for all environment strings that have to be in lowercase ....
	 */
	lcase(s_menu);
	lcase(s_screen);
}

/*
 *  Peekc - 
 *  Return the next char associated with 
 *  efd (environment file descripter
 *
 *  This routine has some knowledge of the 
 *  file parsing state so that it knows
 *  if there has been a premature eof.  This
 *  way I can avoid checking for premature eof
 *  every time a character is read.
 */
peekc()
{
	ch = 0;
	/*
	 * we make sure that the strings never get filled past 
	 * the end, this way we only have to check for these
	 * things once
	 */
	if (plabel > &blabel[10])
		plabel = &blabel[10];
	if (pstring > &bstring[24])
		pstring = &bstring[24];
	if (read(fd, &ch, 1) < 1 && pstate != 0) {
		/*
		 * When looking for the end of the string,
		 * Let the eof look like newlines
		 */
		if (pstate >= 2)
			return('\n');
		fatal("rogue.opt: incorrect file format\n");
	}
	if (ch == 26)
		ch = '\n';
	return(ch);
}

/*
 * Getenv: UNIX compatable call
 *
 *	  label - label of thing in environment
 *
 *	  STATUS - returns the string associated with the label
 *			   or NULL (0) if it is not present
 */
#ifdef LUXURY
getenv(label)
	char *label;
{
	register int i;

	for (i=0 ; i<MAXEP ; i++ )
	{
		if ( strcmp(label,element[i].e_label) == MATCH )
			return(element[i].e_string);
	}
	return(NULL);
}
#endif

/*
 * Putenv: Put something into the environment
 *
 *	  label  - label of thing in environment
 *	  string - string associated with the label
 *
 *	  No meaningful return codes to save data space
 *	  Just ingnores strange labels
 */
putenv(label,string)
    char *label, *string;
{
    register int i;

    for (i=0 ; i<MAXEP ; i++)
    {
	if ( strcmp(label,element[i].e_label) == MATCH )
	    stccpy(element[i].e_string, string, element[i].strlen);
    }
}

#ifdef LUXURY
is_set(label,string)
	char *label,*string;
{
	return(!strcmp(string,getenv(label)));
}
#endif
