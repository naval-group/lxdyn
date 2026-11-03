/*
 * demo_scripts.cpp
 *
 * The post-processing scripts, embedded as data.
 *
 * This file replaces tools/embed_demo.py (and, before it, build.zig's
 * DemoGenStep). That generator wrote 55 lines of
 * Python to emit one `<<"line"<<std::endl` per source line; everything the
 * emitted code actually did — the two substitutions and the H5 write — happened
 * at *runtime*, so none of it needed generating. `#embed` puts the bytes in
 * directly and the generator ceases to exist, which is also what removes the
 * build's last bare `python3`.
 *
 * `#embed` is C23; clang accepts it in C++ as an extension (zig 0.17 ships
 * clang 22), hence -Wno-c23-extensions on this one file in build.zig. The
 * embedded files are recorded in clang's depfile, so editing a script rebuilds
 * this TU — which the `has_side_effects`
 * shell step it replaced could not do.
 */

#include "demo_scripts.hpp"

#include <string>

#include "xdyn/interface_hdf5/h5_tools.hpp"

namespace
{
    // The file's bytes verbatim, nothing appended. Both scripts are newline-terminated,
    // so they already end in exactly one newline — which is also how the generator's
    // last `<<std::endl` left it. Appending another '\n' here would add a trailing blank
    // line that exists in neither the source file nor the text this replaces.
    //
    // suffix(,) rather than a leading comma: it emits the separator only when the
    // resource is non-empty, so an empty script stays a valid (empty) array instead
    // of a syntax error.
    const char matlab_script[] = {
#embed "../../postprocessing/MatLab/xdyn_postProcess.m" suffix(,)
        '\0'
    };

    const char python_script[] = {
#embed "../../postprocessing/Python/demoPython.py" suffix(,)
        '\0'
    };

    // datasetName carries the *script's* extension while the C++ side is named after
    // the extension-free stem. Conflating the two silently dropped
    // /scripts/* from every HDF5 file for years. Covered by the integration test
    // extra_hdf5_output_when_required, which h5dumps these exact two dataset names.
    void exportScripts(
            const H5::H5File& h5File,
            const std::string& fileName,
            const std::string& baseName,
            const std::string& destination,
            const std::string& datasetName,
            const char* script)
    {
        std::string text = H5_Tools::replaceString(script, "hdf5Filename_.h5", H5_Tools::getBasename(fileName));
        H5_Tools::replaceStringInPlace(text, "hdf5Group_", baseName);
        H5_Tools::write(h5File, destination + "/" + datasetName, text);
    }
}

void exportMatLabScripts(
        const H5::H5File& h5File,
        const std::string& fileName,
        const std::string& baseName,
        const std::string& destination)
{
    exportScripts(h5File, fileName, baseName, destination, "demoMatLab.m", matlab_script);
}

void exportPythonScripts(
        const H5::H5File& h5File,
        const std::string& fileName,
        const std::string& baseName,
        const std::string& destination)
{
    exportScripts(h5File, fileName, baseName, destination, "demoPython.py", python_script);
}
