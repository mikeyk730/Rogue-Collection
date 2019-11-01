/*
bug list:
+ item traits not initialized
+ call it not working
+ reading identify scroll crashes
+ map scroll corrupts level flags
+ archery teleport trap loop
+ run strategies don't execute
+ regeneration ring loop with detect monster ability
- unpinning loop (seed: 1569794071)
- sleeping monster blocking door (seed: 1569806185)
- teleport trap at doorway infinite loop

 * Rog-O-Matic
 * Automatically exploring the dungeons of doom.
 *
 * Copyright (C) 2008 by Anthony Molinaro
 * Copyright (C) 1985 by Appel, Jacobson, Hamey, and Mauldin.
 *
 * This file is part of Rog-O-Matic.
 *
 * Rog-O-Matic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Rog-O-Matic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rog-O-Matic.  If not, see <http://www.gnu.org/licenses/>.
 */

/*****************************************************************
 *
 * History:     I.    Andrew Appel & Guy Jacobson, 10/81 [created]
 *              II.   Andrew Appel & Guy Jacobson, 1/82  [added search]
 *              III.  Michael Mauldin, 3/82              [added termcap]
 *              IV.   Michael Mauldin, 3/82              [searching]
 *              V.    Michael Mauldin, 4/82              [cheat mode]
 *              VI.   Michael Mauldin, 4/82              [object database]
 *              VII.  All three, 5/82                    [running away]
 *              VIII. Michael Mauldin, 9/82              [improved cheating]
 *              IX.   Michael Mauldin, 10/82             [replaced termcap]
 *              X.    Mauldin, Hamey,  11/82             [Fixes, Rogue 5.2]
 *              XI.   Mauldin,  11/82                    [Fixes, Score lock]
 *              XII.  Hamey, Mauldin,  06/83             [Fixes, New Replay]
 *              XIII. Mauldin, Hamey,  11/83             [Fixes, Rogue 5.3]
 *              XIV.  Mauldin          01/85             [Fixes, UT mods]
 *              0.0.0 Anthony Molinaro 03/2008           [Restored]
 *
 * General:
 *
 * This is the main routine for the player process, which decodes the
 * Rogue output and sends commands back. This process is execl'd by the
 * rogomatic process (cf. setup.c) which also execl's the Rogue process,
 * conveniently connecting the two via two pipes.
 *
 * Source Files:
 *
 *      arms.c          Armor, Weapon, and Ring handling functions
 *      command.c       Effector interface, sends cmds to Rogue
 *      database.c      Memory for objects "discovered"
 *      debug.c         Contains the debugging functions
 *      explore.c       Path searching functions, exploration
 *      findscore.c     Reads Rogue scoreboard
 *      io.c            I/O functions, Sensory interface
 *      main.c          Main Program for 'player' (this file)
 *      mess.c          Handles messages from Rogue
 *      monsters.c      Monster handling utilities
 *      mover.c         Creates command strings to accomplish moves
 *      rooms.c         Room specific functions, new levels
 *      scorefile.c     Score file handling utilities
 *      search.c        Does shortest path
 *      setup.c         Main program for 'rogomatic'
 *      strategy.c      Makes high level decisions
 *      survival.c      Find cycles and places to run to
 *      tactics.c       Medium level intelligence
 *      things.c        Builds commands, part of Effector interface
 *      titlepage.c     Prints the animated copyright notice
 *      utility.c       Miscellaneous Unix (tm) functions
 *      worth.c         Evaluates the items in the pack
 *
 * Include files:
 *
 *      globals.h       External defs for all global variables
 *      install.h       Machine dependent DEFINES
 *      termtokens.h    Defines various tokens to/from Rogue
 *      types.h         Global DEFINES, macros, and typedefs.
 *
 * Other files which may be included with your distribution include
 *
 *      rplot           A shell script, prints a scatter plot of Rog's scores.
 *      rgmplot.c       A program used by rplot.
 *      datesub.l       A program used by rplot.
 *      histplot.c      A program which plots a histogram of Rgm's scores.
 *
 * Acknowledgments
 *
 *	The UTexas modifications included in this distribution
 *	came from Dan Reynolds, and are included by permission.
 *	Rog-O-Matics first total winner against version 5.3 was
 *	on a UTexas computer.
 *****************************************************************/

#ifdef _WIN32
#include <Windows.h>
#undef MOUSE_MOVED
#undef OPTIONAL
#undef NEAR
#include <io.h>
#include <process.h>
#define pid_t int
#endif

