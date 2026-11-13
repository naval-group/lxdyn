#ifndef SIMULATORYAMLPARSERTEST_HPP_
#define SIMULATORYAMLPARSERTEST_HPP_

#include <ssc/random_data_generator.hpp>

#include "gtest/gtest.h"
struct YamlSimulatorInput;
class SimulatorYamlParserTest : public ::testing::Test {
protected:
    SimulatorYamlParserTest();
    ssc::random_data_generator::DataGenerator a;
    const YamlSimulatorInput& yaml;
    const YamlSimulatorInput& old_yaml;
};

#endif /* SIMULATORYAMLPARSERTEST_HPP_ */
