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
 * learn.c:
 *
 * Genetic learning component.
 */

# include <stdio.h>
# include <stdlib.h>
# include "types.h"

# define TRIALS(g)		((g)->score.count)
# define NONE		(-1)
# define MAXM		100
# define ALLELE		100
# define ZEROSTAT	{0, 0, 0, 0, 0}

typedef struct {
  int   id, creation, father, mother, dna[MAXKNOB];
  statistic score, level;
}               genotype;

extern int knob[];
extern double mean(), stdev(), sqrt();
extern FILE *wopen();

static int inittime=0, trialno=0, lastid=0;
static int crosses=0, shifts=0, mutations=0;
static statistic g_score = ZEROSTAT;
static statistic g_level = ZEROSTAT;
static genotype *genes[MAXM];
static int length = 0;
static int mindiff = 10, pmutate = 4, pshift = 2, mintrials = 1;
static double step = 0.33; /* standard deviations from the mean */
static FILE *glog=NULL;

static int compgene();
static int randompool (register int m);
static int printdna (FILE *f, register genotype *gene);
static int summgene (register FILE *f, register genotype *gene);
static int parsegene (register char *buf, register genotype *gene);
static int writegene (register FILE *gfil, register genotype *g);
static int initgene (register genotype *gene);
static int birth (register FILE *f, register genotype *gene);
static int cross (register int father, register int mother, register int new);
static int mutate (register int father, register int new);
static int shift (register int father, register int new);
static int selectgene (register int e1, register int e2);
static int unique (register int new);
static int untested ();
static int youngest ();
static int makeunique (register int new);
static int triangle (register int n);
static int badgene (register int e1, register int e2);

/*
 * Start a new gene pool
 */

initpool (k, m)
{
  inittime = time (0);

  if (glog) fprintf (glog, "Gene pool initalized, k %d, m %d, %s",
                       k, m, ctime (&inittime));

  randompool (m);
}

/*
 * Summarize the current gene pool
 */

analyzepool (full)
int full;
{
  register int g;

  qsort (genes, length, sizeof (*genes), compgene);

  printf ("Gene pool size %d, started %s", length, ctime (&inittime));
  printf ("Trials %d, births %d (crosses %d, mutations %d, shifts %d)\n",
          trialno, lastid, crosses, mutations, shifts);
  printf ("Mean score %1.0lf+%1.0lf, Mean level %3.1lf+%3.1lf\n\n",
          mean (&g_score), stdev (&g_score),
          mean (&g_level), stdev (&g_level));

  /* Give average of each gene */
  if (full == 2) {
    statistic gs;
    register int k;
    extern char *knob_name[];

    for (k=0; k<MAXKNOB; k++) {
      clearstat (&gs);

      for (g=0; g<length; g++)
        { addstat (&gs, genes[g]->dna[k]); }

      printf ("%s%5.2lf+%1.2lf\n", knob_name[k], mean (&gs), stdev (&gs));
    }
  }

  /* List detail of gene pool */
  else {
    for (g=0; g<length; g++) {
      printf ("Living: "); summgene (stdout, genes[g]);

      if (full) {
        if (genes[g]->mother)
          printf ("  Parents: %3d,%-3d", genes[g]->father, genes[g]->mother);
        else
          printf ("  Parent:  %3d,   ", genes[g]->father);

        printf ("  best %4.0lf/%-2.0lf",
                genes[g]->score.high, genes[g]->level.high);
        printf ("    DNA  "); printdna (stdout, genes[g]); printf ("\n\n");
      }
    }
  }
}

/*
 * setknobs: Read gene pool, pick genotype, and set knobs accordingly.
 */

setknobs (newid, knb, best, avg)
int *newid, *knb, *best, *avg;
{
  register int i, g;

  ++trialno;

  g = pickgenotype ();	/* Pick one genotype */
  *newid = genes[g]->id;

  for (i=0; i<MAXKNOB; i++)	/* Set the knobs for that genotype */
    knb[i] = genes[g]->dna[i];

  *best = genes[g]->score.high;
  *avg = (int) mean (&(genes[g]->score));
}

