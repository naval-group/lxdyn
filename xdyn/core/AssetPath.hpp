/*
 * AssetPath.hpp
 *
 * Root against which the relative mesh and seabed paths written inside a YAML are resolved.
 */

#ifndef ASSETPATH_HPP_
#define ASSETPATH_HPP_

#include <string>
#include <vector>

namespace xdyn
{
    /** \brief Value of $XDYN_ASSETS_PATH, empty when it is unset. */
    std::string assets_root_from_environment();

    /** \brief --assets-path, else the environment, else the directory holding the first YAML. */
    std::string assets_root(const std::string& from_command_line,
                            const std::string& from_environment,
                            const std::vector<std::string>& yaml_filenames);

    void set_assets_root(const std::string& root);

    /** \brief Joins path to the root. Absolute paths, and everything when no root is set, are returned unchanged. */
    std::string resolve_asset(const std::string& path);
}

#endif /* ASSETPATH_HPP_ */
