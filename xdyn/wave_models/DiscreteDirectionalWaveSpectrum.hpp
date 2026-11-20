/*
 * DiscreteDirectionalWaveSpectrum.hpp
 *
 *  Created on: Jul 31, 2014
 *      Author: cady
 */

#ifndef DISCRETEDIRECTIONALWAVESPECTRUM_HPP_
#define DISCRETEDIRECTIONALWAVESPECTRUM_HPP_

#include <functional>
#include <vector>

/** \author cady
 *  \date Jul 31, 2014, 1:08:15 PM
 *  \brief Used by 'discretize'
 *  \ingroup wave_models
 *  \section ex1 Example
 *  \snippet wave_models/unit_tests/DirectionalWaveSpectrumTest.cpp
 * DirectionalWaveSpectrumTest example
 *  \section ex2 Expected output
 *  \snippet wave_models/unit_tests/DirectionalWaveSpectrumTest.cpp
 * DirectionalWaveSpectrumTest expected output
 */
struct DiscreteDirectionalWaveSpectrum {
    /// Discretized spectral density (in s m^2/rad)
    std::vector<double> Si;
    /// Spatial spreading (in 1/rad)
    std::vector<double> Dj;
    /// Angular frequencies the spectrum was discretized at (in rad/s)
    std::vector<double> omega;
    /// Directions between 0 & 2pi the spatial spreading was discretized at (in rad)
    std::vector<double> psi;
    /// Discretized wave number (for each frequency) in rad/m
    std::vector<double> k;
    /// Random phases, for each (frequency, direction) couple (but time invariant) in radian phases
    /// *phase[i_freq][i_dir]*
    std::vector<std::vector<double>> phase;

    /// Factor used when computing the dynamic pressure (no unit)
    std::function<double(double, double, double)> pdyn_factor;
    /// Factor used when computing the orbital velocity (no unit)
    std::function<double(double, double, double)> pdyn_factor_sh;
    void check_sizes() const;
};

/** \brief Used by the wave models (eg. Airy, Stokes, etc.)
 *  \details Avoids having to double loop on frequency and direction. All vectors should have the
 * same size as each element corresponds to a (direction, omega) pair: that way we only need one
 * loop (over, say, i) to go over all possible (omega[i], psi[i]).
 *  \ingroup wave_models
 *  \section ex1 Example
 *  \snippet wave_models/unit_tests/DirectionalWaveSpectrumTest.cpp
 * DirectionalWaveSpectrumTest example
 *  \section ex2 Expected output
 *  \snippet wave_models/unit_tests/DirectionalWaveSpectrumTest.cpp
 * DirectionalWaveSpectrumTest expected output
 */
struct FlatDiscreteDirectionalWaveSpectrum {
    /// Amplitude (in m), for each angular frequency omega, and direction
    std::vector<double> a;
    /// Angular frequencies the spectrum was discretized at (in rad/s), for each angular frequency
    /// omega, and direction
    std::vector<double> omega;
    /// Directions between 0 & 2pi the spatial spreading was discretized at (in rad)
    std::vector<double> psi;
    /// Cosinus directions between 0 & 2pi the spatial spreading was discretized at (so we do not
    /// compute it each time), for each angular frequency omega, and direction
    std::vector<double> cos_psi;
    /// Sinus directions between 0 & 2pi the spatial spreading was discretized at (so we do not
    /// compute it each time), for each angular frequency omega, and direction
    std::vector<double> sin_psi;
    /// Discretized wave number (for each frequency) (in 1/m), for each angular frequency omega,
    /// i.e. same size as omega
    std::vector<double> k;
    /// Random phases, for each (frequency, direction) couple (but time invariant) in radian, for
    /// each angular frequency omega, and direction
    std::vector<double> phase;
    /// Factor used when computing the dynamic pressure (no unit)
    std::function<double(double, double, double)> pdyn_factor;
    /// Factor used when computing the orbital velocity (no unit)
    std::function<double(double, double, double)> pdyn_factor_sh;
    std::vector<double> get_periods()
        const;  //< Get the ray periods as a vector, from omega attribute (in s)
};

#endif /* DISCRETEDIRECTIONALWAVESPECTRUM_HPP_ */
