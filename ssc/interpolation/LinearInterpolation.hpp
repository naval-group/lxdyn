/*
 * LinearInterpolation.hpp
 *
 * \date 8 nov. 2012, 14:32:11
 * \author cec
 */

#ifndef LINEARINTERPOLATION_HPP_
#define LINEARINTERPOLATION_HPP_
#include "ConstantStepInterpolator.hpp"
#include <vector>

namespace ssc
{
    namespace interpolation
    {
        /** \author cec
         *  \brief This class was created to
         *  \details It has the following responsibilities:
         *  - Resp. 1 ...
         *  \section ex1 Example
         *  \snippet interpolation/unit_tests/src/LinearInterpolationTest.cpp LinearInterpolationTest example
         *  \section ex2 Expected output
         *  \snippet interpolation/unit_tests/src/LinearInterpolationTest.cpp LinearInterpolationTest expected output
         */

        class LinearInterpolation : public ConstantStepInterpolator
        {
            public:
                LinearInterpolation(const double& xmin_, const double& xmax_, const std::vector<double>& y_);

            private:
                LinearInterpolation();
                double get_f() const;
                double get_df(const size_t derivative_order) const;
                void compute_coefficients_for_ith_interval(const double x0, const size_t i);
                std::vector<double> x0;
                std::vector<double> x1;
                std::vector<double> y0;
                std::vector<double> y1;
        };
    }
}
#endif /* LINEARINTERPOLATION_HPP_ */
