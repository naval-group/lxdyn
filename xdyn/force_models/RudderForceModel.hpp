/*
 * RudderForceModel.hpp
 *
 *  Created on: Jan 30, 2015
 *      Author: cady
 */

#ifndef RUDDERFORCEMODEL_HPP_
#define RUDDERFORCEMODEL_HPP_

#include "WageningenControlledForceModel.hpp"
#include <memory>

class RudderForceModel : public ForceModel
{
    public:
        struct Yaml : WageningenControlledForceModel::Yaml
        {
            Yaml();
            Yaml(const WageningenControlledForceModel::Yaml& yaml);
            double Ar;                                                      //!< Rudder area (in m^2) (cf. "Maneuvering Technical Manual", J. Brix, Seehafen Verlag, p. 76 fig. 1.2.4)
            double b;                                                       //!< Rudder height (in m) (cf. "Maneuvering Technical Manual", J. Brix, Seehafen Verlag, p. 76 fig. 1.2.4)
            double effective_aspect_ratio_factor;                           //!< Non-dimensional (cf. "Maneuvering Technical Manual", J. Brix, Seehafen Verlag, p. 97 § b)
            double lift_coeff;                                              //!< Non-dimensional: lift is multiplied by it (for tuning)
            double drag_coeff;                                              //!< Non-dimensional: drag is multiplied by it (for tuning)
            YamlCoordinates position_of_the_rudder_frame_in_the_body_frame; //!< Position of the rudder in the body frame
        };

        RudderForceModel(const Yaml& input, const std::string& body_name, const EnvironmentAndFrames& env);
        Wrench get_force(const BodyStates& states, const double t, const EnvironmentAndFrames& env, const std::map<std::string,double>& commands) const override;
        ssc::kinematics::Vector6d get_rudder_force(const BodyStates& states, const double t, const EnvironmentAndFrames& env, const std::map<std::string,double>& commands, const double T) const;
        static Yaml parse(const std::string& yaml);
        static std::string model_name();


        template <typename T> struct InOutWake
        {
            InOutWake() : in_wake(), outside_wake() {}
            T in_wake;
            T outside_wake;
        };

        struct RudderModel
        {
            RudderModel(const Yaml& parameters,
                        const double rho,
                        const double nu
                        );
            virtual ~RudderModel(){}

            /**  \brief Calculates the rudder area (in or outside wake)
              *  \returns Rudder area (in m^2)
              */
            InOutWake<double> get_Ar(const double CTh //!< Thrust loading coefficient, Cf. "Manoeuvring Technical Manual", J. Brix, Seehafen Verlag p. 84, eq. 1.2.20
                                    ) const;

            /**  \brief Calculates the angle between the fluid flow & the rudder
              *  \details When positive, the flow is coming towards the rudder's port side
              *  \returns Angle of attack (in radian)
              *  \snippet force_models/unit_tests/RudderForceModelTest.cpp RudderForceModelTest get_alpha_example
              */
            double get_angle_of_attack(const double rudder_angle, //!< Rudder angle (in radian): positive if rudder on port side
                                       const double fluid_angle   //!< Angle of the fluid at the rudder location in the ship's reference frame (0 if the fluid is propagating along -X, positive if fluid is coming from starboard)
                                      ) const;

            /**  \brief Calculates the norm of the lift force acting on the rudder
              *  \returns Lift force (in Newton)
              */
            double get_lift(const double Vs,    //!< Norm of the speed of the ship relative to the fluid
                            const double Cl,    //!< Rudder lift coefficient (non-dimensional)
                            const double alpha, //!< Angle between the propeller's wake & the rudder (in radian)
                            const double area   //!< Rudder area (in or outside wake) in m^2
                            ) const;

            /**  \brief Calculates the norm of the drag force acting on the rudder
              *  \returns Drag force (in Newton)
              *  \note While the lift is multiply by cos(aoa) (aka cos(alpha)) to take into
              *        account the reduced lift for high angle, the drag should not.
              */
            double get_drag(const double Vs,    //!< Norm of the speed of the ship relative to the fluid
                            const double Cl,    //!< Rudder lift coefficient (non-dimensional)
                            const double area   //!< Rudder area (in or outside wake) in m^2
                            ) const;

            /**  \brief Calculates the drag coefficient (non-dimensional)
              *   \ref // Soeding formula, "Maneuvering Technical Manual", J. Brix, Seehafen Verlag, p. 78 eq. 1.2.9
              */
            double get_Cd(const double Vs, //!< Norm of the speed of the ship relative to the fluid
                          const double Cl  //!< Rudder lift coefficient (non-dimensional)
                          ) const;

