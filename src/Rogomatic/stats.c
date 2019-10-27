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
 * stats.c:
 *
 * A package for maintaining probabilities and statistics.
 *
 * Functions:
 *
 *    A probability is a simple count of Bernoulli trials.
 *
 *	clearprob:	Clear a probability.
 *	addprob:	Add success/failure to a probability.
 *	prob:		Calculate p(success) of a statistic.
 *	parseprob:	Parse a probability from a string.
 *	writeprob:	Write a probability to a file.
 *
 *    A statistic is a random variable with a mean and stdev.
 *
 *	clearstat:	Clear a statistic.
 *	addstat:	Add a data point to a statistic.
 *	mean:		Calculate the mean of a statistic.
 *	stdev:		Calculate the std. dev. of a statistic.
 *	parsestat:	Parse a statistic from a string.
 *	writestat:	Write a statistic to a file.
 */

# include <stdio.h>
# include <math.h>
# include "types.h"
# include "rogomatic.h"

/*
 * clearprob: zero a probability structure.
 */

void clearprob (p)
register  probability *p;
{
  p->fail = p->win = 0;
}

/*
 * addprob: Add a data point to a probability
 */

void addprob (p, success)
register probability *p;
register int success;
{
  if (success)	p->win++;
  else		p->fail++;
}

/*
 * prob: Calculate a probability
 */

double prob (p)
register probability *p;
{
  register int trials = p->fail + p->win;

  if (trials < 1)	return (0.0);
  else			return ((double) p->win / trials);
}

/*
 * parseprob: Parse a probability structure from buffer 'buf'
 */

void parseprob (buf, p)
register char *buf;
register probability *p;
{
  p->win = p->fail = 0;
  sscanf (buf, "%d %d", &p->fail, &p->win);
}

/*
 * writeprob. Write the value of a probability structure to file 'f'.
 */

void writeprob (f, p)
register FILE *f;
register probability *p;
{
  fprintf (f, "%d %d", p->fail, p->win);
}

/*
 * clearstat: zero a statistic structure.
 */

void clearstat (s)
register  statistic * s;
{
  s->count = 0;
  s->sum = s->sumsq = s->low = s->high = 0.0;
}

/*
 * addstat: Add a data point to a statistic
 */

void addstat (s, datum)
register statistic *s;
register int datum;
{
  double d = (double) datum;

  s->count++;
  s->sum += d;
  s->sumsq += d*d;

  if (s->count < 2)	s->low = s->high = d;
  else if (d < s->low)	s->low = d;
  else if (d > s->high)	s->high = d;
}

/*
 * mean: Return the mean of a statistic
 */

double mean (s)
const register statistic *s;
{
  if (s->count < 1)	return (0.0);
  else			return (s->sum / s->count);
}

/*
 * stdev: Return the standard deviation of a statistic
 */

double stdev (s)
register statistic *s;
{
  register n = s->count;

  if (n < 2)	return (0.0);
  else		return (sqrt ((n * s->sumsq - s->sum * s->sum) / (n * (n-1))));
}

/*
 * parsestat: Parse a statistic structure from buffer 'buf'
 */

void parsestat (buf, s)
register char *buf;
register statistic *s;
{
  s->count = 0;
  s->sum = s->sumsq = s->low = s->high = 0.0;
  sscanf (buf, "%d %lf %lf %lf %lf",
          &s->count, &s->sum, &s->sumsq, &s->low, &s->high);
}

/*
 * writestat. Write the value of a statistic structure to file 'f'.
 */

void writestat (f, s)
register FILE *f;
register statistic *s;
{
  fprintf (f, "%d %lg %lg %lg %lg",
           s->count, s->sum, s->sumsq, s->low, s->high);
}
