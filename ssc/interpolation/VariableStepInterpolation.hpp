/*
 * VariableStepInterpolation.hpp
 *
 * \date 10 sept. 2013, 11:55:03
 * \author cec
 */

#ifndef VARIABLESTEPINTERPOLATION_HPP_
#define VARIABLESTEPINTERPOLATION_HPP_

#include "ssc/interpolation/Interpolator.hpp"
#include <memory>
#include <vector>


namespace ssc
{
    namespace interpolation
    {
        /** \author cec
         *  \ingroup interpolation
         *  \brief Responsibility
         *  \details
         *  \section ex1 Example
         *  \snippet interpolation/unit_tests/src/VariableStepInterpolationTest.cpp VariableStepInterpolationTest example
         *  \section ex2 Expected output
         *  \snippet interpolation/unit_tests/src/VariableStepInterpolationTest.cpp VariableStepInterpolationTest expected output
         */

        class IndexFinder;
        class VariableStepInterpolation : public Interpolator
        {
            public:
                VariableStepInterpolation();
                ~VariableStepInterpolation() override;
                explicit VariableStepInterpolation(const std::vector<double>& x, const bool allow_queries_outside_bounds = false);

            protected:
                size_t get_index(const double x0_) const;
                double get_f() const override = 0;
                double get_df(const size_t derivative_order) const override = 0;
                void update_coefficients_if_necessary(const double x0) override;

            private:
                std::shared_ptr<IndexFinder> index;

            protected:
                int idx;
                double x0;
        };
    }
}

#endif /* VARIABLESTEPINTERPOLATION_HPP_ */
