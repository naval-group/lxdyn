/*
 * SimulatorBuilder.hpp
 *
 *  Created on: Jun 16, 2014
 *      Author: cady
 */

#ifndef SIMULATORBUILDER_HPP_
#define SIMULATORBUILDER_HPP_

#include <map>
#include <ssc/data_source.hpp>
#include <ssc/kinematics.hpp>

#include "xdyn/core/EnvironmentAndFrames.hpp"
#include "xdyn/core/Sim.hpp"
#include "xdyn/external_data_structures/GeometricTypes3d.hpp"
#include "xdyn/external_data_structures/YamlSimulatorInput.hpp"

class BodyBuilder;
using MeshMap = std::map<std::string, VectorOfVectorOfPoints>;

/*  \brief Builds a Sim object which can be used with quicksolve
 *  \details This class is used to configure the Sim object for simulation.
 *           Models are built by the make_force_model and add_environment_model it is given.
 *  \ingroup simulator
 *  \section ex1 Example
 *  \snippet simulator/unit_tests/SimulatorBuilderTest.cpp SimulatorBuilderTest example
 *  \section ex2 Expected output
 *  \snippet simulator/unit_tests/SimulatorBuilderTest.cpp SimulatorBuilderTest expected output
 */
class SimulatorBuilder {
public:
    explicit SimulatorBuilder(
        const YamlSimulatorInput& input,
        ForcePtr (*make_force_model)(
            const YamlModel& yaml,
            const std::string& body_name,
            const EnvironmentAndFrames& env),
        void (*add_environment_model)(const YamlModel& yaml, EnvironmentAndFrames& env),
        const double t0 = 0.0,
        const ssc::data_source::DataSource& command_listener = ssc::data_source::DataSource());

    /**  \brief Builds a Sim object using the supplied mesh map (one mesh per body)
     *  \details This function is mainly used in the integration tests.
     *  \snippet simulator/unit_tests/SimulatorBuilderTest.cpp SimulatorBuilderTest build_example
     */
    Sim build(
        const MeshMap& input_meshes  //!< Map containing a mesh for each body
    ) const;

    /**  \brief Builds a Sim object reading the meshes from files
     *  \details Reads the STL data from an STL file & call the version of
     *           the build method that accepts a MeshMap as input.
     *  \snippet simulator/unit_tests/SimulatorBuilderTest.cpp SimulatorBuilderTest build_example
     */
    Sim build() const;

    std::vector<BodyPtr> get_bodies(
        const MeshMap& meshes,
        const std::vector<bool>& bodies_contain_surface_forces,
        std::map<std::string, double> Tmax) const;
    EnvironmentAndFrames build_environment_and_frames() const;
    std::vector<ListOfForces> get_forces(const EnvironmentAndFrames& env) const;
    StateType get_initial_states() const;
    YamlSimulatorInput get_parsed_yaml() const;
    MeshMap make_mesh_map() const;

    /**  \brief Create a Kinematics object with transforms from NED to each body
     *  \returns KinematicsPtr containing the initial transforms
     */
    void add_initial_transforms(
        const std::vector<BodyPtr>& bodies,  //!< Bodies containing the initial coordinates
        ssc::kinematics::KinematicsPtr& k) const;

    std::vector<bool> are_there_surface_forces_acting_on_body(
        const std::vector<ListOfForces>& forces) const;

private:
    SimulatorBuilder();  // Disabled
    std::map<std::string, double> get_max_history_length(
        const std::vector<ListOfForces>& forces_for_all_bodies) const;
    ListOfForces forces_from(const YamlBody& body, const EnvironmentAndFrames& env) const;
    VectorOfVectorOfPoints get_mesh(const YamlBody& body) const;

    YamlSimulatorInput input;
    std::shared_ptr<BodyBuilder> builder;
    ForcePtr (*make_force_model)(
        const YamlModel& yaml,
        const std::string& body_name,
        const EnvironmentAndFrames& env);
    void (*add_environment_model)(const YamlModel& yaml, EnvironmentAndFrames& env);
    ssc::data_source::DataSource command_listener;
    double t0;  //!< First time step (to initialize state history)
};

#endif /* SIMULATORBUILDER_HPP_ */
