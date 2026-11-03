/* src/cubspl.f -- translated by f2c (version 20100827).
   You must link the resulting object file with libf2c:
	on Microsoft Windows system, link with libf2c.lib;
	on Linux or Unix systems, link with .../path/to/libf2c.a -lm
	or, if you install libf2c.a in a standard place, with -lf2c -lm
	-- in that order, at the end of the command line, as in
		cc *.o -lf2c -lm
	Source for libf2c is in /netlib/f2c/libf2c.zip, e.g.,

		http://www.netlib.org/f2c/libf2c.zip
*/

#include "f2c.h"

int cubspl_(doublereal *tau, doublereal *c__, integer *n,
	integer *ibcbeg, integer *ibcend);
/* Subroutine */ int cubspl_(doublereal *tau, doublereal *c__, integer *n,
	integer *ibcbeg, integer *ibcend)
{
    /* System generated locals */
    integer i__1;
    doublereal d__1;

    /* Local variables */
    static doublereal g;
    static integer i__, j, l, m;
    static doublereal dtau, divdf1, divdf3;

/*  from  * a practical guide to splines *  by c. de boor */
/*     ************************  input  *************************** */
/*     n = number of data points. assumed to be .ge. 2. */
/*     (tau(i), c(1,i), i=1,...,n) = abscissae and ordinates of the */
/*        data points. tau is assumed to be strictly increasing. */
/*     ibcbeg, ibcend = boundary condition indicators, and */
/*     c(2,1), c(2,n) = boundary condition information. specifically, */
/*        ibcbeg = 0  means no boundary condition at tau(1) is given. */
/*           in this case, the not-a-knot condition is used, i.e. the */
/*           jump in the third derivative across tau(2) is forced to */
/*           zero, thus the first and the second cubic polynomial pieces */
/*           are made to coincide.) */
/*        ibcbeg = 1  means that the slope at tau(1) is made to equal */
/*           c(2,1), supplied by input. */
/*        ibcbeg = 2  means that the second derivative at tau(1) is */
/*           made to equal c(2,1), supplied by input. */
/*        ibcend = 0, 1, or 2 has analogous meaning concerning the */
/*           boundary condition at tau(n), with the additional infor- */
/*           mation taken from c(2,n). */
/*     ***********************  output  ************************** */
/*     c(j,i), j=1,...,4; i=1,...,l (= n-1) = the polynomial coefficients */
/*        of the cubic interpolating spline with interior knots (or */
/*        joints) tau(2), ..., tau(n-1). precisely, in the interval */
/*        (tau(i), tau(i+1)), the spline f is given by */
/*           f(x) = c(1,i)+h*(c(2,i)+h*(c(3,i)+h*c(4,i)/3.)/2.) */
/*        where h = x - tau(i). the function program *ppvalu* may be */
/*        used to evaluate f or its derivatives from tau,c, l = n-1, */
/*        and k=4. */
/* ****** a tridiagonal linear system for the unknown slopes s(i) of */
/*  f  at tau(i), i=1,...,n, is generated and then solved by gauss elim- */
/*  ination, with s(i) ending up in c(2,i), all i. */
/*     c(3,.) and c(4,.) are used initially for temporary storage. */
    /* Parameter adjustments */
    c__ -= 5;
    --tau;

    /* Function Body */
    l = *n - 1;
/* ompute first differences of tau sequence and store in c(3,.). also, */
/* ompute first divided difference of data and store in c(4,.). */
    i__1 = *n;
    for (m = 2; m <= i__1; ++m) {
	c__[(m << 2) + 3] = tau[m] - tau[m - 1];
/* L10: */
	c__[(m << 2) + 4] = (c__[(m << 2) + 1] - c__[(m - 1 << 2) + 1]) / c__[
		(m << 2) + 3];
    }
/* onstruct first equation from the boundary condition, of the form */
/*             c(4,1)*s(1) + c(3,1)*s(2) = c(2,1) */
    if ((i__1 = *ibcbeg - 1) < 0) {
	goto L11;
    } else if (i__1 == 0) {
	goto L15;
    } else {
	goto L16;
    }
L11:
    if (*n > 2) {
	goto L12;
    }
/*     no condition at left end and n = 2. */
    c__[8] = 1.f;
    c__[7] = 1.f;
    c__[6] = c__[12] * 2.f;
    goto L25;
/*     not-a-knot condition at left end and n .gt. 2. */
L12:
    c__[8] = c__[15];
    c__[7] = c__[11] + c__[15];
/* Computing 2nd power */
    d__1 = c__[11];
    c__[6] = ((c__[11] + c__[7] * 2.f) * c__[12] * c__[15] + d__1 * d__1 *
	    c__[16]) / c__[7];
    goto L19;
/*     slope prescribed at left end. */
L15:
    c__[8] = 1.f;
    c__[7] = 0.f;
    goto L18;
/*     second derivative prescribed at left end. */
L16:
    c__[8] = 2.f;
    c__[7] = 1.f;
    c__[6] = c__[12] * 3.f - c__[11] / 2.f * c__[6];
L18:
    if (*n == 2) {
	goto L25;
    }
/*  if there are interior knots, generate the corresp. equations and car- */
/*  ry out the forward pass of gauss elimination, after which the m-th */
/*  equation reads    c(4,m)*s(m) + c(3,m)*s(m+1) = c(2,m). */
L19:
    i__1 = l;
    for (m = 2; m <= i__1; ++m) {
	g = -c__[(m + 1 << 2) + 3] / c__[(m - 1 << 2) + 4];
	c__[(m << 2) + 2] = g * c__[(m - 1 << 2) + 2] + (c__[(m << 2) + 3] *
		c__[(m + 1 << 2) + 4] + c__[(m + 1 << 2) + 3] * c__[(m << 2)
		+ 4]) * 3.f;
/* L20: */
	c__[(m << 2) + 4] = g * c__[(m - 1 << 2) + 3] + (c__[(m << 2) + 3] +
		c__[(m + 1 << 2) + 3]) * 2.f;
    }
/* onstruct last equation from the second boundary condition, of the form */
/*           (-g*c(4,n-1))*s(n-1) + c(4,n)*s(n) = c(2,n) */
/*     if slope is prescribed at right end, one can go directly to back- */
/*     substitution, since c array happens to be set up just right for it */
/*     at this point. */
    if ((i__1 = *ibcend - 1) < 0) {
	goto L21;
    } else if (i__1 == 0) {
	goto L30;
    } else {
	goto L24;
    }
L21:
    if (*n == 3 && *ibcbeg == 0) {
	goto L22;
    }
/*     not-a-knot and n .ge. 3, and either n.gt.3 or  also not-a-knot at */
/*     left end point. */
    g = c__[(*n - 1 << 2) + 3] + c__[(*n << 2) + 3];
/* Computing 2nd power */
    d__1 = c__[(*n << 2) + 3];
    c__[(*n << 2) + 2] = ((c__[(*n << 2) + 3] + g * 2.f) * c__[(*n << 2) + 4]
	    * c__[(*n - 1 << 2) + 3] + d__1 * d__1 * (c__[(*n - 1 << 2) + 1]
	    - c__[(*n - 2 << 2) + 1]) / c__[(*n - 1 << 2) + 3]) / g;
    g = -g / c__[(*n - 1 << 2) + 4];
    c__[(*n << 2) + 4] = c__[(*n - 1 << 2) + 3];
    goto L29;
/*     either (n=3 and not-a-knot also at left) or (n=2 and not not-a- */
/*     knot at left end point). */
L22:
    c__[(*n << 2) + 2] = c__[(*n << 2) + 4] * 2.f;
    c__[(*n << 2) + 4] = 1.f;
    goto L28;
/*     second derivative prescribed at right endpoint. */
L24:
    c__[(*n << 2) + 2] = c__[(*n << 2) + 4] * 3.f + c__[(*n << 2) + 3] / 2.f *
	     c__[(*n << 2) + 2];
    c__[(*n << 2) + 4] = 2.f;
    goto L28;
L25:
    if ((i__1 = *ibcend - 1) < 0) {
	goto L26;
    } else if (i__1 == 0) {
	goto L30;
    } else {
	goto L24;
    }
L26:
    if (*ibcbeg > 0) {
	goto L22;
    }
/*     not-a-knot at right endpoint and at left endpoint and n = 2. */
    c__[(*n << 2) + 2] = c__[(*n << 2) + 4];
    goto L30;
L28:
    g = -1.f / c__[(*n - 1 << 2) + 4];
/* omplete forward pass of gauss elimination. */
L29:
    c__[(*n << 2) + 4] = g * c__[(*n - 1 << 2) + 3] + c__[(*n << 2) + 4];
    c__[(*n << 2) + 2] = (g * c__[(*n - 1 << 2) + 2] + c__[(*n << 2) + 2]) /
	    c__[(*n << 2) + 4];
/* arry out back substitution */
L30:
    j = l;
L40:
    c__[(j << 2) + 2] = (c__[(j << 2) + 2] - c__[(j << 2) + 3] * c__[(j + 1 <<
	     2) + 2]) / c__[(j << 2) + 4];
    --j;
    if (j > 0) {
	goto L40;
    }
/* ****** generate cubic coefficients in each interval, i.e., the deriv.s */
/*  at its left endpoint, from value and slope at its endpoints. */
    i__1 = *n;
    for (i__ = 2; i__ <= i__1; ++i__) {
	dtau = c__[(i__ << 2) + 3];
	divdf1 = (c__[(i__ << 2) + 1] - c__[(i__ - 1 << 2) + 1]) / dtau;
	divdf3 = c__[(i__ - 1 << 2) + 2] + c__[(i__ << 2) + 2] - divdf1 * 2.f;
	c__[(i__ - 1 << 2) + 3] = (divdf1 - c__[(i__ - 1 << 2) + 2] - divdf3)
		* 2.f / dtau;
/* L50: */
	c__[(i__ - 1 << 2) + 4] = divdf3 / dtau * (6.f / dtau);
    }
    return 0;
} /* cubspl_ */

