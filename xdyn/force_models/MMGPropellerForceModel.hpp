/*
 * MMGPropellerForceModel.hpp
 *
 *  Created on: Oct 2, 2024
 *      Author: fbickert
 */

#ifndef MMGPROPELLERFORCEMODEL_HPP_
#define MMGPROPELLERFORCEMODEL_HPP_

#include "AbstractWageningen.hpp"

/** \details This class was created to define the propeller model used in the MMG model
 * 
 *  It is based on H. Yasukawa and Y. Yoshimura, “Introduction of MMG standard method for ship maneuvering predictions,” Journal of Marine Science and Technology, vol. 20, no. 1, pp. 37–52, Nov. 2014, doi: 10.1007/s00773-014-0293-y.
 * 
 *  \addtogroup ForceModels
 *  \ingroup module
 *  \section ex1 Example
 *  \snippet module/unit_tests/MMGPropellerForceModelTest.cpp
 */
class MMGPropellerForceModel : public AbstractWageningen
{
    public:
        struct Yaml : public AbstractWageningen::Yaml
        {
            Yaml();
            Yaml(const AbstractWageningen::Yaml& y);
            double k0;
            double k1;
            double k2;
        };
        MMGPropellerForceModel(const Yaml& input, const std::string& body_name, const EnvironmentAndFrames& env);
        static Yaml parse(const std::string& yaml);

        virtual double get_Kt(const std::map<std::string,double>& commands, const double J) const;//!< inherited function to compute K_T given J
        virtual double get_Kq(const std::map<std::string,double>& commands, const double J) const;//!< inherited function to compute K_Q which is always null here
        virtual double get_wake_factor(const BodyStates& states) const;//!< inherited function to compute the wake factor given the leeway angle
        static std::string model_name();

    private:
        double m_k0;//!< constant term in the second-order polynomial MMG propeller model
        double m_k1;//!< linear term in the second-order polynomial MMG propeller model
        double m_k2;//!< quadratic term in the second-order polynomial MMG propeller model
};
#endif /* MMGPROPELLERFORCEMODEL_HPP */
