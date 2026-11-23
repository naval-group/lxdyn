/*
 * controller_table.hpp
 */

#ifndef CONTROLLER_TABLE_HPP_
#define CONTROLLER_TABLE_HPP_

#include <memory>
#include <string>
#include <vector>

#include "xdyn/controllers/Controller.hpp"
#include "xdyn/input/YamlController.hpp"

using ControllerFactory =
    std::shared_ptr<Controller> (*)(const double tstart, const YamlController& yaml, Sim& sys);

/** \brief The factory of the controller of type `type`, nullptr when none has that type */
ControllerFactory find_controller(const std::string& type);

std::vector<std::string> controller_names();

#endif /* CONTROLLER_TABLE_HPP_ */
