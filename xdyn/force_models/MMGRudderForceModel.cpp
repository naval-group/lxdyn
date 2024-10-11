/*
 * MMGRudderForceModel.cpp
 *
 *  Created on: Oct 7, 2024
 *      Author: fbickert
 */

#include "MMGRudderForceModel.hpp"
#include "xdyn/core/yaml2eigen.hpp"
#include "xdyn/yaml_parser/external_data_structures_parsers.hpp"

#include <ssc/yaml_parser.hpp>
#include "yaml.h"

#define _USE_MATH_DEFINE
#include <cmath>
#define PI M_PI

///////////////////////////////////////////////////////////////
//// FUNCTIONS IDENTICAL TO THOSE OF CLASS RUDDERFORCEMODEL ///
///////////////////////////////////////////////////////////////

double MMGRudderForceModel::RudderModel::get_D() const
{
    return m_D;
}

Eigen::Vector3d MMGRudderForceModel::RudderModel::get_rudder_location() const
{
    return position_of_the_rudder_frame_in_the_body_frame;
}

double MMGRudderForceModel::RudderModel::get_angle_of_attack(
    const double rudder_angle, //!< Rudder angle (in radian): positive if rudder on port side
    const double fluid_angle   //!< Angle of the fluid in the ship's reference frame (0 if the fluid is propagating along -X, positive if fluid is coming from starboard)
    ) const
{
    return  rudder_angle-fluid_angle;
}

double MMGRudderForceModel::RudderModel::get_fluid_angle(
    const ssc::kinematics::Point Vs   //!< Ship speed relative to the fluid, inside & outside wake
    ) const
{
    return atan2(Vs.y(), Vs.x());
}

MMGRudderForceModel::InOutWake<ssc::kinematics::Vector6d> MMGRudderForceModel::RudderModel::get_wrench(
    const double rudder_angle,                                      //!< Rudder angle (in radian): positive if rudder on port side
    const MMGRudderForceModel::InOutWake<ssc::kinematics::Point>& Vs,  //!< Speed of the ship relative to the fluid (in m/s)
    const MMGRudderForceModel::InOutWake<double>& area                 //!< Rudder area (in or outside wake) in m^2
    ) const
{
    MMGRudderForceModel::InOutWake<ssc::kinematics::Vector6d> ret;
    ret.in_wake = get_wrench(rudder_angle, Vs.in_wake, area.in_wake);
    ret.outside_wake = get_wrench(rudder_angle, Vs.outside_wake, area.outside_wake);
    return ret;
}

Wrench MMGRudderForceModel::get_force(
    const BodyStates& states, const double t,
    const EnvironmentAndFrames& env,
    const std::map<std::string,double>& commands) const
{
    const Wrench propeller_wrench_propeller_frame_at_P = propulsionModel.get_force(states, t, env, commands);// propeller tensor in propeller frame expressed at the propeller location
    const std::string frame = propeller_wrench_propeller_frame_at_P.get_frame();// get propeller frame
    can_find_internal_frame(env.k);// Check if the internal frame is accessible
    const Wrench propeller_wrench_body_frame_at_P = propeller_wrench_propeller_frame_at_P.change_frame(body_name,env.k);// propeller tensor in body frame expressed at the propeller location
    const ssc::kinematics::Vector6d rudder_force = get_rudder_force(states, t, env, commands, (double)std::max(propeller_wrench_body_frame_at_P.X(),0.)); // rudder forces and moments in propeller frame expressed at the propeller location
    /*
    Negative propeller thrust means that the propeller wake is forward and therefore there is no acceleration on the rudder. In that case we consider a 0 thrust because a negative thrust would fails the calculation of CTh (negative square root calculation).
    */
    const Wrench rudder_wrench_body_frame_at_R(ssc::kinematics::Point(body_name, rudderModel.get_rudder_location()), body_name, rudder_force);// rudder tensor in body frame expressed at the rudder location
    const Wrench rudder_wrench_body_frame_at_P=rudder_wrench_body_frame_at_R.transport_to(ssc::kinematics::Point(frame,0,0,0),env.k);// rudder tensor in body frame expressed at the propeller location
    const Wrench rudderAndPropeller_wrench_body_frame_at_P = rudder_wrench_body_frame_at_P + propeller_wrench_body_frame_at_P;//rudder+propeller wrench in body frame expressed at the propeller location

    *m_propeller_wrench_internal_frame_at_P=propeller_wrench_propeller_frame_at_P;// Save propeller torser in internal frame
    
    // Compute and save propeller torsers in body and NED frame
    const Wrench propeller_wrench_body_frame_at_Ob(propeller_wrench_body_frame_at_P.transport_to(ssc::kinematics::Point(body_name,0,0,0),env.k));
    *m_propeller_wrench_body_frame_at_Ob=propeller_wrench_body_frame_at_Ob;
    const Wrench propeller_wrench_NED_frame_at_G(propeller_wrench_body_frame_at_Ob.change_point_and_frame(states.G,"NED", env.k));
    *m_propeller_wrench_NED_frame_at_G=propeller_wrench_NED_frame_at_G;
    // Compute and save rudder torsers in body and NED frame
    const Wrench rudder_wrench_body_frame_at_Ob(rudder_wrench_body_frame_at_P.transport_to(ssc::kinematics::Point(body_name,0,0,0),env.k));
    *m_rudder_wrench_body_frame_at_Ob=rudder_wrench_body_frame_at_Ob;
    const Wrench rudder_wrench_NED_frame_at_G(rudder_wrench_body_frame_at_Ob.change_point_and_frame(states.G,"NED", env.k));
    *m_rudder_wrench_NED_frame_at_G=rudder_wrench_NED_frame_at_G;

    return rudderAndPropeller_wrench_body_frame_at_P;
}

