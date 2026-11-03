/* filon.f -- translated by f2c (version 20100827).
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
#include <stdio.h>

#define COUT(x) printf("in file %s, line %i: %s = %f\n", __FILE__, __LINE__, #x, (doublereal)x);

#include "ssc/f2c/f2c.h"

/* Table of constant values */

static integer c__2 = 2;

/* Subroutine */ int filon(D_fp f, int *obj, doublereal *a, doublereal *b, doublereal *
	fpa, doublereal *fpb, doublereal *fba, doublereal *fbb, doublereal *w,
	 doublereal *eps, integer *maxh, integer *lc, integer *ls, doublereal 
	*c__, doublereal *s)
{
    /* Initialized data */

    static doublereal pi = 3.141592653589793;

    /* System generated locals */
    integer i__1, i__2;
    doublereal d__1;

    /* Local variables */
    static doublereal h__;
    static integer i__, m, n;
    static doublereal x, h2, t2, t3, w1, fa, fb, tc, fx, ts, wx;
    static integer llc, lls, mxn, nst;
    static doublereal alfa, beta, cosa, cosb, sina, sinb, teta, temp, tmax, 
	    pvtc[7], pvts[7];
    extern /* Subroutine */ int endt2_(doublereal *, doublereal *, doublereal 
	    *, doublereal *, integer *, integer *);
    static doublereal temp1, temp2, temp3, delta, tmaxb, epsil;
    static integer nstop;
    static doublereal sumcos, sumsin;

/* *********************************************************************72 */

/* c FSPL2 estimates a Fourier integral using Filon's method. */

/*  Discussion: */

/*    This routine computes the Fourier integrals */
/*      C = integral ( a <= x <= b ) f(x) cos(wx) dx */
/*      S = integral ( a <= x <= b ) f(x) sin(wx) dx */
/*    with a spline procedure.  Repeated Richardson extrapolation is used. */

/*  Modified: */

/*    20 May 2014 */

/*  Author: */

/*    Original Fortran77 version by Bo Einarsson. */
/*    This Fortran77 version by John Burkardt. */

/*  References: */

/*    Stephen Chase, Lloyd Fosdick, */
/*    An Algorithm for Filon Quadrature, */
/*    Communications of the Association for Computing Machinery, */
/*    Volume 12, Number 8, August 1969, pages 453-457. */

/*    Stephen Chase, Lloyd Fosdick, */
/*    Algorithm 353: */
/*    Filon Quadrature, */
/*    Communications of the Association for Computing Machinery, */
/*    Volume 12, Number 8, August 1969, pages 457-458. */

/*    Bo Einarsson, */
/*    Numerical calculation of Fourier integrals with cubic splines, */
/*    Communications of the ACM, */
/*    Volume 8, pages 457-458, 1969. */

/*  Parameters: */

/*    F(X) = THE FUNCTION TO BE INTEGRATED, SUPPLIED BY THE USER */
/*    AND DECLARED 'EXTERNAL' IN THE CALLING PROGRAM. */

/*    Input, double precision A, B, the lower and upper quadrature limits. */
/*    If B .le. A, the computation is not carried out, and the signs */
/*    of LC, LS and EPS are reversed to indicate an error. */

/*    FPA AND FPB ARE THE VALUES OF THE DERIVATIVE OF F(X). */
/*    FBA AND FBB ARE THE CORRESPONDING VALUES OF THE SECOND */
/*    DERIVATIVE AT THE POINTS A AND B. */

/*    Input, double precision W, the angular frequency. */

/*    EPS = REQUIRED ACCURACY, DEFINED BY */
/*         |ERROR| < =  EPS*(1.+|C|) */
/*    AND */
/*         |ERROR| < =  EPS*(1.+|S|) */
/*    IF CONVERGENCE IS NOT OBTAINED, THE VALUE */
/*    OF EPS IS RETURNED WITH NEGATIVE SIGN. */

/*    MAXH = THE MAXIMUM NUMBER OF PARTITIONS (INTERVAL HALVINGS). */
/*    IN THIS ROUTINE THE INTERNAL VARIABLE MXN DEFINED BELOW */
/*    IS USED INSTEAD OF MAXH. */

/*    LC POSITIVE ON ENTRY INDICATES THAT C IS WANTED. */
/*    LS POSITIVE ON ENTRY INDICATES THAT S IS WANTED. */

/*    ON EXIT LC AND LS GIVE THE NUMBER OF PARTITIONS USED */
/*    FOR THE COMPUTATION OF C AND S. */

/*    Output, double precision C, the value of the cosine integral, */
/*    if requested. */

/*    Output, double precision S, the value of the sine integral, */
/*    if requested. */

    if (*eps < 0.) {
	*lc = -(*lc);
	*ls = -(*ls);
	return 0;
    }
    if (*b <= *a) {
	*eps = -(*eps);
	*lc = -(*lc);
	*ls = -(*ls);
	return 0;
    }
    n = 1;
    w1 = abs(*w);
    temp = (*b - *a) * 2. * w1 / pi;
    if (2. < temp) {
	n = (integer) (log(temp) / .693);
    }

/*  0.693 = alog(2.) rounded downwards. */

/* Computing MAX */
    i__1 = *maxh, i__2 = n + 1;
    mxn = max(i__1,i__2);
    fa = (*f)(obj,a);
    fb = (*f)(obj,b);
    cosa = cos(w1 * *a);
    sina = sin(w1 * *a);
    cosb = cos(w1 * *b);
    sinb = sin(w1 * *b);
    h__ = (*b - *a) / (doublereal) pow(c__2, n);
    nstop = pow(c__2, n) - 1;
    nst = 1;

/*  TMAX is the switch-over point for TETA. */
/*  Analysis shows that with a 56 bit floating point mantissa, */
/*    tmax = 0.2D+00 */
/*  is suitable, while with a 24 bit mantissa, */
/*    tmax = 1.0E+00 */
/*  is preferred. */

    tmax = .2;

/*  tmaxb is the switch-over point in beta, where the */
/*  cancellation is strongest. */

    tmaxb = tmax * 5.;

/*  Save information from input LS and LC, because they will */
/*  also be used for output. */

    if (0 < *ls) {
	lls = 1;
    } else {
	lls = 2;
    }
    if (0 < *lc) {
	llc = 1;
    } else {
	llc = 2;
    }
    sumcos = (fa * cosa + fb * cosb) * .5;
    sumsin = (fa * sina + fb * sinb) * .5;

/*  All of the above is executed only once per call. */

/*  Now the iteration begins. */
/*  The constant 'M' is used in the Richardson extrapolation. */
/*  M-1 is the number of times the original step length 'H' */
/*  has been divided by two. */

    m = 1;
L10:
    h2 = h__ * h__;
    teta = w1 * h__;
    i__1 = nstop;
    i__2 = nst;
    for (i__ = 1; i__2 < 0 ? i__ >= i__1 : i__ <= i__1; i__ += i__2) {
	x = *a + h__ * (doublereal) i__;
	wx = w1 * x;
	fx = (*f)(obj,&x);
	if (lls == 1) {
	    sumsin += fx * sin(wx);
	}
	if (llc == 1) {
	    sumcos += fx * cos(wx);
	}
    }
    t2 = teta * teta;
/* Computing 2nd power */
    d__1 = sin(teta * .5);
    temp = 1. - d__1 * d__1 / 1.5;

/*  Power series for small TETA. */

    if (teta <= tmax) {
	alfa = teta * (1. - t2 * (.13333333333333333 - t2 * (
		.011309523809523809 - t2 * (5.1587301587301593e-4 - t2 * (
		1.4680535513868846e-5 - t2 * 181. / 6.19164e8))))) / 12.;
	delta = t2 * (.011111111111111112 - t2 * (4.1335978835978834e-4 - t2 *
		 (7.7160493827160496e-6 - t2 / 11404800.))) - 
		.083333333333333329;
	epsil = 1. - t2 * (.16666666666666666 - t2 * (.0125 - t2 * (
		5.6216931216931216e-4 - t2 * (1.7085537918871254e-5 - t2 / 
		2661120.))));
	t3 = t2;
	beta = teta * h2 * (1. - t2 / 21. * (1. - t2 * (.020833333333333332 - 
		t2 * (2.5252525252525253e-4 - t3 / 494208.)))) / 180.;

/*  Closed form for larger TETA. */

    } else {
/* Computing 2nd power */
	d__1 = teta * .5;
	temp1 = d__1 * d__1;
/* Computing 2nd power */
	d__1 = sin(teta * .5);
	temp2 = d__1 * d__1 / temp1;
	temp3 = sin(teta) / teta;
	alfa = (temp - temp2 * temp3) / teta;
	delta = (temp - temp2) / t2;
	epsil = temp2 * temp2;
	if (teta <= tmaxb) {
	    t3 = t2 * (1. - t2 * (.0057142857142857143 - t2 * (
		    2.4509803921568626e-5 - t2 / 12209400.)));
	    beta = teta * h2 * (1. - t2 / 21. * (1. - t2 * (
		    .020833333333333332 - t2 * (2.5252525252525253e-4 - t3 / 
		    494208.)))) / 180.;
	} else {
	    beta = (temp - temp3) / (teta * w1 * w1);
	}
    }

/*  Integration formulas. */

    if (lls == 1) {
	ts = h__ * ((beta * *fbb - alfa * fb) * cosb + (alfa * fa - beta * *
		fba) * cosa + delta * h__ * (*fpb * sinb - *fpa * sina) + 
		epsil * sumsin) / temp;
	endt2_(pvts, &ts, eps, s, &lls, &m);
	*ls = n;
    }
    if (llc == 1) {
	tc = h__ * ((alfa * fb - beta * *fbb) * sinb + (beta * *fba - alfa * 
		fa) * sina + delta * h__ * (*fpb * cosb - *fpa * cosa) + 
		epsil * sumcos) / temp;
	endt2_(pvtc, &tc, eps, c__, &llc, &m);
	*lc = n;
    }

/*  Now test to see if done. */

    if (llc + lls <= 3) {
	++n;

/*  This is the beginning of the iteration. */

	if (n <= mxn) {
	    h__ *= .5;
	    nst = 2;
	    nstop = pow(c__2, n);
	    ++m;
	    goto L10;
	}
	*eps = -(*eps);
    }
    if (0 < *ls && *w < 0.) {
	*s = -(*s);
    }
    return 0;
} /* fspl2_ */

