/*
 * test_systems.hpp
 *
 * \date 21 mars 2014
 *      Author: cec
 */

#ifndef TEST_SYSTEMS_HPP_
#define TEST_SYSTEMS_HPP_

#include "ssc/solver/ContinuousSystem.hpp"

typedef std::vector<double> StateType;

struct SimpleODE : public ssc::solver::ContinuousSystem
{
    SimpleODE(const StateType &x0) { state = x0; }

    void dx_dt(const StateType &x, StateType &dxdt, double)
    {
        state = x;
        dxdt[0] = 2; //*t;//2*x[0];
    }

    void initialize_system_outputs_before_first_observation() {}
    void set_discrete_state(const std::string &, const double) {}
    double get_input_value(const std::string &) const { return -1; }
    double get_state_value(const std::string &) const { return -1; }
};

struct QuadraticODE : public ssc::solver::ContinuousSystem
{
    QuadraticODE(const StateType &x0) { state = x0; }

    void dx_dt(const StateType &x, StateType &dxdt, double t)
    {
        state = x;
        dxdt[0] = 2 * t;
    }

    void initialize_system_outputs_before_first_observation() {}
    void set_discrete_state(const std::string &, const double) {}
    double get_input_value(const std::string &) const { return -1; }
    double get_state_value(const std::string &) const { return -1; }
};

#endif /* TEST_SYSTEMS_HPP_ */