/*
 * evalknobs: Add a data point to the gene pool
 */

evalknobs (gid, score, level)
int gid, score, level;
{
  register int g;

  /* Find out which gene has the correct id */
  for (g=0; g<length; g++)
    if (gid == genes[g]->id) break;

  /* If he got deleted by someone else, blow it off */
  if (g >= length) return;

  /* Add information about performance */
  addstat (&(genes[g]->score), score);
  addstat (&g_score, score);
  addstat (&(genes[g]->level), level);
  addstat (&g_level, level);

  if (glog) {
    fprintf (glog, "Trial %4d, Id %3d -> %4d/%-2d  ",
             trialno, genes[g]->id, score, level);

    fprintf (glog, "age %2d, %4.0lf+%-4.0lf  %4.1lf+%3.1lf\n",
             TRIALS(genes[g]),
             mean (&(genes[g]->score)), stdev (&(genes[g]->score)),
             mean (&(genes[g]->level)), stdev (&(genes[g]->level)));
  }
}

/*
 * openlog: Open the gene log file
 */

FILE *rogo_openlog (genelog)
register char *genelog;
{
  glog = wopen (genelog, "a");
  return (glog);
}

/*
 * closelog: Close the log file
 */

void rogo_closelog ()
{
  if (glog) fclose (glog);
}

/*
 * pickgenotype: Run one trial, record performance, and do some learning
 */

pickgenotype ()
{
  register int youth, father, mother, new;

  /* Find genotype with fewer trials than needed to measure its performance */
  youth = untested ();

  if (youth >= 0) return (youth);

  /*
   * Have a good measure of all genotypes, pick a father, a mother, and
   * a loser and create a new genotype using genetic operators.
   */

  father = selectgene (NONE, NONE);
  mother = selectgene (father, NONE);
  new = badgene (father, mother);

  /* If no losers yet, return the youngest */
  if (new < 0) return (youngest ());

  /* Shift a single genotype with probability pshift */
  if (rogo_randint (100) < pshift) {
    if (glog) {
      fprintf (glog, "Select: "); summgene (glog, genes[father]);
      fprintf (glog, "Death:  "); summgene (glog, genes[new]);
    }

    shift (father, new);
  }

  /* Mutate a single genotype with probability pmutate */
  else if (rogo_randint (100-pshift) < pmutate) {
    if (glog) {
      fprintf (glog, "Select: "); summgene (glog, genes[father]);
      fprintf (glog, "Death:  "); summgene (glog, genes[new]);
    }

    mutate (father, new);
  }

  /* Cross two genotypes with probability 1-pshift-pmutate */
  else {
    if (glog) {
      fprintf (glog, "Select: "); summgene (glog, genes[father]);
      fprintf (glog, "Select: "); summgene (glog, genes[mother]);
      fprintf (glog, "Death:  "); summgene (glog, genes[new]);
    }

    cross (father, mother, new);
  }

  /* Log the birth */
  if (glog) birth (glog, genes[new]);

  return (new);  		/* Evaluate the new genotype */
}

/*
 * readgenes: Open the genepool for reading, and fill the current gene pool.
 * Returns true if the file was was, and 0 if there was no fail.  Exits
 * if the file exists and cannot be read.
 */

readgenes (genepool)
register char *genepool;
{
  char buf[BUFSIZ];
  register char *b;
  register int g=0;
  FILE *gfil;

  if ((gfil = fopen (genepool, "r")) == NULL) {
    if (fexists (genepool))
      quit (1, "Cannot open file '%s'\n", genepool);
    else
      return (0);
  }

  /* Read the header line */
  b = buf;
  fgets (b, BUFSIZ, gfil);
  sscanf (b, "%d %d %d %d %d %d",
          &inittime, &trialno, &lastid, &crosses, &shifts, &mutations);
  SKIPTO ('|', b);
  parsestat (b, &g_score);
  SKIPTO ('|', b);
  parsestat (b, &g_level);

  /* Now read in each genotype */
  while (fgets (buf, BUFSIZ, gfil) && length < MAXM-1) {
    if (g >= length) {
      genes[g] = (genotype *) malloc (sizeof (**genes));
      length++;
    }

    initgene (genes[g]);
    parsegene (buf, genes[g++]);
  }

  fclose (gfil);
  return (1);
}

