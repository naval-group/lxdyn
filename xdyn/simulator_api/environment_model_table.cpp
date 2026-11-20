/*
 * environment_model_table.cpp
 */

#include "environment_model_table.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>
#include <memory>
#include <optional>

#include "xdyn/core/ModelTable.hpp"
#include "xdyn/current_models/ConstantUWCurrentModel.hpp"
#include "xdyn/current_models/DefaultUWCurrentModel.hpp"
#include "xdyn/current_models/EkmanUWCurrentModel.hpp"
#include "xdyn/exceptions/InvalidInputException.hpp"
#include "xdyn/grpc/SurfaceElevationFromGRPC.hpp"
#include "xdyn/input/YamlGRPC.hpp"
#include "xdyn/wave_models/Airy.hpp"
#include "xdyn/wave_models/BretschneiderSpectrum.hpp"
#include "xdyn/wave_models/Cos2sDirectionalSpreading.hpp"
#include "xdyn/wave_models/DefaultSurfaceElevation.hpp"
#include "xdyn/wave_models/DiracDirectionalSpreading.hpp"
#include "xdyn/wave_models/DiracSpectralDensity.hpp"
#include "xdyn/wave_models/JonswapSpectrum.hpp"
#include "xdyn/wave_models/PiersonMoskowitzSpectrum.hpp"
#include "xdyn/wave_models/Stretching.hpp"
#include "xdyn/wave_models/SurfaceElevationFromWaves.hpp"
#include "xdyn/wave_models/discretize.hpp"
#include "xdyn/wave_models/wave_mesh.hpp"
#include "xdyn/wind_models/DefaultWindModel.hpp"
#include "xdyn/wind_models/LogWindVelocityProfile.hpp"
#include "xdyn/wind_models/PowerLawWindVelocityProfile.hpp"
#include "xdyn/wind_models/UniformWindVelocityProfile.hpp"
#include "xdyn/yaml_parser/environment_parsers.hpp"

