/*
 * CubicRunoutSplines.hpp
 *
 * \date 1 août 2012, 09:57:37
 * \author cec
 */

#ifndef CUBICRUNOUTSPLINES_HPP_
#define CUBICRUNOUTSPLINES_HPP_
#include "Splines.hpp"


namespace ssc
{
    namespace interpolation
    {
        /** \author cec
         *  \brief This class was created to choose a cubic runout for cubic splines
         *  \details It has the following responsibility:
         *  - Implement the get_endpoint_value method
         */
        class CubicRunoutSplines : public Splines
        {
            public:
                CubicRunoutSplines();
                CubicRunoutSplines(const double& xmin_, const double& xmax_, const std::vector<double>& y);
                virtual ~CubicRunoutSplines() {}
            private:
                double get_endpoint_value() const;
        };
    }
}

#endif /* CUBICRUNOUTSPLINES_HPP_ */
