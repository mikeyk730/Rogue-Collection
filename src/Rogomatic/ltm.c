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
 * ltm.c:
 *
 * This file contains functions for maintaining a database or "long
 * term memory"
 */

#ifdef _WIN32
#include <Windows.h>
#undef MOUSE_MOVED
#undef OPTIONAL
#undef NEAR
#include <time.h>
#endif

# include <curses.h>
# include <math.h>
# include <string.h>
# include "types.h"
# include "globals.h"
# include "install.h"

static int nosave = 0;		/* True ==> dont write ltm back out */
static char ltmnam[100];	/* Long term memory file name */

/*
 * mapcharacter: Read a character help message
 */

void mapcharacter (ch, str)
char ch, *str;
{
  dwait (D_CONTROL, "mapcharacter called: '%c' ==> '%s'", ch, str);

  /* Ancient versions of Rogue had no wands or staves */
  if (ch == '/' && stlmatch (str, "unknown"))
    version = RV36A;

  /* Dont map any unknown character */
  else if (stlmatch (str, "unknown"))
    ;

  /* If it is a monster, set its array index */
  else if (ch >= 'a' && ch <= 'z')
    { monindex[ch-'a'+ 1] = addmonhist (str); }
}

/*
 * addmonhist:  Return the monster index of a given monster name in the
 * history array.  Create an entry if none exists.
 */

int addmonhist (monster)
char *monster;
{
  register int m;

  /* Search for the monsters entry in the table */
  for (m=0; m<nextmon; m++)
    if (streq (monster, monhist[m].m_name))
      return (m);

  if (nextmon >= MAXMON)			/* Check for overflow */
    dwait (D_FATAL, "Overflowed monster array");

  strcpy (monhist[nextmon].m_name, monster);	/* Copy in the name */
  return (nextmon++);				/* Return the index */
}

/*
 * findmonster:  Return the monster index of a given monster name in the
 * history array.  Return -1 if the monster is not in the table.
 */

int findmonster (monster)
char *monster;
{
  register int m;

  /* Search for the monsters entry in the table */
  for (m=0; m<nextmon; m++)
    if (streq (monster, monhist[m].m_name))
      return (m);

  return (-1);
}

/*
 * saveltm: Write the new monster information out to the long term memory
 * file for this version of Rogue.  Be careful about serializing
 * access to the output file.
 */

void saveltm (score)
int score;
{
  register int m;
  register FILE *ltmfil;

  if (nextmon < 1 || nosave || getenv("NOLTM")) return;

  dwait (D_CONTROL, "Saveltm called, writing file '%s'", ltmnam);

  /* Disable interrupts and open the file for writing */
  critical ();

  /* Only write out the new results if we can get write access */
  if (lock_file (getLockFile (), MAXLOCK)) {
    if ((ltmfil = wopen (ltmnam, "w")) == NULL)
      { dwait (D_WARNING, "Can't write long term memory file '%s'...", ltmnam); }
    else {
      /* Write the ltm file header */
      fprintf (ltmfil, "Count %d, sum %d, start %lld, saved %d\n",
               ltm.gamecnt+1, ltm.gamesum+score,
               ltm.inittime, ltm.timeswritten+1);

      /* Now write a line for each monster */
      for (m = 0; m < nextmon; m++) {
        fprintf (ltmfil, "%s|", monhist[m].m_name);
        writeprob (ltmfil, &monhist[m].wehit);    fprintf (ltmfil, "|");
        writeprob (ltmfil, &monhist[m].theyhit);  fprintf (ltmfil, "|");
        writeprob (ltmfil, &monhist[m].arrowhit); fprintf (ltmfil, "|");
        writestat (ltmfil, &monhist[m].htokill);   fprintf (ltmfil, "|");
        writestat (ltmfil, &monhist[m].damage);   fprintf (ltmfil, "|");
        writestat (ltmfil, &monhist[m].atokill);  fprintf (ltmfil, "|\n");
      }

      /* Close the file and unlock it */
      fclose (ltmfil);
    }

    unlock_file (getLockFile ());
  }

  /* Re-enable interrupts */
  uncritical ();
}

