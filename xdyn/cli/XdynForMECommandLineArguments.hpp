/*
 * XdynForMECommandLineArguments.hpp
 *
 *  Created on: Nov 13, 2018
 *      Author: cady
 */

#ifndef XDYNFORMECOMMANDLINEARGUMENTS_HPP_
#define XDYNFORMECOMMANDLINEARGUMENTS_HPP_

#include <string>
#include <vector>

struct XdynForMECommandLineArguments
{
    XdynForMECommandLineArguments();
    std::vector<std::string> yaml_filenames;
    std::string assets_path;
    bool catch_exceptions{false};
    std::string address;
    short unsigned int port{0};
    bool empty() const;
    bool verbose{false};
    bool show_help{false};
    bool show_websocket_debug_information{false};
    bool grpc{false};
};

#endif /* XDYNFORMECOMMANDLINEARGUMENTS_HPP_ */
