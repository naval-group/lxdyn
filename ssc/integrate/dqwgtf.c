/* dqwgtf.f -- translated by f2c (version 20100827). */

#include "ssc/fortran.h"
#include "ssc/integrate/quadrature.h"

doublereal dqwgtf_(
    doublereal* x,
    doublereal* omega,
    doublereal* p2,
    doublereal* p3,
    doublereal* p4,
    integer* integr)
{
    /* System generated locals */
    doublereal ret_val;

    /* Builtin functions */
    double cos(doublereal), sin(doublereal);

    /* Local variables */
    static doublereal omx;

    /* The weight signature dqk15w calls; cos and sin weights use only omega. */
    (void)p2;
    (void)p3;
    (void)p4;

    /* ***begin prologue  dqwgtf */
    /* ***refer to   dqk15w */
    /* ***routines called  (none) */
    /* ***revision date 810101   (yymmdd) */
    /* ***keywords  cos or sin in weight function */
    /* ***author  piessens,robert, appl. math. & progr. div. - k.u.leuven */
    /*           de doncker,elise,appl. math. * progr. div. - k.u.leuven */
    /* ***end prologue  dqwgtf */

    /* ***first executable statement  dqwgtf */
    omx = *omega * *x;
    switch (*integr) {
        case 1:
            goto L10;
        case 2:
            goto L20;
    }
L10:
    ret_val = cos(omx);
    goto L30;
L20:
    ret_val = sin(omx);
L30:
    return ret_val;
} /* dqwgtf_ */
