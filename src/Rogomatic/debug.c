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
 * debug.c:
 *
 * This file contains the code for the debugger.  Rogomatic has one of
 * the tensest internal debuggers around, because in the early days it
 * had an incredible number of bugs, with no way to repeat an error
 * (because Rogue uses a different dungeon each time).
 */

# include <curses.h>
# include <setjmp.h>
# include <string.h>
# include "types.h"
# include "globals.h"
# include "install.h"

/*
 * Debugging wait loop: Handle the usual Rogomatic command chars, and also
 * allows dumping the flags '^' command. Exits when a non-command char is
 * typed. To use, just put a "dwait (type, "message");" wherever you need
 * debugging messages, and hit a space or a cr to continue
 */

/* VARARGS2 */
int dwait (int msgtype, char* f, ...)
{
  char msg[128];
  int r, c;

  /* Build the actual message */
  va_list args;
  va_start(args, f);
  vsprintf (msg, f, args);
  va_end(args);

  /* Log the message if the error is severe enough */
  if (!replaying && (msgtype & (D_FATAL | D_ERROR | D_WARNING))) {
    char errfn[128]; FILE *errfil;

    sprintf (errfn, "%s/error%s", getRgmDir (), versionstr);

    if ((errfil = wopen (errfn, "a")) != NULL) {
      fprintf (errfil, "User %s, error type %d:  %s\n\n",
               getname(), msgtype, msg);

      if (msgtype & (D_FATAL | D_ERROR)) {
        dosnapshot();
        printsnap (errfil);
        summary (errfil, NEWLINE);
        fprintf (errfil, "\f\n");
      }

      fclose (errfil);
    }
  }

  if (msgtype & D_FATAL) {
    extern jmp_buf commandtop;			/* From play */
    saynow (msg);
    playing = 0;
    quitrogue ("fatal error trap", Gold, SAVED);
    longjmp (commandtop, 0);
  }

  if (! debug (msgtype | D_INFORM)) {	/* If debugoff */
    if (msgtype & D_SAY)			  /* Echo? */
      { saynow (msg); return (1); }		  /* Yes => win */

    return (0);					  /* No => lose */
  }

  if (*msg) { mvaddstr (0, 0, msg); clrtoeol (); }	/* Write msg */

  if (noterm) return (1);				/* Exit if no user */

  /* Debugging loop, accept debugging commands from user */
  while (1) {
    refresh ();

    printf("Waiting for debug command...\n");
    switch (getch()/*fgetc (stdin)*/) {
      case '?':
        say ("i=inv, d=debug !=stf, @=mon, #=wls, $=id, ^=flg, &=chr");
        break;
      case 'i': at (1,0); dumpinv ((FILE *) NULL); at (row, col); break;
      case 'd': toggledebug (); 	break;
      case 't': transparent = 1;        break;
      case '!': dumpstuff ();           break;
      case '@': dumpmonster ();         break;
      case '#': dumpwalls ();           break;
      case '^': promptforflags ();	break;
      case '&':

        if (getscrpos ("char", &r, &c))
          saynow ("Char at %d,%d '%c'", r, c, screen[r][c]);

        break;
      case '(': dumpdatabase (); at (row, col); break;
      case ')': new_mark++; markcycles (DOPRINT); at (row, col); break;
      case '~': saynow ("Version %d, quit at %d", version, quitat); break;
      case '/': dosnapshot (); break;
      default: at (row, col); return (1);
    }
  }
}

/*
 * promptforflags: Prompt the user for a location and dump its flags.
 */

void promptforflags ()
{
  int r, c;

  if (getscrpos ("flags", &r, &c)) {
    mvprintw (0, 0, "Flags for %d,%d ", r, c);
    dumpflags (r, c);
    clrtoeol ();
    at (row, col);
  }
}

/*
 * dumpflags: Create a message line for the scrmap flags of a particular
 *            square.  Note that the fnames[] array must match the
 *            various flags defined in "types.h".
 */