/*
 * parsegene: Given a string representing a genotype and a genotype
 * structure, fill the structure according to the string.
 */

static parsegene (buf, gene)
register char *buf;
register genotype *gene;
{
  register int i;

  /* Get genotype specific info */
  sscanf (buf, "%d %d %d %d", &gene->id, &gene->creation,
          &gene->father, &gene->mother);

  /* Read each DNA gene */
  SKIPTO ('|', buf);
  SKIPCHAR (' ', buf);

  for (i=0; ISDIGIT (*buf); i++) {
    if (i < MAXKNOB) gene->dna[i] = atoi (buf);

    SKIPDIG (buf);
    SKIPCHAR (' ', buf);
  }

  /* Read the score and level performance stats */
  SKIPTO ('|', buf);
  parsestat (buf, &(gene->score));
  SKIPTO ('|', buf);
  parsestat (buf, &(gene->level));
}

/*
 * writegenes: Write the gene pool 'genes' out to file 'genepool'
 */

writegenes (genepool)
register char *genepool;
{
  register FILE *gfil;
  register int g;

  /* Open the gene file */
  if ((gfil = wopen (genepool, "w")) == NULL)
    quit (1, "Cannot open file '%s'\n", genepool);

  /* Write the header line */
  fprintf (gfil, "%d %d %d %d %d %d",
           inittime, trialno, lastid, crosses, shifts, mutations);
  fprintf (gfil, "|");
  writestat (gfil, &g_score);
  fprintf (gfil, "|");
  writestat (gfil, &g_level);
  fprintf (gfil, "|\n");

  /* Loop through each genotype */
  for (g=0; g<length; g++)
    writegene (gfil, genes[g]);

  fclose (gfil);
}

/*
 * Write out one line representing the gene.
 */

static writegene (gfil, g)
register FILE *gfil;
register genotype *g;
{
  register int i;

  /* Print genotype specific info */
  fprintf (gfil, "%3d %4d %3d %3d|", g->id, g->creation,
           g->father, g->mother);

  /* Write out dna */
  for (i=0; i<MAXKNOB; i++) {
    fprintf (gfil, "%2d", g->dna[i]);

    if (i < MAXKNOB-1) fprintf (gfil, " ");
  }

  fprintf (gfil, "|");

  /* Write out statistics */
  writestat (gfil, &(g->score));
  fprintf (gfil, "|");
  writestat (gfil, &(g->level));
  fprintf (gfil, "|\n");
}

/*
 * initgene: Allocate a new genotype structure, set everything to 0.
 */

static initgene (gene)
register genotype *gene;
{
  register int i;

  /* Clear genoptye specific info */
  gene->id = gene->creation = gene->father = gene->mother = 0;

  /* Clear the dna */
  for (i = 0; i < MAXKNOB; i++) gene->dna[i] = 0;

  /* Clear the statictics */
  clearstat (&(gene->score));
  clearstat (&(gene->level));
}

/*
 * compgene: Compare two genotypes in terms of score.
 */

static int compgene (a, b)
genotype **a, **b;
{
  register int result;

  result = (int) mean (&((*b)->score)) -
           (int) mean (&((*a)->score));

  if (result) return (result);
  else return ((*a)->id - (*b)->id);
}

/*
 * summgene: Summarize a single genotype
 */

static summgene (f, gene)
register FILE *f;
register genotype *gene;
{
  fprintf (f, "%3d age %2d, created %4d, ",
           gene->id, TRIALS(gene), gene->creation);
  fprintf (f, "score %5.0lf+%-4.0lf level %4.1lf+%-3.1lf\n",
           mean (&(gene->score)), stdev (&(gene->score)),
           mean (&(gene->level)), stdev (&(gene->level)));
}

/*
 * Birth: Summarize Record the birth of a genotype.
 */

