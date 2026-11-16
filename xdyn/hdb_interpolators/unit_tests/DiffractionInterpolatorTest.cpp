/*
 * DiffractionInterpolatorTest.cpp
 *
 *  Created on: Dec 16, 2014
 *      Author: cady
 */

#include "DiffractionInterpolatorTest.hpp"

#include <cmath>

#include "RaoInterpolator.hpp"
#include "xdyn/core/unit_tests/test_data.hpp"
#include "xdyn/external_data_structures/YamlRAO.hpp"
#include "xdyn/hdb_interpolators/HDBParser.hpp"
#define PI M_PI

DiffractionInterpolatorTest::DiffractionInterpolatorTest()
    : a(ssc::random_data_generator::DataGenerator(989899))
{
}

TEST_F(DiffractionInterpolatorTest, example)
{
    //! [DiffractionInterpolatorTest example]
    const HDBParser parser = HDBParser::from_string(test_data::read("tutorials/test_ship.hdb"));
    YamlRAO yaml;
    yaml.mirror = true;
    RaoInterpolator radiation(parser, yaml);
    //! [DiffractionInterpolatorTest example]
    //! [DiffractionInterpolatorTest expected output]
    const size_t k = 0;  // X-axis
    double module = radiation.interpolate_module(k, 64.0, 0.0);
    ASSERT_NEAR(3.378373E+03, module, 1E-9);
    module = radiation.interpolate_module(k, 125.0, 30.0 / 180.0 * PI);
    ASSERT_NEAR(9.067188E+02, module, 1E-9);
    double phase = radiation.interpolate_phase(k, 64.0, 0.0);
    ASSERT_NEAR(2.088816E+00, phase, 1E-9);
    phase = radiation.interpolate_phase(k, 125.0, 30.0 / 180.0 * PI);
    ASSERT_NEAR(2.123063E+00, phase, 1E-9);
    //! [DiffractionInterpolatorTest expected output]
}

TEST_F(DiffractionInterpolatorTest, example_deprecated_aquaplus_format)
{
    const HDBParser parser = HDBParser::from_string(test_data::read("tutorials/test_ship.hdb"));
    YamlRAO yaml;
    yaml.mirror = true;
    RaoInterpolator radiation(parser, yaml);
    const size_t k = 0;  // X-axis
    double module = radiation.interpolate_module(k, 64.0, 0.0);
    ASSERT_NEAR(3.378373E+03, module, 1E-9);
    module = radiation.interpolate_module(k, 125.0, 30.0 / 180.0 * PI);
    ASSERT_NEAR(9.067188E+02, module, 1E-9);
    double phase = radiation.interpolate_phase(k, 64.0, 0.0);
    ASSERT_NEAR(2.088816E+00, phase, 1E-9);
    phase = radiation.interpolate_phase(k, 125.0, 30.0 / 180.0 * PI);
    ASSERT_NEAR(2.123063E+00, phase, 1E-9);
}
