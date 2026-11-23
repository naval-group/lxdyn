/*
 * controller_table.cpp
 */

#include "controller_table.hpp"

#include <array>

#include "xdyn/controllers/CSVController.hpp"
#include "xdyn/controllers/PIDController.hpp"
#include "xdyn/core/ModelTable.hpp"

namespace {
std::shared_ptr<Controller> pid(const double tstart, const YamlController& yaml, Sim&)
{
    return std::make_shared<PIDController>(tstart, yaml.dt, yaml.name, yaml.rest_of_the_yaml);
}

std::shared_ptr<Controller> csv(const double tstart, const YamlController& yaml, Sim&)
{
    return std::make_shared<CSVController>(tstart, yaml.name, yaml.rest_of_the_yaml);
}

const auto& table()
{
    static const std::array table{
        ModelEntry<ControllerFactory>{"PID", pid},
        ModelEntry<ControllerFactory>{"CSV", csv},
    };
    return table;
}
}  // namespace

ControllerFactory find_controller(const std::string& type)
{
    return find_model(table(), type);
}

std::vector<std::string> controller_names()
{
    return model_names(table());
}
