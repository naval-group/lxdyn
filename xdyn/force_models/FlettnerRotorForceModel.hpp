#ifndef FORCE_MODELS_INC_FLETTNERROTORFORCEMODEL_HPP_
#define FORCE_MODELS_INC_FLETTNERROTORFORCEMODEL_HPP_

#include <Eigen/Dense>
#include <map>
#include <memory>  // std::unique_ptr
#include <ssc/interpolation.hpp>
#include <ssc/kinematics.hpp>
#include <string>
#include <utility>  // std::pair
#include <vector>

#include "xdyn/core/BodyStates.hpp"
#include "xdyn/core/ForceModel.hpp"

class FlettnerRotorForceModel : public ForceModel {
public:
    struct Input {
        std::string name;
        YamlCoordinates calculation_point_in_body_frame;
        double diameter{0.0};
        double length{0.0};
        std::vector<double> spin_ratio;
        std::vector<double> lift_coefficient;
        std::vector<double> drag_coefficient;
    };

    FlettnerRotorForceModel(
        const Input& input,
        const std::string& body_name,
        const EnvironmentAndFrames& env);
    FlettnerRotorForceModel() = delete;

    static std::string model_name();
    static Input parse(const std::string& yaml);
    Wrench get_force(
        const BodyStates& states,
        const double t,
        const EnvironmentAndFrames& env,
        const std::map<std::string, double>& commands) const override;

protected:
    void extra_observations(Observer& observer) const override;

private:
    const Eigen::Vector3d calculation_point;
    /// Rotor radius (in m) for the computation of the spin ratio
    const double radius;
    /// Reference area (in square metres) of the wing, for lift and drag normalization
    const double reference_area;

    // The interpolators need to be behind pointers because interpolation is non-const
    std::unique_ptr<ssc::interpolation::SplineVariableStep> Cl;
    std::unique_ptr<ssc::interpolation::SplineVariableStep> Cd;

    std::pair<double, double> sr_bounds;  //!< Interpolation bounds

    // Needs to be a pointer to be saved during get_force, which is const
    const std::unique_ptr<double> spin_ratio;
};

#endif /* FORCE_MODELS_INC_FLETTNERROTORFORCEMODEL_HPP_ */
