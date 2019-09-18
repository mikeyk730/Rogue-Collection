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
 * histplot.c:
 *
 * This program takes a Rog-O-Matic log file and produces a histogram
 * of the scores.
 */

# include <stdio.h>
# include <stdlib.h>
# include <string.h>

# define SKIPARG	while (*++(*argv)); --(*argv)

# define BWIDTH 200
# define NUMBUK 51
# define BUCKET(n) (((n)+BWIDTH/2)/BWIDTH)
# define isdigit(c) ((c) >= '0' && (c) <= '9')
# define NOMON 29

int cheat = 0;

main (argc, argv)
int argc;
char *argv[];
{
  int score = 0, maxfreq = 0, lowscore = 0, min = 200, killnum = 0;
  int bucket[NUMBUK], killed[NUMBUK][NOMON], level = 0, dolev = 0;
  int total[NOMON];
  register int i, j, h, f;
  char killer[100], plot[128];

  /* Zero the buckets */
  for (i = NUMBUK; i--; ) {
    bucket[i] = 0;

    for (j = NOMON; j--; )
      killed[i][j] = 0;
  }

  for (j = NOMON; j--;)
    total[j] = 0;

  /* Get the options */
  while (--argc > 0 && (*++argv)[0] == '-')
    while (*++(*argv)) {
      switch (**argv) {
        case 'c': cheat++; break; /* List cheat games */
        case 'l': dolev++; break; /* Plot level instead of score */
        case 'a': min = atoi (*argv+1); SKIPARG; break;
        default:  printf ("Usage: histplot [-cl] [-aNNNN]\n");
          exit (1);
      }
    }

  /*  Print out the header */
  printf ("         %s  Histogram of Rog-O-Matic %s\n\n",
          dolev ? "" : "            ", dolev ? "Levels" : "Scores");
  printf ("\n");

  if (dolev)
    printf ("Games     1   5   10   15   20   25   30\n");
  else
    printf ("Games    0      2000      4000      6000      8000     10000\n");

  /* While more scores do action for each score */
  while (getscore (&score, killer, &level) != EOF) {
    if (score < min) { lowscore++; continue; }

    if (dolev) { h = level; }
    else       { if ((h = BUCKET(score)) >= NUMBUK) h = NUMBUK-1; }

    bucket[h]++;

    if (stlmatch ("arrow", killer))			killnum = 1;
    else if (stlmatch ("black unicorn", killer))	killnum = 'u'-'a'+2;
    else if (stlmatch ("bolt", killer))			killnum = 1;
    else if (stlmatch ("dart", killer))			killnum = 1;
    else if (stlmatch ("fatal error trap", killer))	killnum = 0;
    else if (stlmatch ("floating eye", killer))		killnum = 'e'-'a'+2;
    else if (stlmatch ("gave", killer))			killnum = 0;
    else if (stlmatch ("giant ant", killer))		killnum = 'a'-'a'+2;
    else if (stlmatch ("hypothermia", killer))		killnum = 'i'-'a'+2;
    else if (stlmatch ("quit", killer))			killnum = 28;
    else if (stlmatch ("starvation", killer))		killnum = 'e'-'a'+2;
    else if (stlmatch ("user", killer))			killnum = 0;
    else if (stlmatch ("venus flytrap", killer))	killnum = 'f'-'a'+2;
    else if (stlmatch ("violet fungi", killer))		killnum = 'f'-'a'+2;
    else killnum = *killer - 'a' + 2;

    killed[h][killnum]++;

    if (bucket[h] > maxfreq) maxfreq = bucket[h];
  }

  for (f = ((maxfreq+9)/10)*10; f; f--) {
    if (dolev) {
      if (f%10 == 0)
        sprintf (plot, "|----+----|----+----|----+----|");
      else if (f%5 == 0)
        sprintf (plot, "|    +    |    +    |    +    |");
      else
        sprintf (plot, "|         |         |         |");
    }
    else {
      if (f%10 == 0)
        sprintf (plot, "|----+----|----+----|----+----|----+----|----+----|");
      else if (f%5 == 0)
        sprintf (plot, "|    +    |    +    |    +    |    +    |    +    |");
      else
        sprintf (plot, "|         |         |         |         |         |");
    }

    for (i = 0; i < NUMBUK; i++)
      if (bucket[i] >= f) {
        plot[i] = '#';

        for (j = NOMON; j--;) {
          if (killed[i][j] > 0) {
            killed[i][j]--;
            plot[i] = "$@ABCDEFGHIJKLMNOPQRSTUVWXYZ#"[j];
            total[j]++;
            break;
          }
        }
      }

    if (f%5 == 0)
      printf ("     %3d %s\n", f, plot);
    else
      printf ("         %s\n", plot);
  }

  if (dolev) {
    printf ("         |----+----|----+----|----+----|\n");
    printf ("          1   5   10   15   20   25   30\n");
  }
  else {
    printf ("         |----+----|----+----|----+----|----+----|----+----|\n");
    printf ("         0      2000      4000      6000      8000     10000\n");
  }


  printf ("\n\n");

  if (total[28])
    printf ("             # Quit\n");

  printf ("           A-Z Monster which killed Rog-O-Matic\n");

  if (total[1])
    printf ("             @ Killed by an arrow, bolt, or dart\n");

  if (total[0])
    printf ("             $ Killed by user or error\n");

  if (lowscore)
    printf ("      %8d scores below %d not printed.\n", lowscore, min);
}

# define LEVELPOS 47

getscore (score, killer, level)
int *score, *level;
char *killer;
{
  int dd, yy;
  char line[128], mmstr[8], player[16], cheated=' ';

  while (fgets (line, 128, stdin)) {
    dd = yy = *score = 0;
    sscanf (line, "%s %d, %d %10s%d%c%17s",
            mmstr, &dd, &yy, player, score, &cheated, killer);

    if (strlen (line) > LEVELPOS) *level = atoi (line+LEVELPOS);

    if (yy > 0 &&
        (cheated != '*' || cheat) &&
        !stlmatch ("saved", killer) &&
        (*score > 2000 || !stlmatch ("user", killer)))
      return (1);
  }

  return (EOF);
}
