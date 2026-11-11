/*
 * ConstantUWCurrentModel.hpp
 */

#ifndef ENVIRONMENT_MODELS_INC_CONSTANTUWCURRENTMODEL_HPP_
#define ENVIRONMENT_MODELS_INC_CONSTANTUWCURRENTMODEL_HPP_

#include <string>

#include "xdyn/environment_models/UWCurrentModel.hpp"

class ConstantUWCurrentModel : public UWCurrentModel
{
public:
    struct Input
    {
        Input();
        double velocity;
        double orientation;
    };
    ConstantUWCurrentModel(const Input& input);
    ~ConstantUWCurrentModel() override;

    Eigen::Vector3d get_UWCurrent(const Eigen::Vector3d&, const double, const double) const override;

    static std::string model_name();
    static Input parse(const std::string&);

private:
    double velocity;
    double orientation;
};

#endif /* ENVIRONMENT_MODELS_INC_CONSTANTUWCURRENTMODEL_HPP_ */
