/*
 * environment_model_table.cpp
 */

#include "environment_model_table.hpp"

#include <array>
#include <memory>

#include "xdyn/core/ModelTable.hpp"
#include "xdyn/current_models/current_model_table.hpp"
#include "xdyn/exceptions/InvalidInputException.hpp"
#include "xdyn/grpc/SurfaceElevationFromGRPC.hpp"
#include "xdyn/input/YamlGRPC.hpp"
#include "xdyn/wave_models/wave_mesh.hpp"
#include "xdyn/wave_models/wave_model_table.hpp"
#include "xdyn/wind_models/wind_model_table.hpp"
#include "xdyn/yaml_parser/environment_parsers.hpp"

namespace {
SurfaceElevationPtr surface_elevation_from_grpc(const std::string& yaml)
{
    const YamlGRPC input = parse_grpc(yaml);
    return std::make_shared<SurfaceElevationFromGRPC>(input, make_wave_mesh(input.output));
}

// grpc/ builds on wave_models/, so wave_models/' table cannot name the gRPC free surface
const auto& grpc_surface_elevations()
{
    static const std::array table{
        ModelEntry<SurfaceElevationFactory>{"grpc", surface_elevation_from_grpc},
    };
    return table;
}

SurfaceElevationFactory find_waves(const std::string& name)
{
    if (const auto factory = find_surface_elevation(name)) {
        return factory;
    }
    return find_model(grpc_surface_elevations(), name);
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
    if (const auto factory = find_waves(yaml.model)) {
        fill(env.w, factory, yaml, "wave");
        return;
    }
    if (const auto factory = find_wind_model(yaml.model)) {
        fill(env.wind, factory, yaml, "wind");
        return;
    }
    if (const auto factory = find_current_model(yaml.model)) {
        fill(env.UWCurrent, factory, yaml, "underwater current");
        return;
    }
    throw_unknown_model(yaml, environment_model_names());
}

std::vector<std::string> environment_model_names()
{
    std::vector<std::string> names = surface_elevation_names();
    for (const auto& more :
         {model_names(grpc_surface_elevations()), wind_model_names(), current_model_names()}) {
        names.insert(names.end(), more.begin(), more.end());
    }
    return names;
}
