#include "random.h"
#include "rogue.h"

Random::Random()
{ }

Random::Random(int seed) :
    seed(seed)
{ }

void Random::set_seed(int s)
{
    seed = s;
}

int Random::get_seed() const
{
    return seed;
}

//rnd: Pick a very random number.
int Random::rnd(int range)
{
    return range < 1 ? 0 : ((ran() + ran()) & 0x7fffffffl) % range;
}

//Random number generator - adapted from the FORTRAN version in "Software Manual for the Elementary Functions" by W.J. Cody, Jr and William Waite.
long Random::ran()
{
    seed *= 125;
    seed -= (seed / 2796203) * 2796203;
    return seed;
}

//rnd: Pick a very random number.
int rnd(int range)
{
    return g_random->rnd(range);
}

//roll: Roll a number of dice
int roll(int number, int sides)
{
    int dtotal = 0;
    while (number--)
        dtotal += rnd(sides) + 1;
    return dtotal;
}
