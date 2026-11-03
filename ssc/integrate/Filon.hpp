/*
 * Filon.hpp
 *
 * \date Dec 10, 2014
 * \author cec
 */

#ifndef FILON_HPP_
#define FILON_HPP_

#include "ssc/integrate/QuadPack.hpp"

namespace ssc
{
    namespace integrate
    {
        class Filon : public QuadPack
        {
            public:
                Filon();
                Filon(const Function& f_);
                Filon(const Function& f_, const double tau);
                double compute_for(const double tau, const double a, const double b, const double eps);

            private:
                double integrate_impl(const Function& f, double a, double b, double eps=1e-6);
                double tau;
        };
    }
}

#endif /* FILON_HPP_ */