# include <curses.h>
# include <ctype.h>
# include <fcntl.h>
# include <signal.h>
# include <setjmp.h>
# include <string.h>
# include <stdlib.h>
# include <sys/types.h>
#ifndef _WIN32
# include <unistd.h>
#endif
# include "types.h"
# include "termtokens.h"
# include "install.h"
# include "globals.h"
# include "../RogueVersions/pc_gfx_macros.h"

/* global data - see globals.h for current definitions */

/* Files */
FILE  *logfile=NULL;		/* File for score log */
FILE  *realstdout=NULL;		/* Real stdout for Emacs, terse mode */
FILE  *snapshot=NULL;		/* File for snapshot command */
FILE  *trogue=NULL;		/* Pipe to Rogue process */


/* Characters */
char  logfilename[100];		/* Name of log file */
char  afterid = '\0';           /* Letter of obj after identify */
char  genelock[100];		/* Gene pool lock file */
char  genelog[100];		/* Genetic learning log file */
char  genepool[100];		/* Gene pool */
char  *genocide;		/* List of monsters to be genocided */
char  genocided[100];		/* List of monsters genocided */
char  lastcmd[NAMSIZ];		/* Copy of last command sent to Rogue */
char  lastname[NAMSIZ];		/* Name of last potion/scroll/wand */
char  nextid = '\0';            /* Next object to identify */
char  screen[MAXROWS][MAXCOLS];		/* Map of current Rogue screen */
char  sumline[128];		/* Termination message for Rogomatic */
char  ourkiller[NAMSIZ];		/* How we died */
char  versionstr[32];		/* Version of Rogue being used */
#ifndef ROGUE_COLLECTION
char  *parmstr;			/* Pointer to process arguments */
#endif
char  pending_call_letter = ' ';	/* If non-blank we have a call it to do */
char  pending_call_name[NAMSIZ];	/*   and this is the name to use */

