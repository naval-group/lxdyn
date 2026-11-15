/*
 * DefaultUWCurrentModel.cpp
 */

#include "DefaultUWCurrentModel.hpp"

DefaultUWCurrentModel::DefaultUWCurrentModel(int)
{
}

std::string DefaultUWCurrentModel::model_name() {return "no UW current";}

int DefaultUWCurrentModel::parse(const std::string&)
{
    return 0;
}

Eigen::Vector3d DefaultUWCurrentModel::get_UWCurrent(const Eigen::Vector3d&, const double, const double) const
{
    return Eigen::Vector3d::Zero();
}
