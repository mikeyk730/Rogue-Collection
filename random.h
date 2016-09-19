struct Random
{
    Random(int seed);

    //rnd: Pick a very random number.
    int rnd(int range);

private:
    //Random number generator - adapted from the FORTRAN version in "Software Manual for the Elementary Functions" by W.J. Cody, Jr and William Waite.
    long ran();

    int seed;
};