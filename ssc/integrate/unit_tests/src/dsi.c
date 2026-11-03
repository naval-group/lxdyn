/* dsi.f -- translated by f2c (version 20100827).
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
static integer c__18 = 18;

doublereal dsi(doublereal *x)
{
    /* Initialized data */

    static doublereal sics[18] = { -.1315646598184841928904275173000457,
	    -.2776578526973601892048287660157299,
	    .0354414054866659179749135464710086,
	    -.002563163144793397765875278836153,
	    1.162365390497009281264921482985e-4,
	    -3.5904327241606042670004347148e-6,
	    8.02342123705710162308652976e-8,-1.3562997692540250649931846e-9,
	    1.79440721599736775567759e-11,-1.908387343087145490737e-13,
	    1.6669989586824330853e-15,-1.21730988368503042e-17,
	    7.54181866993865e-20,-4.014178842446e-22,1.8553690716e-24,
	    -7.5166966e-27,2.69113e-29,-8.58e-32 };
    static doublereal pi2 = 1.57079632679489661923132169163975;
    static integer nsi = 0;
    static doublereal xsml = 0.;

    /* System generated locals */
    real r__1;
    doublereal ret_val, d__1;

    /* Local variables */
    static doublereal f, g;
    static doublereal absx;
    static doublereal cosx;
    extern doublereal d1mach_(integer *);
    extern /* Subroutine */ int d9sifg_(doublereal *, doublereal *, 
	    doublereal *);
    extern doublereal dcsevl_(doublereal *, doublereal *, integer *);
    extern integer initds_(doublereal *, integer *, real *);

/* december 1980 edition, w. fullerton, bell labs. */

/* series for si   on the interval  0.00000e+00 to  1.60000e+01 */
/*                                        with weighted error   8.58e-32 */
/*                                         log weighted error  31.07 */
/*                               significant figures required  30.53 */
/*                                    decimal places required  31.69 */



    if (nsi != 0) {
	goto L10;
    }
    r__1 = (real) d1mach_(&c__3) * .1f;
    nsi = initds_(sics, &c__18, &r__1);
    d__1 = d1mach_(&c__3);
    xsml = sqrt(d__1);

L10:
    absx = abs(*x);
    if (absx > 4.) {
	goto L20;
    }
    ret_val = *x;
    if (absx < xsml) {
	return ret_val;
    }

    d__1 = (*x * *x - 8.) * .125;
    ret_val = *x * (dcsevl_(&d__1, sics, &nsi) + .75);
    return ret_val;

L20:
    d9sifg_(&absx, &f, &g);
    cosx = cos(absx);
    ret_val = pi2 - f * cosx - g * sin(*x);
    if (*x < 0.) {
	ret_val = -ret_val;
    }

    return ret_val;
} /* dsi_ */

