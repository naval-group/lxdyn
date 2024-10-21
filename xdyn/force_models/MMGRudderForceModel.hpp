/*
 * MMGRudderForceModel.hpp
 *
 *  Created on: Oct 7, 2024
 *      Author: fbickert
 */

#ifndef MMGRUDDERFORCEMODEL_HPP_
#define MMGRUDDERFORCEMODEL_HPP_

#include "MMGPropellerForceModel.hpp"

/** \details This class was created to define the rudder model used in the MMG model
 * 
 *  It is based on H. Yasukawa and Y. Yoshimura, “Introduction of MMG standard method for ship maneuvering predictions,” Journal of Marine Science and Technology, vol. 20, no. 1, pp. 37–52, Nov. 2014, doi: 10.1007/s00773-014-0293-y.
 * 
 *  \addtogroup ForceModels
 *  \ingroup module
 *  \section 
 *  \snippet module/unit_tests/MMGRudderForceModelTest.cpp
 */
class MMGRudderForceModel : public ForceModel
{
  public:
    struct Yaml : MMGPropellerForceModel::Yaml
    {
        Yaml();
        Yaml(const MMGPropellerForceModel::Yaml& yaml);
        double Ar; //!< Rudder projected area without considering the horn part(in m^2)
        double b;  //!< Rudder span (in m)
        double xH; //!< Longitudinal coordinate of acting point of the additional lateral force
        double lR; //!< Effective longitudinal coordinate of rudder position in formula of $\beta_R$
        double tR; //!< Steering resistance deduction factor
        double aH; //!< Rudder force increase factor
        std::vector<double> gammaR; //!< Flow straightening coefficient (first value for negative
                                    //!< $\beta_R$,second value for positive $\beta_R$)
        double epsilon;             //!< Ratio of wake fraction at propeller and rudder positions
        double kappaMmg; //!< An experimental constant for expressing the fluid velocity at the
                         //!< rudder location
        double effective_aspect_ratio; //!< Rudder aspect ratio (considering the horn part)
        YamlCoordinates position_of_the_rudder_frame_in_the_body_frame; //!< Position of the rudder
                                                                        //!< in the body frame
    };

    MMGRudderForceModel(const Yaml& input, const std::string& body_name,
                        const EnvironmentAndFrames& env);

    Wrench get_force(const BodyStates& states, const double t, const EnvironmentAndFrames& env,
                     const std::map<std::string, double>& commands) const override;
    ssc::kinematics::Vector6d get_rudder_force(const BodyStates& states, const double t,
                                               const EnvironmentAndFrames& env,
                                               const std::map<std::string, double>& commands,
                                               const double T) const;
    static Yaml parse(const std::string& yaml);
    static std::string model_name();

    struct RudderModel
    {
        RudderModel(const Yaml& parameters, const double rho);
        virtual ~RudderModel() {}

        /**  \brief Returns the rudder area
         *  \returns Rudder area (in m^2)
         */
        double get_Ar() const;

        /**  \brief Calculates the angle between the fluid flow & the rudder
         *  \details When positive, the flow is coming towards the rudder's port side
         *  \returns Angle of attack (in radian)
         *  \snippet force_models/unit_tests/RudderForceModelTest.cpp RudderForceModelTest
         * get_alpha_example
         */
        double get_angle_of_attack(
            const double
                rudder_angle,        //!< Rudder angle (in radian): positive if rudder on port side
            const double fluid_angle //!< Angle of the fluid at the rudder location in the ship's
                                     //!< reference frame (0 if the fluid is propagating along -X,
                                     //!< positive if fluid is coming from starboard)
        ) const;

        /**  \brief Calculates the norm of the gradient force acting on the rudder
         *  \returns force (in Newton)
         */
        double get_Fn(
            const double area,  //!< Rudder area (in m^2)
            const double speed, //!< Norm of the fluid velocity at the rudder location (in m/s)
            const double rudder_inflow_angle)
            const; //!< Effective angle between the rudder and the inflow (in rad)

