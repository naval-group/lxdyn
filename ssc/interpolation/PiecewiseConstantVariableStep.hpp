/*
 * PiecewiseConstantVariableStep.hpp
 *
 * \date 11 avr. 2013, 08:44:51
 * \author cec
 */

#ifndef PIECEWISECONSTANTVARIABLESTEP_HPP_
#define PIECEWISECONSTANTVARIABLESTEP_HPP_

#include "ssc/exception_handling/Exception.hpp"
#include "ssc/interpolation/VariableStepInterpolation.hpp"

#include <vector>

#if defined(_MSC_VER)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

namespace ssc
{
    namespace interpolation
    {
        class PiecewiseConstantVariableStepException : public ::ssc::exception_handling::Exception
        {
            public:
                PiecewiseConstantVariableStepException(const std::string& message, const std::string& file, const std::string& function, const unsigned int line) :
                    Exception(message, file, function, line)
                {
                }
        };


        template <typename U> double pc_f(const typename std::vector<U>& , const size_t )
        {
            return 0;
        }

        template <> inline double pc_f<double>(const typename std::vector<double>& y_, const size_t idx)
        {
            return y_[idx];
        }

        /** \author cec
         *  \brief This class was created to
         *  \details It has the following responsibilities:
         *  - Resp. 1 ...
         *  \section ex1 Example
         *  \snippet interpolation/unit_tests/src/PiecewiseConstantVariableStepTest.cpp PiecewiseConstantVariableStepTest example
         *  \section ex2 Expected output
         *  \snippet interpolation/unit_tests/src/PiecewiseConstantVariableStepTest.cpp PiecewiseConstantVariableStepTest expected output
         */

        template <class T> class PiecewiseConstantVariableStep: public VariableStepInterpolation
        {
            public:
                PiecewiseConstantVariableStep() : y_(std::vector<T>()), val(T())
                {
                }

                PiecewiseConstantVariableStep(const std::vector<double>& x, const typename std::vector<T>& y, const bool allow_queries_outside_bounds = false) :
                    VariableStepInterpolation(x, allow_queries_outside_bounds),
                y_(y),val(T())
                {
                    const size_t n = x.size();
                    const std::string context = "When interpolating between x and y, ";
                    if (y.size() != n)
                    {
                        THROW(__PRETTY_FUNCTION__, PiecewiseConstantVariableStepException, context << "there should be as many x-values as y-values: received " << n << " x-values but " << y.size() << " y-values.");
                    }
                    if (x.size() < 2)
                    {
                        THROW(__PRETTY_FUNCTION__, PiecewiseConstantVariableStepException, context << "there should be at least two x-values. Received " << x.size());
                    }
                }

                T typed_f(const double x0)
                {
                    update_coefficients_if_necessary(x0);
                    return y_[(size_t)idx];
                }

                using Interpolator::df;
                T df(const double x0, const size_t derivative_order)
                {
                    (void) x0;
                    (void) derivative_order;
                    return T();
                }

                double f(const double x0)
                {
                    update_coefficients_if_necessary(x0);
                    return pc_f<T>(y_, (size_t)idx);
                }

            private:
                double get_f() const
                {
                    return 0;
                }

                double get_df(const size_t derivative_order) const
                {
                    (void) derivative_order;
                    return 0;
                }

                std::vector<T> y_;
                T val;
        };
    }
}

#endif /* PIECEWISECONSTANTVARIABLESTEP_HPP_ */
