#ifndef SIMULATORYAMLPARSERTEST_HPP_
#define SIMULATORYAMLPARSERTEST_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator.hpp>
struct YamlSimulatorInput;
class SimulatorYamlParserTest : public ::testing::Test
{
    protected:
        SimulatorYamlParserTest();
        ~SimulatorYamlParserTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;
        const YamlSimulatorInput& yaml;
        const YamlSimulatorInput& old_yaml;
};

#endif  /* SIMULATORYAMLPARSERTEST_HPP_ */