/* Integers */
int   aggravated = 0;		/* True if we have aggravated this level */
int   agoalc = NONE;		/* Goal square to arch from (col) */
int   agoalr = NONE;		/* Goal square to arch from (row) */
int   arglen = 0;		/* Length in bytes of argument space */
int   ammo = 0;                 /* How many missiles? */
int   arrowshot = 0;		/* True if an arrow shot us last turn */
int   atrow, atcol;		/* Current position of the Rogue (@) */
int   atrow0, atcol0;		/* Position at start of turn */
int   attempt = 0;		/* Number times we searched whole level */
int   badarrow = 0;		/* True if cursed/lousy arrow in hand */
int   beingheld = 0;		/* True if a fungus has ahold of us */
int   beingstalked = 0;		/* True if recently hit by inv. stalker */
int   blinded = 0;		/* True if blinded */
int   blindir = 0;		/* Last direction we moved when blind */
int   cancelled = 0;		/* True ==> recently zapped w/cancel */
int   cheat = 0;		/* True ==> cheat, use bugs, etc. */
int   checkrange = 0;           /* True ==> check range */
int   chicken = 0;		/* True ==> check run away code */
int   compression = 1;		/* True ==> move more than one square/turn */
int   confused = 0;		/* True if we are confused */
int   cosmic = 0;		/* True if we are hallucinating */
int   currentarmor = NONE;	/* Index of our armor */
int   currentweapon = NONE;     /* Index of our weapon */
int   cursedarmor = 0;		/* True if our armor is cursed */
int   cursedweapon = 0;		/* True if we are wielding cursed weapon */
int   darkdir = NONE;		/* Direction of monster being arched */
int   darkturns = 0;		/* Distance to monster being arched */
int   debugging = 0;	/* Debugging options in effect */
int   didreadmap = 0;		/* Last level we read a map on */
int   doorlist[40];		/* List of doors on this level */
int   doublehasted = 0; 	/* True if double hasted (Rogue 3.6) */
int   droppedscare = 0;		/* True if we dropped 'scare' on this level */
int   diddrop = 0;	/* True if we dropped anything on this spot */
int   emacs = 0;		/* True ==> format output for Emacs */
int   exploredlevel = 0;	/* We completely explored this level */
int   floating = 0;		/* True if we are levitating */
int   foughtmonster = 0;	/* True if recently fought a monster */
int   foundarrowtrap = 0;	/* Found arrow trap this level */
int   foundtrapdoor = 0;	/* Found trap door this level */
int   goalc = NONE;		/* Current goal square (col) */
int   goalr = NONE;		/* Current goal square (row) */
int   goodarrow = 0;		/* True if good (magic) arrow in hand */
int   goodweapon = 0;		/* True if weapon in hand worth >= 100 */
int   gplusdam = 1;		/* Our plus damage from strength */
int   gplushit = 0;		/* Our plus to hit from strength */
int   hasted = 0;		/* True if hasted */
int   hitstokill = 0;		/* # times we hit last monster killed */
int   interrupted = 0;		/* True if at commandtop from onintr() */
int   knowident = 0;            /* Found an identify scroll? */
int   larder = 1;               /* How much food? */
int   lastate = 0;		/* Time we last ate */
int   lastdamage = 0;           /* Amount of last hit by a monster */
int   lastdrop = NONE;		/* Last object we tried to drop */
int   lastfoodlevel = 1;	/* Last level we found food */
int   lastmonster = NONE;	/* Last monster we tried to hit */
int   lastobj = NONE;		/* What did we last use */
int   lastwand = NONE;		/* Index of last wand */
int   leftring = NONE;		/* Index of our left ring */
int   logdigested = 0;		/* True if log file has been read by replay */
int   logging = 0;		/* True if keeping record of game */
int   lyinginwait = 0;          /* True if we waited for a monster */
int   maxobj = 22;              /* How much can we carry */
int   missedstairs = 0;         /* True if we searched everywhere */
int   morecount = 0;            /* Number of messages since last command */
int   msgonscreen = 0;		/* Set implies message at top */
int   newarmor = 1;             /* Change in armor status? */
int  *newdoors = NULL;		/* New doors on screen */
int   newring = 1;              /* Change in ring status? */
int   newweapon = 1;            /* Change in weapon status? */
int   nohalf = 0;		/* True ==> no halftime show */
int   noterm = 0;		/* True ==> no user watching */
int   objcount = 0;             /* Number of objects */
int   ourscore = 0;		/* Final score when killed */
int   playing = 1;		/* True if still playing game */
int   poorarrow = 0;		/* True if arrow has missed */
int   protected = 0;		/* True if we protected our armor */
int   putonseeinv = 0;          /* Turn when last put on see inv ring */
int   quitat = BOGUS;		/* Score to beat, quit if within 10% more */
int   redhands = 0;		/* True if we have red hands */
int   replaying = 0;		/* True if replaying old game */
int   revvideo = 0;		/* True if in rev. video mode */
int   rightring = NONE;		/* Index of our right ring */
int   rogpid = 0;		/* Pid of rogue process */
int   room[9];			/* Flags for each room */
int   row, col;			/* Current cursor position */
int   scrmap[MAXROWS][MAXCOLS];		/* Flags bits for level map */
int   singlestep = 0;		/* True ==> go one turn */
int   slowed = 0;		/* True ==> recently zapped w/slow monster */
int   stairrow, staircol;	/* Position of stairs on this level */
int   startecho = 0;		/* True ==> turn on echoing on startup */
int   teleported = 0;		/* # times teleported this level */
int   terse = 0;		/* True ==> terse mode */
int   transparent = 0;		/* True ==> user command mode */
int   trapc = NONE;		/* Location of arrow trap, this level (col) */
int   trapr = NONE;		/* Location of arrow trap, this level (row) */
int   urocnt = 0;               /* Un-identified Rogue Object count */
int   usesynch = 0;             /* Set when the inventory is correct */
int   usingarrow = 0;		/* True ==> wielding an arrow froma trap */
int   version;			/* Rogue version, integer */
int   wplusdam = 2;		/* Our plus damage from weapon bonus */
int   wplushit = 1;		/* Our plus hit from weapon bonus */
int   zone = NONE;		/* Current screen zone, 0..8 */
int   zonemap[9][9];		/* Map of zones connections */
int   g_seed = 0;
int   g_expect_extra_bytes = 0;
int   g_move_delay = 0;
int   g_level_delay = 0;
int   g_last_stuck_level = -1;
int   g_debug = 1;
int   g_debug_protocol = 0;

/* Functions */
void (*istat)(int);
void onintr (int sig);
char getroguetoken (), *getname();

/* Stuff list, list of objects on this level */
stuffrec slist[MAXSTUFF]; 	int slistlen=0;

/* Monster list, list of monsters on this level */
monrec mlist[MAXMONST];		int mlistlen=0;

char targetmonster = '@';	/* Monster we are arching at */

/* Monster attribute and Long term memory arrays */
attrec monatt[26];		/* Monster attributes */
lrnrec ltm;			/* Long term memory -- general */
ltmrec monhist[MAXMON];		/* Long term memory -- creatures */
int nextmon = 0;		/* Length of LTM */
int monindex[27];		/* Index into monhist array */

