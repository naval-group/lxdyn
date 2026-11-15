/*
 * XdynForCSCommandLineArguments.cpp
 *
 *  Created on: Nov 14, 2018
 *      Author: cady
 */


#include "XdynForCSCommandLineArguments.hpp"

XdynForCSCommandLineArguments::XdynForCSCommandLineArguments() :  address("127.0.0.1")
{
}

bool XdynForCSCommandLineArguments::empty() const
{
    return yaml_filenames.empty() and (initial_timestep == 0) and (port == 0);
}
