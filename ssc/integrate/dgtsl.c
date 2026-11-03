/* dgtsl.f -- translated by f2c (version 20100827).
   You must link the resulting object file with libf2c:
	on Microsoft Windows system, link with libf2c.lib;
	on Linux or Unix systems, link with .../path/to/libf2c.a -lm
	or, if you install libf2c.a in a standard place, with -lf2c -lm
	-- in that order, at the end of the command line, as in
		cc *.o -lf2c -lm
	Source for libf2c is in /netlib/f2c/libf2c.zip, e.g.,

		http://www.netlib.org/f2c/libf2c.zip
*/

#include "ssc/f2c/f2c.h"

/* Subroutine */ int dgtsl_(integer *n, doublereal *c__, doublereal *d__, 
	doublereal *e, doublereal *b, integer *info)
{
    /* System generated locals */
    integer i__1;
    doublereal d__1, d__2;

    /* Local variables */
    static integer k;
    static doublereal t;
    static integer kb, kp1, nm1, nm2;


/*     dgtsl given a general tridiagonal matrix and a right hand */
/*     side will find the solution. */

/*     on entry */

/*        n       integer */
/*                is the order of the tridiagonal matrix. */

/*        c       double precision(n) */
/*                is the subdiagonal of the tridiagonal matrix. */
/*                c(2) through c(n) should contain the subdiagonal. */
/*                on output c is destroyed. */

/*        d       double precision(n) */
/*                is the diagonal of the tridiagonal matrix. */
/*                on output d is destroyed. */

/*        e       double precision(n) */
/*                is the superdiagonal of the tridiagonal matrix. */
/*                e(1) through e(n-1) should contain the superdiagonal. */
/*                on output e is destroyed. */

/*        b       double precision(n) */
/*                is the right hand side vector. */

/*     on return */

/*        b       is the solution vector. */

/*        info    integer */
/*                = 0 normal value. */
/*                = k if the k-th element of the diagonal becomes */
/*                    exactly zero.  the subroutine returns when */
/*                    this is detected. */

/*     linpack. this version dated 08/14/78 . */
/*     jack dongarra, argonne national laboratory. */

/*     no externals */
/*     fortran dabs */

/*     internal variables */

/*     begin block permitting ...exits to 100 */

    /* Parameter adjustments */
    --b;
    --e;
    --d__;
    --c__;

    /* Function Body */
    *info = 0;
    c__[1] = d__[1];
    nm1 = *n - 1;
    if (nm1 < 1) {
	goto L40;
    }
    d__[1] = e[1];
    e[1] = 0.;
    e[*n] = 0.;

    i__1 = nm1;
    for (k = 1; k <= i__1; ++k) {
	kp1 = k + 1;

/*              find the largest of the two rows */

	if ((d__1 = c__[kp1], abs(d__1)) < (d__2 = c__[k], abs(d__2))) {
	    goto L10;
	}

/*                 interchange row */

	t = c__[kp1];
	c__[kp1] = c__[k];
	c__[k] = t;
	t = d__[kp1];
	d__[kp1] = d__[k];
	d__[k] = t;
	t = e[kp1];
	e[kp1] = e[k];
	e[k] = t;
	t = b[kp1];
	b[kp1] = b[k];
	b[k] = t;
L10:

/*              zero elements */

	if (c__[k] != 0.) {
	    goto L20;
	}
	*info = k;
/*     ............exit */
	goto L100;
L20:
	t = -c__[kp1] / c__[k];
	c__[kp1] = d__[kp1] + t * d__[k];
	d__[kp1] = e[kp1] + t * e[k];
	e[kp1] = 0.;
	b[kp1] += t * b[k];
/* L30: */
    }
L40:
    if (c__[*n] != 0.) {
	goto L50;
    }
    *info = *n;
    goto L90;
L50:

/*           back solve */

    nm2 = *n - 2;
    b[*n] /= c__[*n];
    if (*n == 1) {
	goto L80;
    }
    b[nm1] = (b[nm1] - d__[nm1] * b[*n]) / c__[nm1];
    if (nm2 < 1) {
	goto L70;
    }
    i__1 = nm2;
    for (kb = 1; kb <= i__1; ++kb) {
	k = nm2 - kb + 1;
	b[k] = (b[k] - d__[k] * b[k + 1] - e[k] * b[k + 2]) / c__[k];
/* L60: */
    }
L70:
L80:
L90:
L100:

    return 0;
} /* dgtsl_ */