void MMGRudderForceModel::extra_observations(Observer& observer) const
{   
    std::string new_force_name=name + "(propeller)";
    // Write the propeller tensor in propeller frame at propeller location
    observer.write_before_solver_step(m_propeller_wrench_internal_frame_at_P->X(),DataAddressing({"efforts",body_name,new_force_name,name,"Fx"},std::string("Fx(")+new_force_name+","+body_name+","+name+")"));
    observer.write_before_solver_step(m_propeller_wrench_internal_frame_at_P->Y(),DataAddressing({"efforts",body_name,new_force_name,name,"Fy"},std::string("Fy(")+new_force_name+","+body_name+","+name+")"));
    observer.write_before_solver_step(m_propeller_wrench_internal_frame_at_P->Z(),DataAddressing({"efforts",body_name,new_force_name,name,"Fz"},std::string("Fz(")+new_force_name+","+body_name+","+name+")"));
    observer.write_before_solver_step(m_propeller_wrench_internal_frame_at_P->K(),DataAddressing({"efforts",body_name,new_force_name,name,"Mx"},std::string("Mx(")+new_force_name+","+body_name+","+name+")"));
    observer.write_before_solver_step(m_propeller_wrench_internal_frame_at_P->M(),DataAddressing({"efforts",body_name,new_force_name,name,"My"},std::string("My(")+new_force_name+","+body_name+","+name+")"));
    observer.write_before_solver_step(m_propeller_wrench_internal_frame_at_P->N(),DataAddressing({"efforts",body_name,new_force_name,name,"Mz"},std::string("Mz(")+new_force_name+","+body_name+","+name+")"));

    // Write the propeller tensor in body frame at body frame origin
    observer.write_before_solver_step(m_propeller_wrench_body_frame_at_Ob->X(),DataAddressing({"efforts",body_name,new_force_name,body_name,"Fx"},std::string("Fx(")+new_force_name+","+body_name+","+body_name+")"));
    observer.write_before_solver_step(m_propeller_wrench_body_frame_at_Ob->Y(),DataAddressing({"efforts",body_name,new_force_name,body_name,"Fy"},std::string("Fy(")+new_force_name+","+body_name+","+body_name+")"));
    observer.write_before_solver_step(m_propeller_wrench_body_frame_at_Ob->Z(),DataAddressing({"efforts",body_name,new_force_name,body_name,"Fz"},std::string("Fz(")+new_force_name+","+body_name+","+body_name+")"));
    observer.write_before_solver_step(m_propeller_wrench_body_frame_at_Ob->K(),DataAddressing({"efforts",body_name,new_force_name,body_name,"Mx"},std::string("Mx(")+new_force_name+","+body_name+","+body_name+")"));
    observer.write_before_solver_step(m_propeller_wrench_body_frame_at_Ob->M(),DataAddressing({"efforts",body_name,new_force_name,body_name,"My"},std::string("My(")+new_force_name+","+body_name+","+body_name+")"));
    observer.write_before_solver_step(m_propeller_wrench_body_frame_at_Ob->N(),DataAddressing({"efforts",body_name,new_force_name,body_name,"Mz"},std::string("Mz(")+new_force_name+","+body_name+","+body_name+")"));

    // Write the propeller tensor in NED frame at CoG
    observer.write_before_solver_step(m_propeller_wrench_NED_frame_at_G->X(),DataAddressing({"efforts",body_name,new_force_name,"NED","Fx"},std::string("Fx(")+new_force_name+","+body_name+","+"NED"+")"));
    observer.write_before_solver_step(m_propeller_wrench_NED_frame_at_G->Y(),DataAddressing({"efforts",body_name,new_force_name,"NED","Fy"},std::string("Fy(")+new_force_name+","+body_name+","+"NED"+")"));
    observer.write_before_solver_step(m_propeller_wrench_NED_frame_at_G->Z(),DataAddressing({"efforts",body_name,new_force_name,"NED","Fz"},std::string("Fz(")+new_force_name+","+body_name+","+"NED"+")"));
    observer.write_before_solver_step(m_propeller_wrench_NED_frame_at_G->K(),DataAddressing({"efforts",body_name,new_force_name,"NED","Mx"},std::string("Mx(")+new_force_name+","+body_name+","+"NED"+")"));
    observer.write_before_solver_step(m_propeller_wrench_NED_frame_at_G->M(),DataAddressing({"efforts",body_name,new_force_name,"NED","My"},std::string("My(")+new_force_name+","+body_name+","+"NED"+")"));
    observer.write_before_solver_step(m_propeller_wrench_NED_frame_at_G->N(),DataAddressing({"efforts",body_name,new_force_name,"NED","Mz"},std::string("Mz(")+new_force_name+","+body_name+","+"NED"+")"));

    new_force_name= name + "(rudder)";
    // Write the rudder tensor in body frame at body frame origin
    observer.write_before_solver_step(m_rudder_wrench_body_frame_at_Ob->X(),DataAddressing({"efforts",body_name,new_force_name,body_name,"Fx"},std::string("Fx(")+new_force_name+","+body_name+","+body_name+")"));
    observer.write_before_solver_step(m_rudder_wrench_body_frame_at_Ob->Y(),DataAddressing({"efforts",body_name,new_force_name,body_name,"Fy"},std::string("Fy(")+new_force_name+","+body_name+","+body_name+")"));
    observer.write_before_solver_step(m_rudder_wrench_body_frame_at_Ob->Z(),DataAddressing({"efforts",body_name,new_force_name,body_name,"Fz"},std::string("Fz(")+new_force_name+","+body_name+","+body_name+")"));
    observer.write_before_solver_step(m_rudder_wrench_body_frame_at_Ob->K(),DataAddressing({"efforts",body_name,new_force_name,body_name,"Mx"},std::string("Mx(")+new_force_name+","+body_name+","+body_name+")"));
    observer.write_before_solver_step(m_rudder_wrench_body_frame_at_Ob->M(),DataAddressing({"efforts",body_name,new_force_name,body_name,"My"},std::string("My(")+new_force_name+","+body_name+","+body_name+")"));
    observer.write_before_solver_step(m_rudder_wrench_body_frame_at_Ob->N(),DataAddressing({"efforts",body_name,new_force_name,body_name,"Mz"},std::string("Mz(")+new_force_name+","+body_name+","+body_name+")"));

    // Write the rudder tensor in NED frame at CoG
    observer.write_before_solver_step(m_rudder_wrench_NED_frame_at_G->X(),DataAddressing({"efforts",body_name,new_force_name,"NED","Fx"},std::string("Fx(")+new_force_name+","+body_name+","+"NED"+")"));
    observer.write_before_solver_step(m_rudder_wrench_NED_frame_at_G->Y(),DataAddressing({"efforts",body_name,new_force_name,"NED","Fy"},std::string("Fy(")+new_force_name+","+body_name+","+"NED"+")"));
    observer.write_before_solver_step(m_rudder_wrench_NED_frame_at_G->Z(),DataAddressing({"efforts",body_name,new_force_name,"NED","Fz"},std::string("Fz(")+new_force_name+","+body_name+","+"NED"+")"));
    observer.write_before_solver_step(m_rudder_wrench_NED_frame_at_G->K(),DataAddressing({"efforts",body_name,new_force_name,"NED","Mx"},std::string("Mx(")+new_force_name+","+body_name+","+"NED"+")"));
    observer.write_before_solver_step(m_rudder_wrench_NED_frame_at_G->M(),DataAddressing({"efforts",body_name,new_force_name,"NED","My"},std::string("My(")+new_force_name+","+body_name+","+"NED"+")"));
    observer.write_before_solver_step(m_rudder_wrench_NED_frame_at_G->N(),DataAddressing({"efforts",body_name,new_force_name,"NED","Mz"},std::string("Mz(")+new_force_name+","+body_name+","+"NED"+")"));
}

