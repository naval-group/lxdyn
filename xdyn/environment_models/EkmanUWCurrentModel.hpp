/*
 * EkmanUWCurrentModel.hpp
 *
 * Three-layer current: a wind-driven Ekman spiral at the top, a uniform background current in the
 * middle, and a seabed-influenced layer at the bottom.
 */

#ifndef ENVIRONMENT_MODELS_INC_EKMANUWCURRENTMODEL_HPP_
#define ENVIRONMENT_MODELS_INC_EKMANUWCURRENTMODEL_HPP_

#include <string>

#include "xdyn/environment_models/UWCurrentModel.hpp"
#include "xdyn/environment_models/Seabed.hpp"

class EkmanUWCurrentModel : public UWCurrentModel
{
public:
    struct Input
    {
        Input();
        Input(Seabed);
        Seabed seabed;
        double top_ekman_depth;     ///< Top layer thickness (m)
        double bottom_ekman_depth;  ///< Bottom layer thickness (m)
        double middle_velocity;     ///< Background current velocity (m/s)
        double middle_orientation;  ///< Background current direction (rad)
        double f_and_sqrt_rho;      ///< Coriolis parameter times sqrt(water density)
        double wind_angle;          ///< Wind direction (rad)
        double wind_stress;         ///< Wind stress (N/m2)
    };
    EkmanUWCurrentModel(const Input& input);
    ~EkmanUWCurrentModel() override;

    Eigen::Vector3d get_UWCurrent(const Eigen::Vector3d& position, const double time, const double wave_height) const override;

    static std::string model_name();
    static Input parse(const std::string&);

private:
    Seabed seabed;
    double top_ekman_depth;
    double bottom_ekman_depth;
    double middle_velocity;
    double middle_orientation;
    double f_and_sqrt_rho;
    double wind_angle;
    double wind_stress;

    Eigen::Vector3d get_top_layer_current(const Eigen::Vector3d&, const Eigen::Vector3d&, const double) const;
    Eigen::Vector3d get_middle_layer_current() const;
    Eigen::Vector3d get_bottom_layer_current(const Eigen::Vector3d&, const Eigen::Vector3d&, const double) const;
};

#endif /* ENVIRONMENT_MODELS_INC_EKMANUWCURRENTMODEL_HPP_ */