/* Genetic learning parameters (and defaults) */
int geneid = 0;		/* Id of genotype */
int genebest = 0;	/* Best score of genotype */
int geneavg = 0;	/* Average score of genotype */
int k_srch =	50;	/* Propensity for searching for traps */
int k_door =	50;	/* Propensity for searching for doors */
int k_rest =	50;	/* Propensity for resting */
int k_arch =	50;	/* Propensity for firing arrows */
int k_exper =	50;	/* Level*10 on which to experiment with items */
int k_run =	50;	/* Propensity for retreating */
int k_wake =	50;	/* Propensity for waking things up */
int k_food =	50;	/* Propensity for hoarding food (affects rings) */
int knob[MAXKNOB] = {50, 50, 50, 50, 50, 50, 50, 50};
char *knob_name[MAXKNOB] = {
  "trap searching:   ",
  "door searching:   ",
  "resting:          ",
  "using arrows:     ",
  "experimenting:    ",
  "retreating:       ",
  "waking monsters:  ",
  "hoarding food:    "
};
/* Door search map */
char timessearched[MAXROWS][MAXCOLS], timestosearch;
int  searchstartr = NONE, searchstartc = NONE, reusepsd=0;
int  new_mark=1, new_findroom=1, new_search=1, new_stairs=1, new_arch=1;

/* Results of last call to makemove() */
int  ontarget= 0, targetrow= NONE, targetcol= NONE;

/* Rog-O-Matics model of his stats */
int   Level = 0, MaxLevel = 0, Gold = 0, Hp = 12, Hpmax = 12;
int   Str = 16, Strmax = 16, Ac = 6, Exp = 0, Explev = 1, turns = 0;
char  Ms[30];	/* The message about his state of hunger */

/* Miscellaneous movement tables */
int   deltrc[8] = { 1,-(MAXCOLS-1),-MAXCOLS,-(MAXCOLS+1),-1,MAXCOLS-1,MAXCOLS,MAXCOLS+1 };
int   deltc[8]  = { 1, 1, 0, -1, -1, -1, 0, 1 };
int   deltr[8]  = { 0, -1, -1, -1, 0, 1, 1, 1 };
char  keydir[8] = { 'l', 'u', 'k', 'y', 'h', 'b', 'j', 'n' };
int   movedir;

/* Map characters on screen into object types */
stuff translate[128] = {
  /* \00x */  none, none, none, none, none, none, none, none,
  /* \01x */ none, none, none, none, none, none, none, none,
  /* \02x */ none, none, none, none, none, none, none, none,
  /* \03x */ none, none, none, none, none, none, none, none,
  /* \04x */ none, potion, none, none, none, none, none, none,
  /* \05x */ hitter, hitter, gold, none, amulet, none, none, wand,
  /* \06x */ none, none, none, none, none, none, none, none,
  /* \07x */ none, none, food, none, none, ring, none, Scroll,
  /* \10x */ none, none, none, none, none, none, none, none,
  /* \11x */ none, none, none, none, none, none, none, none,
  /* \12x */ none, none, none, none, none, none, none, none,
  /* \13x */ none, none, none, armor, none, armor, none, none,
  /* \14x */ none, none, none, none, none, none, none, none,
  /* \15x */ none, none, none, none, none, none, none, none,
  /* \16x */ none, none, none, none, none, none, none, none,
  /* \17x */ none, none, none, none, none, none, none, none
};

/* Inventory, contents of our pack */
invrec inven[MAXINV]; int invcount = 0;

/* Time history */
timerec timespent[50];

/* End of the game messages */
char *termination = "perditus";
char *gamename = "Rog-O-Matic";
char roguename[100];

/* Used by onintr() to restart Rgm at top of command loop */
jmp_buf  commandtop;

/*
 * Main program
 */