/* Subroutine */ int endt2_(doublereal *prevot, doublereal *quant, doublereal 
	*eps, doublereal *value, integer *l, integer *m)
{
    /* Initialized data */

    static doublereal rich[7] = { 0.,15.,63.,255.,1023.,4095.,16383. };

    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer k;
    static doublereal diff, reps, temp1, temp2;

/* *********************************************************************72 */

/* c ENDT2 performs Richardson extrapolation. */

/*  Discussion: */

/*    ENDT2 performs Richardson extra- */
/*    polation of the values 'quant' which are introduced into */
/*    the routine each time it is called, each time with */
/*    increasing value of 'm', starting with m = 1. */

/*    the current values are stored in the array 'prevot', where 'prevot(1)' */
/*    at exit is equal to 'quant'.  the best value for the moment */
/*    is given in 'value'.  endt2 requires the present value to */
/*    agree with the previous value to within eps2, where */
/*    eps2 = eps*(1.0 + abs(present value)). */

/*    eps is supplied by the user. */

/*    the error expansion is of the form */
/*    error = c4*h**4 + c6*h**6 + c8*h**8 + ... + cn*h**n + ... */

    /* Parameter adjustments */
    --prevot;

    /* Function Body */

/*  rich(1) is not used. */
/*  rich(k) = 2**(2*k) - 1, k = 2,3,4,5,6,7. */

    temp2 = prevot[1];
    prevot[1] = *quant;
    temp1 = *quant;
    if (*m == 1) {
	*value = temp1;
	return 0;
    }
    reps = *eps * (abs(*quant) + 1.);
    i__1 = *m;
    for (k = 2; k <= i__1; ++k) {
	diff = temp1 - temp2;
	if (abs(diff) <= reps) {
	    *l = 2;
	    *value = temp1;
	    return 0;
	}
	if (k == 8) {
	    *value = temp1;
	    return 0;
	}
	temp1 += diff / rich[k - 1];
	temp2 = prevot[k];
	prevot[k] = temp1;
    }
    *value = temp1;
    return 0;
} /* endt2_ */

