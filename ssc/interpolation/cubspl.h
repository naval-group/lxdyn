/* de Boor's cubic spline interpolation, callable from C and C++. */
#ifndef SSC_INTERPOLATION_CUBSPL_H
#define SSC_INTERPOLATION_CUBSPL_H

#ifdef __cplusplus
extern "C" {
#endif

int cubspl_(double* tau, double* c, int* n, int* ibcbeg, int* ibcend);

#ifdef __cplusplus
}
#endif

#endif
