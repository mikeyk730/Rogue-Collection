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

mapcharacter (ch, str)
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

saveltm (score)
int score;
{
  register int m;
  register FILE *ltmfil;

  if (nextmon < 1 || nosave) return;

  dwait (D_CONTROL, "Saveltm called, writing file '%s'", ltmnam);

  /* Disable interrupts and open the file for writing */
  critical ();

  /* Only write out the new results if we can get write access */
  if (lock_file (getLockFile (), MAXLOCK)) {
    if ((ltmfil = wopen (ltmnam, "w")) == NULL)
      { dwait (D_WARNING, "Can't write long term memory file '%s'...", ltmnam); }
    else {
      /* Write the ltm file header */
      fprintf (ltmfil, "Count %d, sum %d, start %d, saved %d\n",
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

/*
 * restoreltm: Read the long term memory file.
 */

restoreltm ()
{
  sprintf (ltmnam, "%s/ltm%d", getRgmDir (), version);
  dwait (D_CONTROL, "Restoreltm called, reading file '%s'", ltmnam);

  clearltm (monhist);			/* Clear the original sums */
  nextmon = 0;				/* Zero the list of monsters */
  monindex[0] = addmonhist ("it");	/* Monster 0 is "it" */

  /* Disable interrupts and open the file for reading */
  critical ();

  /* Only read the long term memory if we can get access */
  if (lock_file (getLockFile (), MAXLOCK)) {
    if (fexists (ltmnam))
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

readltm ()
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
      sscanf (buf, "Count %d, sum %d, start %d, saved %d",
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

parsemonster (monster)
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

clearltm (ltmarr)
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

dumpmonstertable ()
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

analyzeltm ()
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
    monatt[i].expdam = ceil (avg_dam*10);
    monatt[i].maxdam = ceil (max_dam);
    monatt[i].mtokill = ceil (avg_arr);
  }
}
