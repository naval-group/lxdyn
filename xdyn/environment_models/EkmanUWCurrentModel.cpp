/*
 * EkmanUWCurrentModel.cpp
 */

#include "EkmanUWCurrentModel.hpp"

#include <cmath>
#include <utility>
#include "xdyn/yaml_parser/yaml_compat.h"
#include "xdyn/yaml_parser/parse_unit_value.hpp"
#include "xdyn/yaml_parser/external_data_structures_parsers.hpp"

EkmanUWCurrentModel::EkmanUWCurrentModel(const Input& input) :
    seabed(input.seabed),
    top_ekman_depth(input.top_ekman_depth),
    bottom_ekman_depth(input.bottom_ekman_depth),
    middle_velocity(input.middle_velocity),
    middle_orientation(input.middle_orientation),
    f_and_sqrt_rho(input.f_and_sqrt_rho),
    wind_angle(input.wind_angle),
    wind_stress(input.wind_stress)
{
}

EkmanUWCurrentModel::~EkmanUWCurrentModel()
{
}

std::string EkmanUWCurrentModel::model_name() {return "ekman current";}

EkmanUWCurrentModel::Input EkmanUWCurrentModel::parse(const std::string& yaml_input)
{
    YAML::Node node = YAML::Load(yaml_input);
    Input input;
    double rho = 1026.0;        // kg/m3
    try_to_parse(node, "rho", rho);
    double omega = 7.2921e-5;   // rad/s, Earth rotation rate
    try_to_parse(node, "omega", omega);
    double rho_air = 1.225;     // kg/m3
    try_to_parse(node, "rho_air", rho_air);
    if (node["seabed file"])
    {
        std::string seabed_file;
        node["seabed file"] >> seabed_file;
        input.seabed = Seabed(seabed_file);
    }
    else if (node["seabed depth"])
    {
        double seabed_depth;
        node["seabed depth"] >> seabed_depth;
        input.seabed = Seabed(seabed_depth);
    }
    else
    {
        THROW(__PRETTY_FUNCTION__, InvalidInputException, "Please provide either a 'seabed file' or a 'seabed depth' for the Ekman current model.");
    }
    double U10;
    xdyn::yaml_parser::parse_uv(node["wind orientation"], input.wind_angle);
    xdyn::yaml_parser::parse_uv(node["U10"], U10);
    const double drag_coefficient = (U10 < 20.2) ? 0.79e-3 + 0.08e-3*U10 : 0.002423;
    input.wind_stress = drag_coefficient * rho_air * U10 * U10;
    xdyn::yaml_parser::parse_uv(node["current velocity"], input.middle_velocity);
    xdyn::yaml_parser::parse_uv(node["current orientation"], input.middle_orientation);
    double latitude;
    xdyn::yaml_parser::parse_uv(node["latitude"], latitude);
    xdyn::yaml_parser::parse_uv(node["top layer thickness"], input.top_ekman_depth);
    xdyn::yaml_parser::parse_uv(node["bottom layer thickness"], input.bottom_ekman_depth);
    input.f_and_sqrt_rho = 2*omega*std::sin(latitude)*std::sqrt(rho);
    return input;
}

Eigen::Vector3d EkmanUWCurrentModel::get_UWCurrent(const Eigen::Vector3d& position, const double, const double wave_height) const
{
    const double z = position(2);
    const double seabed_height = seabed.get_height(position(0), position(1));
    const Eigen::Vector3d middle_current = get_middle_layer_current();
    // Below two Ekman depths the spiral contributes less than 0.2%, so each layer stops there
    if (z > wave_height && z < (wave_height + 2*top_ekman_depth))
    {
        return get_top_layer_current(position, middle_current, wave_height);
    }
    else if (z >= (wave_height + 2*top_ekman_depth) && z <= (seabed_height - 2*bottom_ekman_depth))
    {
        return middle_current;
    }
    else if (z > (seabed_height - 2*bottom_ekman_depth) && z < seabed_height)
    {
        return get_bottom_layer_current(position, middle_current, seabed_height);
    }
    return Eigen::Vector3d::Zero();
}

Eigen::Vector3d EkmanUWCurrentModel::get_top_layer_current(const Eigen::Vector3d& position,
                                                           const Eigen::Vector3d& middle_current,
                                                           const double) const
{
    const double z = position(2);
    const double sgn_f = (f_and_sqrt_rho >= 0) ? 1.0 : -1.0;
    const double V0 = std::sqrt(2)*M_PI*wind_stress/(top_ekman_depth*f_and_sqrt_rho);
    const double decay_factor = top_ekman_depth/M_PI;
    const double u = middle_current(0) + sgn_f*V0*std::exp(-z/decay_factor)*std::cos(M_PI/4 - z/decay_factor - sgn_f*wind_angle);
    const double v = middle_current(1) + V0*std::exp(-z/decay_factor)*std::sin(M_PI/4 - z/decay_factor + sgn_f*wind_angle);
    // Vertical velocity would have to be composed with the orbital velocities, which is not done yet
    return Eigen::Vector3d(u, v, 0.);
}

Eigen::Vector3d EkmanUWCurrentModel::get_middle_layer_current() const
{
    return Eigen::Vector3d(middle_velocity*std::cos(middle_orientation), middle_velocity*std::sin(middle_orientation), 0.);
}

Eigen::Vector3d EkmanUWCurrentModel::get_bottom_layer_current(const Eigen::Vector3d& position,
                                                              const Eigen::Vector3d& middle_current,
                                                              const double seabed_height) const
{
    const double depth_factor = M_PI*(seabed_height - position(2))/bottom_ekman_depth;
    const double u = middle_current(0)*(1 - std::exp(-depth_factor)*std::cos(depth_factor)) - middle_current(1)*std::exp(-depth_factor)*std::sin(depth_factor);
    const double v = middle_current(0)*std::exp(-depth_factor)*std::sin(depth_factor) + middle_current(1)*(1 - std::exp(-depth_factor)*std::cos(depth_factor));
    return Eigen::Vector3d(u, v, 0.);
}

EkmanUWCurrentModel::Input::Input() :
    seabed(0.),
    top_ekman_depth(),
    bottom_ekman_depth(),
    middle_velocity(),
    middle_orientation(),
    f_and_sqrt_rho(),
    wind_angle(),
    wind_stress()
{
}

EkmanUWCurrentModel::Input::Input(Seabed seabed_input) :
    seabed(std::move(seabed_input)),
    top_ekman_depth(),
    bottom_ekman_depth(),
    middle_velocity(),
    middle_orientation(),
    f_and_sqrt_rho(),
    wind_angle(),
    wind_stress()
{
}