GAME_MAIN(argc, argv, env)
int   argc;
char *argv[];
char *env[];
{
  char  ch, *s, *getenv(), *statusline(), msg[128];
  int startingup = 1;
  register int  i;

  if (argc <= 2) {
     printf("Improper usage\n");
     exit(1);
  }

  char* env_value;
  if ((env_value = getenv("ROGOMATIC_DEBUG_PROTOCOL")) != NULL) {
      g_debug_protocol = strcmp(env_value, "true") == 0;
  }
  if ((env_value = getenv("ROGOMATIC_DELAY")) != NULL) {
      g_move_delay = atoi(env_value);
  }
  if ((env_value = getenv("ROGOMATIC_LEVEL_DELAY")) != NULL) {
      g_level_delay = atoi(env_value);
  }

  debuglog_open ("debuglog.player");

  /*
   * Initialize some storage
   */

  sprintf (genocided, "");
  sprintf (lastcmd, "i");
  sprintf (ourkiller, "unknown");
  sprintf (sumline, "");
  sprintf (versionstr, "");

  for (i = MAXCOLS * 24; i--; ) screen[0][i] = ' ';

  /*
   * Get the process id of this player program if the
   * environment variable is set which requests this be
   * done.  Then create the file name with the PID so
   * that the debugging scripts can find it and use the
   * PID.
   *
   * This code can be removed if you don't need to use
   * the debugging scripts.
   *
   */

  /* Process ID */
  pid_t pid;
  char pidfilename[NAMSIZ];
  FILE *pidfp;

  if (getenv("GETROGOMATICPID") != NULL) {
    pid = getpid ();
    memset (pidfilename, '\0', NAMSIZ);
    sprintf (pidfilename, "rogomaticpid.%d", pid);
    if ((pidfp = fopen (pidfilename, "w")) == NULL) {
      fprintf (stderr, "Can't open '%s'.\n", pidfilename);
      exit(1);
    }
  }

  /*
   * The first argument to player is a two character string encoding
   * the file descriptors of the pipe ends. See setup.c for call.
   *
   * If we get 'ZZ', then we are replaying an old game, and there
   * are no pipes to read/write.
   */

  if (argv[1][0] == 'Z') {
    replaying = 1;
    gamename = "Iteratum Rog-O-Maticus";
    termination = "finis";
    strcpy (logfilename, argv[4]);
    startreplay (&logfile, logfilename);
  }
  else {
    int frogue_fd = argv[1][0] - 'a';
    int trogue_fd = argv[1][1] - 'a';
    open_frogue_fd (frogue_fd);
    trogue = fdopen (trogue_fd, "wb");
    setbuf (trogue, NULL);
    open_frogue_debuglog ("debuglog.frogue");
  }

  /* The second argument to player is the process id of Rogue */
  if (argc > 2) rogpid = atoi (argv[2]);

  /* The third argument is an option list */
  if (argc > 3) sscanf (argv[3], "%d,%d,%d,%d,%d,%d,%d,%d",
                          &cheat, &noterm, &g_debug, &nohalf,
                          &emacs, &terse, &transparent, &quitat);
  if (g_debug) {
      debugging = D_NORMAL | D_WARNING;
      startecho = 1;
  }

  /* The fourth argument is the Rogue name */
  if (argc > 4)	strcpy (roguename, argv[4]);
  else		sprintf (roguename, "Rog-O-Matic %s", RGMVER);

#ifndef ROGUE_COLLECTION
  /* Now count argument space and assign a global pointer to it */
  arglen = 0;

  for (i=0; i<argc; i++) {
    register int len = strlen (argv[i]);
    arglen += len + 1;

    while (len >= 0) argv[i][len--] = ' ';
  }

  parmstr = argv[0];	arglen--;
  parmstr[arglen] = '\0';/* I don't like this business with muck with ps, but
                            I think the lack of a null is a problem - NYM */
#endif
  /* If we are in one-line mode, then squirrel away stdout */
  if (emacs || terse) {
    realstdout = fdopen (dup (fileno (stdout)), "w");
    freopen ("/dev/null", "w", stdout);
  }

  initscr (); crmode (); noecho ();	/* Initialize the Curses package */

  saynow("Rog-O-Matic is waiting for Rogue to start...");

  if (startecho)
    toggleecho ();		/* Start logging? */

  clear ();				/* Clear the screen */
  getrogver ();				/* Figure out Rogue version */

  if (!replaying) {
    initseed();
    restoreltm ();			/* Get long term memory of version */
    startlesson ();			/* Start genetic learning */
  }

  /*
   * Give a hello message
   */

  if (replaying)
    sprintf (msg, " Replaying log file %s, version %s.",
             logfilename, versionstr);
  else
    sprintf (msg, " %s: version %s, genotype %d, quit at %d.",
             roguename, versionstr, geneid, quitat);

  if (emacs)
    { fprintf (realstdout, "%s  (%%b)", msg); fflush (realstdout); }
  else if (terse)
    { fprintf (realstdout, "%s\n", msg); fflush (realstdout); }
  else
    { saynow (msg); }

#ifdef ROGUE_COLLECTION
  debuglog("--seed %d --genes \"%d %d %d %d %d %d %d %d\"\n",
      g_seed, knob[0], knob[1], knob[2], knob[3],
      knob[4], knob[5], knob[6], knob[7]);
#endif

  /*
   * Now that we have the version figured out, we can properly
   * interpret the screen.  Force a redraw by sending a redraw
   * screen command (^L for old, ^R for new).
   *
   * Also identify wands (/), so that we can differentiate
   * older Rogue 3.6 from Rogue 3.6 with extra magic...
   */
  if (version < RV53A)
      sendnow("%c//;", get_redraw_key());
  else
      sendnow("%c;", get_redraw_key());

  /*
   * If we are not replaying an old game, we must position the
   * input after the next form feed, which signals the start of
   * the level drawing.
   */
  {
    if (!replaying)
      while ((int) (ch = getroguetoken ()) != CL_TOK && (int) ch != EOF) {
        /* FIXME: If you start next to a monster this will get stuck, as
           pressing 'v' takes time in version 3.6, so rogue will be waiting
           for input and we will be waiting for rogue to print a CL_TOK,
           so deadlock - NYM */
      }
  }

  /*
   * Note: If we are replaying, the logfile is now in synch
   */
  getrogue (ill, 2);  /* Read the input up to end of first command */

  /* Identify all 26 monsters */
  if (!replaying)
    for (ch = 'A'; ch <= 'Z'; ch++) rogo_send ("/%c", ch);

  /*
   * Signal handling. On an interrupt, Rogomatic goes into transparent
   * mode and clears what state information it can. This code is styled
   * after that in "UNIX Programming -- Second Edition" by Brian
   * Kernigan & Dennis Ritchie. I sure wouldn't have thought of it.
   */

  istat = signal (SIGINT, SIG_IGN); /* save original status */
  setjmp (commandtop);              /* save stack position */

  if (istat != SIG_IGN)
    signal (SIGINT, onintr);

  if (interrupted) {
    saynow ("Interrupt [enter command]:");
    interrupted = 0;
    transparent = 1;
  }

  if (transparent) noterm = 0;

  while (playing) {
    refresh ();

    /* If we have any commands to send, send them */
    while (resend ()) {
      if (startingup) showcommand (lastcmd);

      sendnow (";");
      getrogue (ill, 2);
      if (ourscore == 0)
        check_frogue_sync();
    }

    if (startingup) {	/* All monsters identified */
      versiondep ();			/* Do version specific things */
      startingup = 0;			/* Clear starting flag */
    }

    if (!playing) break;	/* In case we died */

    /*
     * No more stored commands, so either get a command from the
     * user (if we are in transparent mode or the user has typed
     * something), or let the strategize module try its luck. If
     * strategize fails we wait for the user to type something. If
     * there is no user (noterm mode) then use ROGQUIT to signal a
     * quit command.
     */

    if ((transparent && !singlestep) ||
        (!emacs && charsavail ()) ||
        !strategize()) {
      ch = (noterm) ? ROGQUIT : getch ();

      switch (ch) {
        case '?': givehelp (); break;

        case '\r':
        case '\n': if (terse)
            { printsnap (realstdout); fflush (realstdout); }
          else
            { singlestep = 1; transparent = 1; }

          break;

          /* Rogue Command Characters */
        case 'H': case 'J': case 'K': case 'L':
        case 'Y': case 'U': case 'B': case 'N':
        case 'h': case 'j': case 'k': case 'l':
        case 'y': case 'u': case 'b': case 'n':
        case 's': command (T_OTHER, "%c", ch); transparent = 1; break;

        case 'f': ch = getch ();

          for (s = "hjklyubnHJKLYUBN"; *s; s++) {
            if (ch == *s) {
              if (run_uses_f_prefix()) command (T_OTHER, "f%c", ch);
              else                 command (T_OTHER, "%c", ctrl (ch));
            }
          }

          transparent = 1; break;

        case '\f':  redrawscreen (); break;

        case 'm':   dumpmonstertable (); break;

        case 'M':   dumpmazedoor (); break;

        case '>': if (atrow == stairrow && atcol == staircol)
            command (T_OTHER, ">");

          transparent = 1; break;

        case '<': if (atrow == stairrow && atcol == staircol &&
                        have (amulet) != NONE) command (T_OTHER, "<");

          transparent = 1; break;

        case 't': transparent = !transparent; break;

        case ')': new_mark++; markcycles (DOPRINT); at (row, col); break;

        case '+': setpsd (DOPRINT); at (row, col); break;

        case 'A': attempt = (attempt+1) % 5;
          saynow ("Attempt %d", attempt); break;

        case 'G': mvprintw (0, 0,
                              "%d: Sr %d Dr %d Re %d Ar %d Ex %d Rn %d Wk %d Fd %d, %d/%d",
                              geneid, k_srch, k_door, k_rest, k_arch,
                              k_exper, k_run, k_wake, k_food, genebest, geneavg);
          clrtoeol (); at (row, col); refresh (); break;

        case ':': chicken = !chicken;
          say (chicken ? "chicken" : "aggressive");
          break;

        case '~': if (replaying)
            saynow ("Replaying log file %s, version %s.",
                    logfilename, versionstr);
          else
            saynow (" %s: version %s, genotype %d, quit at %d.",
                    roguename, versionstr, geneid, quitat);

          break;

        case '[': at (0,0);
          printw ("%s = %d, %s = %d, %s = %d, %s = %d.",
                  "hitstokill", hitstokill,
                  "goodweapon", goodweapon,
                  "usingarrow", usingarrow,
                  "goodarrow", goodarrow);
          clrtoeol ();
          at (row, col);
          refresh ();
          break;

        case '-': saynow (statusline ());
          break;

        case '`': clear ();
          summary ((FILE *) NULL, '\n');
          pauserogue ();
          break;

        case '|': clear ();
          timehistory ((FILE *) NULL, '\n');
          pauserogue ();
          break;

        case 'r': resetinv (); say ("Inventory reset."); break;

        case 'i': clear (); dumpinv ((FILE *) NULL); pauserogue (); break;

        case '/': dosnapshot ();
          break;

        case '(': clear (); dumpdatabase (); pauserogue (); break;

        case 'c': cheat = !cheat;
          say (cheat ? "cheating" : "righteous");
          break;

        case 'd': toggledebug ();	break;

        case 'e': toggleecho ();        break;

        case '!': dumpstuff ();         break;

        case '@': dumpmonster ();       break;

        case '#': dumpwalls ();         break;

        case '%': clear (); havearmor (1, DOPRINT, ANY); pauserogue (); break;

        case ']': clear (); havearmor (1, DOPRINT, RUSTPROOF);
          pauserogue (); break;

        case '=': clear (); havering (1, DOPRINT); pauserogue (); break;

        case '$': clear (); haveweapon (1, DOPRINT); pauserogue (); break;

        case '^': clear (); havebow (1, DOPRINT); pauserogue (); break;

        case '{': promptforflags (); break;

        case '&': saynow ("Object count is %d.", objcount); break;

        case '*': blinded = !blinded;
          saynow (blinded ? "blinded" : "sighted");
          break;

        case 'C': cosmic = !cosmic;
          saynow (cosmic ? "cosmic" : "boring");
          break;

        case 'E': dwait (D_ERROR, "Testing the ERROR trap..."); break;

        case 'F': dwait (D_FATAL, "Testing the FATAL trap..."); break;

        case 'R': if (replaying) {
            positionreplay (); getrogue (ill, 2);

            if (transparent) singlestep = 1;
          }
          else
            saynow ("Replay position only works in replay mode.");

          break;

#ifndef ROGUE_COLLECTION
        case 'S': quitrogue ("saved", Gold, SAVED);
          playing = 0; break;
#endif

        case 'Q': quitrogue ("user typing quit", Gold, FINISHED);
          playing = 0; break;

        case ROGQUIT: dwait (D_ERROR, "Strategize failed, gave up.");
          quitrogue ("gave up", Gold, SAVED); break;
      }
    }
    else {
      singlestep = 0;
    }
  }

  if (! replaying) {
    saveltm (Gold);			/* Save new long term memory */
    endlesson ();			/* End genetic learning */
  }

  /* Print termination messages */
  at (23, 0);
  clrtoeol ();
//  clear ();
  refresh ();
  endwin (); nocrmode (); noraw (); echo ();

  if (emacs) {
    if (*sumline) fprintf (realstdout, " %s", sumline);
  }
  else if (terse) {
    if (*sumline) fprintf (realstdout, "%s\n",sumline);

    fprintf (realstdout, "%s %s est.\n", gamename, termination);
  }
  else {
    if (*sumline) printf ("%s\n",sumline);

    printf ("%s %s est.\n", gamename, termination);
  }

  /*
   * Rename log file, if it is open
   */

  if (logging) {
    char lognam[128];

    /* Make up a new log file name */
    sprintf (lognam, "%0.4s.%d.%d", ourkiller, MaxLevel, ourscore);

    /* Close the open file */
    toggleecho ();

#ifndef _WIN32
    /* Rename the log file */
    if (link (ROGUELOG, lognam) == 0) {
      unlink (ROGUELOG);
      printf ("Log file left on %s\n", lognam);
    }
    else
      printf ("Log file left on %s\n", ROGUELOG);
#endif
  }

  close_frogue_debuglog ();
  debuglog_close ();

  flushinp();
  sendnow("\n");
#ifdef ROGUE_COLLECTION
  sendcnow(EOF);
#endif
  exit (0);
}

