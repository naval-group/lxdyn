/* initds.f -- translated by f2c (version 20100827). */

#include "slatec.h"
#include "ssc/fortran.h"
#include "ssc/integrate/quadrature.h"

/* DECK INITDS */
integer initds_(doublereal* os, integer* nos, real* eta)
{
    /* System generated locals */
    integer ret_val, i__1;

    /* Local variables */
    static integer i__, ii;
    static real err;

    /* ***BEGIN PROLOGUE  INITDS */
    /* ***PURPOSE  Determine the number of terms needed in an orthogonal */
    /*            polynomial series so that it meets a specified accuracy. */
    /* ***LIBRARY   SLATEC (FNLIB) */
    /* ***CATEGORY  C3A2 */
    /* ***TYPE      DOUBLE PRECISION (INITS-S, INITDS-D) */
    /* ***KEYWORDS  CHEBYSHEV, FNLIB, INITIALIZE, ORTHOGONAL POLYNOMIAL, */
    /*             ORTHOGONAL SERIES, SPECIAL FUNCTIONS */
    /* ***AUTHOR  Fullerton, W., (LANL) */
    /* ***DESCRIPTION */

    /*  Initialize the orthogonal series, represented by the array OS, so */
    /*  that INITDS is the number of terms needed to insure the error is no */
    /*  larger than ETA.  Ordinarily, ETA will be chosen to be one-tenth */
    /*  machine precision. */

    /*             Input Arguments -- */
    /*   OS     double precision array of NOS coefficients in an orthogonal */
    /*          series. */
    /*   NOS    number of coefficients in OS. */
    /*   ETA    single precision scalar containing requested accuracy of */
    /*          series. */

    /* ***REFERENCES  (NONE) */
    /* ***ROUTINES CALLED  XERMSG */
    /* ***REVISION HISTORY  (YYMMDD) */
    /*   770601  DATE WRITTEN */
    /*   890531  Changed all specific intrinsics to generic.  (WRB) */
    /*   890831  Modified array declarations.  (WRB) */
    /*   891115  Modified error message.  (WRB) */
    /*   891115  REVISION DATE from Version 3.2 */
    /*   891214  Prologue converted to Version 4.0 format.  (BAB) */
    /*   900315  CALLs to XERROR changed to CALLs to XERMSG.  (THJ) */
    /* ***END PROLOGUE  INITDS */
    /* ***FIRST EXECUTABLE STATEMENT  INITDS */
    /* Parameter adjustments */
    --os;

    /* Function Body */
    err = 0.f;
    i__1 = *nos;
    for (ii = 1; ii <= i__1; ++ii) {
        i__ = *nos + 1 - ii;
        err += dabs((real)os[i__]);
        if (err > *eta) {
            goto L20;
        }
        /* L10: */
    }

L20:
    ret_val = i__;

    return ret_val;
} /* initds_ */
