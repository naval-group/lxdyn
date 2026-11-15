/*
 * InputData.cpp
 *
 *  Created on: Jun 24, 2014
 *      Author: cady
 */

#include "XdynCommandLineArguments.hpp"

bool XdynCommandLineArguments::empty() const
{
    return wave_output.empty() and output_filename.empty()
            and yaml_filenames.empty() and (initial_timestep == 0) and (tstart == 0) and (tend == 0);
}
