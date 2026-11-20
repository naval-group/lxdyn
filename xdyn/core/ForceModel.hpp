/*
 * ForceModel.hpp
 *
 *  Created on: Oct 22, 2014
 *      Author: cady
 */

#ifndef FORCEMODEL_HPP_
#define FORCEMODEL_HPP_

#include <map>
#include <ssc/kinematics.hpp>

#include "xdyn/core/EnvironmentAndFrames.hpp"
#include "xdyn/core/Wrench.hpp"
#include "xdyn/input/YamlBody.hpp"
#include "xdyn/input/YamlPosition.hpp"

namespace ssc {
namespace data_source {
class DataSource;
}
}  // namespace ssc
struct BodyStates;
struct YamlRotation;

class ForceModel;
using ForcePtr = std::shared_ptr<ForceModel>;
using ListOfForces = std::vector<ForcePtr>;

class Observer;

/**
 * @brief This allows us to observe the force before integration without calculating the model
 * unnecessarily
 */
class Memoization {
public:
    Memoization() = delete;
    explicit Memoization(
        const std::function<Wrench(
            const BodyStates&,
            const double,
            const EnvironmentAndFrames&,
            const std::map<std::string, double>&)>& callback);
    Wrench run_if_not_cached(
        const BodyStates& states,
        const double t,
        const EnvironmentAndFrames& env,
        const std::map<std::string, double>&);

private:
    bool is_cached(
        const double t,
        const std::vector<double>& states,
        const std::vector<double>& current_command_values) const;
    double date_of_latest_force_in_body_frame;
    std::vector<double> state_used_for_last_evaluation;
    std::vector<double> commands_used_for_last_evaluation;
    std::function<Wrench(
        const BodyStates&,
        const double,
        const EnvironmentAndFrames&,
        const std::map<std::string, double>&)>
        callback;
    Wrench cached_force;
};

/** \brief These force models read commands from a DataSource.
 *  \details Provides facilities to the derived classes to retrieve the commands
 *  \addtogroup model_wrappers
 *  \ingroup model_wrappers
 *  \section ex1 Example
 *  \snippet core/unit_tests/ForceModelTest.cpp ForceModelTest example
 *  \section ex2 Expected output
 *  \snippet core/unit_tests/ForceModelTest.cpp ForceModelTest expected output
 */
class ForceModel {
public:
    ForceModel(
        const std::string& name,
        const std::vector<std::string>& commands,
        const YamlPosition& internal_frame,
        const std::string& body_name_,
        const EnvironmentAndFrames& env);
    ForceModel(
        const std::string& name,
        const std::vector<std::string>& commands,
        const std::string& body_name_,
        const EnvironmentAndFrames& env);
    virtual ~ForceModel() = default;
    ssc::kinematics::Wrench operator()(
        const BodyStates& states,
        const double t,
        const EnvironmentAndFrames& env,
        ssc::data_source::DataSource& command_listener);
    ssc::kinematics::Wrench
    operator()(const BodyStates& states, const double t, const EnvironmentAndFrames& env);
    virtual Wrench get_force(
        const BodyStates& states,
        const double t,
        const EnvironmentAndFrames& env,
        const std::map<std::string, double>& commands) const = 0;
    std::string get_name() const;
    virtual double get_Tmax() const;  // Can be overloaded if model needs access to History (not a
                                      // problem, just has to say how much history to keep)
    std::string get_body_name() const;
    virtual bool is_a_surface_force_model() const;
    std::vector<std::string> get_command_names() const;

    void feed(
        Observer& observer,
        ssc::kinematics::KinematicsPtr& k,
        ssc::data_source::DataSource& command_listener,
        const double t) const;
    virtual void extra_observations(Observer& observer) const;

protected:
    std::vector<std::string> commands;
    std::string name;
    std::string body_name;
    void can_find_internal_frame(const ssc::kinematics::KinematicsPtr& k) const;

private:
    ForceModel();  // Deactivated
    double get_command(
        const std::string& command_name,
        ssc::data_source::DataSource& command_listener,
        const double t) const;
    std::map<std::string, double> get_commands(
        ssc::data_source::DataSource& command_listener,
        const double t) const;

    bool has_internal_frame;
    std::string known_reference_frame;
    ssc::kinematics::Wrench latest_force_in_body_frame;
    Memoization memo;
};

#endif /* FORCEMODEL_HPP_ */
