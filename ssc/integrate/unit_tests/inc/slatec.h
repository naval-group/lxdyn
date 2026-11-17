/* SLATEC's cosine and sine integrals, the reference values of the quadrature tests. */
#ifndef SSC_INTEGRATE_SLATEC_H
#define SSC_INTEGRATE_SLATEC_H

#ifdef __cplusplus
extern "C" {
#endif

double dci(double* x);
double dsi(double* x);
int d9sifg_(double* x, double* f, double* g);
double dcsevl_(double* x, double* a, int* n);
int initds_(double* os, int* nos, float* eta);

#ifdef __cplusplus
}
#endif

#endif
