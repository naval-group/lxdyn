#include "HydroPolarForceModel.hpp"
#include "xdyn/yaml_parser/external_data_structures_parsers.hpp"
#include "xdyn/yaml_parser/yaml_compat.h"
#include <iostream>
#include "xdyn/yaml_parser/parse_unit_value.hpp"
#include <memory>
#include <ssc/kinematics.hpp>
#include <algorithm>
#include <cmath>

HydroPolarForceModel::Input::Input():
    name(),
    internal_frame(),
    angle_of_attack(),
    lift_coefficient(),
    drag_coefficient(),
    moment_coefficient(),
    reference_area(0.0),
    chord_length(),
    use_waves_velocity(false),
    angle_command()
{}

std::string HydroPolarForceModel::model_name()
{
    return "hydrodynamic polar";
}

HydroPolarForceModel::HydroPolarForceModel(const Input& input, const std::string& body_name_, const EnvironmentAndFrames& env):
        ForceModel(input.name, input.angle_command ? std::vector<std::string>({input.angle_command.get()}) : std::vector<std::string>(), input.internal_frame, body_name_, env),
        Cl(),
        Cd(),
        Cm(),
        reference_area(input.reference_area),
        chord_length(input.chord_length),
        symmetry(),
        use_waves_velocity(input.use_waves_velocity),
        angle_command(input.angle_command),
        relative_velocity(new double(0)),
        angle_of_attack(new double(0))
        // relative_velocity & angle_of_attack need to be stored for outputting, but because ForceModel::get_force(...) is const, it can't modify normal variables. That's why they're hidden behind pointers, to break constness
{
    if (input.lift_coefficient.size()==0)
    {
        THROW(__PRETTY_FUNCTION__, InvalidInputException, "An empty vector was provided for the angle of attack, which must be provided from either -180° or 0deg (symmetry) to 180deg.");
    }
    const double min_alpha = *std::min_element(input.angle_of_attack.begin(),input.angle_of_attack.end());
    const double max_alpha = *std::max_element(input.angle_of_attack.begin(),input.angle_of_attack.end());
    const double eps = 0.1*M_PI/180;
    if (input.lift_coefficient.size()!=input.angle_of_attack.size() || input.drag_coefficient.size()!=input.angle_of_attack.size())
    {
        THROW(__PRETTY_FUNCTION__, InvalidInputException, "Angle of attack, lift coefficient and drag coefficient must all have the same size.");
    }
    if(min_alpha > eps || max_alpha < M_PI-eps)
    {
        THROW(__PRETTY_FUNCTION__, InvalidInputException, "Angle of attack must be provided from either -180° or 0deg (symmetry) to 180deg.");
    }
    if (max_alpha > M_PI+eps)
    {
        std::cerr << "WARNING: In hydrodynamic polar force model '" << name << "', you provided a maximum angle of attack higher than 180deg. All values over 180deg will be ignored." << std::endl;
    }
    if (min_alpha > -eps) // min_alpha is close to 0
    {
        symmetry = true;
    }
    else if (min_alpha > -M_PI+eps) // min_alpha is between -pi and 0 (but not close enough to either)
    {
        std::cerr << "WARNING: In hydrodynamic polar force model '" << name << "', you provided a minimum angle of attack between -180deg and 0deg. Symmetry will be assumed and values under 0deg will be ignored." << std::endl;
        symmetry = true;
    }
    else if (min_alpha > -M_PI-eps) // min_alpha is close to -pi
    {
        symmetry = false;
    }
    else // min_alpha is under -pi (but not close enough)
    {
        std::cerr << "WARNING: In hydrodynamic polar force model '" << name << "', you provided a minimum angle of attack lower than -180deg. All values under -180deg will be ignored." << std::endl;
        symmetry = false;
    }
    Cl = std::make_unique<ssc::interpolation::SplineVariableStep>(input.angle_of_attack, input.lift_coefficient);
    Cd = std::make_unique<ssc::interpolation::SplineVariableStep>(input.angle_of_attack, input.drag_coefficient);
    if (input.moment_coefficient.is_initialized())
    {
        if (input.moment_coefficient.get().size()!=input.angle_of_attack.size())
        {
            THROW(__PRETTY_FUNCTION__, InvalidInputException, "Angle of attack and moment coefficient must have the same size.");
        }
        Cm = std::make_unique<ssc::interpolation::SplineVariableStep>(input.angle_of_attack, input.moment_coefficient.get());
    }
    if (use_waves_velocity && env.w.use_count()==0)
    {
        THROW(__PRETTY_FUNCTION__, InvalidInputException, "In order to take into account the orbital velocity of waves, a wave model must be defined in the 'environment models' section.");
    }
}

