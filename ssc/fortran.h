/* The types and macros f2c's output expects, with libf2c's definitions. */
#ifndef SSC_FORTRAN_H
#define SSC_FORTRAN_H

typedef int integer;
typedef float real;
typedef double doublereal;
typedef long int logical;
typedef doublereal (*D_fp)();

#define TRUE_ (1)
#define FALSE_ (0)
#define abs(x) ((x) >= 0 ? (x) : -(x))
#define dabs(x) (doublereal) abs(x)
#define min(a, b) ((a) <= (b) ? (a) : (b))
#define max(a, b) ((a) >= (b) ? (a) : (b))

#endif
