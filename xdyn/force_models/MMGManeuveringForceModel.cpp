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
#include "MMGPropellerForceModel.hpp"

#define PI M_PI

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
        a(0.0),
        b(0.0),
        GM(0.0),
        zH(0.0)
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
    node["a"] >> ret.a;
    node["b"] >> ret.b;
    xdyn::yaml_parser::parse_uv(node["GM"],ret.GM);
    xdyn::yaml_parser::parse_uv(node["zH"],ret.zH);

    return ret;
}

Matrix66 MMGManeuveringForceModel::get_inertia_matrix_in_Ob(Matrix66 inertia_matrix_in_G, Eigen::Vector3d G_in_MMG_frame)
{
    /* The transport of the inertia matrix from G to Ob is done as in
     T. I. Fossen, Handbook of Marine Craft Hydrodynamics and Motion Control. Wiley, 2011. doi: 10.1002/9781119994138.
    */

    // First the cross product operator S is defined (eq. 2.10, p20)
    Matrix3x3 S = Matrix3x3::Zero();
    S(0,1)=-G_in_MMG_frame(2);
    S(0,2)=G_in_MMG_frame(1);
    S(1,2)=-G_in_MMG_frame(0);
    S(1,0)=-S(0,1);
    S(2,0)=-S(0,2);
    S(2,1)=-S(1,2);
    
    // The transformation matrix is defined (eq. 3.24, p49)
    Matrix66 H_mat = Matrix6x6::Zero();
    for (int i = 0; i < 6; i++)
    {
        H_mat(i,i)=1.;
    }
    H_mat.block<3,3>(0,3)=S.transpose();

    // The inertia matrix at MMG center of frame is then computed (eq. 3.26, p50)
    return H_mat.transpose()*inertia_matrix_in_G*H_mat;
}


Wrench MMGManeuveringForceModel::get_force(const BodyStates& states, const double /*t*/, const EnvironmentAndFrames& env, const std::map<std::string,double>& /*commands*/) const
{
    // Initialize the output tensor
    ssc::kinematics::Vector6d tau = ssc::kinematics::Vector6d::Zero();

    // Get states
    const double u = states.u();
    const double v = states.v();
    const double r = states.r();
    const double phi = MMGPropellerForceModel::wrapToPi(states.get_angles(env.rot).phi);
    const double dphi_dt = states.p();

    // Get CoG location
    const Eigen::Vector3d G_mmg_frame = states.G - env.k->get(body_name, name).get_point();// G position in MMG frame

    // Get inertia and added mass
    const double body_mass = states.solid_body_inertia(2,2);//body mass
    const double Ix = states.solid_body_inertia(3,3);// roll moment of solid inertia in G
    const double mx = states.added_mass_matrix(0,0);
    const double my = states.added_mass_matrix(1,1);
    const double Jx = get_inertia_matrix_in_Ob(states.added_mass_matrix,G_mmg_frame)(3,3);//roll moment of added mass in Ob 

    // Compute roll damping coefficients
    // Warning : the solid inertia is in G while the added mass inertia is in Ob ! This is strange but it is the way it is done in the reference article.
    const double K_phidot=-2*input.a/PI*sqrt(body_mass*env.g*input.GM*(Jx+Ix)) ;
    const double K_phidot2=-0.75*input.b*180/PI*(Jx+Ix);

    const double zH_mmg_frame = input.zH - env.k->get(body_name, name).get_point().z();// vertical position of the hull force point of application in MMG frame

    // Compute the hull tensor
    const double vm = v - G_mmg_frame(0)*r+G_mmg_frame(2)*dphi_dt;
    const double U = hypot(u, vm);
    if (U!=0)
    {
        const double vm_ = vm/U;
        const double r_ = r*input.Lpp/U;
        const double X_ = -input.R0 + input.Xvv*vm_*vm_ + input.Xrr*r_*r_ + input.Xvr*vm_*r_ + input.Xvvvv*vm_*vm_*vm_*vm_ + input.Xvphi*vm_*phi + input.Xrphi*r_*phi + input.Xphiphi*phi*phi;
        const double Y_ = input.Yv*vm_ + input.Yr*r_ + input.Yvvv*vm_*vm_*vm_ + input.Yvrr*vm_*r_*r_  + input.Yrrr*r_*r_*r_ + input.Yrvv*r_*vm_*vm_ + input.Yphi*phi + input.Yphivv*phi*vm_*vm_ + input.Yvphiphi*vm_*phi*phi + input.Yphirr*phi*r_*r_ + input.Yrphiphi*r_*phi*phi;
        const double N_ = input.Nv*vm_ + input.Nr*r_ + input.Nvvv*vm_*vm_*vm_ + input.Nvrr*vm_*r_*r_  + input.Nrrr*r_*r_*r_ + input.Nrvv*r_*vm_*vm_ + input.Nphi*phi + input.Nphivv*phi*vm_*vm_ + input.Nvphiphi*vm_*phi*phi + input.Nphirr*phi*r_*r_ + input.Nrphiphi*r_*phi*phi;

        tau(0) = 0.5*env.rho*pow(U,2)*input.Lpp*input.T*X_;
        tau(1) = 0.5*env.rho*pow(U,2)*input.Lpp*input.T*Y_;
        tau(3) = -zH_mmg_frame*tau(1);
        tau(5) = 0.5*env.rho*pow(U,2)*input.Lpp*input.Lpp*input.T*N_;
    }
    // We had the heeling righting moment and the roll damping terms
    tau(3)+=- body_mass*env.g*input.GM*phi + K_phidot*dphi_dt + K_phidot2 * dphi_dt*abs(dphi_dt);
    // We then had the coriolis and centripetal hydrodynamic forces
    tau(0)+= my*vm*r;
    tau(1)-= mx*u*r;

    return Wrench(ssc::kinematics::Point(name, 0, 0, 0), body_name, tau);
}
