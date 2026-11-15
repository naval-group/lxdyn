/*
 * YamlWaveModelInput.cpp
 *
 *  Created on: Aug 7, 2014
 *      Author: cady
 */

#include "YamlWaveModelInput.hpp"

YamlDiscretization::YamlDiscretization() :  repetition_sizes()
{}

YamlSpectrum::YamlSpectrum():
    model(std::string()),
    model_yaml(std::string()),
    directional_spreading_type(std::string()),
    directional_spreading_yaml(std::string()),
    spectral_density_type(std::string()),
    spectral_density_yaml(std::string()),
    stretching()
{}

YamlStretching::YamlStretching()
{}

YamlWaveModel::YamlWaveModel():
    discretization(),
    spectra(),
    spectra_from_rays(),
    output()
{}

YamlRays::YamlRays():
    a(),
    psi(),
    omega(),
    k(),
    phase()
{}

YamlSpectrumFromRays::YamlSpectrumFromRays():
    model(),
    model_yaml(),
    rays(),
    stretching()
{}

YamlJonswap::YamlJonswap()
{}

YamlBretschneider::YamlBretschneider()
{}

YamlCos2s::YamlCos2s()
{}

YamlDiracDirection::YamlDiracDirection()
{}

YamlDiracSpectrum::YamlDiracSpectrum()
{}

YamlPiersonMoskowitz::YamlPiersonMoskowitz()
{}

YamlDefaultWaveModel::YamlDefaultWaveModel():
              //!< Constant free surface elevation (in meters)
    output(YamlWaveOutput())
{}
