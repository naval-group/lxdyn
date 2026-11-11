/*
 * builders.hpp
 *
 *  Created on: Aug 12, 2014
 *      Author: cady
 */

#ifndef BUILDERS_HPP_
#define BUILDERS_HPP_

#include "xdyn/core/DefaultSurfaceElevation.hpp"
#include "xdyn/core/SurfaceElevationBuilder.hpp"
#include "xdyn/core/SurfaceElevationFromWaves.hpp"
#include "xdyn/environment_models/Airy.hpp"
#include "xdyn/environment_models/BretschneiderSpectrum.hpp"
#include "xdyn/environment_models/Cos2sDirectionalSpreading.hpp"
#include "xdyn/environment_models/DiracDirectionalSpreading.hpp"
#include "xdyn/environment_models/DiracSpectralDensity.hpp"
#include "xdyn/environment_models/DiscreteDirectionalWaveSpectrum.hpp"
#include "xdyn/environment_models/JonswapSpectrum.hpp"
#include "xdyn/environment_models/PiersonMoskowitzSpectrum.hpp"

using SurfaceElevationInterfacePtr = std::shared_ptr<SurfaceElevationInterface>;
using WaveSpectralDensityPtr = std::shared_ptr<WaveSpectralDensity>;
using WaveDirectionalSpreadingPtr = std::shared_ptr<WaveDirectionalSpreading>;

// The constructors below must accept exactly what SimulatorBuilder::can_parse<T>() passes
// (same signature as the primary templates) and be defined inline: these specializations
// are what can_parse actually constructs now that they are visible at the call site.
template <>
class SurfaceElevationBuilder<DefaultSurfaceElevation> : public SurfaceElevationBuilderInterface
{
    public:
        SurfaceElevationBuilder(const std::shared_ptr<std::vector<WaveModelBuilderPtr> >& wave_parsers_,
                                const std::shared_ptr<std::vector<DirectionalSpreadingBuilderPtr> >& directional_spreading_parsers_,
                                const std::shared_ptr<std::vector<SpectrumBuilderPtr> >& spectrum_parsers_) :
            SurfaceElevationBuilderInterface(wave_parsers_, directional_spreading_parsers_, spectrum_parsers_)
        {}
        boost::optional<SurfaceElevationInterfacePtr> try_to_parse(const std::string& model, const std::string& yaml) const override;
};

struct YamlDiscretization;
struct YamlSpectrum;
struct YamlSpectrumFromRays;

template <>
class SurfaceElevationBuilder<SurfaceElevationFromWaves> : public SurfaceElevationBuilderInterface
{
    public:
        SurfaceElevationBuilder(const std::shared_ptr<std::vector<WaveModelBuilderPtr> >& wave_parsers_,
                                const std::shared_ptr<std::vector<DirectionalSpreadingBuilderPtr> >& directional_spreading_parsers_,
                                const std::shared_ptr<std::vector<SpectrumBuilderPtr> >& spectrum_parsers_) :
            SurfaceElevationBuilderInterface(wave_parsers_, directional_spreading_parsers_, spectrum_parsers_)
        {}
        boost::optional<SurfaceElevationInterfacePtr> try_to_parse(const std::string& model, const std::string& yaml) const override;

    private:
        SurfaceElevationBuilder();
        WaveModelPtr parse_wave_model(const YamlDiscretization& discretization, const YamlSpectrum& spectrum) const;
        WaveModelPtr parse_wave_model(const YamlSpectrumFromRays& spectrum) const;
        DiscreteDirectionalWaveSpectrum parse_directional_spectrum(const YamlDiscretization& discretization, const YamlSpectrum& spectrum) const;
        FlatDiscreteDirectionalWaveSpectrum parse_flat_spectrum(const YamlSpectrumFromRays& spectrum) const;
        WaveSpectralDensityPtr parse_spectral_density(const YamlSpectrum& spectrum) const;
        WaveDirectionalSpreadingPtr parse_directional_spreading(const YamlSpectrum& spectrum) const;
};

class SurfaceElevationFromGRPC;
template <>
class SurfaceElevationBuilder<SurfaceElevationFromGRPC> : public SurfaceElevationBuilderInterface
{
    public:
        SurfaceElevationBuilder(const std::shared_ptr<std::vector<WaveModelBuilderPtr> >& wave_parsers_,
                                const std::shared_ptr<std::vector<DirectionalSpreadingBuilderPtr> >& directional_spreading_parsers_,
                                const std::shared_ptr<std::vector<SpectrumBuilderPtr> >& spectrum_parsers_) :
            SurfaceElevationBuilderInterface(wave_parsers_, directional_spreading_parsers_, spectrum_parsers_)
        {}
        boost::optional<SurfaceElevationInterfacePtr> try_to_parse(const std::string& model, const std::string& yaml) const override;
};

template <>
class WaveModelBuilder<Airy> : public WaveModelBuilderInterface
{
    public:
        WaveModelBuilder(const std::shared_ptr<std::vector<DirectionalSpreadingBuilderPtr> >& directional_spreading_parsers_,
                         const std::shared_ptr<std::vector<SpectrumBuilderPtr> >& spectrum_parsers_) :
            WaveModelBuilderInterface(directional_spreading_parsers_, spectrum_parsers_)
        {}
        boost::optional<WaveModelPtr> try_to_parse(const std::string& model, const DiscreteDirectionalWaveSpectrum& spectrum, const std::string& yaml) const override;
        boost::optional<WaveModelPtr> try_to_parse(const std::string& model, const FlatDiscreteDirectionalWaveSpectrum& spectrum, const std::string& yaml) const override;
};

template <>
class SpectrumBuilder<BretschneiderSpectrum> : public SpectrumBuilderInterface
{
    public:
        SpectrumBuilder() : SpectrumBuilderInterface() {}
        boost::optional<WaveSpectralDensityPtr> try_to_parse(const std::string& model, const std::string& yaml) const override;
};

template <>
class SpectrumBuilder<JonswapSpectrum> : public SpectrumBuilderInterface
{
    public:
        SpectrumBuilder() : SpectrumBuilderInterface() {}
        boost::optional<WaveSpectralDensityPtr> try_to_parse(const std::string& model, const std::string& yaml) const override;
};

template <>
class SpectrumBuilder<PiersonMoskowitzSpectrum> : public SpectrumBuilderInterface
{
    public:
        SpectrumBuilder() : SpectrumBuilderInterface() {}
        boost::optional<WaveSpectralDensityPtr> try_to_parse(const std::string& model, const std::string& yaml) const override;
};

template <>
class SpectrumBuilder<DiracSpectralDensity> : public SpectrumBuilderInterface
{
    public:
        SpectrumBuilder() : SpectrumBuilderInterface() {}
        boost::optional<WaveSpectralDensityPtr> try_to_parse(const std::string& model, const std::string& yaml) const override;
};

template <>
class DirectionalSpreadingBuilder<DiracDirectionalSpreading> : public DirectionalSpreadingBuilderInterface
{
    public:
        DirectionalSpreadingBuilder() : DirectionalSpreadingBuilderInterface(){}
        boost::optional<WaveDirectionalSpreadingPtr> try_to_parse(const std::string& model, const std::string& yaml) const override;
};

template <>
class DirectionalSpreadingBuilder<Cos2sDirectionalSpreading> : public DirectionalSpreadingBuilderInterface
{
    public:
        DirectionalSpreadingBuilder() : DirectionalSpreadingBuilderInterface(){}
        boost::optional<WaveDirectionalSpreadingPtr> try_to_parse(const std::string& model, const std::string& yaml) const override;
};

#endif /* BUILDERS_HPP_ */
