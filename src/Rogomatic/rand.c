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
 * rand.c:
 *
 * A very random generator, period approx 6.8064e16.
 *
 * Uses algorithm M, "Art of Computer Programming", Vol 2. 1969, D.E.Knuth.
 *
 * Two generators are used to derive the high and low parts of sequence X,
 * and another for sequence Y. These were derived by Michael Mauldin.
 *
 * Usage:  initialize by calling srand(seed), then rand() returns a random
 *         number from 0..2147483647. srand(0) uses the current time as
 *         the seed.
 *
 * Author: Michael Mauldin, June 14, 1983.
 */

/* Rand 1, period length 444674 */
# define MUL1 1156
# define OFF1 312342
# define MOD1 1334025
# define RAND1 (seed1=((seed1*MUL1+OFF1)%MOD1))
# define Y      RAND1

/* Rand 2, period length 690709 */
# define MUL2 1366
# define OFF2 827291
# define MOD2 1519572
# define RAND2 (seed2=((seed2*MUL2+OFF2)%MOD2))

/* Rand 3, period length 221605 */
# define MUL3 1156
# define OFF3 198273
# define MOD3 1329657
# define RAND3 (seed3=((seed3*MUL3+OFF3)%MOD3))

/*
 * RAND2 generates 19 random bits, RAND3 generates 17. The X sequence
 * is made up off both, and thus has 31 random bits.
 */

# define X    ((RAND2<<13 ^ RAND3>>3) & 017777777777)

# define AUXLEN 97
static int seed1=872978, seed2=518652, seed3=226543, auxtab[AUXLEN];

rogo_srand (seed)
int seed;
{
  register int i;

  if (seed == 0) seed = time (0);

  /* Set the three random number seeds */
  seed1 = (seed1+seed) % MOD1;
  seed2 = (seed2+seed) % MOD2;
  seed3 = (seed3+seed) % MOD3;

  for (i=AUXLEN; i--; )
    auxtab[i] = X;
}

int rogo_rand ()
{
  register int j, result;

  j = AUXLEN * Y / MOD1;	/* j random from 0..AUXLEN-1 */
  result = auxtab[j];
  auxtab[j] = X;
  return (result);
}

rogo_randint (max)
register int max;
{
  register int j, result;

  j = AUXLEN * Y / MOD1;	/* j random from 0..AUXLEN-1 */
  result = auxtab[j];
  auxtab[j] = X;
  return (result % max);
}
