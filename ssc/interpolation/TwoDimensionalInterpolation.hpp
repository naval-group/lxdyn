/*
 * TwoDimensionalSplines.hpp
 *
 * \date 13 nov. 2012, 16:50:25
 * \author cec
 */
#include "ssc/macros/tr1_macros.hpp"
#include TR1INC(memory)
#include "ssc/exception_handling/Exception.hpp"

#include <vector>

#if defined(_MSC_VER)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#define not !
#endif

class ParabolicRunoutSplines;
#ifndef TWODIMENSIONALSPLINES_HPP_
#define TWODIMENSIONALSPLINES_HPP_

namespace ssc
{
    namespace interpolation
    {
        class TwoDimensionalInterpolationException : public ::ssc::exception_handling::Exception
        {
            public:
                TwoDimensionalInterpolationException(const std::string& message, const std::string& file, const std::string& function, const unsigned int line) :
                    Exception(message, file, function, line)
                {
                }
        };

        /** \author cec
         *  \brief This class was created to
         *  \details It has the following responsibilities:
         *  - Resp. 1 ...
         *  \section ex1 Example
         *  \snippet splines/unit_tests/src/TwoDimensionalSplinesTest.cpp TwoDimensionalSplinesTest example
         *  \section ex2 Expected output
         *  \snippet splines/unit_tests/src/TwoDimensionalSplinesTest.cpp TwoDimensionalSplinesTest expected output
         */
        template <typename T> class TwoDimensionalInterpolation
        {
            public:
                TwoDimensionalInterpolation(const double& xmin_, const double& xmax_, const double& ymin_, const double& ymax_, const std::vector<std::vector<double> >& val) : interpolators(std::vector<TR1(shared_ptr)<T> >()),
                xmin(xmin_),
                xmax(xmax_)
                {
                    if (val.size() < 2)
                    {
                        THROW(__PRETTY_FUNCTION__, TwoDimensionalInterpolationException, "val must contain at least two vectors");
                    }
                    const size_t n = val.front().size();
                    for (auto it = val.begin() ; it != val.end() ; ++it)
                    {
                        if (it->size() != n)
                        {
                            THROW(__PRETTY_FUNCTION__, TwoDimensionalInterpolationException, "Not all vectors in val have the same number of elements");
                        }
                        if (it->size()<2)
                        {
                            THROW(__PRETTY_FUNCTION__, TwoDimensionalInterpolationException, "At least one vector in val has fewer than 2 elements");
                        }
                        interpolators.push_back(TR1(shared_ptr)<T>(new T(ymin_, ymax_, *it)));
                    }
                }

                double f(const double& x, const double& y)
                {
                    std::vector<double> interpolated_values_for_x_fixed;
                    for (auto it = interpolators.begin() ; it != interpolators.end() ; ++it)
                    {
                        interpolated_values_for_x_fixed.push_back((*it)->f(y));
                    }
                    T final_interpolation(xmin,xmax,interpolated_values_for_x_fixed);
                    return final_interpolation.f(x);
                }
            private:
                TwoDimensionalInterpolation();
                std::vector<TR1(shared_ptr)<T> > interpolators;
                double xmin;
                double xmax;
        };
    }
}

#endif /* TWODIMENSIONALSPLINES_HPP_ */
