/*
 * Interpolator.hpp
 *
 * \date 9 sept. 2013, 11:11:02
 * \author cec
 */

#ifndef INTERPOLATOR_HPP_
#define INTERPOLATOR_HPP_

#include <cstddef> // For size_t

namespace ssc
{
    namespace interpolation
    {
        /** \author cec
         *  \ingroup interpolation
         *  \brief Responsibility
         *  \details
         *  \section ex1 Example
         *  \snippet interpolation/unit_tests/src/InterpolatorTest.cpp InterpolatorTest example
         *  \section ex2 Expected output
         *  \snippet interpolation/unit_tests/src/InterpolatorTest.cpp InterpolatorTest expected output
         */
        class Interpolator
        {
            public:
                Interpolator();
                virtual ~Interpolator();
                virtual double f(const double x0);
                double df(const double x0, const size_t derivative_order=1);

            protected:
                virtual double get_f() const = 0;
                virtual double get_df(const size_t derivative_order) const = 0;
                virtual void update_coefficients_if_necessary(const double x0) = 0;
        };
    }
}

#endif /* INTERPOLATOR_HPP_ */
