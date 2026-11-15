/*
 * PowerLawWindVelocityProfile.hpp
 *
 *  Created on: 10 déc. 2020
 *      Author: mcharlou2016
 */

#ifndef ENVIRONMENT_MODELS_INC_POWERLAWWINDVELOCITYPROFILE_HPP_
#define ENVIRONMENT_MODELS_INC_POWERLAWWINDVELOCITYPROFILE_HPP_

#include "WindMeanVelocityProfile.hpp"

class PowerLawWindVelocityProfile : public WindMeanVelocityProfile
{
public:
    struct Input : public WindMeanVelocityProfile::Input
    {
        double alpha{};
        double z_ref{};
    };

    explicit PowerLawWindVelocityProfile(const Input& input);

    double get_wind_velocity(const double z) const override;
    static std::string model_name();
    static Input parse(const std::string& yaml_input);

private:
    double alpha;
    double z_ref;
};

#endif /* ENVIRONMENT_MODELS_INC_POWERLAWWINDVELOCITYPROFILE_HPP_ */
