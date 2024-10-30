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
        k2(),
        C1(),
        C2()
{
}

MMGPropellerForceModel::Yaml::Yaml(const AbstractWageningen::Yaml& y) :
        AbstractWageningen::Yaml(y),
        k0(),
        k1(),
        k2(),
        C1(),
        C2()        
{
}

MMGPropellerForceModel::Yaml MMGPropellerForceModel::parse(const std::string& yaml)
{
    YAML::Node node = YAML::Load(yaml);
    Yaml ret = AbstractWageningen::parse(yaml);
    node["k0"]  >> ret.k0;
    node["k1"] >> ret.k1;
    node["k2"] >> ret.k2;
    node["C1"] >> ret.C1;
    node["C2"] >> ret.C2;

    return ret;
}

MMGPropellerForceModel::MMGPropellerForceModel(const Yaml& input, const std::string& body_name_, const EnvironmentAndFrames& env):
            AbstractWageningen(input, body_name_, env), 
            m_k0(input.k0), 
            m_k1(input.k1), 
            m_k2(input.k2),
            m_C1(input.C1),
            m_C2(input.C2),
            m_longitudinal_position_of_propeller_in_body_frame(),
            m_Jmax(std::max((-m_k1+sqrt(m_k1*m_k1-4*m_k2*m_k0))/2/m_k2,(-m_k1-sqrt(m_k1*m_k1-4*m_k2*m_k0))/2/m_k2))//larger root of the 2nd degree polynom k2*J^2+k1*J+k0
{
    can_find_internal_frame(env.k);
    m_longitudinal_position_of_propeller_in_body_frame= env.k->get(body_name,name).get_point().x();
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
    Accounts for the wake factor dependency on the leeway angle as in H. Yasukawa and Y. Yoshimura, “Introduction of MMG standard method for ship maneuvering predictions,” Journal of Marine Science and Technology, vol. 20, no. 1, pp. 37–52, Nov. 2014, doi: 10.1007/s00773-014-0293-y.
    */
    double beta=atan2(-states.v(),states.u());//leeway angle at midship
    double U=sqrt(states.u()*states.u()+states.v()*states.v());//ship velocity
    
    double C2;  
    // Equation 15
    double beta_P=beta-m_longitudinal_position_of_propeller_in_body_frame*states.r()/U;

    if (beta_P<=0)
    {
        C2=m_C2[0];
    }
    else
    {
        C2=m_C2[1];
    }

    // Equation 14
    // double ratio =1+(1-cos(beta)*cos(beta))*(1-abs(beta));   
    
    // Equation (16)
    double ratio= 1+(1-exp(-m_C1*abs(beta_P)))*(C2-1);
    
    double wake_fac=1-(1-w)*ratio;

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

double MMGPropellerForceModel::get_longitudinal_position_in_body_frame() const
{
    return m_longitudinal_position_of_propeller_in_body_frame;
}
