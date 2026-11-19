/*
 * force_model_tableTest.cpp
 */

#include <algorithm>
#include <string>

#include "ConstantForceModel.hpp"
#include "force_model_table.hpp"
#include "gtest/gtest.h"
#include "xdyn/exceptions/InvalidInputException.hpp"

namespace {
std::string error_from(const YamlModel& yaml)
{
    try {
        make_force_model(yaml, "body", EnvironmentAndFrames());
    } catch (const InvalidInputException& e) {
        return e.get_message();
    }
    return "";
}
}  // namespace

// Two models answering to one name would leave one of them unreachable, silently
TEST(make_force_model, every_name_is_unique)
{
    auto names = force_model_names();
    std::sort(names.begin(), names.end());
    const auto duplicate = std::adjacent_find(names.begin(), names.end());
    ASSERT_EQ(names.end(), duplicate) << "'" << *duplicate << "' names two force models";
}

TEST(make_force_model, an_unknown_name_suggests_the_closest_known_one)
{
    const auto message = error_from(YamlModel("gravit", "", 7));
    EXPECT_NE(std::string::npos, message.find("'gravit', defined line 7")) << message;
    EXPECT_NE(std::string::npos, message.find("Did you mean 'gravity'?")) << message;
}

TEST(make_force_model, a_yaml_error_names_the_model_and_its_line)
{
    const auto message = error_from(YamlModel(ConstantForceModel::model_name(), "[", 12));
    EXPECT_NE(std::string::npos, message.find("Invalid input data for model 'constant force'."))
        << message;
    EXPECT_NE(std::string::npos, message.find("defined line 12 of the YAML file")) << message;
}
