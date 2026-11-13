/*
 * Burcher.hpp
 *
 * \date Dec 11, 2014
 * \author cec
 */

#ifndef BURCHER_HPP_
#define BURCHER_HPP_

#include <stddef.h>

#include <cstddef>
#include <vector>

#include "ssc/integrate/Integrator.hpp"

namespace ssc {
namespace integrate {
class Burcher : public Integrator {
public:
    Burcher();
    explicit Burcher(const Function& f);
    Burcher(const Function& f, const double tau);
    double integrate_n_steps(const Function& f, double a, double b, size_t n);
    double compute_for(const double tau, const double a, const double b, const double eps);

private:
    double integrate_impl(const Function& f, double a, double b, double eps = 1e-6) override;
    double tau;
};
}  // namespace integrate
}  // namespace ssc

#endif /* BURCHER_HPP_ */