#ifndef _WIN32
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int CopyFile(const char *from, const char *to, int ignored)
{
    int fd_to, fd_from;
    char buf[4096];
    ssize_t nread;
    int saved_errno;

    fd_from = open(from, O_RDONLY);
    if (fd_from < 0)
        return -1;

    fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd_to < 0)
        goto out_error;

    while (nread = read(fd_from, buf, sizeof buf), nread > 0)
    {
        char *out_ptr = buf;
        ssize_t nwritten;

        do {
            nwritten = write(fd_to, out_ptr, nread);

            if (nwritten >= 0)
            {
                nread -= nwritten;
                out_ptr += nwritten;
            }
            else if (errno != EINTR)
            {
                goto out_error;
            }
        } while (nread > 0);
    }

    if (nread == 0)
    {
        if (close(fd_to) < 0)
        {
            fd_to = -1;
            goto out_error;
        }
        close(fd_from);

        /* Success! */
        return 0;
    }

  out_error:
    saved_errno = errno;

    close(fd_from);
    if (fd_to >= 0)
        close(fd_to);

    errno = saved_errno;
    return -1;
}
#endif

/*
 * restoreltm: Read the long term memory file.
 */

void copyltm()
{
    if (!nosave && !getenv("NOLTM"))
    {
        char dest[80];
        sprintf(dest, "%s/ltm%d.%d", getRgmDir(), version, g_seed);
        CopyFile(ltmnam, dest, TRUE);
    }
}

void restoreltm ()
{
  /* mdk: load ltm from seed file if it exists. */
  sprintf (ltmnam, "%s/ltm%d.%d", getRgmDir (), version, g_seed);
  nosave = 1;
  if (!fexists(ltmnam)) {
      sprintf (ltmnam, "%s/ltm%d", getRgmDir (), version);
      nosave = 0;
  }
  dwait (D_CONTROL, "Restoreltm called, reading file '%s'", ltmnam);

  clearltm (monhist);			/* Clear the original sums */
  nextmon = 0;				/* Zero the list of monsters */
  monindex[0] = addmonhist ("it");	/* Monster 0 is "it" */

  /* Disable interrupts and open the file for reading */
  critical ();

  /* Only read the long term memory if we can get access */
  if (lock_file (getLockFile (), MAXLOCK)) {
    if (fexists (ltmnam) && !getenv("NOLTM"))
      readltm ();
    else {
      dwait (D_CONTROL | D_SAY,
             "Starting long term memory file '%s'...", ltmnam);
      ltm.gamecnt = ltm.gamesum = ltm.timeswritten = 0;
      ltm.inittime = time (0);
    }

    unlock_file (getLockFile ());
  }
  else {
    saynow ("Warning: could not lock long term memory file!");
    nosave = 1;
  }

  uncritical ();
}

/*
 * readltm: Read in the long term memory file for this version of Rogue
 * into storage.  Be careful about serializing access to the file.
 */

void readltm ()
{
  char buf[BUFSIZ];
  register FILE *ltmfil;

  if ((ltmfil = fopen (ltmnam, "r")) == NULL) {
    nosave = 1;
    dwait (D_WARNING | D_SAY,
           "Could not read long term memory file '%s'...", ltmnam);
  }
  else {
    /* Read the ltm file header */
    if (fgets (buf, BUFSIZ, ltmfil))
      sscanf (buf, "Count %d, sum %d, start %lld, saved %d",
              &ltm.gamecnt, &ltm.gamesum,
              &ltm.inittime, &ltm.timeswritten);

    /* Read each monster line */
    while (fgets (buf, BUFSIZ, ltmfil))
      parsemonster (buf);

    fclose (ltmfil);
  }
}

/*
 * parsemonster: parse one line from the ltm file.
 */