char *fnames[] = {
  "been",    "cango",    "door",     "hall",     "psd",     "room",
  "safe",    "seen",     "deadend",  "stuff",    "trap",    "arrow",
  "trapdor", "teltrap",  "gastrap",  "beartrap", "dartrap", "waterap",
  "monster", "wall",     "useless",  "scarem",   "stairs",  "runok",
  "boundry", "sleeper",  "everclr"
};

void dumpflags (r, c)
int   r, c;
{
  char **f; int b;

  printw (":");

  for (f=fnames, b=1;   b<=EVERCLR;   b = b * 2, f++)
    if (scrmap[r][c] & b)
      printw ("%s:", *f);
}

/*
 * Timehistory: print a time analysis of the game.
 */

void timehistory (f, sep)
FILE *f;
char sep;
{
  register int i, j;
  char s[2048];

  timespent[0].timestamp = 0;

  sprintf (s, "Time Analysis: %s%c%c",
           "othr hand fght rest move expl rung grop srch door total",
           sep, sep);

  for (i=1; i<=MaxLevel; i++) {
    sprintf (s, "%slevel %2d:     ", s, i);

    for (j = T_OTHER; j < T_LISTLEN; j++)
      sprintf (s, "%s%5d", s, timespent[i].activity[j]);

    sprintf (s, "%s%6d%c",
             s, timespent[i].timestamp - timespent[i-1].timestamp, sep);
  }

  if (f == NULL)
    addstr (s);
  else
    fprintf (f, "%s", s);
}

/*
 * toggledebug: Set the value of the debugging word.
 */

void toggledebug ()
{
  char debugstr[100];
  int type = debugging & ~(D_FATAL | D_ERROR | D_WARNING);

  if (debugging == D_ALL)         debugging = D_NORMAL;
  else if (debugging == D_NORMAL) debugging = D_NORMAL | D_SEARCH;
  else if (type == D_SEARCH)      debugging = D_NORMAL | D_BATTLE;
  else if (type == D_BATTLE)      debugging = D_NORMAL | D_MESSAGE;
  else if (type == D_MESSAGE)     debugging = D_NORMAL | D_PACK;
  else if (type == D_PACK)        debugging = D_NORMAL | D_MONSTER;
  else if (type == D_MONSTER)     debugging = D_NORMAL | D_CONTROL;
  else if (type == D_CONTROL)     debugging = D_NORMAL | D_SCREEN;
  else if (type == D_SCREEN)      debugging = D_NORMAL | D_WARNING;
  else if (!debug (D_INFORM))     debugging = D_NORMAL | D_WARNING | D_INFORM;
  else                            debugging = D_ALL;

  strncpy (debugstr, "Debugging :", 100);

  if (debug(D_FATAL))     strcat (debugstr, "fatal:");

  if (debug(D_ERROR))     strcat (debugstr, "error:");

  if (debug(D_WARNING))   strcat (debugstr, "warn:");

  if (debug(D_INFORM))    strcat (debugstr, "info:");

  if (debug(D_SEARCH))    strcat (debugstr, "search:");

  if (debug(D_BATTLE))    strcat (debugstr, "battle:");

  if (debug(D_MESSAGE))   strcat (debugstr, "msg:");

  if (debug(D_PACK))      strcat (debugstr, "pack:");

  if (debug(D_CONTROL))   strcat (debugstr, "ctrl:");

  if (debug(D_SCREEN))    strcat (debugstr, "screen:");

  if (debug(D_MONSTER))   strcat (debugstr, "monster:");

  saynow (debugstr);
}

/*
 * getscrpos: Prompt the user for an x,y coordinate on the screen.
 */

int getscrpos (msg, r, c)
char *msg;
int *r, *c;
{
  char buf[256];

  saynow ("At %d,%d: enter 'row,col' for %s: ", atrow, atcol, msg);

  printf("Waiting for coord...\n");
  if (getnstr(buf, 256)/*fgets (buf, 256, stdin)*/) {
    sscanf (buf, "%d,%d", r, c);

    if (*r>=1 && *r<(MAXROWS-1) && *c>=0 && *c<=(MAXCOLS-1))
      return (1);
    else
      say ("%d,%d is not on the screen!", *r, *c);
  }

  at (row, col);
  return (0);
}
