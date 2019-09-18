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
 * gene.c:
 *
 * Initialize and summarize the gene pool
 */

# include <stdio.h>
# include <stdlib.h>
# include "types.h"
# include "install.h"

int knob[MAXKNOB];
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

char genelock[100];
char genelog[100];
char genepool[100];

main (argc, argv)
int   argc;
char *argv[];
{
  int m=10, init=0, seed=0, version=RV53A, full=0;

  /* Get the options */
  while (--argc > 0 && (*++argv)[0] == '-') {
    while (*++(*argv)) {
      switch (**argv) {
        case 'a':	full=2; break;
        case 'i':	init++; break;
        case 'f':	full=1; break;
        case 'm':	m = atoi(*argv+1); SKIPARG;
          printf ("Gene pool size %d.\n", m);
          break;
        case 's':	seed = atoi(*argv+1); SKIPARG;
          printf ("Random seed %d.\n", m);
          break;
        case 'v':	version = atoi(*argv+1); SKIPARG;
          printf ("Rogue version %d.\n", version);
          break;
        default:	quit (1,"Usage: gene [-if] [-msv<value>] [genepool]\n");
      }
    }
  }

  if (argc > 0) {
    if (readgenes (argv[0]))		/* Read the gene pool */
      analyzepool (full);		/* Print a summary */
    else
      fprintf (stderr, "Cannot read file '%s'\n", argv[0]);

    exit (0);
  }

  /* No file argument, assign the gene log and pool file names */
  sprintf (genelock, "%s/GeneLock%d", getRgmDir (), version);
  sprintf (genelog, "%s/GeneLog%d", getRgmDir (), version);
  sprintf (genepool, "%s/GenePool%d", getRgmDir (), version);

  critical ();				/* Disable interrupts */

  if (lock_file (genelock, MAXLOCK)) {
    if (init) {
      rogo_srand (seed);			/* Set the random number generator */
      rogo_openlog (genelog);		/* Open the gene log file */
      initpool (MAXKNOB, m);		/* Random starting point */
      writegenes (genepool);		/* Write out the gene pool */
      rogo_closelog ();			/* Close the log file */
    }
    else if (! readgenes (genepool))	/* Read the gene pool */
      quit (1, "Cannot read file '%s'\n", genepool);

    unlock_file (genelock);
  }
  else
    quit (1, "Cannot access file '%s'\n", genepool);

  uncritical ();			/* Re-enable interrupts */
  analyzepool (full);			/* Print a summary */
}
