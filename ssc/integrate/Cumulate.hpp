/*
 * Cumulate.hpp
 *
 * \date 8 oct. 2013, 08:22:31
 * \author cec
 */

#ifndef CUMULATE_HPP_
#define CUMULATE_HPP_

#include "ssc/pimpl_idiom/generic_pimpl.hpp"

/** \author cec
 *  \ingroup integrate
 *  \brief Calculates the integral of function defined by (x,y) pairs.
 *  \details This class calculates the sum (integral) of a variable
over time. Unlike the other classes in the same module (Simpson, Quadpack,
etc.), it allows adding points to the function being integrated. The
function is not defined analytically but by (x,y) pairs. The integration
scheme is trapezoidal integration.
 *  \section ex1 Example
 *  \snippet integrate/unit_tests/src/CumulateTest.cpp CumulateTest example
 *  \section ex2 Expected output
 *  \snippet integrate/unit_tests/src/CumulateTest.cpp CumulateTest expected output
 */
namespace ssc
{
    namespace integrate
    {
        class Cumulate
        {
            public:
                Cumulate();
                ~Cumulate();
                void add(const double x, const double y);
                double integrate(const double a, const double b);

            private:
                class Impl;
                Pimpl<Impl> pimpl;
        };
    }
}

#endif /* CUMULATE_HPP_ */
