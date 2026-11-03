/*
 * ParabolicRunoutSplines.hpp
 *
 * \date 1 août 2012, 09:50:24
 * \author cec
 */

#ifndef PARABOLICRUNOUTSPLINES_HPP_
#define PARABOLICRUNOUTSPLINES_HPP_
#include "Splines.hpp"

namespace ssc
{
    namespace interpolation
    {
        /** \author cec
         *  \brief This class was created to choose a parabolic runout for cubic splines
         *  \details It has the following responsibility:
         *  - Implement the get_endpoint_value method
         */
        class ParabolicRunoutSplines : public Splines
        {
            public:
                ParabolicRunoutSplines();
                ParabolicRunoutSplines(const double& xmin_, const double& xmax_, const std::vector<double>& y);
                virtual ~ParabolicRunoutSplines() {}
            private:
                double get_endpoint_value() const;
        };
    }
}

#endif /* PARABOLICRUNOUTSPLINES_HPP_ */