///////////////////////////////////////////////////////////////
//// FUNCTIONS DIFFERENT THAN THOSE OF CLASS RUDDERFORCEMODEL ///
///////////////////////////////////////////////////////////////

std::string MMGRudderForceModel::model_name() {return "MMG propeller+rudder";}

MMGRudderForceModel::Yaml::Yaml(const MMGPropellerForceModel::Yaml& yaml) :
    MMGPropellerForceModel::Yaml(yaml),
    Ar(),
    b(),
    xH(),
    lR(),
    tR(),
    aH(),
    gammaR(),
    epsilon(),
    kappaMmg(),
    effective_aspect_ratio(),
    position_of_the_rudder_frame_in_the_body_frame()
{
}

MMGRudderForceModel::RudderModel::RudderModel(
    const Yaml& parameters_, const double rho_) :
        m_Ar(parameters_.Ar),
        m_b(parameters_.b),
        m_D(parameters_.diameter),
        m_xH(parameters_.xH),
        m_lR(parameters_.lR),
        m_tR(parameters_.tR),
        m_aH(parameters_.aH),
        m_gammaR(parameters_.gammaR),
        m_epsilon(parameters_.epsilon),
        m_kappaMmg(parameters_.kappaMmg),
        m_effective_aspect_ratio(parameters_.effective_aspect_ratio),
        m_rho(rho_),
        position_of_the_rudder_frame_in_the_body_frame(
            parameters_.position_of_the_rudder_frame_in_the_body_frame.x,
            parameters_.position_of_the_rudder_frame_in_the_body_frame.y,
            parameters_.position_of_the_rudder_frame_in_the_body_frame.z)
{
}

