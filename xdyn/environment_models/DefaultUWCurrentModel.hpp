/*
 * DefaultUWCurrentModel.hpp
 */

#ifndef ENVIRONMENT_MODELS_INC_DEFAULTUWCURRENTMODEL_HPP_
#define ENVIRONMENT_MODELS_INC_DEFAULTUWCURRENTMODEL_HPP_

#include <string>

#include "xdyn/environment_models/UWCurrentModel.hpp"

class DefaultUWCurrentModel : public UWCurrentModel
{
public:
    explicit DefaultUWCurrentModel(int); // Constructor argument is a dummy in order to be able to call DefaultUWCurrentModel(DefaultUWCurrentModel::parse(...)) from parser
    ~DefaultUWCurrentModel() override;

    Eigen::Vector3d get_UWCurrent(const Eigen::Vector3d& position, const double t, const double wave_height) const override;

    static std::string model_name();
    static int parse(const std::string&);
};

#endif /* ENVIRONMENT_MODELS_INC_DEFAULTUWCURRENTMODEL_HPP_ */
