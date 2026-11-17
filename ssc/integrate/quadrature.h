/* The Fortran-derived quadrature routines, callable from C and C++. */
#ifndef SSC_INTEGRATE_QUADRATURE_H
#define SSC_INTEGRATE_QUADRATURE_H

#ifdef __cplusplus
extern "C" {
#endif

int dqags_(
    double (*f)(void* obj, double* x),
    void* obj,
    double* a,
    double* b,
    double* epsabs,
    double* epsrel,
    double* result,
    double* abserr,
    int* neval,
    int* ier,
    int* limit,
    int* lenw,
    int* last,
    int* iwork,
    double* work);
int dqagse_(
    double (*f)(void* obj, double* x),
    void* obj,
    double* a,
    double* b,
    double* epsabs,
    double* epsrel,
    int* limit,
    double* result,
    double* abserr,
    int* neval,
    int* ier,
    double* alist,
    double* blist,
    double* rlist,
    double* elist,
    int* iord,
    int* last);
int dqk21_(
    double (*f)(void* obj, double* x),
    void* obj,
    double* a,
    double* b,
    double* result,
    double* abserr,
    double* resabs,
    double* resasc);
int dqawoe_(
    double (*f)(void* obj, double* x),
    void* obj,
    double* a,
    double* b,
    double* omega,
    int* integr,
    double* epsabs,
    double* epsrel,
    int* limit,
    int* icall,
    int* maxp1,
    double* result,
    double* abserr,
    int* neval,
    int* ier,
    int* last,
    double* alist,
    double* blist,
    double* rlist,
    double* elist,
    int* iord,
    int* nnlog,
    int* momcom,
    double* chebmo);
int dqc25f_(
    double (*f)(void* obj, double* x),
    void* obj,
    double* a,
    double* b,
    double* omega,
    int* integr,
    int* nrmom,
    int* maxp1,
    int* ksave,
    double* result,
    double* abserr,
    int* neval,
    double* resabs,
    double* resasc,
    int* momcom,
    double* chebmo);
int dqk15w_(
    double (*f)(void* obj, double* x),
    void* obj,
    double (*w)(double* x, double* omega, double* p2, double* p3, double* p4, int* integr),
    double* p1,
    double* p2,
    double* p3,
    double* p4,
    int* kp,
    double* a,
    double* b,
    double* result,
    double* abserr,
    double* resabs,
    double* resasc);
double dqwgtf_(double* x, double* omega, double* p2, double* p3, double* p4, int* integr);
int dqcheb_(double* x, double* fval, double* cheb12, double* cheb24);
int dqelg_(int* n, double* epstab, double* result, double* abserr, double* res3la, int* nres);
int dqpsrt_(
    int* limit,
    int* last,
    int* maxerr,
    double* ermax,
    double* elist,
    int* iord,
    int* nrmax);
int dgtsl_(int* n, double* c, double* d, double* e, double* b, int* info);
double d1mach_(int* i);
int filon(
    double (*f)(void* obj, double* x),
    void* obj,
    double* a,
    double* b,
    double* fpa,
    double* fpb,
    double* fba,
    double* fbb,
    double* w,
    double* eps,
    int* maxh,
    int* lc,
    int* ls,
    double* c,
    double* s);

#ifdef __cplusplus
}
#endif

#endif
