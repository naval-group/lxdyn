#include "MMGManeuveringForceModel.hpp"
#include "xdyn/core/Body.hpp"
#include "xdyn/core/BodyStates.hpp"
#include "xdyn/core/EnvironmentAndFrames.hpp"
#include "xdyn/external_data_structures/YamlPosition.hpp"
#include "xdyn/yaml_parser/parse_unit_value.hpp"
#include <ssc/kinematics.hpp>
#include <Eigen/Dense>
#include "xdyn/yaml_parser/yaml_compat.h"
#include <cmath>

MMGManeuveringForceModel::Input::Input():
        application_point(YamlCoordinates()),
        Lpp(0.0),
        T(0.0),
        R0(0.0),
        Xvv(0.0),
        Xrr(0.0),
        Xvr(0.0),
        Xvvvv(0.0),
        Xvphi(0.0),
        Xrphi(0.0),
        Xphiphi(0.0),
        Yv(0.0),
        Yr(0.0),
        Yvvv(0.0),
        Yrvv(0.0),
        Yvrr(0.0),
        Yrrr(0.0),
        Yphi(0.0),
        Yphivv(0.0),
        Yvphiphi(0.0),
        Yphirr(0.0),
        Yrphiphi(0.0),
        Nv(0.0),
        Nr(0.0),
        Nvvv(0.0),
        Nrvv(0.0),
        Nvrr(0.0),
        Nrrr(0.0),
        Nphi(0.0),
        Nphivv(0.0),
        Nvphiphi(0.0),
        Nphirr(0.0),
        Nrphiphi(0.0),
        Kphi(0.0),
        Kphiphi(0.0),
        mx(0.0),
        my(0.0),
        GM(0.0)
{}

std::string MMGManeuveringForceModel::model_name() {return "MMG maneuvering";}

MMGManeuveringForceModel::MMGManeuveringForceModel(const Input& input_, const std::string& body_name, const EnvironmentAndFrames& env):
        ForceModel(MMGManeuveringForceModel::model_name(), {}, YamlPosition(input_.application_point, YamlAngle(), body_name), body_name, env),
        input(input_)
{}

MMGManeuveringForceModel::Input MMGManeuveringForceModel::parse(const std::string& yaml)
{
    YAML::Node node = YAML::Load(yaml);
    Input ret;

    node["calculation point in body frame"] >> ret.application_point;
    xdyn::yaml_parser::parse_uv(node["Lpp"], ret.Lpp);
    xdyn::yaml_parser::parse_uv(node["T"], ret.T);
    node["R0"] >> ret.R0;
    node["Xvv"] >> ret.Xvv;
    node["Xrr"] >> ret.Xrr;
    node["Xvr"] >> ret.Xvr;
    node["Xvvvv"] >> ret.Xvvvv;
    node["Xvphi"] >> ret.Xvphi;
    node["Xrphi"] >> ret.Xrphi;
    node["Xphiphi"] >> ret.Xphiphi;
    node["Yv"] >> ret.Yv;
    node["Yr"] >> ret.Yr;
    node["Yvvv"] >> ret.Yvvv;
    node["Yrvv"] >> ret.Yrvv;
    node["Yvrr"] >> ret.Yvrr;
    node["Yrrr"] >> ret.Yrrr;
    node["Yphi"] >> ret.Yphi;
    node["Yphivv"] >> ret.Yphivv;
    node["Yvphiphi"] >> ret.Yvphiphi;
    node["Yphirr"] >> ret.Yphirr;
    node["Yrphiphi"] >> ret.Yrphiphi;  
    node["Nv"] >> ret.Nv;
    node["Nr"] >> ret.Nr;
    node["Nvvv"] >> ret.Nvvv;
    node["Nrvv"] >> ret.Nrvv;
    node["Nvrr"] >> ret.Nvrr;
    node["Nrrr"] >> ret.Nrrr;
    node["Nphi"] >> ret.Nphi;
    node["Nphivv"] >> ret.Nphivv;
    node["Nvphiphi"] >> ret.Nvphiphi;
    node["Nphirr"] >> ret.Nphirr;
    node["Nrphiphi"] >> ret.Nrphiphi;
    node["Kphi"] >> ret.Kphi;
    node["Kphiphi"] >> ret.Kphiphi;
    node["mx"] >> ret.mx;
    node["my"] >> ret.my;
    node["GM"] >> ret.GM;

    return ret;
}

Wrench MMGManeuveringForceModel::get_force(const BodyStates& states, const double /*t*/, const EnvironmentAndFrames& env, const std::map<std::string,double>& /*commands*/) const
{
    ssc::kinematics::Vector6d tau = ssc::kinematics::Vector6d::Zero();
    const double u = states.u();
    const double v = states.v();
    const double r = states.r();
    const double phi = states.get_angles(env.rot).phi;
    const double dphi_dt = states.p();

    const double body_mass = states.solid_body_inertia(2,2);
    const double xG = states.G.v(0) - env.k->get(body_name, name).get_point().v(0);
    const double zG = states.G.v(2) - env.k->get(body_name, name).get_point().v(2);

    const double vm = v - xG*r+zG*dphi_dt;
    const double U = hypot(u, vm);
    if (U!=0)
    {
        const double u_ = u/U;
        const double vm_ = vm/U;
        const double r_ = r*input.Lpp/U;
        const double X_ = -input.R0 + input.Xvv*vm_*vm_ + input.Xrr*r_*r_ + input.Xvr*vm_*r_ + input.Xvvvv*vm_*vm_*vm_*vm_ + input.Xvphi*vm_*phi + input.Xrphi*r_*phi + input.Xphiphi*phi*phi + input.my*vm_*r_;
        const double Y_ = input.Yv*vm_ + input.Yr*r_ + input.Yvvv*vm_*vm_*vm_ + input.Yvrr*vm_*r_*r_  + input.Yrrr*r_*r_*r_ + input.Yrvv*r_*vm_*vm_ + input.Yphi*phi + input.Yphivv*phi*vm_*vm_ + input.Yvphiphi*vm_*phi*phi + input.Yphirr*phi*r_*r_ + input.Yrphiphi*r_*phi*phi - input.mx*u_*r_;
        const double N_ = input.Nv*vm_ + input.Nr*r_ + input.Nvvv*vm_*vm_*vm_ + input.Nvrr*vm_*r_*r_  + input.Nrrr*r_*r_*r_ + input.Nrvv*r_*vm_*vm_ + input.Nphi*phi + input.Nphivv*phi*vm_*vm_ + input.Nvphiphi*vm_*phi*phi + input.Nphirr*phi*r_*r_ + input.Nrphiphi*r_*phi*phi;
        const double K = - body_mass*env.g*input.GM*phi + input.Kphi*dphi_dt + input.Kphiphi * dphi_dt*abs(dphi_dt);
        tau(0) = 0.5*env.rho*pow(U,2)*input.Lpp*input.T*X_;
        tau(1) = 0.5*env.rho*pow(U,2)*input.Lpp*input.T*Y_;
        tau(3) = K;
        tau(5) = 0.5*env.rho*pow(U,2)*input.Lpp*input.Lpp*input.T*N_;
    }
    return Wrench(ssc::kinematics::Point(name, 0, 0, 0), body_name, tau);
}
