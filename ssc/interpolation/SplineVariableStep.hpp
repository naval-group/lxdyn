/*
 * SplineVariableStep.hpp
 *
 * \date 9 sept. 2013, 10:47:42
 * \author cec
 */

#ifndef SPLINEVARIABLESTEP_HPP_
#define SPLINEVARIABLESTEP_HPP_

#include "ssc/interpolation/VariableStepInterpolation.hpp"
#include "ssc/exception_handling/Exception.hpp"
#include <memory>

#include <vector>

namespace ssc
{
    namespace interpolation
    {
        class SplineVariableStepException : public ::ssc::exception_handling::Exception
        {
            public:
                SplineVariableStepException(const std::string& message, const std::string& file, const std::string& function, const unsigned int line) :
                    Exception(message, file, function, line)
                {
                }
        };


        /** \author cec
         *  \ingroup interpolation
         *  \brief Performs a cubic spline interpolation for non-equidistant abscissae
         *  \section ex1 Example
         *  \snippet interpolation/unit_tests/src/SplineVariableStepTest.cpp SplineVariableStepTest example
         *  \section ex2 Expected output
         *  \snippet interpolation/unit_tests/src/SplineVariableStepTest.cpp SplineVariableStepTest expected output
         */

        class SplineVariableStep : public VariableStepInterpolation
        {
            public:
                SplineVariableStep();
                SplineVariableStep(const std::vector<double>& x, const std::vector<double>& y, const bool allow_queries_outside_bounds = false);
                ~SplineVariableStep() override;
                SplineVariableStep(SplineVariableStep&&) noexcept;
                SplineVariableStep& operator=(SplineVariableStep&&) noexcept;

            private:
                double get_f() const override;
                double get_df(const size_t derivative_order) const override;

                class Impl;
                std::unique_ptr<Impl> pimpl;
        };
    }
}

#endif /* SPLINEVARIABLESTEP_HPP_ */
