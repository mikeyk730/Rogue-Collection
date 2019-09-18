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
 * findscore.c:
 *
 * Read the Rogue scoreboard to determine a goal score.
 */

# include <stdio.h>
# include "install.h"
# define TEMPFL "/tmp/RscoreXXXXXX"
# define ISDIGIT(c) ((c) >= '0' && (c) <= '9')

findscore (rogue, roguename)
register char *rogue, *roguename;
{
  register int score, best = -1;
  char cmd[100], buffer[BUFSIZ];
  register char *s, *tmpfname = TEMPFL;
  FILE *tmpfil;
  char tmpbuffer[256];
  snprintf (tmpbuffer, 256, "%s", tmpfname);

  /* Run 'rogue -s', and put the scores into a temp file */
  sprintf (cmd, "%s -s >%s", rogue, mktemp (tmpbuffer));
  system (cmd);

  /* If no temp file created, return default score */
  if ((tmpfil = fopen (tmpbuffer, "r")) == NULL)
    return (best);

  /* Skip to the line starting with 'Rank...'. */
  while (fgets (buffer, BUFSIZ, tmpfil) != NULL)
    if (stlmatch (buffer, "Rank")) break;

  if (! feof (tmpfil)) {
    while (fgets (buffer, BUFSIZ, tmpfil) != NULL) {
      s = buffer;				/* point s at buffer */

      while (ISDIGIT (*s)) s++;			/* Skip over rank */

      while (*s == ' ' || *s == '\t') s++;	/* Skip to score */

      score = atoi (s);				/* Read score */

      while (ISDIGIT (*s)) s++;			/* Skip over score */

      while (*s == ' ' || *s == '\t') s++;	/* Skip to player */

      if (stlmatch (s, roguename)) {	/* Found our heros name */
        if (best < 0) best = score;		/* Rogy is on top! */

        break;					/* 'best' is now target */
      }

      if (score < BOGUS &&
          (score < best || best < 0))		/* Save smallest score */
        best = score;				/*  above Rogy's score */
    }
  }

  unlink (tmpbuffer);

  /* Don't quit for very small scores, it's not worth it */
  if (best < 2000) best = -1;

  return (best);
}
