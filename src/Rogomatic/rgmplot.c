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
 * rgmplot.c:
 *
 * This program takes a Rog-O-Matic score file sorted by date and score,
 * and produces a scatter plot of the scores.
 */

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# define WIDTH 50
# define AVLEN 7
# define SCALE(n) (((n)+100)/200)
# define isdigit(c) ((c) >= '0' && (c) <= '9')

char *month[] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

int doavg = 0, cheat = 0, min = -1;

main (argc, argv)
int argc;
char *argv[];
{
  int mm, dd, yy, score = 0, lastday = -1, lastmon = -1, lastyy = -1, h;
  int sumscores = 0, numscores = 0, i;
  int sum[AVLEN], num[AVLEN], rsum, rnum, davg, ravg;
  char player[100], plot[128], cheated;

  /* Clear out the rolling average statistics */
  for (i = 0; i < AVLEN; i++)
    sum[i] = num[i] = 0;

  /* Get the options */
  while (--argc > 0 && (*++argv)[0] == '-')
    while (*++(*argv)) {
      switch (**argv) {
        case 'c': cheat++; break; /* List cheat games */
        case 'a': doavg++; break; /* Print average */
        default:  printf ("Usage: rgmplot [-ac] [mininum]\n");
          exit (1);
      }
    }

  if (argc > 0) min = atoi (argv[0]);

  /*  Print out the header */
  printf ("\t\t   Scatter Plot of Rog-O-Matic Scores versus time\n\n");

  if (min > 0)
    printf ("\t\t              Scores greater than %d\n\n", min);

  printf ("\t\t0      2000      4000      6000      8000     10000\n");
  printf ("\t\t|----+----|----+----|----+----|----+----|----+----|\n");


  /* Build an empty plot line */
  strcpy (plot, "|                                                 |");

  /* While more scores do action for each score */
  while (getscore (&mm, &dd, &yy, player, &score, &cheated) != EOF) {
    /* Change days, overprint the average for day, rolling avg */
    if ((dd != lastday || mm != lastmon || yy != lastyy) && lastday > 0) {
      if (doavg) {
        rsum = *sum; rnum = *num;

        for (i = 1; i < AVLEN; i++)
          { rsum += sum[i]; rnum += num[i]; }

        davg = SCALE ((*num > 0) ? (*sum / *num) : 0);
        ravg = SCALE ((rnum > 0) ? (rsum / rnum) : 0);

        /* Roll the daily average statistics */
        for (i = AVLEN-1; i > 0; i--)
          { sum[i] = sum[i-1]; num[i] = num[i-1]; }

        *sum = *num = 0;

        /* Print a '*' for the daily average */
        if (davg > 0 && davg < WIDTH)
          plot[davg] = '*';

        /* Print a '###' for the rolling average */
        if (ravg > 0 && ravg < WIDTH-1)
          plot[ravg-1] = plot[ravg] = plot[ravg+1] = '#';
      }

      printf ("%3s %2d %4d\t%s\n", month[lastmon-1], lastday, lastyy, plot);
      strcpy (plot, "|                                                 |");

    }

    if (score > EOF) {
      if ((h = SCALE(score)) >= WIDTH)  sprintf (plot, "%s %d", plot, score);
      else if (plot[h] == '9')          ;
      else if (isdigit(plot[h]))        plot[h]++;
      else                              plot[h] = '1';

      *sum += score;
      ++*num;

      sumscores += score;
      ++numscores;

      lastday = dd; lastmon = mm; lastyy = yy;
    }
  }

  printf ("\t\t|----+----|----+----|----+----|----+----|----+----|\n");
  printf ("\t\t0      2000      4000      6000      8000     10000\n");


  if (numscores > 0)
    printf ("\nAverage score %d, total games %d.\n\n",
            sumscores/numscores, numscores);

  printf ("1-9    Number of games in range.\n");

  if (doavg) {
    printf (" *     Average of day's scores.\n");
    printf ("###    Rolling %d day average.\n", AVLEN);
  }
}


getlin (s)
char *s;
{
  int ch, i;
  static int endfile = 0;

  if (endfile) return (EOF);

  for (i=0; (ch = getchar()) != EOF && ch != '\n'; i++)
    s[i] = ch;

  s[i] = '\0';

  if (ch == EOF) {
    endfile = 1;
    strcpy (s, "-1 -1, -1 string -1 ");
    return (20);
  }

  return (i);
}

getscore (mm, dd, yy, player, score, cheated)
int *mm, *dd, *yy, *score;
char *player, *cheated;
{
  char line[128], reason[32];

  while (getlin (line) != EOF) {
    sscanf (line, "%d %d, %d %10s%d%c%17s",
            mm, dd, yy, player, score, cheated, reason);

    if ((*score >= min || *score < 0) &&
        (*cheated != '*' || cheat) &&
        !stlmatch (reason, "saved") &&
        (*score > 2000 || !stlmatch (reason, "user")))
      return (1);
  }

  return (EOF);
}
