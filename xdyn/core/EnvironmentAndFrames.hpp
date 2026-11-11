/*
 * EnvironmentAndFrames.hpp
 *
 *  Created on: Jun 16, 2014
 *      Author: cady
 */

#ifndef ENVIRONMENTANDFRAMES_HPP_
#define ENVIRONMENTANDFRAMES_HPP_

#include "xdyn/core/Body.hpp"
#include "xdyn/core/StateLayout.hpp"
#include "xdyn/core/SurfaceElevationInterface.hpp"
#include "xdyn/environment_models/WindModel.hpp"
#include "xdyn/environment_models/UWCurrentModel.hpp"
#include "xdyn/external_data_structures/YamlRotation.hpp"
#include <optional>
#include <ssc/kinematics.hpp>

class Observer;

struct EnvironmentAndFrames
{
    EnvironmentAndFrames ();
    void feed (Observer &observer, double t,
               const std::vector<BodyPtr> &bodies,
               const StateType &state) const;
    SurfaceElevationPtr w;
    WindModelPtr wind;
    UWCurrentModelPtr UWCurrent;
    ssc::kinematics::KinematicsPtr k;
    double rho;
    double nu;
    double g;
    YamlRotation rot;

    void set_rho_air (const double value);
    double get_rho_air () const;
    /**  \brief Underwater current at a point of the NED frame, zero if no current model was defined
      */
    Eigen::Vector3d get_UWCurrent (const Eigen::Vector3d& position, const double t) const;

    private:
        std::optional<double> rho_air;
};

#endif /* ENVIRONMENTANDFRAMES_HPP_ */
