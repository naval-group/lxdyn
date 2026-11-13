/*
 * YamlRadiationDamping.hpp
 *
 *  Created on: Nov 27, 2014
 *      Author: cady
 */

#ifndef YAMLRADIATIONDAMPING_HPP_
#define YAMLRADIATIONDAMPING_HPP_

#include <string>

enum class TypeOfQuadrature
{
    RECTANGLE,
    TRAPEZOIDAL,
    SIMPSON,
    GAUSS_KRONROD,
    BURCHER,
    CLENSHAW_CURTIS,
    FILON
};

#include "xdyn/external_data_structures/YamlCoordinates.hpp"

struct YamlRadiationDamping {
    /// Path to a HDB file storing the radiation damping matrices
    std::string hdb_filename;
    /// Path to a PRECAL_R file storing the radiation damping matrices
    std::string precal_r_filename;
    /// What integration algorithm to use to compute retardation functions?
    TypeOfQuadrature type_of_quadrature_for_cos_transform{};
    /// What integration algorithm to use to compute the radiation forces (convolution)?
    TypeOfQuadrature type_of_quadrature_for_convolution{};
    /// How many points to use to compute the spline interpolating the retardation functions?
    size_t nb_of_points_for_retardation_function_discretization{0};
    /// Lower bound of the cosine integral used to calculate the retardation functions from the
    /// radiation dampings
    double omega_min{0};
    /// Upper bound of the cosine integral used to calculate the retardation functions from the
    /// radiation dampings
    double omega_max{0};
    /// Lower bound of the convolution integral, to calculate Fr
    double tau_min{0};
    /// Upper bound of the convolution integral, to calculate Fr
    double tau_max{0};
    /// Should the program output
    bool output_Br_and_K{};
    /// Where were the damping matrices (read from the HDB file) computed?
    YamlCoordinates calculation_point_in_body_frame;
    /// Should the constant (mean) velocity be subtracted from the total velocity before in this
    /// model
    bool remove_constant_speed{false};
    /// Should a forward speed correction be applied (if the HDB results are at zero forward speed)
    bool forward_speed_correction{false};
};

#endif /* YAMLRADIATIONDAMPING_HPP_ */
