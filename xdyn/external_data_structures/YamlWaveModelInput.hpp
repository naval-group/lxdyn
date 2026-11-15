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

struct YamlDiscretization
{
    YamlDiscretization();
    size_t nfreq{0};           //!< Nb of frequencies in the discretization
    size_t ndir{0};            //!< Nb of directions in the discretization
    double omega_min{0};       //!< First angular frequency (in rad/s)
    double omega_max{0};       //!< Last angular frequency (in rad/s)
    double energy_fraction{0}; //!< Between 0 and 1: sum(S(omega[i]).S(psi[j]),taken into account)/sum(S(omega[i]).S(psi[j]),total)
    bool equal_energy_bins{false}; //!< False (by default) or true. When false, omegas are sampled regularly between omega_min and omega_max. When true, they are sampled such that the integral of the spectrum is constant between two consecutive omega values.
    bool periodic{false};          //!< False (by default) or true. When true, directions and frequencies are chosen so the wave field repeats over 'repetition sizes'.
    std::vector<double> repetition_sizes; //!< Lengths (in m) the wave field repeats over, largest first. Only used when 'periodic' is true.
};

struct YamlStretching
{
    YamlStretching();
    double delta{0}; //!< 0 for Wheeler stretching, 1 for linear extrapolation
    double h{0}; //!< Depth (in meters) over which the stretching is taken into account. Should usually be equal to "depth" (or 0 for no stretching)
};

struct YamlSpectrum
{
    YamlSpectrum();
    std::string model;                      //!< Wave model (eg. airy)
    std::string model_yaml;                 //!< Model data in YAML format
    std::string directional_spreading_type; //!< Spreading type (eg. cos2s)
    std::string directional_spreading_yaml; //!< Model data in YAML format
    std::string spectral_density_type;      //!< Type of spectrum (eg. jonswap)
    std::string spectral_density_yaml;      //!< Spectral model parameters in YAML format
    double depth{0};                           //!< Water depth (in meters): 0 for infinite depth
    YamlStretching stretching;              //!< Stretching model for orbital wave velocities (delta-stretching model)
};

struct YamlRays
{
    YamlRays();
    std::vector<double> a;
    std::vector<double> psi;
    std::vector<double> omega;
    std::vector<double> k;
    std::vector<double> phase;
};

struct YamlSpectrumFromRays
{
    YamlSpectrumFromRays();
    std::string model;                      //!< Wave model (eg. airy)
    std::string model_yaml;                 //!< Model data in YAML format
    YamlRays rays;                          //!< List of rays
    double depth{0.0};                           //!< Water depth (in meters): 0 for infinite depth
    YamlStretching stretching;              //!< Stretching model for orbital wave velocities (delta-stretching model)
};

struct YamlWaveModel
{
    YamlWaveModel();
    YamlDiscretization discretization; //!< Spectral discretization parameters
    std::vector<YamlSpectrum> spectra; //!< Wave spectra to generate
    std::vector<YamlSpectrumFromRays> spectra_from_rays;  //!< Wave spectra to generate
    YamlWaveOutput output;             //!< Defines what wave data is outputted during the simulation & how it is generated
};

struct YamlWaveFromRaysModel
{
    YamlWaveFromRaysModel();
    std::vector<YamlSpectrumFromRays> spectra;  //!< Wave spectra to generate
    YamlWaveOutput output;                      //!< Defines what wave data is outputted during the simulation & how it is generated
};

struct YamlJonswap
{
    YamlJonswap();
    double Hs{0};    //!< Significant wave height (in meters)
    double Tp{0};    //!< Mean wave period (in seconds)
    double gamma{0}; //!< Non-dimensional peak shape parameter
};

struct YamlBretschneider
{
    YamlBretschneider();
    double Hs{0};    //!< Significant wave height (in meters)
    double Tp{0};    //!< Mean wave period (in seconds)
};

struct YamlCos2s
{
    YamlCos2s();
    double psi0{0}; //!< Primary wave direction in radians.
    double s{0};    //!< Exponent
};

struct YamlDiracDirection
{
    YamlDiracDirection();
    double psi0{0}; //!< Primary wave direction in radians.
};

struct YamlDiracSpectrum
{
    YamlDiracSpectrum();
    double omega0{0}; //!< Angular frequency (\f$2\pi f\f$) in rad/s of the significant wave height
    double Hs{0};     //!< Significant wave height (in meters)
};

struct YamlPiersonMoskowitz
{
    YamlPiersonMoskowitz();
    double Hs{0};     //!< Significant wave height (in meters)
    double Tp{0};    //!< Mean wave period (in seconds)
};

struct YamlDefaultWaveModel
{
    YamlDefaultWaveModel();
    double zwave{0};          //!< Constant free surface elevation (in meters)
    YamlWaveOutput output; //!< Defines what wave data is outputted during the simulation & how it is generated
};

#endif /* YAMLWAVEMODELINPUT_HPP_ */