MMGRudderForceModel::InOutWake<ssc::kinematics::Point> MMGRudderForceModel::RudderModel::get_vs(
    const double CTh,//!< Thrust loading coefficient, Cf. "Manoeuvring Technical Manual", J. Brix, Seehafen Verlag p. 84, eq. 1.2.20
    const double Va,//!< Projection of the ship speed (relative to the current) on the X-axis of the ship's reference frame (m/s)
    const double vR//!< Projection of the ship speed (relative to the current) on the Y-axis of the ship's reference frame (m/s)
    ) const
{
    MMGRudderForceModel::InOutWake<ssc::kinematics::Point> Vrud;
    double eta=m_D/m_b;//propeller to rudder ratio
    // Ship speed (relative to the current) in the ship's reference frame (m/s)
    Vrud.in_wake.x() = m_epsilon*Va*sqrt(eta*pow(1+m_kappaMmg*(sqrt(1+CTh)-1),2)+1-eta);
    Vrud.in_wake.y() = vR;
    Vrud.outside_wake.x() = Va;
    Vrud.outside_wake.y() = vR;
    return Vrud;
}

double MMGRudderForceModel::RudderModel::get_vr(const double u, const double vm,const double r) const
{
    double beta=atan2(-vm, u);//hull drift angle at midship, as defined in MMG
    double U=sqrt(u*u+vm*vm);
    // Equation (24)
    double betaR=beta-m_lR*r/U;
    
    // Define which value of $\gamma_R$ is to be used
    double signedGammaR;
    if (betaR<0)
    {
        signedGammaR=m_gammaR[0];
    }
    else
    {
        signedGammaR=m_gammaR[1];

    }
    //Equation (23)
    double vR=U*signedGammaR*betaR;
    return vR;
}

MMGRudderForceModel::InOutWake<double> MMGRudderForceModel::RudderModel::get_Ar() const
{
    // Equation (40)
    InOutWake<double> ar;
    ar.in_wake = m_D/m_b*m_Ar;
    ar.outside_wake = m_Ar-ar.in_wake;
    return ar;
}

ssc::kinematics::Vector6d MMGRudderForceModel::RudderModel::get_force(
    const double Fn, //!< Norm of the lift (in N)
    const double rudder_angle //!< Rudder angle (in rad)
    ) const
{
    ssc::kinematics::Vector6d ret = ssc::kinematics::Vector6d::Zero();
    // Equation (18)
    // First we compute the rudder force without considering the additional force acting on the hull (represented by the coefficients a_H and x_H) 
    ret(0)=-(1-m_tR)*Fn*sin(rudder_angle);
    ret(1)=-Fn*cos(rudder_angle);
    // Then we compute the additional force acting on the hull, represented by the coefficients a_H and x_H 
    ssc::kinematics::Vector6d ret_add = ssc::kinematics::Vector6d::Zero();
    ret_add(2)=-m_aH*Fn*cos(rudder_angle);
    // This force is located at (x_H-x_R) forward from the rudder location.
    ret_add(5)=(m_xH-position_of_the_rudder_frame_in_the_body_frame(2))*ret_add(2);

    return ret+ret_add;
}

