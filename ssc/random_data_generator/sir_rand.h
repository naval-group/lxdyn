/*
 *  PSEUDO RANDOM NUMBER GENERATOR
 *  
 *  Based on G. Marsaglia KISS
 */
#ifndef PRNG_SIR
#define PRNG_SIR
extern double sir_rand_u01(void); /* produce a double in [0,1) with uniform distribution */
extern double sir_rand_n01(void); /* produce a double with normal distribution N(0,1) - mean 0, variance 1 */
extern void sir_rand_setseed (int put); /* set the seed of the PRNG */
double sir_rand_ui64_to_double ( unsigned long u, unsigned long v);
#endif
