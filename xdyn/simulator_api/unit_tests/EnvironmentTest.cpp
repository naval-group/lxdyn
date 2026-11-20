#include "EnvironmentTest.hpp"

#include <cmath>
#include <fstream>

#include "xdyn/core/EnvironmentAndFrames.hpp"
#include "xdyn/core/unit_tests/test_data.hpp"
#include "xdyn/exceptions/InvalidInputException.hpp"
#include "xdyn/simulator_api/simulator_api.hpp"
#include "xdyn/yaml_parser/SimulatorYamlParser.hpp"
#define PI M_PI

TEST_F(EnvironmentTest, can_get_wave_elevation_with_test_data_full_example)
{
    const std::string stl_filename("test_ship.stl");
    std::ofstream stl(stl_filename);
    stl << test_data::read("tutorials/cube.stl");
    stl.close();
    Sim sim = get_system(test_data::read("yaml/full_example.yml"));
    auto waves = sim.get_env().w;
    std::vector<double> res =
        waves->get_and_check_wave_height(std::vector<double>{0.0}, std::vector<double>{0.0}, 0);
    ASSERT_EQ(res.size(), 1);
    ASSERT_EQ(res[0], 12);
    EXPECT_EQ(0, remove(stl_filename.c_str()));
}

TEST_F(EnvironmentTest, can_get_wave_elevation_with_simple_waves)
{
    Sim sim = get_system(test_data::read("yaml/simple_waves.yml"));
    auto waves = sim.get_env().w;
    std::vector<double> res =
        waves->get_and_check_wave_height(std::vector<double>{0.0}, std::vector<double>{0.0}, 0);
    ASSERT_EQ(res.size(), 1);
    ASSERT_NE(res[0], 0.0);
}

TEST_F(EnvironmentTest, can_get_wave_elevation_with_waves_from_a_list_of_rays)
{
    Sim sim = get_system(test_data::read("yaml/waves_from_a_list_of_rays.yml"));
    auto waves = sim.get_env().w;
    std::vector<double> res =
        waves->get_and_check_wave_height(std::vector<double>{0.0}, std::vector<double>{0.0}, 0);
    ASSERT_EQ(res.size(), 1);
    const double expected_value = -0.1 * sin(50.0 / 180.0 * PI) - 0.5 * sin(90.0 / 180.0 * PI) -
                                  0.1 * sin(110.0 / 180.0 * PI) - 0.5 * sin(130.0 / 180.0 * PI);
    ASSERT_NEAR(res[0], expected_value, 1e-9);
}

TEST_F(EnvironmentTest, check_that_system_can_be_generated_without_any_body)
{
    StateType ret(0, 0);
    ASSERT_EQ(ret.size(), 0);
}
