/*
 * UWCurrentModel.hpp
 *
 * Base class for underwater current models, following the WindModel pattern.
 */

#ifndef ENVIRONMENT_MODELS_INC_UWCURRENTMODEL_HPP_
#define ENVIRONMENT_MODELS_INC_UWCURRENTMODEL_HPP_

#include <Eigen/Dense>
#include <memory>

class UWCurrentModel;
using UWCurrentModelPtr = std::shared_ptr<UWCurrentModel>;

class UWCurrentModel {
public:
    virtual ~UWCurrentModel() = default;
    /**  \param wave_height z of the free surface above the queried point, in NED (so positive down)
     */
    virtual Eigen::Vector3d get_UWCurrent(
        const Eigen::Vector3d& position,
        const double t,
        const double wave_height) const = 0;
};

#endif /* ENVIRONMENT_MODELS_INC_UWCURRENTMODEL_HPP_ */
