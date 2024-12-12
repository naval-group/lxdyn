/*
 * MMGPropellerForceModel.hpp
 *
 *  Created on: Oct 2, 2024
 *      Author: fbickert
 */

#include "MMGPropellerForceModel.hpp"
#include "xdyn/yaml_parser/external_data_structures_parsers.hpp"
#include "xdyn/yaml_parser/yaml_compat.h"
#define PI M_PI

#include <cmath>

std::string MMGPropellerForceModel::model_name() {return "MMG propeller";}

MMGPropellerForceModel::Yaml::Yaml() :
        k0(),
        k1(),
        k2(),
        C0(),
        C1(),
        C2(),
        application_point(YamlCoordinates())
{
}

MMGPropellerForceModel::Yaml::Yaml(const AbstractWageningen::Yaml& y) :
        AbstractWageningen::Yaml(y),
        k0(),
        k1(),
        k2(),
        C0(),
        C1(),
        C2(),
        application_point(YamlCoordinates())
{
}

MMGPropellerForceModel::Yaml MMGPropellerForceModel::parse(const std::string& yaml)
{
    YAML::Node node = YAML::Load(yaml);
    Yaml ret = AbstractWageningen::parse(yaml);
    node["k0"]  >> ret.k0;
    node["k1"] >> ret.k1;
    node["k2"] >> ret.k2;
    node["C0"] >> ret.C0;
    node["C1"] >> ret.C1;
    node["C2"] >> ret.C2;
    node["calculation point in body frame"] >> ret.application_point;

    return ret;
}

MMGPropellerForceModel::MMGPropellerForceModel(const Yaml& input, const std::string& body_name_, const EnvironmentAndFrames& env):
            AbstractWageningen(input, body_name_, env),
            m_k0(input.k0),
            m_k1(input.k1),
            m_k2(input.k2),
            m_C0(input.C0),
            m_C1(input.C1),
            m_C2(input.C2),
            m_position_of_propeller_in_MMG_frame(),
            m_position_of_MMG_frame_in_body_frame(input.application_point.x,input.application_point.y,input.application_point.z),
            m_Jmax(std::max((-m_k1+sqrt(m_k1*m_k1-4*m_k2*m_k0))/2/m_k2,(-m_k1-sqrt(m_k1*m_k1-4*m_k2*m_k0))/2/m_k2))//larger root of the 2nd degree polynom k2*J^2+k1*J+k0
{
    can_find_internal_frame(env.k);
    m_position_of_propeller_in_MMG_frame= env.k->get(body_name,name).get_point().v-m_position_of_MMG_frame_in_body_frame;
}

double MMGPropellerForceModel::get_Kt(const std::map<std::string,double>&, const double J) const
{
    /*
    Computes K_T as in the MMG method described in H. Yasukawa and Y. Yoshimura, “Introduction of MMG standard method for ship maneuvering predictions,” Journal of Marine Science and Technology, vol. 20, no. 1, pp. 37–52, Nov. 2014, doi: 10.1007/s00773-014-0293-y.
    */
    check(J);
    const double saturated_J=saturate(J);
    double ret=m_k0+m_k1*saturated_J+m_k2*saturated_J*saturated_J;
    return ret;
}

double MMGPropellerForceModel::get_Kq(const std::map<std::string,double>&, const double) const
{
    double ret = 0;
    return ret;
}

double MMGPropellerForceModel::wrapToPi(double x)
{
    x = fmod(x + PI,2.*PI);
    if (x <= 0)
        x += 2.*PI;
    return x - PI;
}

double MMGPropellerForceModel::get_wake_factor(const BodyStates& states) const
{
    /*
    Accounts for the wake factor dependency on the leeway angle
    */
    const double xG = get_CoG_position_in_MMG_frame(states).x();// Longitudinal position of the CoG in MMG frame
    const double zG = get_CoG_position_in_MMG_frame(states).z();// Vertical position of the CoG in MMG frame
    const double xP = get_propeller_position_in_MMG_frame().x();// Longitudinal position of the propeller in MMG frame
    const double zP = get_propeller_position_in_MMG_frame().z();// Vertical position of the propeller in MMG frame

    const double vm = states.v() - xG*states.r() + zG*states.p();// Lateral ship velocity at midship in body frame
    double beta=atan2(-vm,states.u());//leeway angle at midship
    double U=sqrt(states.u()*states.u()+vm*vm);//ship velocity

    double C2;
    double beta_P=wrapToPi(beta-xP*states.r()/U + zP*states.p()/U);

    if (beta_P<0)
    {
        C2=m_C2[0];
    }
    else
    {
        C2=m_C2[1];
    }

    /*
    The formula implemented here combines the formula in Yasukawa & Yoshimura (2014) and the formula in Okuda et. al. (2023)
    If it is the first formula which is wanted then C0 must be set to 0. so that wp=1-(1-wp0)*ratio
    If it is the second formula which is wanted then C2 must be set to [1.,1.] so that ratio=1 and then wp=wp0*exp(C0*betaP^2)
    */
    double ratio= 1.+(1.-exp(-m_C1*std::abs(beta_P)))*(C2-1.);
    double wake_fac=1.-(1.-w*exp(m_C0*beta_P*beta_P))*ratio;

    if (wake_fac<0)
    {
        wake_fac=0;
        std::cerr << "Warning: the computed wake factor is negative! Saturated to 0 to continue simulation." << std::endl;
    }

    return wake_fac;
}

void MMGPropellerForceModel::check(const double J) const
{
    if ((J<0) or (J>m_Jmax))
    {
        std::cerr << "Warning: MMG propeller model used outside of its domain. Maybe n is too small ? Invalid advance ratio J: expected 0 <= J <=  " << m_Jmax << "  but got J=" << J << ". ";
    }
    if (J<0)
    {
        std::cerr <<"Saturating at 0 to continue simulation." << std::endl;
    }
    if (J>m_Jmax)
    {
        std::cerr <<"Saturating at " << m_Jmax << " to continue simulation." << std::endl;
    }
}

double MMGPropellerForceModel::saturate(const double J) const
{
    return std::max(std::min(J,m_Jmax),0.);
}

Eigen::Vector3d MMGPropellerForceModel::get_propeller_position_in_MMG_frame() const
{
    return m_position_of_propeller_in_MMG_frame;
}

Eigen::Vector3d MMGPropellerForceModel::get_MMG_frame_position_in_body_frame() const
{
    return m_position_of_MMG_frame_in_body_frame;
}

Eigen::Vector3d MMGPropellerForceModel::get_CoG_position_in_MMG_frame(const BodyStates& states) const
{
    return states.G.v-m_position_of_MMG_frame_in_body_frame;
}