/*
 * RadiationDampingForceModelTest.hpp
 *
 *  Created on: Dec 1, 2014
 *      Author: cady
 */

#ifndef RADIATIONDAMPINGFORCEMODELTEST_HPP_
#define RADIATIONDAMPINGFORCEMODELTEST_HPP_

#include <memory>
#include <ssc/macros.hpp>
#include <ssc/random_data_generator/DataGenerator.hpp>

#include "HDBParserForTests.hpp"
#include "gtest/gtest.h"
#include "xdyn/core/Body.hpp"
#include "xdyn/input/YamlRadiationDamping.hpp"

class RadiationDampingForceModelTest : public ::testing::Test {
protected:
    RadiationDampingForceModelTest();
    std::shared_ptr<HDBParser> get_hdb_data(
        const YamlRadiationDamping& yaml,
        const bool only_diagonal_terms = true) const;
    YamlRadiationDamping get_yaml_data(const bool show_debug) const;

    ssc::random_data_generator::DataGenerator a;
};

#endif /* RADIATIONDAMPINGFORCEMODELTEST_HPP_ */
