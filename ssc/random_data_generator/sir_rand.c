/*
 *  PSEUDO RANDOM NUMBER GENERATOR
 *
 *  Based on G. Marsaglia KISS
 */


/*
    This PRNG combines:

    (1) The congruential generator x(n)=69069*x(n-1)+1327217885 with a period
            of 2^32,
    (2) A 3-shift shift-register generator with a period of 2^32-1,
    (3) Two 16-bit multiply-with-carry generators with a period of
            597273182964842497 > 2^59.

    The overall period exceeds 2^123.

    http://www.ciphersbyritter.com/NEWS4/RANDC.HTM#369F6FCA.74C7C041@stat.fsu.edu

    The above web site has an archive of a newsgroup posting from George
    Marsaglia with the statement:

    Subject: Random numbers for C: Improvements.
    Date: Fri, 15 Jan 1999 11:41:47 -0500
    From: George Marsaglia <geo@stat.fsu.edu>
    Message-ID: <369F6FCA.74C7C041@stat.fsu.edu>
    References: <369B5E30.65A55FD1@stat.fsu.edu>
    Newsgroups: sci.stat.math,sci.math,sci.math.numer-analysis
    Lines: 93

    As I hoped, several suggestions have led to
    improvements in the code for RNG's I proposed for
    use in C. (See the thread "Random numbers for C: Some
    suggestions" in previous postings.) The improved code
    is listed below.

    A question of copyright has also been raised.  Unlike
    DIEHARD, there is no copyright on the code below. You
    are free to use it in any way you want, but you may
    wish to acknowledge the source, as a courtesy.

*/

#include <math.h>
#include <string.h>

#include "ssc/random_data_generator/sir_rand.h"


/* convert integer on 64 bits into a double in the range of [0,1).
 * we mask off the bits that don't fit into the mantissa
 * before converting to the real type
 * then multiply by the correct shift  */

double sir_rand_ui64_to_double ( unsigned long u, unsigned long v)
{
    static double fshift=0.00000000023283064365386962890625; /* this is pow(2.0,-32.0) */
    unsigned long mask;
    mask = (~ 0uL) << (64 - 53); /* assume double has 53 bit mantissa */
    v = v & mask;
    return ((double) v * fshift + (double) u ) * fshift;
}

#define SIR_SL(k, n)    ((k)^((k)<<(n)))
#define SIR_SR(k, n)    ((k)^((k)>>(n)))

/*   Reference for the seed:
    From: "George Marsaglia" <g...@stat.fsu.edu>
    Newsgroups: sci.math
    Message-ID: <e7CcnWxczriWssCjXTWc3A@comcast.com>

    The KISS RNG uses four seeds, x, y, z, c,
    with 0<=x<2^32, 0<y<2^32, 0<=z<2^32, 0<=c<698769069
    except that the two pairs
    z=0,c=0 and z=2^32-1,c=698769068
    should be avoided.  */

#define KISS_DEFAULT_SEED_1 123456789, 362436069, 521288629, 316191069
#define KISS_DEFAULT_SEED_2 987654321, 458629013, 582859209, 438195021

static unsigned int sir_rand_seed[] = {
    KISS_DEFAULT_SEED_1,
    KISS_DEFAULT_SEED_2,
};

/*
static unsigned int sir_rand_default_seed[] = {
    KISS_DEFAULT_SEED_1,
    KISS_DEFAULT_SEED_2,
};
*/

#define SIR_RAND_SIZE sizeof(sir_rand_seed)/sizeof(sir_rand_seed[0])

static unsigned int * const sir_rand_seed_1 = sir_rand_seed;
static unsigned int * const sir_rand_seed_2 = sir_rand_seed + 4;

/* sir_rand_kernel() returns an integer value in the range of
    (0, unsigned int_HUGE].  The distribution of pseudorandom numbers
    should be uniform.  */

static unsigned int sir_rand_kernel(unsigned int * seed)
{
    unsigned int kiss;

    seed[0] = 69069 * seed[0] + 1327217885;
    seed[1] = SIR_SL(SIR_SR(SIR_SL(seed[1],13),17),5);
    seed[2] = 18000 * (seed[2] & 65535) + (seed[2] >> 16);
    seed[3] = 30903 * (seed[3] & 65535) + (seed[3] >> 16);
    kiss = seed[0] + seed[1] + (seed[2] << 16) + seed[3];

    return kiss;
}

/*  This function produces a double value from the uniform distribution
        with range [0,1).  */

double sir_rand_u01()
{
    unsigned long u,v;

    u = sir_rand_kernel (sir_rand_seed_1);
    v = sir_rand_kernel (sir_rand_seed_2);
    return sir_rand_ui64_to_double (u,v);
}


static void
sir_rand_scramble_seed (unsigned char *dest, unsigned char *src, int size)
{
    int i;

    for (i = 0; i < size; i++)
        dest[(i % 2) * (size / 2) + i / 2] = src[i];
}

/*
static void
sir_rand_unscramble_seed (unsigned char *dest, unsigned char *src, int size)
{
    int i;

    for (i = 0; i < size; i++)
        dest[i] = src[(i % 2) * (size / 2) + i / 2];
}
*/


/* sir_rand_setseed is used to seed the PRNG with a user provided seed.  */

void
sir_rand_setseed (int put)
{
    size_t i;
    unsigned char seed[sizeof sir_rand_seed];
    /*  We copy the seed given by the user.  */
    for(i=0;i<SIR_RAND_SIZE;i++) {
        put=36969*(put&65535)+(put>>16);
        memcpy (seed + i * sizeof(unsigned int),
            &put,
            sizeof(unsigned int));
    }

    /* We put it after scrambling the bytes, to paper around users who
    provide seeds with quality only in the lower or upper part.  */
    sir_rand_scramble_seed((unsigned char *) sir_rand_seed, seed, int(sizeof(sir_rand_seed)));
}


/* Toss a double with normal N(0,1) distribution
 * using a Box Muller algorithm
 */
double sir_rand_n01()
{
    static int i=0;
    static double y=0.0;
    if( i==0 ) {
        double s;
        static double x=0.0;
        do {
            x=sir_rand_u01();
            y=sir_rand_u01();
            x = (x-0.5)*2.0;
            y = (y-0.5)*2.0;
            s = x*x+y*y;
        } while(s > 1.0 || s == 0.0);
        x=x*sqrt(-2.0*log(s)/s);
        y=y*sqrt(-2.0*log(s)/s);
        return x;
    } else {
        return y;
    }
    i = 1-i;
}
