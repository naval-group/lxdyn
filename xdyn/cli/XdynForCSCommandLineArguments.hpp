/*
 * XdynForCSCommandLineArguments.hpp
 *
 *  Created on: Nov 14, 2018
 *      Author: cady
 */

#ifndef XDYNFORCSCOMMANDLINEARGUMENTS_HPP_
#define XDYNFORCSCOMMANDLINEARGUMENTS_HPP_

#include <string>
#include <vector>

struct XdynForCSCommandLineArguments
{
    XdynForCSCommandLineArguments();
    bool empty() const;
    std::vector<std::string> yaml_filenames;
    std::string solver;
    std::string assets_path;
    double initial_timestep{};
    bool catch_exceptions{};
    std::string address;
    short unsigned int port{0};
    bool verbose{false};
    bool show_help{false};
    bool show_websocket_debug_information{false};
    bool grpc{false};
};

#endif /* XDYNFORCSCOMMANDLINEARGUMENTS_HPP_ */
