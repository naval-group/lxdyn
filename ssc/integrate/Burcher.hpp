/*
 * Burcher.hpp
 *
 * \date Dec 11, 2014
 * \author cec
 */

#ifndef BURCHER_HPP_
#define BURCHER_HPP_

#include "ssc/integrate/Integrator.hpp"
#include <cstddef>
#include <vector>
#include <stddef.h>

namespace ssc
{
    namespace integrate
    {
        class Burcher : public Integrator
        {
            public:
                Burcher();
                Burcher(const Function& f);
                Burcher(const Function& f, const double tau);
                double integrate_n_steps(const Function& f, double a, double b, size_t n);
                double compute_for(const double tau, const double a, const double b, const double eps);

            private:
                double integrate_impl(const Function& f, double a, double b, double eps=1e-6);
                double tau;
        };
    }
}

#endif /* BURCHER_HPP_ */
