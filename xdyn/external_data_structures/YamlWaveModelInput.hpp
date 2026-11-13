/*
 * YamlWaveModelInput.hpp
 *
 *  Created on: Aug 7, 2014
 *      Author: cady
 */

#ifndef YAMLWAVEMODELINPUT_HPP_
#define YAMLWAVEMODELINPUT_HPP_

#include <string>
#include <vector>

#include "xdyn/external_data_structures/YamlWaveOutput.hpp"

struct YamlDiscretization {
    /// Nb of frequencies in the discretization
    size_t nfreq{0};
    /// Nb of directions in the discretization
    size_t ndir{0};
    /// First angular frequency (in rad/s)
    double omega_min{0};
    /// Last angular frequency (in rad/s)
    double omega_max{0};
    /// Between 0 and 1: sum(S(omega[i]).S(psi[j]),taken into
    /// account)/sum(S(omega[i]).S(psi[j]),total)
    double energy_fraction{0};
    /// False (by default) or true. When false, omegas are sampled regularly between omega_min and
    /// omega_max. When true, they are sampled such that the integral of the spectrum is constant
    /// between two consecutive omega values.
    bool equal_energy_bins{false};
    /// False (by default) or true. When true, directions and frequencies are chosen so the wave
    /// field repeats over 'repetition sizes'.
    bool periodic{false};
    /// Lengths (in m) the wave field repeats over, largest first. Only used when 'periodic' is
    /// true.
    std::vector<double> repetition_sizes;
};

struct YamlStretching {
    /// 0 for Wheeler stretching, 1 for linear extrapolation
    double delta{0};
    /// Depth (in meters) over which the stretching is taken into account. Should usually be equal
    /// to "depth" (or 0 for no stretching)
    double h{0};
};

struct YamlSpectrum {
    /// Wave model (eg. airy)
    std::string model;
    /// Model data in YAML format
    std::string model_yaml;
    /// Spreading type (eg. cos2s)
    std::string directional_spreading_type;
    /// Model data in YAML format
    std::string directional_spreading_yaml;
    /// Type of spectrum (eg. jonswap)
    std::string spectral_density_type;
    /// Spectral model parameters in YAML format
    std::string spectral_density_yaml;
    /// Water depth (in meters): 0 for infinite depth
    double depth{0};
    /// Stretching model for orbital wave velocities (delta-stretching model)
    YamlStretching stretching;
};

struct YamlRays {
    std::vector<double> a;
    std::vector<double> psi;
    std::vector<double> omega;
    std::vector<double> k;
    std::vector<double> phase;
};

struct YamlSpectrumFromRays {
    /// Wave model (eg. airy)
    std::string model;
    /// Model data in YAML format
    std::string model_yaml;
    /// List of rays
    YamlRays rays;
    /// Water depth (in meters): 0 for infinite depth
    double depth{0.0};
    /// Stretching model for orbital wave velocities (delta-stretching model)
    YamlStretching stretching;
};

struct YamlWaveModel {
    /// Spectral discretization parameters
    YamlDiscretization discretization;
    /// Wave spectra to generate
    std::vector<YamlSpectrum> spectra;
    /// Wave spectra to generate
    std::vector<YamlSpectrumFromRays> spectra_from_rays;
    /// Defines what wave data is outputted during the simulation & how it is generated
    YamlWaveOutput output;
};

struct YamlWaveFromRaysModel {
    YamlWaveFromRaysModel();
    /// Wave spectra to generate
    std::vector<YamlSpectrumFromRays> spectra;
    /// Defines what wave data is outputted during the simulation & how it is generated
    YamlWaveOutput output;
};

struct YamlJonswap {
    double Hs{0};     //!< Significant wave height (in meters)
    double Tp{0};     //!< Mean wave period (in seconds)
    double gamma{0};  //!< Non-dimensional peak shape parameter
};

struct YamlBretschneider {
    double Hs{0};  //!< Significant wave height (in meters)
    double Tp{0};  //!< Mean wave period (in seconds)
};

struct YamlCos2s {
    double psi0{0};  //!< Primary wave direction in radians.
    double s{0};     //!< Exponent
};

struct YamlDiracDirection {
    double psi0{0};  //!< Primary wave direction in radians.
};

struct YamlDiracSpectrum {
    double omega0{0};  //!< Angular frequency (\f$2\pi f\f$) in rad/s of the significant wave height
    double Hs{0};      //!< Significant wave height (in meters)
};

struct YamlPiersonMoskowitz {
    double Hs{0};  //!< Significant wave height (in meters)
    double Tp{0};  //!< Mean wave period (in seconds)
};

struct YamlDefaultWaveModel {
    /// Constant free surface elevation (in meters)
    double zwave{0};
    /// Defines what wave data is outputted during the simulation & how it is generated
    YamlWaveOutput output;
};

#endif /* YAMLWAVEMODELINPUT_HPP_ */
