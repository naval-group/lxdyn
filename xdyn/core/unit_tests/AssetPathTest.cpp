/*
 * AssetPathTest.cpp
 */

#include "AssetPathTest.hpp"
#include "xdyn/core/AssetPath.hpp"

#include <boost/filesystem.hpp>

AssetPathTest::AssetPathTest()
{}

AssetPathTest::~AssetPathTest()
{}

void AssetPathTest::SetUp()
{}

// The root is process-wide, so a test that sets it has to hand it back.
void AssetPathTest::TearDown()
{
    xdyn::set_assets_root("");
}

TEST_F(AssetPathTest, no_root_leaves_the_path_untouched)
{
    ASSERT_EQ("meshes/hull.stl", xdyn::resolve_asset("meshes/hull.stl"));
}

TEST_F(AssetPathTest, an_empty_path_stays_empty)
{
    xdyn::set_assets_root("models");
    ASSERT_EQ("", xdyn::resolve_asset(""));
}

TEST_F(AssetPathTest, a_relative_path_is_joined_to_the_root)
{
    xdyn::set_assets_root("models");
    const boost::filesystem::path resolved(xdyn::resolve_asset("hull.stl"));
    ASSERT_EQ("models", resolved.parent_path().string());
    ASSERT_EQ("hull.stl", resolved.filename().string());
}

TEST_F(AssetPathTest, a_root_without_a_trailing_separator_still_yields_a_separator)
{
    xdyn::set_assets_root("models");
    ASSERT_NE("modelshull.stl", xdyn::resolve_asset("hull.stl"));
}

TEST_F(AssetPathTest, an_absolute_path_ignores_the_root)
{
    const std::string absolute = boost::filesystem::absolute("hull.stl").string();
    xdyn::set_assets_root("models");
    ASSERT_EQ(absolute, xdyn::resolve_asset(absolute));
}

TEST_F(AssetPathTest, the_command_line_wins_over_the_environment_and_the_yaml)
{
    ASSERT_EQ("from_cli", xdyn::assets_root("from_cli", "from_env", {"scenarios/tutorial.yml"}));
}

TEST_F(AssetPathTest, the_environment_wins_over_the_yaml)
{
    ASSERT_EQ("from_env", xdyn::assets_root("", "from_env", {"scenarios/tutorial.yml"}));
}

TEST_F(AssetPathTest, the_root_falls_back_to_the_directory_of_the_first_yaml)
{
    ASSERT_EQ("scenarios", xdyn::assets_root("", "", {"scenarios/tutorial.yml", "elsewhere/waves.yml"}));
}

TEST_F(AssetPathTest, a_yaml_in_the_working_directory_yields_no_root)
{
    ASSERT_EQ("", xdyn::assets_root("", "", {"tutorial.yml"}));
}

TEST_F(AssetPathTest, no_yaml_at_all_yields_no_root)
{
    ASSERT_EQ("", xdyn::assets_root("", "", {}));
}
