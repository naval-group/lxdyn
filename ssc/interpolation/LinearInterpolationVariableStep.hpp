/*
 * LinearInterpolationVariableStep.hpp
 *
 * \date 10 avr. 2013, 12:54:12
 * \author cec
 */

#ifndef LINEARINTERPOLATIONVARIABLESTEP_HPP_
#define LINEARINTERPOLATIONVARIABLESTEP_HPP_

#include <memory>
#include "ssc/interpolation/ConstantStepInterpolator.hpp"
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
         *  \snippet interpolation/unit_tests/src/LinearInterpolationVariableStepTest.cpp LinearInterpolationVariableStepTest example
         *  \section ex2 Expected output
         *  \snippet interpolation/unit_tests/src/LinearInterpolationVariableStepTest.cpp LinearInterpolationVariableStepTest expected output
         */
        class LinearInterpolationVariableStep : public ConstantStepInterpolator
        {
            public:
                LinearInterpolationVariableStep(const std::vector<double>& x, const std::vector<double>& y);

            private:
                LinearInterpolationVariableStep();
                void compute_coefficients_for_ith_interval(const double x0, const size_t i) override;
                double get_f() const override;
                double get_df(const size_t derivative_order) const override;
                void find_index_of_interval_containing(const double val) override;
                class LinearInterpolationVariableStepImpl;
                std::shared_ptr<LinearInterpolationVariableStepImpl> pimpl;
        };
    }
}

#endif /* LINEARINTERPOLATIONVARIABLESTEP_HPP_ */
