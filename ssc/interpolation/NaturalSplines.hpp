/*
 * NaturalSpline.hpp
 *
 * \date 1 août 2012, 09:37:58
 * \author cec
 */

#ifndef NATURALSPLINES_HPP_
#define NATURALSPLINES_HPP_

#include "Splines.hpp"

namespace ssc
{
    namespace interpolation
    {
        /** \author cec
         *  \brief This class was created to choose a linear runout for cubic splines
         *  \details It has the following responsibility:
         *  - Implement the get_endpoint_value method
         */
        class NaturalSplines : public Splines
        {
            public:
                NaturalSplines();
                NaturalSplines(const double& xmin_, const double& xmax_, const std::vector<double>& y);
                virtual ~NaturalSplines() {}
            private:
                double get_endpoint_value() const;
        };
    }
}

#endif /* NATURALSPLINES_HPP_ */