HydroPolarForceModel::Input HydroPolarForceModel::parse(const std::string& yaml)
{
    YAML::Node node = YAML::Load(yaml);
    Input ret;
    node["name"] >> ret.name;
    xdyn::yaml_parser::parse_uv(node["angle of attack"], ret.angle_of_attack);
    ret.lift_coefficient = extract_vector_of_doubles(node, "lift coefficient");
    ret.drag_coefficient = extract_vector_of_doubles(node, "drag coefficient");
    parse_optional(node, "moment coefficient", ret.moment_coefficient);
    xdyn::yaml_parser::parse_uv(node["reference area"], ret.reference_area);
    if (node["chord length"])
    {
        double cord_length; // Intermediate value is necessary to call xdyn::yaml_parser::parse_uv
        xdyn::yaml_parser::parse_uv(node["chord length"], cord_length);
        ret.chord_length = cord_length;
    }
    node["position of calculation frame"] >> ret.internal_frame;
    node["take waves orbital velocity into account"] >> ret.use_waves_velocity;
    parse_optional(node, "angle command", ret.angle_command);
    return ret;
}

Wrench HydroPolarForceModel::get_force(const BodyStates& states, const double t, const EnvironmentAndFrames& env, const std::map<std::string,double>& commands) const
{
    // get_rot_from_ned_to_body() returns the body->NED transform in spite of its name: core/Body.cpp
    // integrates dx/dt = R*(u,v,w), and (u,v,w) and (p,q,r) are body-frame quantities.
    const ssc::kinematics::RotationMatrix ctm_body_to_ned = states.get_rot_from_ned_to_body();
    const ssc::kinematics::RotationMatrix ctm_body_to_foil = env.k->get(name, body_name).get_rot();
    const Eigen::Vector3d body_position_in_ned(states.x(), states.y(), states.z());
    const Eigen::Vector3d body_velocity(states.u(), states.v(), states.w());
    const Eigen::Vector3d body_angular_velocity(states.p(), states.q(), states.r());
    const Eigen::Vector3d foil_position_in_body = env.k->get(body_name, name).get_point().v;
    const Eigen::Vector3d foil_position_in_ned = body_position_in_ned + ctm_body_to_ned*foil_position_in_body;
    // Built from body-frame quantities alone, so the inflow cannot depend on the vessel's heading
    Eigen::Vector3d flow_in_foil = ctm_body_to_foil*(body_velocity + body_angular_velocity.cross(foil_position_in_body));
    double water_surface_height = 0.;
    if (env.w.use_count())
    {
        const auto wave_height = env.w->get_and_check_wave_height({foil_position_in_ned(0)}, {foil_position_in_ned(1)}, t);
        if (use_waves_velocity)
        {
            const auto orbital_velocity = env.w->get_and_check_orbital_velocity(env.g, {foil_position_in_ned(0)}, {foil_position_in_ned(1)}, {foil_position_in_ned(2)}, t, wave_height);
            const Eigen::Vector3d wave_velocity_in_ned(orbital_velocity.m(0,0), orbital_velocity.m(1,0), orbital_velocity.m(2,0));
            flow_in_foil -= env.k->get(name, orbital_velocity.get_frame()).get_rot()*wave_velocity_in_ned;
        }
        water_surface_height = wave_height.at(0);
    }
    flow_in_foil -= ctm_body_to_foil*ctm_body_to_ned.transpose()*env.get_UWCurrent(foil_position_in_ned, t);
    const double beta = -std::atan2(flow_in_foil(1), flow_in_foil(0)); // Incident angle of the flow, in [-pi,pi]
    const double U = std::hypot(flow_in_foil(0), flow_in_foil(1)); // Apparent flow velocity in the foil's (x,y) plane
    double alpha = beta; // Angle of attack
    if (angle_command)
    {
        alpha += commands.at(angle_command.get());
    }
    alpha = std::remainder(alpha, 2*M_PI); // Putting alpha in [-pi,pi]
    *angle_of_attack = alpha;
    *relative_velocity = U;
    Wrench ret(ssc::kinematics::Point(name,0,0,0), name);
    // NED z points down, so the calculation point is submerged when it lies below the free surface
    if (foil_position_in_ned(2) < water_surface_height)
    {
        std::cerr << "WARNING: In hydrodynamic polar force model '" << name << "', the calculation point is outside of the water (z = " << foil_position_in_ned(2) << "). In consequence, no force is being applied by this model." << std::endl;
        return ret;
    }
    const double alpha_prime = (symmetry && alpha<0) ? -alpha : alpha;
    const double lift = 0.5*Cl->f(alpha_prime)*env.rho*U*U*reference_area;
    const double drag = 0.5*Cd->f(alpha_prime)*env.rho*U*U*reference_area;
    const double lift_sign = (alpha>=0) ? 1. : -1.;
    ret.X() = -drag*std::cos(beta) + lift_sign*lift*std::sin(beta);
    ret.Y() =  drag*std::sin(beta) + lift_sign*lift*std::cos(beta);
    if (Cm)
    {
        const double normalization_cubic_length = chord_length.is_initialized() ? reference_area*chord_length.get() : std::pow(reference_area, 1.5);
        ret.N() = lift_sign*0.5*Cm->f(alpha_prime)*env.rho*U*U*normalization_cubic_length;
    }
    return ret;
}

void HydroPolarForceModel::extra_observations(Observer& observer) const
{
    observer.write_before_solver_step(*angle_of_attack, DataAddressing({"efforts",body_name,name,"alpha"},std::string("alpha(")+name+","+body_name+")"));
    observer.write_before_solver_step(*relative_velocity, DataAddressing({"efforts",body_name,name,"U"},std::string("U(")+name+","+body_name+")"));
}
