/*
 * MMGPropellerForceModel.hpp
 *
 *  Created on: Oct 2, 2024
 *      Author: fbickert
 */

#include "MMGPropellerForceModel.hpp"
#include "xdyn/yaml_parser/yaml_compat.h"

std::string MMGPropellerForceModel::model_name() {return "MMG propeller";}

MMGPropellerForceModel::Yaml::Yaml() :
        k0(),
        k1(),
        k2()
{
}

MMGPropellerForceModel::Yaml::Yaml(const AbstractWageningen::Yaml& y) :
        AbstractWageningen::Yaml(y),
        k0(),
        k1(),
        k2()
{
}

MMGPropellerForceModel::Yaml MMGPropellerForceModel::parse(const std::string& yaml)
{
    YAML::Node node = YAML::Load(yaml);
    Yaml ret = AbstractWageningen::parse(yaml);
    node["k0"]  >> ret.k0;
    node["k1"] >> ret.k1;
    node["k2"] >> ret.k2;

    return ret;
}

MMGPropellerForceModel::MMGPropellerForceModel(const Yaml& input, const std::string& body_name_, const EnvironmentAndFrames& env):
            AbstractWageningen(input, body_name_, env), 
            m_k0(input.k0), 
            m_k1(input.k1), 
            m_k2(input.k2),
            m_Jmax(std::max((-m_k1+sqrt(m_k1*m_k1-4*m_k2*m_k0))/2/m_k2,(-m_k1-sqrt(m_k1*m_k1-4*m_k2*m_k0))/2/m_k2))//larger root of the 2nd degree polynom k2*J^2+k1*J+k0
{
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

double MMGPropellerForceModel::get_wake_factor(const BodyStates& states) const
{
    /*
    Accounts for the wake factor dependency on the leeway angle using the formula (14) in H. Yasukawa and Y. Yoshimura, “Introduction of MMG standard method for ship maneuvering predictions,” Journal of Marine Science and Technology, vol. 20, no. 1, pp. 37–52, Nov. 2014, doi: 10.1007/s00773-014-0293-y.
    */
    double leeway_angle_rad=atan2(states.v(),states.u());
    double ret =1-(1-w)*(1+(1-cos(leeway_angle_rad)*cos(leeway_angle_rad))*(1-abs(leeway_angle_rad)));
    return ret;
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