void parsemonster (monster)
char *monster;
{
  register char *attrs;
  char *index();
  register int m;

  /* Separate the monster name from the attributes */
  if ((attrs = strchr (monster, '|')) == NULL) return;

  *attrs++ = '\0';

  /* Find the monsters entry in long term memory */
  m = addmonhist (monster);

  /* Now parse the probabilities and statistics */
  parseprob (attrs, &monhist[m].wehit);		SKIPTO ('|', attrs);
  parseprob (attrs, &monhist[m].theyhit);	SKIPTO ('|', attrs);
  parseprob (attrs, &monhist[m].arrowhit);	SKIPTO ('|', attrs);
  parsestat (attrs, &monhist[m].htokill);	SKIPTO ('|', attrs);
  parsestat (attrs, &monhist[m].damage);	SKIPTO ('|', attrs);
  parsestat (attrs, &monhist[m].atokill);	SKIPTO ('|', attrs);
}

/*
 * clearltm: Clear a whole long term memory array.
 */

void clearltm (ltmarr)
register ltmrec *ltmarr;
{
  register int i;

  for (i=0; i<MAXMON; i++) {
    ltmarr[i].m_name[0] = '\0';
    clearprob (&ltmarr[i].wehit);
    clearprob (&ltmarr[i].theyhit);
    clearprob (&ltmarr[i].arrowhit);
    clearstat (&ltmarr[i].htokill);
    clearstat (&ltmarr[i].damage);
    clearstat (&ltmarr[i].atokill);
  }
}

/*
 * dumpmonstertable: Format and print the monster table on the screen
 */

void dumpmonstertable ()
{
  register int m;
  char monc;

  clear (); mvprintw (0,0,"Monster table:");
  analyzeltm ();

  for (m=0, monc='A';  m<26;  m++, monc++) {
    if (m < 13) at (m+2, 0);
    else        at (m-11, 40);

    printw ("%c: %s", monc, monname (monc));

    if (monhist[monindex[m+1]].damage.count > 0)
      printw (" (%d,%d)", monatt[m].expdam, monatt[m].maxdam);
    else
      printw (" <%d>", monatt[m].maxdam);

    if (monhist[monindex[m+1]].atokill.count > 0)
      printw (" [%d]", monatt[m].mtokill);
  }

  pauserogue ();
}

/*
 * analyzeltm: Set the monatt array based on current long term memory.
 */

void analyzeltm ()
{
  register int m, i;
  double avg_dam = 0.6*Level+3, max_dam = 7.0+Level, avg_arr = 4.0;
  double phit, mean_dam, stdev_dam, three_dev;

  /* Loop through each monster in this game (not whole ltm file) */
  for (i=0; i<26; i++) {
    m = monindex[i+1];

    /* Calculate expected and maximum damage done by monster */
    if (monhist[m].damage.count > 3) {
      mean_dam = mean (&monhist[m].damage);
      stdev_dam = stdev (&monhist[m].damage);
      max_dam = monhist[m].damage.high;

      avg_dam = mean_dam * prob (&monhist[m].theyhit);
      three_dev = mean_dam + 3 * stdev_dam;

      if (max_dam > three_dev && monhist[m].damage.count > 10) {
        max_dam = mean_dam + stdev_dam;
        monhist[m].damage.high = max_dam;
      }
    }
    else if (monhist[m].damage.high > 0.0)
      max_dam = monhist[m].damage.high;

    /* Calculate average arrows fired to killed monster */
    if (monhist[m].atokill.count > 2) {
      phit = prob (&monhist[m].arrowhit); phit = max (phit, 0.1);
      avg_arr = mean (&monhist[m].atokill) / phit;
    }

    /* Now store the information in the monster tables */
    monatt[i].expdam = (int)ceil (avg_dam*10);
    monatt[i].maxdam = (int)ceil (max_dam);
    monatt[i].mtokill = (int)ceil (avg_arr);
  }
}
