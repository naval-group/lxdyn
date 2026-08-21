/*
 * AssetPath.cpp
 */

#include "AssetPath.hpp"

#include <boost/filesystem.hpp>
#include <cstdlib>

namespace
{
    // Set once from main(), never read from the environment where a path is resolved: that keeps a
    // simulation's meshes a function of its arguments rather than of the shell that launched it.
    std::string& assets_root_storage()
    {
        static std::string root;
        return root;
    }
}

std::string xdyn::assets_root_from_environment()
{
    const char* const from_environment = std::getenv("XDYN_ASSETS_PATH");
    return from_environment ? from_environment : "";
}

std::string xdyn::assets_root(const std::string& from_command_line,
                              const std::string& from_environment,
                              const std::vector<std::string>& yaml_filenames)
{
    if (not(from_command_line.empty())) return from_command_line;
    if (not(from_environment.empty()))  return from_environment;
    if (yaml_filenames.empty())         return "";
    return boost::filesystem::path(yaml_filenames.front()).parent_path().string();
}

void xdyn::set_assets_root(const std::string& root)
{
    assets_root_storage() = root;
}

std::string xdyn::resolve_asset(const std::string& path)
{
    if (assets_root_storage().empty() or path.empty()) return path;
    const boost::filesystem::path relative_or_absolute(path);
    if (relative_or_absolute.is_absolute()) return path;
    return (boost::filesystem::path(assets_root_storage()) / relative_or_absolute).string();
}
