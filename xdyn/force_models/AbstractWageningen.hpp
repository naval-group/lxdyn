/*
 * AbstractWageningen.hpp
 *
 *  Created on: Jun 28, 2015
 *      Author: cady
 */

#ifndef ABSTRACTWAGENINGEN_HPP_
#define ABSTRACTWAGENINGEN_HPP_

#include "xdyn/core/ForceModel.hpp"

/** \details This class was created to define a standard propeller model in xdyn
 *  \addtogroup module
 *  \ingroup module
 *  \section ex1 Example
 *  \snippet module/unit_tests/AbstractWageningenTest.cpp AbstractWageningenTest example
 *  \section ex2 Expected output
 *  \snippet module/unit_tests/AbstractWageningenTest.cpp AbstractWageningenTest expected output
 */
class AbstractWageningen : public ForceModel
{
    public:
        struct Yaml
        {
            Yaml();
            virtual ~Yaml(){}
            std::string name;
            YamlPosition position_of_propeller_frame;
            double wake_coefficient;
            double relative_rotative_efficiency;
            double thrust_deduction_factor;
            bool rotating_clockwise;
            double diameter;
        };
        AbstractWageningen(const Yaml& input, const std::string& body_name, const EnvironmentAndFrames& env);
        virtual double get_Kt(const std::map<std::string,double>& commands, const double J) const = 0;
        virtual double get_Kq(const std::map<std::string,double>& commands, const double J) const = 0;
        virtual double get_wake_factor(const BodyStates& states)  const;
        Wrench get_pure_thrust_force(const BodyStates& states, const double t, const EnvironmentAndFrames& env, const std::map<std::string,double>& commands) const;//!< Compute the pure thrust (before applying the thrust deduction)
        Wrench get_force(const BodyStates& states, const double t, const EnvironmentAndFrames& env, const std::map<std::string,double>& commands) const;
        double get_advance_speed_in_body_frame(const BodyStates& states, const double t, const EnvironmentAndFrames& env) const;//!< Get the inflow advance speed at the propeller location (along the body frame x-axis)
        double get_advance_speed_in_propeller_frame(const BodyStates& states, const double t, const EnvironmentAndFrames& env) const;//!< Get the inflow advance speed at the propeller location projected on the propeller frame x-axis
        double get_advance_ratio(const std::map<std::string,double>& commands, const double Va) const;
        static Yaml parse(const std::string& yaml);

    protected:
        double w;

    private:
        AbstractWageningen();
        double eta_R;
        double t;
        double kappa;
        double D;
        double D4;
        double D5;
};
#endif /* ABSTRACTWAGENINGEN_HPP_ */
