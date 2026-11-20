/*
 * PIDController.hpp
 *
 *  Created on: Fev, 12 2021
 *      Author: lincker
 */

#ifndef PIDCONTROLLER_HPP_
#define PIDCONTROLLER_HPP_

#include <ssc/solver.hpp>
#include <string>

#include "Controller.hpp"
#include "xdyn/core/State.hpp"

class PIDController : public Controller {
public:
    struct Yaml {
        explicit Yaml(const std::string& yaml);
        /// Proportional gain
        /* PID Controller gains */
        double Kp{};
        /// Integral gain
        double Ki{};
        /// Derivative gain
        double Kd{};
        /// Weights associated to each state, used to compute the controller's measured input,
        /* PID Controller input */
        std::map<std::string, double> state_weights;
        //   with the convention that any missing state has weight 0.
        //   For example "u - 2v" -> { {"u", 1}, {"v", -2} }
        std::string setpoint_name;
        /* PID Controller output */
        std::string command_name;
    };

    PIDController(
        const double tstart,
        const double dt,
        const std::string& name,
        const std::string& yaml);

    std::vector<std::string> get_command_names() const override;
    const Yaml yaml;  //!< Controller-specific yaml

private:
    /**
     * @brief Updates the controller output value in the datasource
     *
     * This method will be called by the "ssc::solver::DiscreteSystem::callback" method.
     * @param time Current simulation time (in seconds).
     * @param system The continuous system. Used to retrieve the continuous states.
     */
    void update_discrete_states(const double time, ssc::solver::ContinuousSystem& system) override;

    /** \brief Gets the value of the controller measured input used by `compute_command` from the
     * system states
     */
    double get_measured_value(const ssc::solver::ContinuousSystem& sys) const;

    /** \brief Computes the command value from the input data
     */
    double compute_command(const double setpoint, const double measured_value, const double t);

    /// False if the controller has never been called and `t_start` and `previous_error`
    /// uninitialized.
    /* PID Controller "memory" */
    bool initialized{false};
    /// Previous time instant at which `compute_command` was called
    double previous_t{0};
    /// Previous error (expected - measured)
    double previous_error{0};
    /// Previous value of the integral term
    double integral_term{0};
};

#endif /* PIDCONTROLLER_HPP_ */
