/*
 * SimulatorBuilder.cpp
 *
 *  Created on: Jun 16, 2014
 *      Author: cady
 */

#include "SimulatorBuilder.hpp"

#include <memory>
#include <ssc/text_file_reader.hpp>

#include "AssetPath.hpp"
#include "BodyBuilder.hpp"
#include "update_kinematics.hpp"
#include "xdyn/exceptions/InternalErrorException.hpp"
#include "xdyn/external_file_formats/stl_reader.hpp"

SimulatorBuilder::SimulatorBuilder(
    const YamlSimulatorInput& input_,
    ForcePtr (*make_force_model_)(
        const YamlModel& yaml,
        const std::string& body_name,
        const EnvironmentAndFrames& env),
    void (*add_environment_model_)(const YamlModel& yaml, EnvironmentAndFrames& env),
    const double t0_,
    const ssc::data_source::DataSource& command_listener_)
    : input(input_)
    , builder(std::make_shared<BodyBuilder>(input.rotations))
    , make_force_model(make_force_model_)
    , add_environment_model(add_environment_model_)
    , command_listener(command_listener_)
    , t0(t0_)
{
}

std::vector<BodyPtr> SimulatorBuilder::get_bodies(
    const MeshMap& meshes,
    const std::vector<bool>& bodies_contain_surface_forces,
    std::map<std::string, double> history_length) const
{
    std::vector<BodyPtr> ret;
    size_t i = 0;
    for (const auto& body : input.bodies) {
        const auto that_mesh = meshes.find(body.name);
        if (that_mesh != meshes.end()) {
            ret.push_back(builder->build(
                body,
                that_mesh->second,
                i,
                t0,
                input.rotations,
                history_length[body.name],
                bodies_contain_surface_forces.at(i)));
            i++;
        } else {
            THROW(
                __PRETTY_FUNCTION__,
                InternalErrorException,
                "Unable to find mesh for '" << body.name << "' in map.");
        }
    }
    return ret;
}

void SimulatorBuilder::add_initial_transforms(
    const std::vector<BodyPtr>& bodies,
    ssc::kinematics::KinematicsPtr& k) const
{
    const StateType x = ::get_initial_states(input.rotations, input.bodies);
    for (size_t i = 0; i < bodies.size(); ++i) {
        k->add(bodies.at(i)->get_transform_from_mesh_to_body());
        k->add(bodies.at(i)->get_transform_from_ned_to_body(x));
    }
}

EnvironmentAndFrames SimulatorBuilder::build_environment_and_frames() const
{
    EnvironmentAndFrames env;
    env.g = input.environmental_constants.g;
    env.rho = input.environmental_constants.rho;
    if (input.environmental_constants.rho_air)
        env.set_rho_air(input.environmental_constants.rho_air.value());
    env.nu = input.environmental_constants.nu;
    env.rot = input.rotations;
    env.k = std::make_shared<ssc::kinematics::Kinematics>();

    for (const auto& model : input.environment) {
        add_environment_model(model, env);
    }
    return env;
}

std::map<std::string, double> SimulatorBuilder::get_max_history_length(
    const std::vector<ListOfForces>& forces_for_all_bodies) const
{
    std::map<std::string, double> ret;
    for (const auto& forces : forces_for_all_bodies) {
        double Tmax = 0;
        for (const auto& force : forces)
            Tmax = std::max(Tmax, force->get_Tmax());
        if (not(forces.empty()))
            ret[forces.front()->get_body_name()] =
                std::max(Tmax, ret[forces.front()->get_body_name()]);
    }
    return ret;
}

std::vector<ListOfForces> SimulatorBuilder::get_forces(const EnvironmentAndFrames& env) const
{
    std::vector<ListOfForces> forces;
    for (auto that_body = input.bodies.begin(); that_body != input.bodies.end(); ++that_body) {
        forces.push_back(forces_from(*that_body, env));
    }
    return forces;
}

ListOfForces SimulatorBuilder::forces_from(const YamlBody& body, const EnvironmentAndFrames& env)
    const
{
    ListOfForces ret;
    for (const auto& model : body.external_forces) {
        ret.push_back(make_force_model(model, body.name, env));
    }
    return ret;
}

std::vector<bool> SimulatorBuilder::are_there_surface_forces_acting_on_body(
    const std::vector<ListOfForces>& forces) const
{
    std::vector<bool> ret;
    for (const auto& forces_acting_on_body : forces) {
        bool has_surface_forces = false;
        for (const auto& force : forces_acting_on_body) {
            has_surface_forces |= force->is_a_surface_force_model();
        }
        ret.push_back(has_surface_forces);
    }
    return ret;
}

Sim SimulatorBuilder::build(const MeshMap& meshes) const
{
    auto env = build_environment_and_frames();
    auto forces = get_forces(env);
    auto history_length = get_max_history_length(forces);
    const auto bodies =
        get_bodies(meshes, are_there_surface_forces_acting_on_body(forces), history_length);
    add_initial_transforms(bodies, env.k);
    return Sim(bodies, forces, env, get_initial_states(), command_listener);
}

StateType SimulatorBuilder::get_initial_states() const
{
    return ::get_initial_states(input.rotations, input.bodies);
}

Sim SimulatorBuilder::build() const
{
    return build(make_mesh_map());
}

YamlSimulatorInput SimulatorBuilder::get_parsed_yaml() const
{
    return input;
}

MeshMap SimulatorBuilder::make_mesh_map() const
{
    MeshMap ret;
    for (auto that_body = input.bodies.begin(); that_body != input.bodies.end(); ++that_body) {
        ret[that_body->name] = get_mesh(*that_body);
    }
    return ret;
}

VectorOfVectorOfPoints SimulatorBuilder::get_mesh(const YamlBody& body) const
{
    if (not(body.mesh.empty())) {
        const ssc::text_file_reader::TextFileReader reader(xdyn::resolve_asset(body.mesh));
        return read_stl(reader.get_contents());
    }
    return VectorOfVectorOfPoints();
}
