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
 * replay.c:
 *
 * Make a table of offsets to the beginning of each level of a
 * Rog-O-Matic log file.
 */

# include <curses.h>
# include <ctype.h>
# include <string.h>
# include "types.h"
# include "globals.h"

# define MAXNUMLEV 50
# define FIRSTLEVSTR "\nR: "
# define NEWLEVSTR "\nR: {ff}"
# define POSITAT   "{ff}"

struct levstruct {
  long pos;
  int  level, gold, hp, hpmax, str, strmax, ac, explev, exp;
} levpos[MAXNUMLEV];
int numlev = 0;

/*
 * positionreplay: Called when user has typed the 'R' command, it fills
 * the level table by calling findlevel if necessary, and then positions
 * the log file to the level requested by the user.
 */

void positionreplay ()
{
  int curlev;
  long curpos;
  char cmd;

  /* Prompt user for a command character, read it, and lower case it */
  saynow ("Which level (f=first, p=previous, c=current, n=next, l=last): ");

  if (isupper ((cmd = getch ()))) cmd = tolower (cmd);

  /* Clear the prompt */
  saynow ("");

  /* If command is not in the list, clear the prompt and exit. */
  switch (cmd) {
    case 'f': case 'p': case 'c': case 'n': case 'l': break;
    default:  return;
  }

  /* Save the current position in the file */
  curpos = ftell (logfile);

  /* Read the log file, if we have not already done so */
  if (!logdigested) {
    saynow ("Reading whole log file to find levels...");

    if (!findlevel (logfile, levpos, &numlev, MAXNUMLEV)) {
      saynow ("Findlevel failed! Let's try to get back to where we were...");
      fseek (logfile, curpos, 0);
      return;
    }

    logdigested++;
  }

  /* Now figure out the current level (so relative commands will work) */
  for (curlev = 0; curlev < numlev-1; curlev++)
    if (levpos[curlev+1].pos > curpos) break;

  /* Now clear the screen, position the log file, and return */
  switch (cmd) {
    case 'f': fseek (logfile, levpos[0].pos, 0); break;

    case 'p': if (curlev > 0) fseek (logfile, levpos[curlev-1].pos, 0);
      else            fseek (logfile, levpos[0].pos, 0); break;

      break;
    case 'c': fseek (logfile, levpos[curlev].pos, 0); break;

    case 'n': if (curlev < numlev-1) fseek (logfile, levpos[curlev+1].pos, 0);
      else            fseek (logfile, levpos[curlev].pos, 0); break;

      break;
    case 'l': fseek (logfile, levpos[numlev-1].pos, 0);
      break;
    default:  fseek (logfile, 0L, 0);
  }

  clearscreen ();	/* Clear the screen */
  Level = -1; 		/* Force a newlevel() call */
}

/*
 * findlevel: Make a table of offsets to the various levels of a
 *             Rog-O-Matic log file.
 */

int findlevel (f, lvpos, nmlev, maxnum)
FILE *f;
struct levstruct *lvpos;
int *nmlev, maxnum;
{
  char ch;
  int l=0;

  *nmlev = 0;

  /* Position file after first newline */
  rewind (f);

  while ((ch = getc (f)) != '\n' && (int) ch != EOF);

  /* This is that start of level one */
  lvpos[l].pos = ftell (f);

  if (!findmatch (f, FIRSTLEVSTR)) {
    rewind (f);
    return (FAILURE);
  }

  fillstruct (f, &lvpos[l]);

  while (++l <= maxnum && findmatch (f, NEWLEVSTR)) {
    fseek (f, -(long)strlen (POSITAT), 1); /* mdk:bugfix signed fix */
    lvpos[l].pos = ftell (f);
    fillstruct (f, &lvpos[l]);
  }

  *nmlev = l;
  rewind (f);
  return (SUCCESS);
}

/*
 * fillstruct: scan the logfile from the current point, and fill in the
 * fields of a levstruct.
 */

void fillstruct (f, lev)
FILE *f;
struct levstruct *lev;
{
  lev->level  = 0;
  lev->gold   = 0;
  lev->hp     = 0;
  lev->hpmax  = 0;
  lev->str    = 0;
  lev->strmax = 0;
  lev->ac     = 0;
  lev->explev = 0;
  lev->exp    = 0;

  if (!findmatch (f, "Level:")) return;

  fscanf (f, "%d", &lev->level);

  if (!findmatch (f, "Gold:")) return;

  fscanf (f, "%d", &lev->gold);

  if (!findmatch (f, "Hp:")) return;

  fscanf (f, "%d(%d)", &lev->hp, &lev->hpmax);

  if (!findmatch (f, "Str:")) return;

  fscanf (f, "%d(%d)", &lev->str, &lev->strmax);

  if (!findmatch (f, ":")) return;		/* Armor class */

  fscanf (f, "%d", &lev->ac);

  if (!findmatch (f, "Exp:")) return;

  fscanf (f, "%d/%d", &lev->explev, &lev->exp);

  saynow ("Found level %d, has %d gold...", lev->level, lev->gold);
}

/*
 * findmatch: read from a stream until string 's' has been read. Returns 0
 * if EOF is read, and 1 if the match is found.  The stream is left
 * immediately after the matched string.
 *
 * Restriction: 's' must not contain prefix of itself as a substring.
 */

int findmatch (f, s)
FILE *f;
char *s;
{
  char *m = s, ch;

  while (*m && (int) (ch = fgetc (f)) != EOF)
    if (ch != *(m++)) m = s;

  if (*m) return (0);
  else    return (1);
}
