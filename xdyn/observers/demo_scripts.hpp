/*
 * demo_scripts.hpp
 *
 * Writes the post-processing scripts shipped in postprocessing/ into an HDF5
 * result file, under /scripts/MatLab and /scripts/Python.
 *
 * Replaces the generated demoMatLab.hpp / demoPython.hpp, which CMake wrote at
 * configure time from CMakeListsGenerateDemo{MatLab,Python}.txt. Both lanes now
 * compile demo_scripts.cpp, which embeds the scripts directly, so there is
 * nothing left to generate and no build step to run before this header is valid.
 */

#ifndef OBSERVERS_AND_API_DEMO_SCRIPTS_HPP_
#define OBSERVERS_AND_API_DEMO_SCRIPTS_HPP_

#include <string>

#include "H5Cpp.h"

void exportMatLabScripts(
    const H5::H5File& h5File,
    const std::string& fileName,
    const std::string& baseName,
    const std::string& destination);

void exportPythonScripts(
    const H5::H5File& h5File,
    const std::string& fileName,
    const std::string& baseName,
    const std::string& destination);

#endif /* OBSERVERS_AND_API_DEMO_SCRIPTS_HPP_ */
