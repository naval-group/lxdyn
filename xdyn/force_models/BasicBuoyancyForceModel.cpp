/*
 * BasicBuoyancyForceModel.cpp
 */

#include "BasicBuoyancyForceModel.hpp"
#include "xdyn/core/Body.hpp"
#include "xdyn/core/EnvironmentAndFrames.hpp"
#include "xdyn/yaml_parser/yaml_compat.h"
#include "xdyn/yaml_parser/parse_unit_value.hpp"

std::string BasicBuoyancyForceModel::model_name(){return "basic buoyancy";}

BasicBuoyancyForceModel::Input::Input() : V()
{}

BasicBuoyancyForceModel::Input BasicBuoyancyForceModel::parse(const std::string& yaml)
{
    YAML::Node node = YAML::Load(yaml);
    Input ret;
    xdyn::yaml_parser::parse_uv(node["volume"], ret.V);
    return ret;
}

BasicBuoyancyForceModel::BasicBuoyancyForceModel(const Input& input, const std::string& body_name_, const EnvironmentAndFrames& env) :
        ForceModel("basic buoyancy", {}, body_name_, env),
        g(env.g),
        rho(env.rho),
        V(input.V)
{}

Wrench BasicBuoyancyForceModel::get_force(const BodyStates& states, const double t, const EnvironmentAndFrames& env, const std::map<std::string,double>&) const
{
    // NED z points down, so the centre of gravity is submerged when it is below the free surface.
    // The volume is all-or-nothing: there is no partial immersion in this model.
    double f = 0;
    if (env.w == nullptr)
    {
        f = -rho*g*V;
    }
    else
    {
        const std::vector<double> x = states.get_current_state_values();
        const std::vector<double> sea_height = env.w->get_and_check_wave_height({x[0]}, {x[1]}, t);
        if (x[2] > sea_height[0]) f = -rho*g*V;
    }
    return Wrench(states.G, "NED", Eigen::Vector3d(0, 0, f), Eigen::Vector3d(0,0,0));
}

double BasicBuoyancyForceModel::potential_energy(const BodyStates&, const std::vector<double>& x) const
{
    // Assumes the body is submerged: this signature carries neither the date nor the environment, so
    // the free surface get_force tests against cannot be sampled here.
    return rho*g*V*x[2];
}