        /**  \brief Wrench (tensor) created by the rudder on the ship \details Expressed in body frame at the rudder location
         */
        ssc::kinematics::Vector6d get_wrench(
            const double
                rudder_angle, //!< Rudder angle (in radian): positive if rudder on port side
            const ssc::kinematics::Point Vs, //!< Mean inflow velocity (body frame) relative to the ship
                                             //!< at the rudder location in m/s
            const double area                //!< Rudder area in m^2
        ) const;

        /**  \brief Tota wrench created by the rudder on the ship \details Expressed in body frame at the rudder
         * location
         */
        ssc::kinematics::Vector6d get_force(const double Fn, //!< Norm of the rudder force (in N)
                                            const double rudder_angle //!< Rudder angle (in rad)
        ) const;

        /**  \brief Calculates the mean inflow velocity relative to the ship at the rudder location \returns Mean inlow velocity relative to the ship at the rudder
         * location in m/s
         */
        ssc::kinematics::Point get_vs(
            const double CTh, //!< Thrust loading coefficient, Cf. "Manoeuvring Technical Manual",
                              //!< J. Brix, Seehafen Verlag p. 84, eq. 1.2.20
            const double Va,  //!< Longitudinal inflow velocity (body frame) at the propeller
                              //!< location (in m/s)
            const double
                vR //!< Lateral inflow velocity (body frame) at the rudder location (in m/s)
        ) const;

        double get_vr(const double u, const double v, const double r) const;

        /**  \brief Calculates the angle of incidence of the fluid at the rudder location
         *  \returns Angle in radian
         */
        double get_fluid_angle(
            const ssc::kinematics::Point Vs //!< Inflow velocity relative to the ship in body frame
                                            //!< (m/s)
        ) const;

        /**  \brief Returns propeller diameter
         *  \returns propeller diameter (in m)
         */
        double get_D() const;

        /**  \brief Returns rudder location in body frame
         *  \returns Rudder location in body frame
         */
        Eigen::Vector3d get_rudder_location() const;

      private:
        RudderModel(); // Disabled
        double m_Ar;   //!< Rudder projected area without considering the horn part (in m^2)
        double m_b;    //!< Rudder span (in m)
        double m_D;    //!< Propeller diameter (in m)
        double m_xH;   //!< Longitudinal coordinate of acting point of the additional lateral force
                       //!< (in m)
        double m_lR;   //!< Effective longitudinal coordinate of rudder position in formula of
                       //!< $\beta_R$ (in m)
        double m_tR;   //!< Steering resistance deduction factor
        double m_aH;   //!< Rudder force increase factor
        std::vector<double> m_gammaR; //!< Flow straightening coefficient (first value for negative
                                      //!< $\beta_R$,second value for positive $\beta_R$)
        double m_epsilon;             //!< Ratio of wake fraction at propeller and rudder positions
        double m_kappaMmg; //!< An experimental constant for expressing the fluid velocity at the
                           //!< rudder location
        double m_effective_aspect_ratio; //!< Rudder aspect ratio (considering the horn part)
        double m_rho;
        Eigen::Vector3d position_of_the_rudder_frame_in_the_body_frame; //!< Position of the rudder
                                                                        //!< in the body frame
    };

  protected:
    void extra_observations(Observer& observer) const override;

  private:
    MMGPropellerForceModel m_propulsionModel; //!< Propeller model
    RudderModel m_rudderModel;                //!< Rudder model
    // These variables are computed at each time step and stored for outputting
    std::unique_ptr<Wrench>
        m_propeller_wrench_internal_frame_at_P; //!< Propeller tensor in internal frame
    std::unique_ptr<Wrench> m_propeller_wrench_body_frame_at_Ob; //!< Propeller tensor in body frame
    std::unique_ptr<Wrench> m_propeller_wrench_NED_frame_at_G;   //!< Propeller tensor in NED frame
    std::unique_ptr<Wrench> m_rudder_wrench_body_frame_at_Ob; //!< Rudder tensor in internal frame
    std::unique_ptr<Wrench> m_rudder_wrench_NED_frame_at_G;   //!< Rudder tensor in NED frame
};

#endif /* MMGRUDDERFORCEMODEL_HPP_ */
