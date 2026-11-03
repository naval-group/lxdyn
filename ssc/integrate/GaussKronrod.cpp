/*
 * QuadPack.cpp
 *
 * \date 5 juil. 2013, 16:48:53
 * \author cec
 */

#include "GaussKronrod.hpp"

#define LIMIT 2500
#define LENW 16010
#define LENIW 500
#define MAXP1 21

#include <iostream>
#include <cassert>
#include <cstdlib>

extern "C"
{
void dqags_(double f(void*,double*), void*, double*, double*, double*, double*, double*, double*, int*, int*, int*, int*, int* last, int* iwork, double* work);
}

ssc::integrate::GaussKronrod::GaussKronrod() : QuadPack(),
                                       iwork(new int[LIMIT]),
                                       work(new double[LENW])
{
}

ssc::integrate::GaussKronrod::GaussKronrod(const Function& f_) : QuadPack(f_),
                                         iwork(new int[LIMIT]),
                                         work(new double[LENW])
{
    for (size_t i = 0 ; i < LIMIT ; ++i)
    {
        iwork[i] = 0;
    }
    for (size_t i = 0 ; i < LENW ; ++i)
    {
        work[i] = 0;
    }
}

double ssc::integrate::GaussKronrod::integrate_impl(const Function& f_, double a, double b, double eps)
{
    f = f_;
    int neval = 0;
    int ier = 0;
    int last = 0;

    double epsabs = eps;
    double epsrel = 1E-4;
    double abserr = 1E-4;

    int lenw = LENW, limit = LIMIT;
    double res = 0;
    dqags_(integrand<GaussKronrod>, (void*)(this), &a, &b, &epsabs, &epsrel, &res, &abserr, &neval, &ier, &limit, &lenw, &last, iwork, work);
    throw_any_errors(ier);
    return res;
}

ssc::integrate::GaussKronrod::~GaussKronrod()
{
    if (iwork) delete [] iwork;
    if (work)  delete [] work;
}

ssc::integrate::GaussKronrod::GaussKronrod(const GaussKronrod& rhs) : QuadPack(rhs.f),
                                          iwork(new int[LIMIT]),
                                          work(new double[LENW])
{
    for (size_t i = 0 ; i < LIMIT ; ++i)
    {
        iwork[i] = rhs.iwork[i];
    }
    for (size_t i = 0 ; i < LENW ; ++i)
    {
        work[i] = rhs.work[i];
    }
}

ssc::integrate::GaussKronrod& ssc::integrate::GaussKronrod::operator=(const GaussKronrod& rhs)
{
    if (this != &rhs)
    {
        if (iwork) delete [] iwork;
        if (work) delete [] work;
        iwork = new int[LIMIT];
        work = new double[LENW];
        for (size_t i = 0 ; i < LIMIT ; ++i)
        {
            iwork[i] = rhs.iwork[i];
        }
        for (size_t i = 0 ; i < LENW ; ++i)
        {
            work[i] = rhs.work[i];
        }
    }
    return *this;
}
