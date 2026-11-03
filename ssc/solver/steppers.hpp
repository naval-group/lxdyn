/*
 * steppers.hpp
 *
 * \date Jun 30, 2014
 * \author cec
 */

#ifndef STEPPERS_HPP_
#define STEPPERS_HPP_

#include <vector>
#include <boost/numeric/odeint/stepper/euler.hpp>
#include <boost/numeric/odeint/stepper/runge_kutta4.hpp>
#include <boost/numeric/odeint/stepper/runge_kutta_cash_karp54.hpp>

namespace ssc
{
    namespace solver
    {
        typedef ::boost::numeric::odeint::euler<std::vector<double> > EulerStepper;
        typedef ::boost::numeric::odeint::runge_kutta4<std::vector<double> > RK4Stepper;
        typedef ::boost::numeric::odeint::runge_kutta_cash_karp54<std::vector<double> > RKCK;
    }
}

#endif /* STEPPERS_HPP_ */
