/*
 * IntegrateOscillatory.cpp
 *
 * \date Dec 9, 2014
 * \author cec
 */

#include "ssc/integrate/ClenshawCurtis.hpp"
#include "ssc/macros/test_macros.hpp"
#define MAX_NB_OF_PARTITIONS 2500 // Maximum number of subdivisions in the partition of (a,b)
#define MAX_NB_OF_CHEBYCHEV_MOMENTS 1000

ssc::integrate::ClenshawCurtis::~ClenshawCurtis()
{
    if (chebychev_moments) delete [] chebychev_moments;
    if (alist) delete [] alist;
    if (blist) delete [] blist;
    if (rlist) delete [] rlist;
    if (elist) delete [] elist;
    if (iord) delete [] iord;
    if (nnlog) delete [] nnlog;
}

ssc::integrate::ClenshawCurtis::ClenshawCurtis(const ClenshawCurtis& rhs) : QuadPack(rhs.f), tau(rhs.tau),
        nb_of_chebychev_moments(rhs.nb_of_chebychev_moments),
                        nb_of_calls(rhs.nb_of_calls),
                        chebychev_moments(new double[nb_of_chebychev_moments]),
                        alist(new double[MAX_NB_OF_PARTITIONS]),
                        blist(new double[MAX_NB_OF_PARTITIONS]),
                        rlist(new double[MAX_NB_OF_PARTITIONS]),
                        elist(new double[MAX_NB_OF_PARTITIONS]),
                        iord(new int[MAX_NB_OF_PARTITIONS]),
                        nnlog(new int[MAX_NB_OF_PARTITIONS])
{
    for (size_t i = 0 ; i < (size_t)nb_of_chebychev_moments ; ++i)
    {
        chebychev_moments[i] = rhs.chebychev_moments[i];
    }
    for (size_t i = 0 ; i < MAX_NB_OF_PARTITIONS ; ++i)
    {
        alist[i] = rhs.alist[i];
        blist[i] = rhs.blist[i];
        rlist[i] = rhs.rlist[i];
        elist[i] = rhs.elist[i];
        iord[i] = rhs.iord[i];
        nnlog[i] = rhs.nnlog[i];
    }
}

ssc::integrate::ClenshawCurtis& ssc::integrate::ClenshawCurtis::operator=(const ClenshawCurtis& rhs)
{
    f = rhs.f;
    tau = rhs.tau;
    nb_of_chebychev_moments = rhs.nb_of_chebychev_moments;
    nb_of_calls = rhs.nb_of_calls;
    if (chebychev_moments) delete [] chebychev_moments;
    if (alist) delete [] alist;
    if (blist) delete [] blist;
    if (rlist) delete [] rlist;
    if (elist) delete [] elist;
    if (iord) delete [] iord;
    if (nnlog) delete [] nnlog;
    chebychev_moments = new double[nb_of_chebychev_moments];
    alist = new double[MAX_NB_OF_PARTITIONS];
    blist = new double[MAX_NB_OF_PARTITIONS];
    rlist = new double[MAX_NB_OF_PARTITIONS];
    elist = new double[MAX_NB_OF_PARTITIONS];
    iord = new int[MAX_NB_OF_PARTITIONS];
    nnlog = new int[MAX_NB_OF_PARTITIONS];
    return *this;
}

double ssc::integrate::ClenshawCurtis::compute_for(const double tau_, const double a, const double b, const double eps)
{
    tau = tau_;
    return integrate_impl(f, a, b, eps);
}

ssc::integrate::ClenshawCurtis::ClenshawCurtis(const double tau_) : QuadPack(), tau(tau_),
        nb_of_chebychev_moments(25*MAX_NB_OF_CHEBYCHEV_MOMENTS),
        nb_of_calls(1),
        chebychev_moments(new double[nb_of_chebychev_moments]),
        alist(new double[MAX_NB_OF_PARTITIONS]),
        blist(new double[MAX_NB_OF_PARTITIONS]),
        rlist(new double[MAX_NB_OF_PARTITIONS]),
        elist(new double[MAX_NB_OF_PARTITIONS]),
        iord(new int[MAX_NB_OF_PARTITIONS]),
        nnlog(new int[MAX_NB_OF_PARTITIONS])
{
    for (size_t i = 0 ; i < (size_t)nb_of_chebychev_moments ; ++i)
    {
        chebychev_moments[i] = 0;
    }
    for (size_t i = 0 ; i < MAX_NB_OF_PARTITIONS ; ++i)
    {
        alist[i] = 0;
        blist[i] = 0;
        rlist[i] = 0;
        elist[i] = 0;
        iord[i] = 0;
        nnlog[i] = 0;
    }
}

ssc::integrate::ClenshawCurtis::ClenshawCurtis(const Function& f_, const double tau_) : QuadPack(f_), tau(tau_),
        nb_of_chebychev_moments(25*MAX_NB_OF_CHEBYCHEV_MOMENTS),
                        nb_of_calls(1),
                        chebychev_moments(new double[nb_of_chebychev_moments]),
                        alist(new double[MAX_NB_OF_PARTITIONS]),
                        blist(new double[MAX_NB_OF_PARTITIONS]),
                        rlist(new double[MAX_NB_OF_PARTITIONS]),
                        elist(new double[MAX_NB_OF_PARTITIONS]),
                        iord(new int[MAX_NB_OF_PARTITIONS]),
                        nnlog(new int[MAX_NB_OF_PARTITIONS])
{
    for (size_t i = 0 ; i < (size_t)nb_of_chebychev_moments ; ++i)
    {
        chebychev_moments[i] = 0;
    }
    for (size_t i = 0 ; i < MAX_NB_OF_PARTITIONS ; ++i)
    {
        alist[i] = 0;
        blist[i] = 0;
        rlist[i] = 0;
        elist[i] = 0;
        iord[i] = 0;
        nnlog[i] = 0;
    }
}

extern "C" int dqawoe_(double f(void*,double*), void*, double *a, double *b, double *omega, int *
        integr, double *epsabs, double *epsrel, int *limit, int *icall,
        int *maxp1, double *result, double *abserr, int *neval, int *
        ier, int *last, double *alist__, double *blist, double *rlist, double *
        elist, int *iord, int *nnlog, int *momcom, double *chebmo);

double ssc::integrate::ClenshawCurtis::integrate_impl(const Function& f_, double a, double b, double eps)
{
    f = f_;
    int weight_function_type = get_internal_type_number(); // 1 for cos, 2 for sin
    double epsabs = eps;
    double epsrel = eps;
    double abserr = 0;
    int limit = MAX_NB_OF_PARTITIONS;
    int maxp1 = MAX_NB_OF_CHEBYCHEV_MOMENTS;
    double result = 0;
    int nb_of_integrand_evaluations = 0;
    int ier = 0;
    int nb_of_subintervals_produced = 0;
    dqawoe_(integrand<ClenshawCurtis>,
           (void*)this,
           &a,
           &b,
           &tau,
           &weight_function_type,
           &epsabs,
           &epsrel,
           &limit,
           &nb_of_calls,
           &maxp1,
           &result,
           &abserr,
           &nb_of_integrand_evaluations,
           &ier,
           &nb_of_subintervals_produced,
           alist,
           blist,
           rlist,
           elist,
           iord,
           nnlog,
           &nb_of_chebychev_moments,
           chebychev_moments);
    throw_any_errors(ier);
    return result;
}

