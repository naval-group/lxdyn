/*
 * RadiationDampingForceModelTest.hpp
 *
 *  Created on: Dec 1, 2014
 *      Author: cady
 */


#ifndef RADIATIONDAMPINGFORCEMODELTEST_HPP_
#define RADIATIONDAMPINGFORCEMODELTEST_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator/DataGenerator.hpp>

#include <ssc/macros.hpp>
#include <memory>

#include "xdyn/core/Body.hpp"
#include "HDBParserForTests.hpp"
#include "xdyn/external_data_structures/YamlRadiationDamping.hpp"

class RadiationDampingForceModelTest : public ::testing::Test
{
    protected:
        RadiationDampingForceModelTest();
        ~RadiationDampingForceModelTest() override;
        void SetUp() override;
        void TearDown() override;
        std::shared_ptr<HDBParser> get_hdb_data(const YamlRadiationDamping& yaml, const bool only_diagonal_terms=true) const;
        YamlRadiationDamping get_yaml_data(const bool show_debug) const;

        ssc::random_data_generator::DataGenerator a;
};

#endif  /* RADIATIONDAMPINGFORCEMODELTEST_HPP_ */
