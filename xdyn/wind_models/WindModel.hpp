/*
 * WindModel.hpp
 *
 *  Created on: 7 janv. 2020
 *      Author: mcharlou2016
 */

#ifndef ENVIRONMENT_MODELS_INC_WINDMODEL_HPP_
#define ENVIRONMENT_MODELS_INC_WINDMODEL_HPP_

#include <Eigen/Dense>
#include <memory>

class WindModel;
using WindModelPtr = std::shared_ptr<WindModel>;

class WindModel {
public:
    virtual ~WindModel() = default;
    virtual Eigen::Vector3d get_wind(const Eigen::Vector3d& position, const double t) const = 0;
};

#endif /* ENVIRONMENT_MODELS_INC_WINDMODEL_HPP_ */
