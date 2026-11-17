/* dcsevl.f -- translated by f2c (version 20100827). */

#include "ssc/fortran.h"

/* Table of constant values */

static integer c__28 = 28;
static integer c__2 = 2;
static integer c__31 = 31;
static integer c__3 = 3;
static integer c__25 = 25;
static integer c__1 = 1;

doublereal dcsevl_(doublereal* x, doublereal* a, integer* n)
{
    /* System generated locals */
    integer i__1;
    doublereal ret_val;

    /* Local variables */
    static integer i__;
    static doublereal b0, b1, b2;
    static integer ni;
    static doublereal twox;

    /* evaluate the n-term chebyshev series a at x.  adapted from */
    /* r. broucke, algorithm 446, c.a.c.m., 16, 254 (1973). */

    /*             input arguments -- */
    /* x      dble prec value at which the series is to be evaluated. */
    /* a      dble prec array of n terms of a chebyshev series.  in eval- */
    /*        uating a, only half the first coef is summed. */
    /* n      number of terms in array a. */

    /* Parameter adjustments */
    --a;

    /* Function Body */
    twox = *x * 2.;
    b1 = 0.;
    b0 = 0.;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
        b2 = b1;
        b1 = b0;
        ni = *n - i__ + 1;
        b0 = twox * b1 - b2 + a[ni];
        /* L10: */
    }

    ret_val = (b0 - b2) * .5;

    return ret_val;
} /* dcsevl_ */
