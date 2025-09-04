/*
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

/*
 * utility.c:
 *
 * This file contains all of the miscellaneous system functions which
 * determine the baud rate, time of day, etc.
 *
 * If CMU is not defined, then various functions from libcmu.a are
 * defined here (otherwise the functions from -lcmu are used).
 */

# include <curses.h>
#ifndef _WIN32
# include <pwd.h>
#else
# include <io.h>
#endif
# include <stdio.h>
# include <stdlib.h>
# include <signal.h>
# include <string.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <time.h>

# include "install.h"
# include "rogomatic.h"


# define TRUE 1
# define FALSE 0

const char* tmode_to_str(int tmode)
{
  switch (tmode)
  {
    case T_OTHER:
      return "OTHER";
    case T_HANDLING:
      return "HANDLING";
    case T_FIGHTING:
      return "FIGHTING";
    case T_RESTING:
      return "RESTING";
    case T_MOVING:
      return "MOVING";
    case T_EXPLORING:
      return "EXPLORING";
    case T_RUNNING:
      return "RUNNING";
    case T_GROPING:
      return "GROPING";
    case T_SEARCHING:
      return "SEARCHING";
    case T_DOORSRCH:
      return "DOORSRCH";
    case T_LISTLEN:
      return "LISTLEN";
  }

  return "BAD_TMODE_PARAM";
}

const char* get_move_type_str(int movetype)
{
    switch (movetype)
    {
    case EXPLORE:
        return "EXPLORE";
    case EXPLORERUN:
        return "EXPLORERUN";
    case RUNTODOOR:
        return "RUNTODOOR";
    case RUNAWAY:
        return "RUNAWAY";
    case SECRETDOOR:
        return "SECRETDOOR";
    case FINDSAFE:
        return "FINDSAFE";
    case GOTOMOVE:
        return "GOTOMOVE";
    case ATTACKSLEEP:
        return "ATTACKSLEEP";
    case ARCHERYMOVE:
        return "ARCHERYMOVE";
    case UNPIN:
        return "UNPIN";
    case UNPINEXP:
        return "UNPINEXP";
    case EXPLOREROOM:
        return "EXPLOREROOM";
    case FINDROOM:
        return "FINDROOM";
    case RESTMOVE:
        return "RESTMOVE";
    case DOWNMOVE:
        return "DOWNMOVE";
    case RUNDOWN:
        return "RUNDOWN";
    case NUMMOVES:
        return "NUMMOVES";
    }

    return "BAD_TYPE";
}

const char* get_debug_str(int d)
{
    switch (d)
    {
    case D_FATAL:
        return "D_FATAL";
    case D_ERROR:
        return "D_ERROR";
    case D_WARNING:
        return "D_WARNING";
    case D_INFORM:
        return "D_INFORM";
    case D_SEARCH:
        return "D_SEARCH";
    case D_BATTLE:
        return "D_BATTLE";
    case D_MESSAGE:
        return "D_MESSAGE";
    case D_PACK:
        return "D_PACK";
    case D_CONTROL:
        return "D_CONTROL";
    case D_SCREEN:
        return "D_SCREEN";
    case D_MONSTER:
        return "D_MONSTER";
    case D_SAY:
        return "D_SAY";
    case D_SCROLL:
        return "D_SCROLL";
    case D_POTION:
        return "D_POTION";
    case D_WAND:
        return "D_WAND";
    case D_RING:
        return "D_RING";
    }

    return "BAD_TYPE";
}

/*
 * rogo_baudrate: Determine the baud rate of the terminal
 */

rogo_baudrate ()
{
#ifndef _WIN32
  return (baudrate());
#else
  return 0;
#endif
}

/*
 * getname: get userid of player.
 */

char *getname ()
{
#ifndef ROGUE_COLLECTION
  static char name[100];
  int   i;
  struct passwd *passwd;

  passwd = getpwuid (getuid ());
  strncpy (name, passwd->pw_name, 100);
//  getpw (getuid (), name);
//  i = 0;
//  while (name[i] != ':' && name[i] != ',')
//    i++;
//  name[i] = '\0';

  return (name);
#else
  return "rogomatic";
#endif
}

/*
 * wopen: Open a file for world access.
 */

FILE *wopen(fname, mode)
char *fname, *mode;
{
  int oldmask;
  FILE *newlog;

  oldmask = umask (0111);
  newlog = fopen (fname, mode);
  umask (oldmask);

  return (newlog);
}

