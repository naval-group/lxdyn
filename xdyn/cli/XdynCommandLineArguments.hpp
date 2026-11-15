/*
 * XdynCommandLineArguments.hpp
 *
 *  Created on: Jun 24, 2014
 *      Author: cady
 */

#ifndef XDYNCOMMANDLINEARGUMENTS_HPP_
#define XDYNCOMMANDLINEARGUMENTS_HPP_

#include <string>
#include <vector>

struct XdynCommandLineArguments
{
    std::vector<std::string> yaml_filenames;
    std::string solver;
    std::string output_filename;
    std::string wave_output;
    std::string assets_path;
    double initial_timestep{0};
    double tstart{0};
    double tend{0};
    bool catch_exceptions{false};
    bool empty() const;
};

#endif /* XDYNCOMMANDLINEARGUMENTS_HPP_ */