wait_for(ch)
register char ch;
{
    register char c;

    if (ch == '\n')
        while ((c = md_readchar(stdscr)) != '\n' && c != '\r')
            continue;
    else
        while (md_readchar(stdscr) != ch)
            continue;
}

/*
 * onintr: The SIGINT handler. Pass interrupts to main loop, setting
 * transparent mode. Also send some synchronization characters to Rogue,
 * and reset some goal variables.
 */

void onintr (int sig)
{
  sendnow ("n\033");            /* Tell Rogue we don't want to quit */
  refresh ();                   /* Clear terminal output */
  clearsendqueue ();            /* Clear command queue */
  setnewgoal ();                /* Don't believe ex */
  transparent = 1;              /* Drop into transprent mode */
  interrupted = 1;              /* Mark as an interrupt */
  noterm = 0;                   /* Allow commands */
  longjmp (commandtop,0);       /* Back to command Process */
}

void initseed()
{
  /* set up random number generation */
  if (getenv("SEED") != NULL) {
    /* if we want repeatable results for testing set
       the environment variable SEED to some positive integer
       value and use a version of rogue that also uses a SEED
       environment variable.  this makes testing so much easier... */
    g_seed = atoi(getenv("SEED"));
    rogo_srand(g_seed);
  }
  else
    /* Start random number generator based upon the current time */
    rogo_srand (0);
}

