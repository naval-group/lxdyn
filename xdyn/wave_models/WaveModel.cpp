/*
 * WaveModel.cpp
 *
 *  Created on: Aug 1, 2014
 *      Author: cady
 */

#include "WaveModel.hpp"

#include <cmath>  // For isnan
#include <cmath>

#include "discretize.hpp"
#include "xdyn/exceptions/InternalErrorException.hpp"

WaveModel::WaveModel(const DiscreteDirectionalWaveSpectrum& spectrum)
    : flat_spectrum(flatten(spectrum))
{
    check_sizes();
}

WaveModel::WaveModel(const DiscreteDirectionalWaveSpectrum& spectrum, const double constant_phase)
    : flat_spectrum(flatten(add_constant_phases(spectrum, constant_phase)))
{
    check_sizes();
}

WaveModel::WaveModel(
    const DiscreteDirectionalWaveSpectrum& spectrum,
    const int random_number_generator_seed)
    : flat_spectrum(flatten(add_random_phases(spectrum, random_number_generator_seed)))
{
    check_sizes();
}

WaveModel::WaveModel(const FlatDiscreteDirectionalWaveSpectrum& spectrum) : flat_spectrum(spectrum)
{
    check_sizes();
}

void WaveModel::check_sizes() const
{
    if (flat_spectrum.omega.empty()) {
        THROW(
            __PRETTY_FUNCTION__,
            InternalErrorException,
            "No 'omega' values defined in FlatDiscreteDirectionalWaveSpectrum");
    }
    if (flat_spectrum.a.empty()) {
        THROW(
            __PRETTY_FUNCTION__,
            InternalErrorException,
            "No 'a' values defined in FlatDiscreteDirectionalWaveSpectrum");
    }
    if (flat_spectrum.k.empty()) {
        THROW(
            __PRETTY_FUNCTION__,
            InternalErrorException,
            "No 'k' values defined in FlatDiscreteDirectionalWaveSpectrum");
    }
    if (flat_spectrum.phase.empty()) {
        THROW(
            __PRETTY_FUNCTION__,
            InternalErrorException,
            "No 'phase' values defined in FlatDiscreteDirectionalWaveSpectrum");
    }

    for (const auto omega : flat_spectrum.omega) {
        if (std::isnan(omega)) {
            THROW(
                __PRETTY_FUNCTION__,
                InternalErrorException,
                "FlatDiscreteDirectionalWaveSpectrum contains NaN values for omega: "
                    << flat_spectrum.omega);
        }
    }

    for (const auto k : flat_spectrum.k) {
        if (std::isnan(k)) {
            THROW(
                __PRETTY_FUNCTION__,
                InternalErrorException,
                "FlatDiscreteDirectionalWaveSpectrum contains NaN values for k: "
                    << flat_spectrum.k);
        }
    }
    for (const auto a : flat_spectrum.a) {
        if (std::isnan(a)) {
            THROW(
                __PRETTY_FUNCTION__,
                InternalErrorException,
                "FlatDiscreteDirectionalWaveSpectrum contains NaN values for a: "
                    << flat_spectrum.a);
        }
    }
}

std::vector<double> WaveModel::get_elevation(
    const std::vector<double>& x,  //!< x-positions in the NED frame (in meters)
    const std::vector<double>& y,  //!< y-positions in the NED frame (in meters)
    const double t                 //!< Current time instant (in seconds)
) const
{
    if (x.size() != y.size()) {
        THROW(
            __PRETTY_FUNCTION__,
            InternalErrorException,
            "Error when calculating surface elevation: the x and y vectors don't have the same size (size of x: "
                << x.size() << ", size of y: " << y.size() << ")");
    }
    return elevation(x, y, t);
}

ssc::kinematics::PointMatrix WaveModel::get_orbital_velocity(
    const double g,                 //!< gravity (in m/s^2)
    const std::vector<double>& x,   //!< x-positions in the NED frame (in meters)
    const std::vector<double>& y,   //!< y-positions in the NED frame (in meters)
    const std::vector<double>& z,   //!< z-positions in the NED frame (in meters)
    const double t,                 //!< Current time instant (in seconds)
    const std::vector<double>& eta  //!< Wave heights at x,y,t (in meters)
) const
{
    if (x.size() != y.size() || x.size() != z.size() || x.size() != eta.size()) {
        THROW(
            __PRETTY_FUNCTION__,
            InternalErrorException,
            "Error when calculating orbital velocity: the x, y, z and eta vectors don't have the same size (size of x: "
                << x.size() << ", size of y: " << y.size() << ", size of z: " << z.size()
                << ", size of eta: " << eta.size() << ")");
    }
    return orbital_velocity(g, x, y, z, t, eta);
}

std::vector<double> WaveModel::get_dynamic_pressure(
    const double rho,                //!< water density (in kg/m^3)
    const double g,                  //!< gravity (in m/s^2)
    const std::vector<double>& x,    //!< x-positions in the NED frame (in meters)
    const std::vector<double>& y,    //!< y-positions in the NED frame (in meters)
    const std::vector<double>& z,    //!< z-positions in the NED frame (in meters)
    const std::vector<double>& eta,  //!< Wave elevations at (x,y) in the NED frame (in meters)
    const double t                   //!< Current time instant (in seconds)
) const
{
    if (x.size() != y.size() || x.size() != z.size() || x.size() != eta.size()) {
        THROW(
            __PRETTY_FUNCTION__,
            InternalErrorException,
            "Error when calculating dynamic pressure: the x, y, z and eta vectors don't have the same size (size of x: "
                << x.size() << ", size of y: " << y.size() << ", size of z: " << z.size()
                << ", size of eta: " << eta.size() << ")");
    }
    return dynamic_pressure(rho, g, x, y, z, eta, t);
}
