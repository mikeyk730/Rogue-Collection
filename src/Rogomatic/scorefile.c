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
 * scorefile.c:
 *
 * This file contains the functions which update the rogomatic scorefile,
 * which lives in <RGMDIR>/rgmscore<versionstr>. LOCKFILE is used to
 * prevent simultaneous accesses to the file. rgmdelta<versionstr>
 * contains new scores, and whenever the score file is printed the delta
 * file is sorted and merged into the rgmscore file.
 */

# include <stdio.h>
# include <stdlib.h>
# include <sys/types.h>
# include <sys/stat.h>
# include "types.h"
# include "globals.h"
# include "install.h"

# define LINESIZE	2048
# define SCORE(s,p)     (atoi (s+p))

static char lokfil[100];

/*
 * add_score: Write a new score line out to the correct rogomatic score
 * file by creating a temporary copy and inserting the new line in the
 * proper place. Be tense about preventing simultaneous access to the
 * score file and catching interrupts and things.
 */

add_score (new_line, vers, ntrm)
char *new_line, *vers;
int ntrm;
{
  int   wantscore = 1;
  char  ch;
  char  newfil[100];
  FILE *newlog;

  sprintf (lokfil, "%s%s", getLockFile (), vers);
  sprintf (newfil, "%s/rgmdelta%s", getRgmDir (), vers);

  /* Defer interrupts while mucking with the score file */
  critical ();

  /*
   * Lock the score file. If lock_file fails, asks the user whether he
   * wishes to wait. If so, then try lock_file five times and then ask
   * again.
   */

  while (lock_file (lokfil, MAXLOCK) == 0)
    if (--wantscore < 1 && !ntrm) {
      printf ("The score file is busy, do you wish to wait? [y/n] ");

      while ((ch = getchar ()) != 'y' && ch != 'n');

      if (ch == 'y')
        wantscore = 5;
      else
        { uncritical (); return; }
    }

  /* Now create a temporary to copy into */
  if ((newlog = wopen (newfil, "a")) == NULL)
    { printf ("\nUnable to write %s\n", newfil); }
  else {
    fprintf (newlog, "%s\n", new_line);
    fclose (newlog);
  }

  /* Write the score to the end of the delta file */

  /* Now close the file, relinquish control of scorefile, and exit */
  unlock_file (lokfil);
  uncritical ();
}

/*
 * dumpscore: Print out the scoreboard.
 */

dumpscore (vers)
char *vers;
{
  char  ch, scrfil[100], delfil[100], newfil[100], allfil[100], cmd[256];
  FILE *scoref, *deltaf;
  int   oldmask, intrupscore ();

  sprintf (lokfil, "%s %s", LOCKFILE, vers);
  sprintf (scrfil, "%s/rgmscore%s", getRgmDir (), vers);
  sprintf (delfil, "%s/rgmdelta%s", getRgmDir (), vers);
  sprintf (newfil, "%s/NewScore%s", getRgmDir (), vers);
  sprintf (allfil, "%s/AllScore%s", getRgmDir (), vers);

  /* On interrupts we must relinquish control of the score file */
  int_exit (intrupscore);

  if (lock_file (lokfil, MAXLOCK) == 0) {
    printf ("Score file busy.\n");
    exit (1);
  }

  deltaf = fopen (delfil, "r");
  scoref = fopen (scrfil, "r");

  /* If there are new scores, sort and merge them into the score file */
  if (deltaf != NULL) {
    fclose (deltaf);

    /* Defer interrupts while mucking with the score file */
    critical ();

    /* Make certain any new files are world writeable */
    oldmask = umask (0);

    /* If we have an old file and a delta file, merge them */
    if (scoref != NULL) {
      fclose (scoref);
      sprintf (cmd, "sort +4nr -o %s %s; sort -m +4nr -o %s %s %s",
               newfil, delfil, allfil, newfil, scrfil);
      system (cmd);

      if (filelength (allfil) != filelength (delfil) + filelength (scrfil)) {
        fprintf (stderr, "Error, new file is wrong length!\n");
        unlink (newfil); unlink (allfil);
        unlock_file (lokfil);
        exit (1);
      }
      else {
        /* New file is okay, unlink old files and pointer swap score file */
        unlink (delfil); unlink (newfil);
        unlink (scrfil);
#ifndef _WIN32
        link (allfil, scrfil); unlink (allfil);
#else
        rename(allfil, scrfil);
#endif
      }

      scoref = fopen (scrfil, "r");
    }
    else
      /* Only have delta file, sort into scorefile and unlink delta */
    {
      sprintf (cmd, "sort +4nr -o %s %s", scrfil, delfil);
      system (cmd);
      unlink (delfil);
      scoref = fopen (scrfil, "r");
    }

    /* Restore umask */
    umask (oldmask);

    /* Restore interrupt status after score file stable */
    uncritical ();
  }

  /* Now any new scores have been put into scrfil, read it */
  if (scoref == NULL) {
    printf ("Can't find %s\nBest score was %d.\n", scrfil, BEST);
    unlock_file (lokfil);
    exit (1);
  }

  printf ("Rog-O-Matic Scores against version %s:\n\n", vers);
  printf ("%s%s", "Date         User        Gold    Killed by",
          "      Lvl  Hp  Str  Ac  Exp\n\n");

  while ((int) (ch = fgetc (scoref)) != EOF)
    putchar (ch);

  fclose (scoref);
  unlock_file (lokfil);

  exit (0);
}

/*
 * intrupscore: We have an interrupt, clean up and unlock the score file.
 */

intrupscore ()
{
  unlock_file (lokfil);
  exit (1);
}

