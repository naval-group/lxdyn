/*
 * model_tables.hpp
 */

#ifndef MODEL_TABLES_HPP_
#define MODEL_TABLES_HPP_

#include <memory>
#include <string>
#include <vector>

#include "xdyn/core/EnvironmentAndFrames.hpp"
#include "xdyn/core/ForceModel.hpp"
#include "xdyn/input/YamlModel.hpp"

class Controller;
class Sim;
struct YamlController;

/** \brief Builds the force model `yaml` names, from force_models/' table or a gRPC one */
ForcePtr make_force_model(
    const YamlModel& yaml,
    const std::string& body_name,
    const EnvironmentAndFrames& env);

/** \brief Builds the model and fills its slot of `env`: the waves, the wind or the current */
void add_environment_model(const YamlModel& yaml, EnvironmentAndFrames& env);

std::vector<std::string> environment_model_names();

/** \brief Builds the controller of `yaml`'s type, from controllers/' table or the gRPC one */
std::shared_ptr<Controller>
make_controller(const double tstart, const YamlController& yaml, Sim& sys);

#endif /* MODEL_TABLES_HPP_ */