static birth (f, gene)
register FILE *f;
register genotype *gene;
{
  if (!glog) return;

  fprintf (f, "Birth:  %d ", gene->id);

  if (gene->mother)
    fprintf (f, "(%d,%d)", gene->father, gene->mother);
  else
    fprintf (f, "(%d)", gene->father);

  fprintf (f, " created %d, DNA ", gene->creation);
  printdna (f, gene);
  fprintf (f, "\n");
}

/*
 * printdna: Print the genotype of a gene
 */

static printdna (f, gene)
FILE *f;
register genotype *gene;
{
  register int i;

  fprintf (f, "(");

  for (i=0; i < MAXKNOB; i++) {
    fprintf (f, "%02d", gene->dna[i]);

    if (i < MAXKNOB-1) fprintf (f, " ");
  }

  fprintf (f, ")");
}

/*
 * cross: Cross two genotypes producing a new genotype
 */

static cross (father, mother, new)
register int father, mother, new;
{
  register int cpoint, i;

  /* Set the new genotypes info */
  genes[new]->id = ++lastid;
  genes[new]->creation = trialno;
  genes[new]->father = genes[father]->id;
  genes[new]->mother = genes[mother]->id;
  clearstat (&(genes[new]->score));
  clearstat (&(genes[new]->level));

  /* Pick a crossover point and dominant parent */
  cpoint = rogo_randint (MAXKNOB-1) + 1;

  /* Fifty/fifty chance we swap father and mother */
  if (rogo_randint (100) < 50)
    { father ^= mother; mother ^= father; father ^= mother; }

  /* Copy the dna over */
  for (i=0; i<MAXKNOB; i++)
    genes[new]->dna[i] = (i<cpoint) ?
                         genes[father]->dna[i] : genes[mother]->dna[i];

  makeunique (new);

  /* Log the crossover */
  if (glog) {
    fprintf (glog, "Crossing %d and %d produces %d\n",
             genes[father]->id,  genes[mother]->id, genes[new]->id);
  }

  crosses++;
}

/*
 * mutate: mutate a genes producing a new gene
 */

static mutate (father, new)
register int father, new;
{
  register int i;

  /* Set the new genotypes info */
  genes[new]->id = ++lastid;
  genes[new]->creation = trialno;
  genes[new]->father = genes[father]->id;
  genes[new]->mother = 0;
  clearstat (&(genes[new]->score));
  clearstat (&(genes[new]->level));

  /* Copy the dna over */
  for (i=0; i<MAXKNOB; i++)
    genes[new]->dna[i] = genes[father]->dna[i];

  /* Randomly change genes until the new genotype is unique */
  do {
    i=rogo_randint (MAXKNOB);
    genes[new]->dna[i] = (genes[new]->dna[i] +
                          triangle (20) + ALLELE) % ALLELE;
  }
  while (!unique (new));

  /* Log the mutation */
  if (glog) {
    fprintf (glog, "Mutating %d produces %d\n",
             genes[father]->id, genes[new]->id);
  }

  mutations++;
}

/*
 * shift: shift a gene producing a new gene
 */

static shift (father, new)
register int father, new;
{
  register int i, offset;

  /* Set the new genotypes info */
  genes[new]->id = ++lastid;
  genes[new]->creation = trialno;
  genes[new]->father = genes[father]->id;
  genes[new]->mother = 0;
  clearstat (&(genes[new]->score));
  clearstat (&(genes[new]->level));

  /* Pick an offset, triangularly distributed around 0, until unique */
  offset = triangle (20);

  for (i=0; i<MAXKNOB; i++)
    genes[new]->dna[i] = (genes[father]->dna[i] +
                          offset + ALLELE) % ALLELE;

  makeunique (new);

  /* Now log the shift */
  if (glog) {
    fprintf (glog, "Shifting %d by %d produces %d\n",
             genes[father]->id, offset, genes[new]->id);
  }

  shifts++;
}

/*
 * randompool: Initialize the pool to a random starting point
 */