/*
 * fexists: return a boolean if the named file exists
 */

int fexists (fn)
char *fn;
{
  struct stat pbuf;

  return (stat (fn, &pbuf) == 0);
}

/*
 * getfilelength: Do a stat and return the length of a file.
 */

int getfilelength (f)
char *f;
{
  struct stat sbuf;

  if (stat (f, &sbuf) == 0)
    return (sbuf.st_size);
  else
    return (-1);
}

/*
 * critical: Disable interrupts
 */

static void   (*hstat)(int), (*istat)(int), (*qstat)(int), (*pstat)(int);

void critical ()
{
// FIXME: when uncommented, get bus errors :(
//  hstat = signal (SIGHUP, SIG_IGN);
//  istat = signal (SIGINT, SIG_IGN);
//  pstat = signal (SIGPIPE, SIG_IGN);
//  qstat = signal (SIGQUIT, SIG_IGN);
}

/*
 * uncritical: Enable interrupts
 */

void uncritical ()
{
// FIXME: when uncommented, get bus errors :(
//  signal (SIGHUP, hstat);
//  signal (SIGINT, istat);
//  signal (SIGPIPE, pstat);
//  signal (SIGQUIT, qstat);
}

/*
 * reset_int: Set all interrupts to default
 */

void reset_int ()
{
#ifndef _WIN32
  signal (SIGHUP, SIG_DFL);
  signal (SIGINT, SIG_DFL);
  signal (SIGPIPE, SIG_DFL);
  signal (SIGQUIT, SIG_DFL);
#endif
}

/*
 * int_exit: Set up a function to call if we get an interrupt
 */

void int_exit (exitproc)
void (*exitproc)(int);
{
#ifndef _WIN32
  if (signal (SIGHUP, SIG_IGN) != SIG_IGN)  signal (SIGHUP, exitproc);

  if (signal (SIGINT, SIG_IGN) != SIG_IGN)  signal (SIGINT, exitproc);

  if (signal (SIGPIPE, SIG_IGN) != SIG_IGN) signal (SIGPIPE, exitproc);

  if (signal (SIGQUIT, SIG_IGN) != SIG_IGN) signal (SIGQUIT, exitproc);
#endif
}

/*
 * lock_file: lock a file for a maximum number of seconds.
 *            Based on the method used in Rogue 5.2.
 */

# define NOWRITE 0

int lock_file (lokfil, maxtime)
const char *lokfil;
int maxtime;
{

  int try;

  struct stat statbuf;
  time_t time ();

start:

  if (creat (lokfil, NOWRITE) > 0)
    return TRUE;

  for (try = 0; try < 60; try++) {
          md_sleep (1);

          if (creat (lokfil, NOWRITE) > 0)
            return TRUE;
        }

  if (stat (lokfil, &statbuf) < 0) {
    creat (lokfil, NOWRITE);
    return TRUE;
  }

  if (time (NULL) - statbuf.st_mtime > maxtime) {
    if (unlink (lokfil) < 0)
      return FALSE;

    goto start;
  }
  else
    return FALSE;
}

/*
 * unlock_file: Unlock a lock file.
 */

void unlock_file (lokfil)
const char *lokfil;
{
  md_unlink (lokfil);
}

# ifndef CMU
/*
 * quit: Defined for compatibility with Berkeley 4.2 system
 */

/* VARARGS2 */
void quit (int code, char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vfprintf (stderr, fmt, args);
  va_end(args);

  exit (code);
}

/*
 * stlmatch  --  match leftmost part of string
 *
 *  Usage:  i = stlmatch (big,small)
 *	int i;
 *	char *small, *big;
 *
 *  Returns 1 iff initial characters of big match small exactly;
 *  else 0.
 *
 *  HISTORY
 * 18-May-82 Michael Mauldin (mlm) at Carnegie-Mellon University
 *      Ripped out of CMU lib for Rog-O-Matic portability
 * 20-Nov-79  Steven Shafer (sas) at Carnegie-Mellon University
 *	Rewritten for VAX from Ken Greer's routine.
 *
 *  Originally from klg (Ken Greer) on IUS/SUS UNIX
 */

int stlmatch (big, small)
char *small, *big;
{
  register char *s, *b;
  s = small;
  b = big;

  do {
    if (*s == '\0')
      return (1);
  }
  while (*s++ == *b++);

  return (0);
}
# endif