            /**  \brief Calculates the lift coefficient (non-dimensional)
             *   \ref // Soeding formula, "Maneuvering Technical Manual", J. Brix, Seehafen Verlag, p. 97 eq 1.2.48 & p. 77 eq. 1.2.8
              */
            double get_Cl(const double alpha_wake //!< Angle of rudder wrt the fluid (in radian)
                         ) const;

            /**  \brief Wrench created by the rudder on the ship, separated into the contributions inside and outside the propeller wake
             *   \details Expressed in body frame at the rudder location
              */
            InOutWake<ssc::kinematics::Vector6d> get_wrench(const double rudder_angle, //!< Rudder angle (in radian): positive if rudder on port side
                                                            const InOutWake<ssc::kinematics::Point>& Vs,           //!< Norm of the speed of the ship relative to the fluid (in m/s)
                                                            const InOutWake<double>& area          //!< Rudder area (in or outside wake) in m^2
                                                             ) const;

            /**  \brief Wrench created by the rudder on the ship
             *   \details Expressed in body frame at the rudder location
              */
            ssc::kinematics::Vector6d get_wrench(const double rudder_angle, //!< Rudder angle (in radian): positive if rudder on port side
                                                 ssc::kinematics::Point Vs,           //!< Speed of the ship relative to the fluid (in m/s)
                                                 const double area          //!< Rudder area (in or outside wake) in m^2
                                                             ) const;

            /**  \brief Tota wrench created by the rudder on the ship, considering the contributions inside and outside the propeller wake
             *   \details Expressed in body frame at the rudder location
              */
            ssc::kinematics::Vector6d get_force(const double lift, //!< Norm of the lift (in N)
                                                const double drag, //!< Norm of the drag (in N)
                                                const double angle //!< Angle between the fluid & the rudder (in radian)
                                                ) const;

            /**  \brief Calculates flow velocity at the rudder location booth inside & outside the propeller wake             
             *   \details Ship speed relative to the fluid, in m/s
             *  \returns Rudder area (in m^2)
              */
            InOutWake<ssc::kinematics::Point> get_vs(const double CTh, //!< Thrust loading coefficient, Cf. "Manoeuvring Technical Manual", J. Brix, Seehafen Verlag p. 84, eq. 1.2.20
                                                     const double Va,  //!< Projection of the ship speed (relative to the current) on the X-axis of the ship's reference frame (m/s)
                                                     const double v    //!< Projection of the ship speed (relative to the current) on the Y-axis of the ship's reference frame (m/s)
                                                     ) const;

            /**  \brief Calculates the angle of incidence of the fluid at the rudder location
              *  \returns Angle in radian
              */
            double get_fluid_angle(    const ssc::kinematics::Point Vs   //!< Ship speed relative to the fluid, inside & outside wake
    ) const;

            /**  \brief Returns propeller diameter
              *  \returns propeller diameter (in m)
              */
            double get_D() const;
            
            /**  \brief Returns rudder location in body frame
              *  \returns Rudder location in body frame
              */
            Eigen::Vector3d get_rudder_frame_location() const;
            
            private:
                RudderModel(); // Disabled
                double Ar;// Rudder lateral projected area (in m^2)
                double D; //!< Propeller diameter (in m)
                double Kr;//!< Contraction factor (cf. Marine Rudders & Control Surfaces, Molland & Turnock, eq. 3.37 p.51)
                double chord;// Rudder mean chord (in m)
                double lambda;// Rudder effective aspect ratio
                double lift_coeff;//Tuning coefficient for the rudder lift
                double drag_coeff;//Tuning coefficient for the rudder drag
                double rho;//Fluid density (in kg/m^3)
                double nu;//Kinematic fluid viscosity (in m^2/s)
                Eigen::Vector3d position_of_the_rudder_frame_in_the_body_frame;//!< Position of the rudder in the body frame
        };
    protected:
        void extra_observations(Observer& observer) const override;


    private:
        WageningenControlledForceModel propulsionModel;//!< Propeller model
        RudderModel rudderModel;//!< Rudder model
        double w; //!< Wake fraction
        
        // The following variables are computed at each time step and stored only for outputting
        std::unique_ptr<Wrench> m_propeller_wrench_internal_frame_at_P; //!< Propeller tensor in internal frame
        std::unique_ptr<Wrench> m_propeller_wrench_body_frame_at_Ob; //!< Propeller tensor in body frame
        std::unique_ptr<Wrench> m_propeller_wrench_NED_frame_at_G; //!< Propeller tensor in NED frame
        std::unique_ptr<Wrench> m_rudder_wrench_body_frame_at_Ob;//!< Rudder tensor in internal frame
        std::unique_ptr<Wrench> m_rudder_wrench_NED_frame_at_G;//!< Rudder tensor in NED frame
};


#endif /* RUDDERFORCEMODEL_HPP_ */