static randompool (m)
register int m;
{
  register int i, g;

  for (g=0; g<m; g++) {
    if (g >= length) {
      genes[g] = (genotype *) malloc (sizeof (**genes));
      length++;
    }

    initgene (genes[g]);
    genes[g]->id = ++lastid;

    for (i=0; i<MAXKNOB; i++) genes[g]->dna[i] = rogo_randint (ALLELE);

    birth (glog, genes[g]);
  }

  length = m;
}

/*
 * selectgene: Select a random gene, weighted by mean score.
 */

static selectgene (e1, e2)
register int e1, e2;
{
  register int total=0;
  register int g;

  /* Find the total worth */
  for (g=0; g<length; g++) {
    if (g==e1 || g==e2) continue;

    /* total += (int) mean (&(genes[g]->score)); */
    total += genes[g]->score.high;
  }

  /* Pick a random number and find the corresponding gene */
  if (total > 0) {
    for (g=0, total=rogo_randint (total); g<length; g++) {
      if (g==e1 || g==e2) continue;

      /* total -= (int) mean (&(genes[g]->score)); */
      total -= genes[g]->score.high;

      if (total < 0) return (g);
    }
  }

  /* Total worth zero, pick any gene at random */
  while ((g = rogo_randint (length))==e1 || g==e2) ;

  return (g);
}

/*
 * unique: Return false if gene is an exact copy of another gene.
 */

static unique (new)
register int new;
{
  register int g, i, delta, sumsquares;

  for (g=0; g<length; g++) {
    if (g != new) {
      sumsquares = 0;

      for (i=0; i<MAXKNOB; i++) {
        delta = genes[g]->dna[i] - genes[new]->dna[i];
        sumsquares += delta * delta;
      }

      if (sumsquares < mindiff) return (0);
    }
  }

  return (1);
}

/*
 * untested: Return the index of the youngest genotype with too few
 * trials to have an accurate measure.  The number of trials is
 * greater for older genotypes.
 */

static untested ()
{
  register int g, y= -1, trials=1e9, newtrials, count=length;

  for (g = rogo_randint (length); count-- > 0; g = (g+1) % length) {
    if (TRIALS (genes[g]) >= trials) continue;

    newtrials = trialno - genes[g]->creation;	/* Turns since creation */

    if (TRIALS (genes[g]) < newtrials / (4 * length) + mintrials)
      { y = g; trials = TRIALS (genes[g]); }
  }

  return (y);
}

/*
 * youngest: Return the index of the youngest genotype
 */

static youngest ()
{
  register int g, y=0, trials=1e9, newtrials, count=length;

  for (g = rogo_randint (length); count-- > 0; g = (g+1) % length) {
    newtrials = TRIALS (genes[g]);

    if (newtrials < trials) { y=g; trials=newtrials; }
  }

  return (y);
}

/*
 * makeunique: Mutate a genotype until it is unique
 */

static makeunique (new)
register int new;
{
  register int i;

  while (!unique (new)) {
    i=rogo_randint (MAXKNOB);
    genes[new]->dna[i] = (genes[new]->dna[i] +
                          triangle (20) + ALLELE) % ALLELE;
  }
}

/*
 * triangle: Return a non-zero triangularly distributed number from -n to n.
 */

static triangle (n)
register int n;
{
  register int val;

  do {
    val = rogo_randint (n) - rogo_randint (n);
  }
  while (val==0);

  return (val);
}

/*
 * badgene: Find the worst performer so far (with the lowest id).
 * only consider genotypes dominated by other genotypes.
 */

static badgene (e1, e2)
register int e1, e2;
{
  register int g, worst, trials;
  double worstval, bestval, avg, dev, value;

  worst = -1; worstval = 1.0e9;
  bestval = -1.0e9;

  for (g=0; g<length; g++) {
    if ((trials = TRIALS (genes[g])) < mintrials) continue;

    avg = mean (&(genes[g]->score));
    dev = stdev (&(genes[g]->score)) / sqrt ((double) trials);
    value = avg - step * dev;

    if (value > bestval) { bestval=value; }

    if (g==e1 || g==e2) continue;

    value = avg + step * dev;

    if (value < worstval) { worst=g; worstval=value; }
  }

  if (worstval < bestval)	return (worst);
  else				return (-1);
}
