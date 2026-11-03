/*
 * ParabolicInterpolation.hpp
 *
 * \date 19 févr. 2013, 09:56:40
 * \author cec
 */

#ifndef PARABOLICINTERPOLATION_HPP_
#define PARABOLICINTERPOLATION_HPP_

#include "ParabolicCoefficients.hpp"
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
         *  \snippet interpolation/unit_tests/src/ParabolicInterpolationTest.cpp ParabolicInterpolationTest example
         *  \section ex2 Expected output
         *  \snippet interpolation/unit_tests/src/ParabolicInterpolationTest.cpp ParabolicInterpolationTest expected output
         */



        class ParabolicInterpolation : public ConstantStepInterpolator
        {
            public:
                ParabolicInterpolation(const double& xmin, const double& xmax, const std::vector<ParabolicCoefficients>& coeffs);

            private:
                ParabolicInterpolation();
                void compute_coefficients_for_ith_interval(const double x0, const size_t i);
                double get_f() const;
                double get_df(const size_t derivative_order) const;
                std::vector<ParabolicCoefficients> coeffs_;
        };
    }
}

#endif /* PARABOLICINTERPOLATION_HPP_ */
