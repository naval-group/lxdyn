/* dci.f -- translated by f2c (version 20100827).
   You must link the resulting object file with libf2c:
	on Microsoft Windows system, link with libf2c.lib;
	on Linux or Unix systems, link with .../path/to/libf2c.a -lm
	or, if you install libf2c.a in a standard place, with -lf2c -lm
	-- in that order, at the end of the command line, as in
		cc *.o -lf2c -lm
	Source for libf2c is in /netlib/f2c/libf2c.zip, e.g.,

		http://www.netlib.org/f2c/libf2c.zip
*/

#include <math.h>
#include "ssc/f2c/f2c.h"

/* Table of constant values */

static integer c__3 = 3;
static integer c__19 = 19;
static integer c__17 = 17;
static integer c__1 = 1;
static integer c__2 = 2;

doublereal dci(doublereal *x)
{
    /* Initialized data */

    static doublereal cics[19] = { -.34004281856055363156281076633129873,
	    -1.03302166401177456807159271040163751,
	    .19388222659917082876715874606081709,
	    -.01918260436019865893946346270175301,
	    .00110789252584784967184098099266118,
	    -4.157234558247208803840231814601e-5,
	    1.09278524300228715295578966285e-6,
	    -2.123285954183465219601280329e-8,3.1733482164348544865129873e-10,
	    -3.76141547987683699381798e-12,3.622653488483964336956e-14,
	    -2.8911528493651852433e-16,1.9432786067649442e-18,
	    -1.115183182650184e-20,5.527858887706e-23,-2.3907013943e-25,
	    9.1001612e-28,-3.07233e-30,9.26e-33 };
    static integer nci = 0;
    static doublereal xsml = 0.;

    /* System generated locals */
    real r__1;
    doublereal ret_val, d__1;

    /* Local variables */
    static doublereal f, g, y;
    static doublereal sinx;
    extern doublereal d1mach_(integer *);
    extern /* Subroutine */ int d9sifg_(doublereal *, doublereal *, 
	    doublereal *);
    extern doublereal dcsevl_(doublereal *, doublereal *, integer *);
    extern integer initds_(doublereal *, integer *, real *);

/* december 1980 edition, w. fullerton, bell labs. */

/* series for ci   on the interval  0.00000e+00 to  1.60000e+01 */
/*                                        with weighted error   9.26e-33 */
/*                                         log weighted error  32.03 */
/*                               significant figures required  32.06 */
/*                                    decimal places required  32.67 */



    if (nci != 0) {
	goto L10;
    }
    r__1 = (real) d1mach_(&c__3) * .1f;
    nci = initds_(cics, &c__19, &r__1);
    d__1 = d1mach_(&c__3);
    xsml = sqrt(d__1);

L10:
    if (*x > 4.) {
	goto L20;
    }
    y = -1.;
    if (*x > xsml) {
	y = (*x * *x - 8.) * .125;
    }

    ret_val = log(*x) - .5 + dcsevl_(&y, cics, &nci);
    return ret_val;

L20:
    d9sifg_(x, &f, &g);
    sinx = sin(*x);
    ret_val = f * sinx - g * cos(*x);

    return ret_val;
} /* dci_ */

