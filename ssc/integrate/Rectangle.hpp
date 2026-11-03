/*
 * Rectangle.hpp
 *
 * \date Nov 28, 2014
 * \author cec
 */

#ifndef RECTANGLE_HPP_
#define RECTANGLE_HPP_

#include "ssc/integrate/Integrator.hpp"

namespace ssc
{
    namespace integrate
    {
        class Rectangle : public Integrator
        {
            public:
                Rectangle();
                Rectangle(const Function& f);

            private:
                double integrate_impl(const Function& f, double a, double b, double eps=1e-6);
        };
    }
}
#endif /* RECTANGLE_HPP_ */
