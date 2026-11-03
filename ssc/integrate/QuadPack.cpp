/*
 * QuadPack.cpp
 *
 * \date Dec 9, 2014
 * \author cec
 */

#include "ssc/integrate/QuadPack.hpp"
#include "ssc/exception_handling/Exception.hpp"
#include <iostream> // For cerr

namespace ssc
{
    namespace integrate
    {
        class QuadPackException : public ssc::exception_handling::Exception
        {
            public:
                QuadPackException(const std::string& message, const std::string& file, const std::string& function, const unsigned int line) :
                                                Exception(message, file, function, line)
                {
                }
        };
    }
}

ssc::integrate::QuadPack::QuadPack(const Function& f_) : Integrator(f_)
{
}

ssc::integrate::QuadPack::QuadPack() : Integrator()
{
}

ssc::integrate::QuadPack::~QuadPack()
{
}

double ssc::integrate::QuadPack::op(double *x)
{
    const double ret = f(*x);
    return ret;
}

void ssc::integrate::QuadPack::throw_any_errors(const int ier) const
{
    if (ier < 0)
    {
        THROW(__PRETTY_FUNCTION__, ssc::integrate::QuadPackException, "ier = " << ier << ": possible memory corruption");
    }
    if (ier == 1)
    {
        THROW(__PRETTY_FUNCTION__, ssc::integrate::QuadPackException, "Maximum number of subdivisions allowed has been achieved. One can allow more sub-divisions by increasing the value of limit (and taking the according dimension adjustments into account). However, if this yields no improvement it is advised to analyze the integrand in order to determine the integration difficulties. If the position of a local difficulty can be determined (e.g. singularity, discontinuity within the interval) one will probably gain from splitting up the interval at this point and calling the integrator on the subranges. If possible, an appropriate special-purpose integrator should be used, which is designed for handling the type of difficulty involved.");
    }
    if (ier == 2)
    {
        std::cerr << "Warning: the occurrence of roundoff error is detected in the numerical integration algorithm (Gauss-Kronrod), which prevents the requested tolerance from being achieved. The error may be under-estimated." << std::endl;
    }
    if (ier == 3)
    {
        THROW(__PRETTY_FUNCTION__, ssc::integrate::QuadPackException, "Extremely bad integrand behaviour occurs at some points of the integration interval.");
    }
    if (ier == 4)
    {
        std::cerr << "Warning: the numerical integration algorithm (Gauss-Kronrod) does not converge. Roundoff error is detected in the extrapolation table. It is presumed that the requested tolerance cannot be achieved, and that the returned result is the best which can be obtained." << std::endl;
    }
    if (ier == 5)
    {
        THROW(__PRETTY_FUNCTION__, ssc::integrate::QuadPackException, "The integral is probably divergent, or slowly convergent.");
    }
    if (ier == 6)
    {
        THROW(__PRETTY_FUNCTION__, ssc::integrate::QuadPackException, "The input is invalid, because (epsabs<=0 and epsrel<max(50*rel.mach.acc.,0.5d-28) or limit<1 or lenw<limit*4., or (integr.ne.1 and integr.ne.2) or icall.lt.1 or maxp1.lt.1.");
    }
}
