/*
 * InputData.cpp
 *
 *  Created on: Jun 24, 2014
 *      Author: cady
 */

#include "XdynForMECommandLineArguments.hpp"

XdynForMECommandLineArguments::XdynForMECommandLineArguments() : yaml_filenames(),
                         assets_path(),
                         address("127.0.0.1")
{
}

bool XdynForMECommandLineArguments::empty() const
{
    return yaml_filenames.empty() and (port == 0);
}