/*
 * startlesson: Genetic learning algorithm, pick a genotype to
 * test this game, and set the parameters (or "knobs") accordingly.
 */

void startlesson ()
{
  sprintf (genelog, "%s/GeneLog%d", getRgmDir (), version);
  sprintf (genepool, "%s/GenePool%d", getRgmDir (), version);
  sprintf (genelock, "%s/GeneLock%d", getRgmDir (), version);

  critical ();				/* Disable interrupts */

  /* mdk: allow genes to be set through the environment for deterministic testing */
  char* genes_var = getenv("GENES");
  if (genes_var != NULL) {
      sscanf(genes_var, "%d %d %d %d %d %d %d %d",
          &knob[0], &knob[1], &knob[2], &knob[3],
          &knob[4], &knob[5], &knob[6], &knob[7]);
  }
  /* Serialize access to the gene pool */
  else if (lock_file (genelock, MAXLOCK)) {	/* Lock the gene pool */
    if (rogo_openlog (genelog) == NULL)	/* Open the gene log file */
      saynow ("Could not open file %s", genelog);

    if (! readgenes (genepool))		/* Read the gene pool */
      initpool (MAXKNOB, 20);		/* Random starting point */

    setknobs (&geneid, knob, &genebest, &geneavg); /* Select a genotype */
    writegenes (genepool);		/* Write out the gene pool */
    rogo_closelog ();			/* Close the gene log file */
    unlock_file (genelock);		/* Unlock the gene pool */
  }
  else
    fprintf (stderr, "Cannot lock gene pool to read '%s'\n", genepool);

  uncritical ();			/* Reenable interrupts */

  /* Cache the parameters for easier use */
  k_srch = knob[K_SRCH];	k_door = knob[K_DOOR];
  k_rest = knob[K_REST];	k_arch = knob[K_ARCH];
  k_exper = knob[K_EXPER];	k_run = knob[K_RUN];
  k_wake = knob[K_WAKE];	k_food = knob[K_FOOD];
}

/*
 * endlesson: if killed, total winner, or quit for scoreboard,
 * evaluate the performance of this genotype and save in genepool.
 */

void endlesson ()
{
  if (geneid > 0 &&
      (stlmatch (termination, "perditus") ||
       stlmatch (termination, "victorius") ||
       stlmatch (termination, "callidus"))) {
    critical ();			/* Disable interrupts */

    if (lock_file (genelock, MAXLOCK)) {	/* Lock the score file */
      rogo_openlog (genelog);		/* Open the gene log file */

      if (readgenes (genepool)) {	/* Read the gene pool */
        evalknobs (geneid,Gold,Level);	/* Add the trial to the pool */
        writegenes (genepool);
      }	/* Write out the gene pool */

      rogo_closelog ();
      unlock_file (genelock);		/* Disable interrupts */
    }
    else
      fprintf (stderr, "Cannot lock gene pool to evaluate '%s'\n", genepool);

    uncritical ();			/* Re-enable interrupts */
  }
}