namespace {
using WaveSpectralDensityPtr = std::shared_ptr<WaveSpectralDensity>;
using WaveDirectionalSpreadingPtr = std::shared_ptr<WaveDirectionalSpreading>;

using SpectrumFactory = WaveSpectralDensityPtr (*)(const std::string& yaml);
using SpreadingFactory = WaveDirectionalSpreadingPtr (*)(const std::string& yaml);
using WaveModelFactory = WaveModelPtr (*)(const FlatDiscreteDirectionalWaveSpectrum& rays);
using SurfaceElevationFactory = SurfaceElevationPtr (*)(const std::string& yaml);

WaveSpectralDensityPtr bretschneider(const std::string& yaml)
{
    const YamlBretschneider data = parse_bretschneider(yaml);
    return std::make_shared<BretschneiderSpectrum>(data.Hs, data.Tp);
}

WaveSpectralDensityPtr jonswap(const std::string& yaml)
{
    const YamlJonswap data = parse_jonswap(yaml);
    return std::make_shared<JonswapSpectrum>(data.Hs, data.Tp, data.gamma);
}

WaveSpectralDensityPtr pierson_moskowitz(const std::string& yaml)
{
    const YamlPiersonMoskowitz data = parse_pierson_moskowitz(yaml);
    return std::make_shared<PiersonMoskowitzSpectrum>(data.Hs, data.Tp);
}

WaveSpectralDensityPtr dirac_spectrum(const std::string& yaml)
{
    const YamlDiracSpectrum data = parse_wave_dirac_spectrum(yaml);
    return std::make_shared<DiracSpectralDensity>(data.omega0, data.Hs);
}

WaveDirectionalSpreadingPtr dirac_spreading(const std::string& yaml)
{
    const YamlDiracDirection data = parse_wave_dirac_direction(yaml);
    return std::make_shared<DiracDirectionalSpreading>(data.psi0);
}

WaveDirectionalSpreadingPtr cos2s(const std::string& yaml)
{
    const YamlCos2s data = parse_cos2s(yaml);
    return std::make_shared<Cos2sDirectionalSpreading>(data.psi0, data.s);
}

// "dirac" names both a spectrum and a spreading, so they need tables of their own
const auto& spectra()
{
    static const std::array table{
        ModelEntry<SpectrumFactory>{"bretschneider", bretschneider},
        ModelEntry<SpectrumFactory>{"dirac", dirac_spectrum},
        ModelEntry<SpectrumFactory>{"jonswap", jonswap},
        ModelEntry<SpectrumFactory>{"pierson-moskowitz", pierson_moskowitz},
    };
    return table;
}

const auto& spreadings()
{
    static const std::array table{
        ModelEntry<SpreadingFactory>{"cos2s", cos2s},
        ModelEntry<SpreadingFactory>{"dirac", dirac_spreading},
    };
    return table;
}

WaveModelPtr airy(const FlatDiscreteDirectionalWaveSpectrum& rays)
{
    return std::make_shared<Airy>(rays);
}

// A wave model turns rays into kinematics; the spectrum, discretization and phases come before it
const auto& wave_models()
{
    static const std::array table{
        ModelEntry<WaveModelFactory>{"airy", airy},
    };
    return table;
}

template <typename Factory, std::size_t N>
Factory find_wave_part(
    const std::array<ModelEntry<Factory>, N>& table,
    const std::string& part,
    const std::string& name)
{
    const Factory factory = find_model(table, name);
    if (factory == nullptr) {
        throw_unknown_name(part, name, model_names(table));
    }
    return factory;
}

template <typename Factory, std::size_t N>
auto build_wave_part(
    const std::array<ModelEntry<Factory>, N>& table,
    const std::string& part,
    const std::string& name,
    const std::string& yaml)
{
    const Factory factory = find_wave_part(table, part, name);
    try {
        return factory(yaml);
    } catch (const InvalidInputException& exception) {
        THROW(
            __PRETTY_FUNCTION__,
            InvalidInputException,
            "Problem when parsing " << part << " '" << name << "': " << exception.get_message());
    }
}

DiscreteDirectionalWaveSpectrum discrete_spectrum(
    const YamlDiscretization& discretization,
    const YamlSpectrum& spectrum)
{
    const WaveSpectralDensityPtr spectral_density = build_wave_part(
        spectra(),
        "spectral density",
        spectrum.spectral_density_type,
        spectrum.spectral_density_yaml);
    const WaveDirectionalSpreadingPtr directional_spreading = build_wave_part(
        spreadings(),
        "directional spreading",
        spectrum.directional_spreading_type,
        spectrum.directional_spreading_yaml);
    if (spectrum.depth > 0) {
        return discretize(
            *spectral_density,
            *directional_spreading,
            discretization.omega_min,
            discretization.omega_max,
            discretization.nfreq,
            discretization.ndir,
            Stretching(spectrum.stretching),
            discretization.equal_energy_bins,
            discretization.periodic,
            discretization.repetition_sizes);
    }
    return discretize(
        *spectral_density,
        *directional_spreading,
        discretization.omega_min,
        discretization.omega_max,
        discretization.nfreq,
        discretization.ndir,
        spectrum.depth,
        Stretching(spectrum.stretching),
        discretization.equal_energy_bins,
        discretization.periodic,
        discretization.repetition_sizes);
}

FlatDiscreteDirectionalWaveSpectrum rays(
    const YamlDiscretization& discretization,
    const YamlSpectrum& spectrum)
{
    const DiscreteDirectionalWaveSpectrum discrete = discrete_spectrum(discretization, spectrum);
    const std::optional<int> seed = parse_seed_of_random_number_generator(spectrum.model_yaml);
    if (seed) {
        return flatten(add_random_phases(discrete, *seed));
    }
    return flatten(add_constant_phases(discrete, 0.0));
}

FlatDiscreteDirectionalWaveSpectrum rays(const YamlSpectrumFromRays& spectrum)
{
    FlatDiscreteDirectionalWaveSpectrum f;
    std::copy(spectrum.rays.a.begin(), spectrum.rays.a.end(), std::back_inserter(f.a));
    std::copy(spectrum.rays.psi.begin(), spectrum.rays.psi.end(), std::back_inserter(f.psi));
    std::copy(spectrum.rays.omega.begin(), spectrum.rays.omega.end(), std::back_inserter(f.omega));
    std::copy(spectrum.rays.k.begin(), spectrum.rays.k.end(), std::back_inserter(f.k));
    std::copy(spectrum.rays.phase.begin(), spectrum.rays.phase.end(), std::back_inserter(f.phase));
    for (const auto& psi : spectrum.rays.psi) {
        f.cos_psi.push_back(cos(psi));
        f.sin_psi.push_back(sin(psi));
    }
    const Stretching stretching(spectrum.stretching);
    if (spectrum.depth == 0.0) {
        // Infinite depth
        f.pdyn_factor = [stretching](const double k, const double z, const double eta) {
            return dynamic_pressure_factor(k, z, eta, stretching);
        };
        f.pdyn_factor_sh = [stretching](const double k, const double z, const double eta) {
            return dynamic_pressure_factor(k, z, eta, stretching);
        };
    } else {
        // Finite depth
        const double h = spectrum.depth;
        f.pdyn_factor = [h, stretching](const double k, const double z, const double eta) {
            return dynamic_pressure_factor(k, z, h, eta, stretching);
        };
        f.pdyn_factor_sh = [h, stretching](const double k, const double z, const double eta) {
            return dynamic_pressure_factor_sh(k, z, h, eta, stretching);
        };
    }
    return f;
}

SurfaceElevationPtr default_surface_elevation(const std::string& yaml)
{
    if (yaml.empty()) {
        THROW(
            __PRETTY_FUNCTION__,
            InvalidInputException,
            "No yaml data detected for default wave model (expected 'constant wave height in NED frame: {value: xx, unit: yy})'");
    }
    const YamlDefaultWaveModel input = parse_default_wave_model(yaml);
    return std::make_shared<DefaultSurfaceElevation>(input.zwave, make_wave_mesh(input.output));
}

SurfaceElevationPtr surface_elevation_from_waves(const std::string& yaml)
{
    const YamlWaveModel input = parse_waves(yaml);
    std::vector<WaveModelPtr> models;
    models.reserve(input.spectra.size() + input.spectra_from_rays.size());
    for (const auto& spectrum : input.spectra) {
        const WaveModelFactory wave_model =
            find_wave_part(wave_models(), "wave model", spectrum.model);
        models.push_back(wave_model(rays(input.discretization, spectrum)));
    }
    for (const auto& spectrum : input.spectra_from_rays) {
        const WaveModelFactory wave_model =
            find_wave_part(wave_models(), "wave model", spectrum.model);
        models.push_back(wave_model(rays(spectrum)));
    }
    return std::make_shared<SurfaceElevationFromWaves>(
        models,
        wave_mesh_size(input.output),
        make_wave_mesh(input.output));
}

SurfaceElevationPtr surface_elevation_from_grpc(const std::string& yaml)
{
    const YamlGRPC input = parse_grpc(yaml);
    return std::make_shared<SurfaceElevationFromGRPC>(input, make_wave_mesh(input.output));
}

template <typename Ptr, typename T>
ModelEntry<Ptr (*)(const std::string&)> parsed()
{
    return {T::model_name(), [](const std::string& yaml) -> Ptr {
                return std::make_shared<T>(T::parse(yaml));
            }};
}

const auto& surface_elevations()
{
    static const std::array table{
        ModelEntry<SurfaceElevationFactory>{"grpc", surface_elevation_from_grpc},
        ModelEntry<SurfaceElevationFactory>{"no waves", default_surface_elevation},
        ModelEntry<SurfaceElevationFactory>{"waves", surface_elevation_from_waves},
    };
    return table;
}

const auto& wind_models()
{
    static const std::array table{
        parsed<WindModelPtr, DefaultWindModel>(),
        parsed<WindModelPtr, LogWindVelocityProfile>(),
        parsed<WindModelPtr, PowerLawWindVelocityProfile>(),
        parsed<WindModelPtr, UniformWindVelocityProfile>(),
    };
    return table;
}

const auto& current_models()
{
    static const std::array table{
        parsed<UWCurrentModelPtr, ConstantUWCurrentModel>(),
        parsed<UWCurrentModelPtr, DefaultUWCurrentModel>(),
        parsed<UWCurrentModelPtr, EkmanUWCurrentModel>(),
    };
    return table;
}

template <typename Ptr>
void fill(
    Ptr& slot,
    Ptr (*factory)(const std::string&),
    const YamlModel& yaml,
    const std::string& category)
{
    if (slot) {
        THROW(
            __PRETTY_FUNCTION__,
            InvalidInputException,
            "More than one " << category << " model was defined: '" << yaml.model
                             << "', defined line " << yaml.index_of_first_line_in_global_yaml
                             << " of the YAML file, is the second one.");
    }
    try {
        slot = factory(yaml.yaml);
    } catch (...) {
        rethrow_with_yaml_context(yaml);
    }
}
}  // namespace

void add_environment_model(const YamlModel& yaml, EnvironmentAndFrames& env)
{
    if (const auto factory = find_model(surface_elevations(), yaml.model)) {
        fill(env.w, factory, yaml, "wave");
        return;
    }
    if (const auto factory = find_model(wind_models(), yaml.model)) {
        fill(env.wind, factory, yaml, "wind");
        return;
    }
    if (const auto factory = find_model(current_models(), yaml.model)) {
        fill(env.UWCurrent, factory, yaml, "underwater current");
        return;
    }
    throw_unknown_model(yaml, environment_model_names());
}

std::vector<std::string> environment_model_names()
{
    std::vector<std::string> names = model_names(surface_elevations());
    for (const auto& name : model_names(wind_models())) {
        names.push_back(name);
    }
    for (const auto& name : model_names(current_models())) {
        names.push_back(name);
    }
    return names;
}
