/*
 * model_tables.cpp
 */

#include "model_tables.hpp"

#include <array>
#include <memory>

#include "xdyn/controllers/controller_table.hpp"
#include "xdyn/core/ModelTable.hpp"
#include "xdyn/current_models/current_model_table.hpp"
#include "xdyn/exceptions/InvalidInputException.hpp"
#include "xdyn/force_models/force_model_table.hpp"
#include "xdyn/grpc/GRPCForceModel.hpp"
#include "xdyn/grpc/GrpcController.hpp"
#include "xdyn/grpc/SurfaceElevationFromGRPC.hpp"
#include "xdyn/input/YamlGRPC.hpp"
#include "xdyn/wave_models/wave_mesh.hpp"
#include "xdyn/wave_models/wave_model_table.hpp"
#include "xdyn/wind_models/wind_model_table.hpp"
#include "xdyn/yaml_parser/environment_parsers.hpp"

namespace {
ForcePtr grpc_force_model(
    const std::string& yaml,
    const std::string& body_name,
    const EnvironmentAndFrames& env)
{
    return std::make_shared<GRPCForceModel>(GRPCForceModel::parse(yaml), body_name, env);
}

SurfaceElevationPtr surface_elevation_from_grpc(const std::string& yaml)
{
    const YamlGRPC input = parse_grpc(yaml);
    return std::make_shared<SurfaceElevationFromGRPC>(input, make_wave_mesh(input.output));
}

std::shared_ptr<Controller>
grpc_controller(const double tstart, const YamlController& yaml, Sim& sys)
{
    return std::shared_ptr<Controller>(
        GrpcController::build(tstart, yaml.name, yaml.rest_of_the_yaml, sys));
}

// grpc/ builds on the model folders, so their tables cannot name the gRPC models
const auto& grpc_force_models()
{
    static const std::array table{
        ModelEntry<ForceModelFactory>{GRPCForceModel::model_name(), grpc_force_model},
    };
    return table;
}

const auto& grpc_surface_elevations()
{
    static const std::array table{
        ModelEntry<SurfaceElevationFactory>{"grpc", surface_elevation_from_grpc},
    };
    return table;
}

const auto& grpc_controllers()
{
    static const std::array table{
        ModelEntry<ControllerFactory>{"GRPC", grpc_controller},
    };
    return table;
}

ForceModelFactory find_force(const std::string& name)
{
    if (const auto factory = find_force_model(name)) {
        return factory;
    }
    return find_model(grpc_force_models(), name);
}

std::vector<std::string> all_force_model_names()
{
    std::vector<std::string> names = force_model_names();
    const auto grpc = model_names(grpc_force_models());
    names.insert(names.end(), grpc.begin(), grpc.end());
    return names;
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

ForcePtr make_force_model(
    const YamlModel& yaml,
    const std::string& body_name,
    const EnvironmentAndFrames& env)
{
    const ForceModelFactory factory = find_force(yaml.model);
    if (factory == nullptr) {
        throw_unknown_model(yaml, all_force_model_names());
    }
    try {
        return factory(yaml.yaml, body_name, env);
    } catch (...) {
        rethrow_with_yaml_context(yaml);
    }
}

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

std::shared_ptr<Controller>
make_controller(const double tstart, const YamlController& yaml, Sim& sys)
{
    auto factory = find_controller(yaml.type);
    if (factory == nullptr) {
        factory = find_model(grpc_controllers(), yaml.type);
    }
    if (factory == nullptr) {
        std::vector<std::string> names = controller_names();
        const auto grpc = model_names(grpc_controllers());
        names.insert(names.end(), grpc.begin(), grpc.end());
        throw_unknown_name("controller type", yaml.type, names);
    }
    return factory(tstart, yaml, sys);
}
