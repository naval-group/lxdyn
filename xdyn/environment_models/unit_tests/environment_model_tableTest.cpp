/*
 * environment_model_tableTest.cpp
 */

#include <algorithm>
#include <string>

#include "environment_model_table.hpp"
#include "gtest/gtest.h"
#include "xdyn/exceptions/InvalidInputException.hpp"

// A name accepted by two of the wave, wind and current tables would fill only the first one's slot
TEST(add_environment_model, every_name_is_unique)
{
    auto names = environment_model_names();
    std::sort(names.begin(), names.end());
    const auto duplicate = std::adjacent_find(names.begin(), names.end());
    ASSERT_EQ(names.end(), duplicate) << "'" << *duplicate << "' names two environment models";
}

TEST(add_environment_model, fills_only_the_slot_of_its_model)
{
    EnvironmentAndFrames env;
    add_environment_model(YamlModel("no wind", ""), env);
    EXPECT_NE(nullptr, env.wind);
    EXPECT_EQ(nullptr, env.w);
    EXPECT_EQ(nullptr, env.UWCurrent);
}

TEST(add_environment_model, an_unknown_name_suggests_the_closest_known_one)
{
    EnvironmentAndFrames env;
    std::string message;
    try {
        add_environment_model(YamlModel("no wave", "", 4), env);
    } catch (const InvalidInputException& e) {
        message = e.get_message();
    }
    EXPECT_NE(std::string::npos, message.find("'no wave', defined line 4")) << message;
    EXPECT_NE(std::string::npos, message.find("Did you mean 'no waves'?")) << message;
}

TEST(add_environment_model, an_unknown_wave_model_suggests_the_closest_known_one)
{
    const std::string waves = R"(
discretization:
   n: 10
   omega min: {value: 0.1, unit: rad/s}
   omega max: {value: 6, unit: rad/s}
   energy fraction: 0.999
spectra:
  - model: airyy
    depth: {value: 100, unit: m}
    seed of the random data generator: 0
    stretching:
      delta: 1
      h: {unit: m, value: 0}
    directional spreading:
       type: dirac
       waves propagating to: {value: 90, unit: deg}
    spectral density:
       type: dirac
       omega0: {value: 0.8976, unit: rad/s}
       Hs: {value: 5, unit: m}
)";
    EnvironmentAndFrames env;
    std::string message;
    try {
        add_environment_model(YamlModel("waves", waves, 7), env);
    } catch (const InvalidInputException& e) {
        message = e.get_message();
    }
    EXPECT_NE(std::string::npos, message.find("wave model 'airyy'")) << message;
    EXPECT_NE(std::string::npos, message.find("Did you mean 'airy'?")) << message;
}