/*
 *  putenv  --  put value into environment
 *
 *  Usage:  i = putenv (name,value)
 *	int i;
 *	char *name, *value;
 *
 *  Putenv associates "value" with the environment parameter "name".
 *  If "value" is 0, then "name" will be deleted from the environment.
 *  Putenv returns 0 normally, -1 on error (not enough core for malloc).
 *
 *  Putenv may need to add a new name into the environment, or to
 *  associate a value longer than the current value with a particular
 *  name.  So, to make life simpler, putenv() copies your entire
 *  environment into the heap (i.e. malloc()) from the stack
 *  (i.e. where it resides when your process is initiated) the first
 *  time you call it.
 *
 *  HISTORY
 * 25-Nov-82 Michael Mauldin (mlm) at Carnegie-Mellon University
 *      Ripped out of CMU lib for Rog-O-Matic portability
 * 20-Nov-79  Steven Shafer (sas) at Carnegie-Mellon University
 *	Created for VAX.  Too bad Bell Labs didn't provide this.  It's
 *	unfortunate that you have to copy the whole environment onto the
 *	heap, but the bookkeeping-and-not-so-much-copying approach turns
 *	out to be much hairier.  So, I decided to do the simple thing,
 *	copying the entire environment onto the heap the first time you
 *	call putenv(), then doing realloc() uniformly later on.
 *	Note that "putenv(name,getenv(name))" is a no-op; that's the reason
 *	for the use of a 0 pointer to tell putenv() to delete an entry.
 *
 */

#define EXTRASIZE 5		/* increment to add to env. size */

static int  envsize = -1;	/* current size of environment */
#ifndef _WIN32
extern char **environ;		/* the global which is your env. */
#endif

static int  findenv (char *name);		/* look for a name in the env. */
static int  newenv (void);		/* copy env. from stack to heap */
static int  moreenv (void);		/* incr. size of env. */

int  rogo_putenv (name, value)
char *name, *value;
{
  register int  i, j;
  register char *p;

  if (envsize < 0) {
    /* first time putenv called */
    if (newenv () < 0)		/* copy env. to heap */
      return (-1);
  }

  i = findenv (name);		/* look for name in environment */

  if (value) {
    /* put value into environment */
    if (i < 0) {
      /* name must be added */
      for (i = 0; environ[i]; i++);

      if (i >= (envsize - 1)) {
        /* need new slot */
        if (moreenv () < 0)
          return (-1);
      }

      p = malloc (strlen (name) + strlen (value) + 2);

      if (p == 0)		/* not enough core */
        return (-1);

      environ[i + 1] = 0;	/* new end of env. */
    }
    else {
      /* name already in env. */
      p = realloc (environ[i],
                   strlen (name) + strlen (value) + 2);

      if (p == 0)
        return (-1);
    }

    sprintf (p, "%s=%s", name, value);/* copy into env. */
    environ[i] = p;
  }
  else {
    /* delete name from environment */
    if (i >= 0) {
      /* name is currently in env. */
      free (environ[i]);

      for (j = i; environ[j]; j++);

      environ[i] = environ[j - 1];
      environ[j - 1] = 0;
    }
  }

  return (0);
}

static int  findenv (name)
char *name;
{
  register char *namechar, *envchar;
  register int  i, found;

  found = 0;

  for (i = 0; environ[i] && !found; i++) {
    envchar = environ[i];
    namechar = name;

    while (*namechar && (*namechar == *envchar)) {
      namechar++;
      envchar++;
    }

    found = (*namechar == '\0' && *envchar == '=');
  }

  return (found ? i - 1 : -1);
}

static int  newenv ()
{
  register char **env, *elem;
  register int  i, esize;

  for (i = 0; environ[i]; i++);

  esize = i + EXTRASIZE + 1;
  env = (char **) malloc (esize * sizeof (elem));

  if (env == 0)
    return (-1);

  for (i = 0; environ[i]; i++) {
    elem = malloc (strlen (environ[i]) + 1);

    if (elem == 0)
      return (-1);

    env[i] = elem;
    strcpy (elem, environ[i]);
  }

  env[i] = 0;
  environ = env;
  envsize = esize;
  return (0);
}

static int  moreenv ()
{
  register int  esize;
  register char **env;

  esize = envsize + EXTRASIZE;
  env = (char **) realloc (environ, esize * sizeof (*env));

  if (env == 0)
    return (-1);

  environ = env;
  envsize = esize;
  return (0);
}
