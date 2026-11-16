/*
 * EkmanUWCurrentTest.cpp
 */

#include "EkmanUWCurrentTest.hpp"
#include "EkmanUWCurrentModel.hpp"

#include <Eigen/Dense>
#include <cstdio>
#include <vector>
#include <complex>
#include <sstream>
#include <stb_image_write.h>

namespace
{
// Written here rather than reused from SeabedTest: depending on another fixture's side effect makes
// the suite order-dependent and breaks under --gtest_filter.
std::string write_seabed_image()
{
    const char* path = "ekmantest_seabed.png";
    const unsigned width = 4;
    const unsigned height = 4;
    const unsigned channels = 4;
    std::vector<uint8_t> rgba_image = {
        75, 75, 75, 255,  72, 72, 72, 255,  73, 73, 73, 255,  69, 69, 69, 255,
        74, 74, 74, 255,  68, 68, 68, 255,  64, 64, 64, 255,  62, 62, 62, 255,
        77, 77, 77, 255,  69, 69, 69, 255,  63, 63, 63, 255,  62, 62, 62, 255,
        84, 84, 84, 255,  73, 73, 73, 255,  65, 65, 65, 255,  64, 64, 64, 255,
    };
    stbi_write_png(path, width, height, channels, rgba_image.data(), width * channels);
    return path;
}
}

EkmanUWCurrentTest::EkmanUWCurrentTest() : a(ssc::random_data_generator::DataGenerator(149555))
{}

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
    const std::string yaml_input = "{seabed file: " + seabed_file + ","
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

namespace
{
// Builds a model with everything fixed except the wind orientation, so a test
// can vary that alone. Background current is zero: these tests are about the
// wind-driven perturbation, and a non-zero geostrophic term would just add a
// constant to every sample.
EkmanUWCurrentModel with_wind_orientation(const std::string& seabed_file,
                                          const double wind_orientation_deg,
                                          const double D_s)
{
    std::ostringstream ss;
    ss << "{seabed file: " << seabed_file << ","
       << " latitude: { value: 47, unit: deg },"
       << " top layer thickness: { value: " << D_s << ", unit: m },"
       << " bottom layer thickness: { value: 1, unit: m },"
       << " current velocity: { value: 0, unit: m/s },"
       << " current orientation: { value: 0, unit: deg },"
       << " U10: { value: 10, unit: m/s },"
       << " wind orientation: { value: " << wind_orientation_deg << ", unit: deg }}";
    return EkmanUWCurrentModel(EkmanUWCurrentModel::parse(ss.str()));
}

std::complex<double> W(const EkmanUWCurrentModel& model, const double depth)
{
    const Eigen::Vector3d c = model.get_UWCurrent(Eigen::Vector3d(0, 0, depth), 0, 0);
    return std::complex<double>(c(0), c(1));
}
}

// The model must SOLVE the Ekman balance
//
//     A_z d2W/dz2 = i f W,   W = u + i v
//
// so W''/W has to be a constant, purely imaginary number. This depends on
// neither A_z, nor the angle convention, nor the layer-thickness definition,
// which is what makes it a usable invariant rather than a golden-value test.
//
// The wind orientation here is deliberately NON-ZERO. The formulation used
// before 2026-08 satisfied this balance only at zero wind angle, so a test
// written at the default 0 deg would have stayed green while every real run
// (20 deg) was wrong. That blind spot is the whole reason this test exists.
TEST_F(EkmanUWCurrentTest, top_layer_satisfies_the_ekman_balance)
{
    const std::string seabed_file = write_seabed_image();
    const double D_s = 40.0;
    const EkmanUWCurrentModel model = with_wind_orientation(seabed_file, 20.0, D_s);

    const double dz = 1e-3;
    std::vector<std::complex<double> > ratios;
    for (int k = 2 ; k < 24 ; ++k) // z = 2 m to 23 m, below 0.6 D_s
    {
        const double z = k;
        ratios.push_back(((W(model, z + dz) - 2.0 * W(model, z) + W(model, z - dz))
                          / (dz * dz)) / W(model, z));
    }
    ASSERT_FALSE(ratios.empty());

    std::complex<double> mean(0, 0);
    for (size_t i = 0 ; i < ratios.size() ; ++i) mean += ratios[i];
    mean /= static_cast<double>(ratios.size());

    // Purely imaginary...
    ASSERT_LT(std::abs(mean.real()), 1e-4 * std::abs(mean.imag()));
    // ...and constant with depth.
    for (size_t i = 0 ; i < ratios.size() ; ++i)
    {
        ASSERT_LT(std::abs(ratios[i] - mean), 1e-3 * std::abs(mean.imag()));
    }
}

// Rotating the wind must rotate the current field RIGIDLY. f and A_z are
// scalars, so the horizontal plane is isotropic and the solution carries no
// preferred direction of its own; only the forcing does. A formulation that
// applies the wind orientation with opposite signs in the two components is not
// a rotation -- it changes the perturbation's magnitude, and by a different
// factor at every depth, which corrupts precisely the vertical shear a
// depth-resolved current model exists to provide.
TEST_F(EkmanUWCurrentTest, rotating_the_wind_preserves_the_current_magnitude)
{
    const std::string seabed_file = write_seabed_image();
    const double D_s = 40.0;
    const double angles[] = {0.0, 20.0, 45.0, 90.0, 180.0, 270.0};

    for (int k = 1 ; k < 24 ; k += 2) // z = 1 m to 23 m, below 0.6 D_s
    {
        const double z = k;
        const double reference = std::abs(W(with_wind_orientation(seabed_file, 0.0, D_s), z));
        for (size_t i = 0 ; i < sizeof(angles) / sizeof(angles[0]) ; ++i)
        {
            const EkmanUWCurrentModel model = with_wind_orientation(seabed_file, angles[i], D_s);
            ASSERT_NEAR(std::abs(W(model, z)), reference, 1e-9 * reference);
        }
    }
}

// The layer thickness is defined as the depth at which the current is OPPOSITE
// the surface current, i.e. a phase rotation of exactly pi. Pinning this ties
// the decay/rotation length scale to the parameter's stated meaning, and would
// catch a factor-of-2 or factor-of-pi drift in what "layer thickness" denotes --
// a real hazard here, since parameters fitted by one implementation are handed
// to another.
TEST_F(EkmanUWCurrentTest, current_reverses_at_one_layer_thickness)
{
    const std::string seabed_file = write_seabed_image();
    const double D_s = 40.0;
    const EkmanUWCurrentModel model = with_wind_orientation(seabed_file, 0.0, D_s);

    const double relative_angle = std::arg(W(model, D_s) / W(model, 1e-4));
    ASSERT_NEAR(std::abs(relative_angle), M_PI, 1e-3);
}