ssc::kinematics::Vector6d MMGRudderForceModel::RudderModel::get_wrench(
    const double rudder_angle, //!< Rudder angle (in radian): positive if rudder on port side
    const ssc::kinematics::Point Vs,           //!< Speed of the ship relative to the fluid (in m/s)
    const double area          //!< Rudder area (in or outside wake) in m^2
    ) const
{
    double rudder_inflow_angle=get_angle_of_attack(rudder_angle,get_fluid_angle(Vs));
    double Fn = get_Fn(m_rho,area,Vs.v.norm(),rudder_inflow_angle);
    return get_force(Fn, rudder_angle);
}

double MMGRudderForceModel::RudderModel::get_Fn(const double rho, const double area, const double speed, const double rudder_inflow_angle) const
{
    //Equation (18): Fujii's formula to have the rudder lift gradient coefficient
    double friction_coeff=6.13*m_effective_aspect_ratio/(m_effective_aspect_ratio+2.25);

    //Equation (19)
    double Fn=0.5*rho*area*speed*speed*friction_coeff*sin(rudder_inflow_angle);

    return Fn;
}

MMGRudderForceModel::MMGRudderForceModel(
    const Yaml& input_,
    const std::string& body_name_,
    const EnvironmentAndFrames& env) :
        ForceModel(input_.name,{"rpm","P/D","beta"},input_.position_of_propeller_frame, body_name_, env),
        propulsionModel(MMGPropellerForceModel(input_, body_name_, env)),
        rudderModel(input_, env.rho),
        w(input_.wake_coefficient),
        m_propeller_wrench_internal_frame_at_P(new Wrench(ssc::kinematics::Point(input_.position_of_propeller_frame.frame,0,0,0))),
        m_propeller_wrench_body_frame_at_Ob(new Wrench(ssc::kinematics::Point(body_name,0,0,0))),
        m_propeller_wrench_NED_frame_at_G(new Wrench(ssc::kinematics::Point("NED",0,0,0))),
        m_rudder_wrench_body_frame_at_Ob(new Wrench(ssc::kinematics::Point(body_name,0,0,0))),
        m_rudder_wrench_NED_frame_at_G(new Wrench(ssc::kinematics::Point("NED",0,0,0)))
{
}

ssc::kinematics::Vector6d MMGRudderForceModel::get_rudder_force(
    const BodyStates& states,
    const double /*t*/,
    const EnvironmentAndFrames& env,
    const std::map<std::string,double>& commands,
    const double T
    ) const
{
    //Equation (41)
    const double Va = states.u()*(1-w);
    const double DVa = rudderModel.get_D()*Va;
    // Thrust loading coefficient, Cf. "Maneuvering Technical Manual", J. Brix, Seehafen Verlag p. 84, eq. 1.2.20
    const double CTh = std::abs(DVa) < 1e-10 ? 8e20 / PI * T / env.rho : 8 / PI * T / (env.rho * DVa*DVa);

    const double rudder_angle = commands.at("beta");
    const double vR = rudderModel.get_vr(states.u(),states.v(),states.r());
    const InOutWake<ssc::kinematics::Point> Vrud = rudderModel.get_vs(CTh, Va, vR);// flow velocity at rudder location, in & outside the propeller wake
    //Equation (21)
    const InOutWake<double> area = rudderModel.get_Ar();// rudder area, in & outside the propeller wake
    const InOutWake<ssc::kinematics::Vector6d> w = rudderModel.get_wrench(rudder_angle, Vrud, area);
    return w.in_wake + w.outside_wake;
}

MMGRudderForceModel::Yaml MMGRudderForceModel::parse(const std::string& yaml)
{
    std::stringstream stream(yaml);
    YAML::Parser parser(stream);
    YAML::Node node;
    parser.GetNextDocument(node);
    Yaml ret(MMGPropellerForceModel::parse(yaml));

    ssc::yaml_parser::parse_uv(node["rudder area"], ret.Ar);
    ssc::yaml_parser::parse_uv(node["rudder height"], ret.b);
    ssc::yaml_parser::parse_uv(node["xH"], ret.xH);
    ssc::yaml_parser::parse_uv(node["lR"], ret.lR);
    
    node["tR"] >> ret.tR;
    node["aH"] >> ret.aH;
    node["gammaR"] >> ret.gammaR;
    node["epsilon"] >> ret.epsilon;
    node["kappa"] >> ret.kappaMmg;
    node["effective aspect ratio"]    >> ret.effective_aspect_ratio;

    node["position of rudder in body frame"] >> ret.position_of_the_rudder_frame_in_the_body_frame;

    return ret;
}


