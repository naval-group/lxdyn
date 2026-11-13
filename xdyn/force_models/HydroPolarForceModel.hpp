#ifndef FORCE_MODELS_INC_HYDROPOLARFORCEMODEL_HPP_
#define FORCE_MODELS_INC_HYDROPOLARFORCEMODEL_HPP_

#include <Eigen/Dense>
#include <memory>
#include <optional>
#include <ssc/interpolation.hpp>

#include "xdyn/core/ForceModel.hpp"

class HydroPolarForceModel : public ForceModel {
public:
    struct Input {
        std::string name;
        YamlPosition internal_frame;
        std::vector<double> angle_of_attack;
        std::vector<double> lift_coefficient;
        std::vector<double> drag_coefficient;
        std::optional<std::vector<double>> moment_coefficient;
        double reference_area{0.0};
        std::optional<double> chord_length;
        bool use_waves_velocity{false};
        std::optional<std::string> angle_command;
    };

    HydroPolarForceModel(
        const Input& input,
        const std::string& body_name,
        const EnvironmentAndFrames& env);
    HydroPolarForceModel() = delete;

    static std::string model_name();
    static Input parse(const std::string& yaml_input);
    Wrench get_force(
        const BodyStates& states,
        const double t,
        const EnvironmentAndFrames& env,
        const std::map<std::string, double>& commands) const override;

protected:
    void extra_observations(Observer& observer) const override;

private:
    // The interpolators need to be behind pointers because interpolation is non-const
    /// Lift coefficient as a function of the apparent flow angle beta
    std::unique_ptr<ssc::interpolation::SplineVariableStep> Cl;
    /// Drag coefficient as a function of the apparent flow angle beta
    std::unique_ptr<ssc::interpolation::SplineVariableStep> Cd;
    /// Moment coefficient as a function of the apparent flow angle beta (optional)
    std::unique_ptr<ssc::interpolation::SplineVariableStep> Cm;
    /// Reference area (in square metres) of the wing, for lift and drag normalization
    const double reference_area;
    /// Chord length (in m), used (optionally) for moment normalization
    std::optional<double> chord_length;
    /// If true, then lift and drag coefficients from -180° to 0° angle of attack are the same as
    /// the coefficients from 0° to 180° (they are symmetric with respect to the foil's x0 axis in
    /// the (x0,y0) plane). Otherwise, the coefficients are assumed to have been given for angle of
    /// attack from -180° to 180°.
    bool symmetry;
    /// If true, the waves orbital velocity is added to the flow velocity acting on the foil
    bool use_waves_velocity;
    /// Optional command for the angle (in rad)
    std::optional<std::string> angle_command;

    // These variables are computed at each time step and stored for outputting
    /// Relative flow velocity at the origin of internal frame (m/s) = U
    const std::unique_ptr<double> relative_velocity;
    /// Angle of attack (rad) = alpha
    const std::unique_ptr<double> angle_of_attack;
};

#endif /* FORCE_MODELS_INC_HYDROPOLARFORCEMODEL_HPP_ */
