/*
 * SimulatorBuilderTest.hpp
 *
 *  Created on: Jun 19, 2014
 *      Author: cady
 */

#ifndef SIMULATORBUILDERTEST_HPP_
#define SIMULATORBUILDERTEST_HPP_

#include <ssc/random_data_generator.hpp>

#include "gtest/gtest.h"
#include "xdyn/core/SimulatorBuilder.hpp"
#include "xdyn/input/YamlSimulatorInput.hpp"

class SimulatorBuilderTest : public ::testing::Test {
protected:
    SimulatorBuilderTest();
    std::string customize(const std::string& body_name, const std::string& something) const;
    std::vector<BodyPtr> get_body_vector(const std::string& name) const;

    ssc::random_data_generator::DataGenerator a;
    const YamlSimulatorInput& input;
    SimulatorBuilder builder;
};

#endif /* SIMULATORBUILDERTEST_HPP_ */
