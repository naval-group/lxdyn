/*
 * EkmanUWCurrentTest.cpp
 */

#include "EkmanUWCurrentTest.hpp"

#include <stb_image_write.h>

#include <Eigen/Dense>
#include <cstdio>
#include <vector>

#include "EkmanUWCurrentModel.hpp"

namespace {
// Written here rather than reused from SeabedTest: depending on another fixture's side effect makes
// the suite order-dependent and breaks under --gtest_filter.
std::string write_seabed_image()
{
    const char* path = "ekmantest_seabed.png";
    const unsigned width = 4;
    const unsigned height = 4;
    const unsigned channels = 4;
    std::vector<uint8_t> rgba_image = {
        75, 75, 75, 255, 72, 72, 72, 255, 73, 73, 73, 255, 69, 69, 69, 255,
        74, 74, 74, 255, 68, 68, 68, 255, 64, 64, 64, 255, 62, 62, 62, 255,
        77, 77, 77, 255, 69, 69, 69, 255, 63, 63, 63, 255, 62, 62, 62, 255,
        84, 84, 84, 255, 73, 73, 73, 255, 65, 65, 65, 255, 64, 64, 64, 255,
    };
    stbi_write_png(path, width, height, channels, rgba_image.data(), width * channels);
    return path;
}
}  // namespace

EkmanUWCurrentTest::EkmanUWCurrentTest() : a(ssc::random_data_generator::DataGenerator(149555)) {}

TEST_F(EkmanUWCurrentTest, test_velocity)
{
    Seabed seabed(65.65);
    EkmanUWCurrentModel::Input input{seabed};
    EkmanUWCurrentModel model(input);
    const Eigen::Vector3d point1(1.8, 13.2, 1.5);
    model.get_UWCurrent(point1, 0, 5.5);
    const Eigen::Vector3d point2(-5, -11.1, 8);
    model.get_UWCurrent(point2, 1, -2);
}

TEST_F(EkmanUWCurrentTest, can_read_a_seabed_from_an_image)
{
    const std::string seabed_file = write_seabed_image();
    const std::string yaml_input = "{seabed file: " + seabed_file +
                                   ","
                                   " latitude: { value: 47, unit: rad },"
                                   " top layer thickness: { value: 10, unit: m },"
                                   " bottom layer thickness: { value: 20, unit: m },"
                                   " current velocity: { value: 1, unit: m/s },"
                                   " current orientation: { value: 30, unit: deg },"
                                   " U10: { value: 1, unit: m/s },"
                                   " wind orientation: { value: 20, unit: deg }}";
    EkmanUWCurrentModel model(EkmanUWCurrentModel::parse(yaml_input));
    const Eigen::Vector3d point1(3.5, -7, 0.2);
    model.get_UWCurrent(point1, 0, 5.5);
    const Eigen::Vector3d point2(-7, 150, 6);
    model.get_UWCurrent(point2, 0, 5.5);
}

TEST_F(EkmanUWCurrentTest, should_throw_if_no_seabed_is_given)
{
    const std::string yaml_input = "{latitude: { value: 47, unit: rad },"
                                   " top layer thickness: { value: 10, unit: m },"
                                   " bottom layer thickness: { value: 20, unit: m },"
                                   " current velocity: { value: 1, unit: m/s },"
                                   " current orientation: { value: 30, unit: deg },"
                                   " U10: { value: 1, unit: m/s },"
                                   " wind orientation: { value: 20, unit: deg }}";
    ASSERT_THROW(EkmanUWCurrentModel::parse(yaml_input), InvalidInputException);
}
