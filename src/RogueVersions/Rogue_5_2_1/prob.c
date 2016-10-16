/*
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981, 1982 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

# include	<curses.h>
# include	"rogue.h"

# undef	max

# define	TRIES	10000

static char *sccsid = "@(#)prob.c	1.3 (Berkeley) 12/17/81";

main(ac, av)
int	ac;
char	**av;
{
	register unsigned int	prob, prob2, exp;
	register struct monster	*mp;
	register unsigned int	max, min, i;
	register unsigned int	max2, min2;

	printf("%17.17s  ----experience--- ----hit points---\n", "");
	printf("%17.17s  %7s %4s %4s %7s %4s %4s lvl\n", "monster", "avg", "min", "max", "avg", "min", "max", "max hp");
	seed = 0;
	for (mp = monsters; mp < &monsters[26]; mp++) {
		i = TRIES;
		prob2 = prob = 0;
		min2 = min = 30000;
		max2 = max = 0;
		while (i--) {
			if ((exp = roll(mp->m_stats.s_lvl, 8)) < min2)
				min2 = exp;
			if (exp > max2)
				max2 = exp;
			prob2 += exp;
			mp->m_stats.s_maxhp = exp;
			if ((exp = mp->m_stats.s_exp + exp_add(mp)) < min)
				min = exp;
			if (exp > max)
				max = exp;
			prob += exp;
		}
		printf("%17.17s: %7.2f %4d %4d %7.2f %4d %4d %3d\n", mp->m_name, ((double) prob) / TRIES, min, max, ((double) prob2) / TRIES, min2, max2, mp->m_stats.s_lvl);
		fflush(stdout);
	}
}

exp_add(mp)
register struct monster *mp;
{
    register unsigned int mod;

    if (mp->m_stats.s_lvl == 1)
	mod = mp->m_stats.s_maxhp / 8;
    else
	mod = mp->m_stats.s_maxhp / 6;
    if (mp->m_stats.s_lvl > 9)
	mod *= 20;
    else if (mp->m_stats.s_lvl > 6)
	mod *= 4;
    return mod;
}

/*
 * roll:
 *	roll a number of dice
 */
roll(number, sides)
register unsigned int number, sides;
{
    register unsigned int dtotal = 0;

    dtotal = number;
    while (number--)
	dtotal += rnd(sides);
    return dtotal;
}

/*
 * rnd:
 *	Pick a very random number.
 */


rnd(range)
register unsigned int range;
{

    return RN % range;
}
