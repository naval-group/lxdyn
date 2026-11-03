/*
 * LinearInterpolationVariableStep.hpp
 *
 * \date 10 avr. 2013, 12:54:12
 * \author cec
 */

#ifndef LINEARINTERPOLATIONVARIABLESTEP_HPP_
#define LINEARINTERPOLATIONVARIABLESTEP_HPP_

#include "ssc/macros/tr1_macros.hpp"
#include TR1INC(memory)
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
                void compute_coefficients_for_ith_interval(const double x0, const size_t i);
                double get_f() const;
                double get_df(const size_t derivative_order) const;
                void find_index_of_interval_containing(const double val);
                class LinearInterpolationVariableStepImpl;
                TR1(shared_ptr)<LinearInterpolationVariableStepImpl> pimpl;
        };
    }
}

#endif /* LINEARINTERPOLATIONVARIABLESTEP_HPP_ */
