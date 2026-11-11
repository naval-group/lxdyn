/*
 * WindMeanVelocityProfile.hpp
 *
 *  Created on: 7 janv. 2020
 *      Author: mcharlou2016
 */

#ifndef ENVIRONMENT_MODELS_INC_WINDMEANVELOCITYPROFILE_HPP_
#define ENVIRONMENT_MODELS_INC_WINDMEANVELOCITYPROFILE_HPP_

#include <Eigen/Dense>
#include <optional>
#include <memory>
#include <string>
#include "xdyn/external_data_structures/YamlModel.hpp"
#include "xdyn/environment_models/WindModel.hpp"

class WindMeanVelocityProfile;
using WindMeanVelocityProfilePtr = std::shared_ptr<WindMeanVelocityProfile>;
using WindMeanVelocityProfileParser = std::function<std::optional<WindMeanVelocityProfilePtr>(const YamlModel& yaml)>;

class WindMeanVelocityProfile : public WindModel
{
public:
    struct Input
    {
        Input();
        double velocity;
        double direction;
    };

    WindMeanVelocityProfile(const Input& input);
    ~WindMeanVelocityProfile() override;

    Eigen::Vector3d get_wind(const Eigen::Vector3d& position, const double t) const override;
    virtual double get_wind_velocity(const double z) const =0;
    static Input parse(const std::string& yaml_input);

protected:
    double velocity;

private:
    Eigen::Vector3d direction;
};

#endif /* ENVIRONMENT_MODELS_INC_WINDMEANVELOCITYPROFILE_HPP_ */
