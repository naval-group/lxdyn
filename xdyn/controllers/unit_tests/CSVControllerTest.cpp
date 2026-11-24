/*
 * CSVControllerTest.cpp
 *
 *  Created on: Mar 16, 2022
 *      Author: cady
 */

#include "CSVControllerTest.hpp"

#include <boost/algorithm/string.hpp>
#include <cmath>
#include <cstdio>  // For remove

#include "xdyn/controllers/CSVController.hpp"
#include "xdyn/core/unit_tests/test_data.hpp"
#include "xdyn/exceptions/InvalidInputException.hpp"
#include "xdyn/yaml_parser/parse_controllers.hpp"
#define PI M_PI

CSVControllerTest::CSVControllerTest() : a(ssc::random_data_generator::DataGenerator(54545))
{
    csv << "t,some column,rpm_co,beta_co\n";
    csv << "0.2,23,65,78\n";
    csv << "0.796,-4.3,965.1,-7.8\n";
    csv.close();
}

std::string CSVControllerTest::test_yaml() const
{
    std::stringstream yaml_string;
    yaml_string << "name: some unique name\n"
                << "type: csv\n"
                << "path: path/../to/file.csv\n"
                << "time column name: t\n"
                << "separator: comma\n"
                << "shift time column to match tstart: true\n"
                << "commands:\n"
                << "    port side propeller(beta): beta_co\n"
                << "    port side propeller(rpm): rpm_co";
    std::string out = yaml_string.str();
    boost::replace_all(out, "path/../to/file.csv", csv.get_filename());
    return out;
}

TEST_F(CSVControllerTest, smoke_test)
{
    ASSERT_NO_THROW(CSVController(0, a.random<std::string>(), test_yaml()));
}

TEST_F(CSVControllerTest, can_parse_yaml_path)
{
    const CSVController controller(0, a.random<std::string>(), test_yaml());
    ASSERT_EQ(csv.get_filename(), controller.yaml.path);
}

TEST_F(CSVControllerTest, can_parse_time_column)
{
    const CSVController controller(0, a.random<std::string>(), test_yaml());
    ASSERT_EQ("t", controller.yaml.time_column);
}

TEST_F(CSVControllerTest, can_parse_separator_and_throw_if_it_is_unknown)
{
    std::string yaml = test_yaml();
    ASSERT_NO_THROW(CSVController(0, a.random<std::string>(), yaml));
    ASSERT_EQ(',', CSVController(0, a.random<std::string>(), yaml).yaml.separator);
    boost::replace_all(yaml, "separator: comma", "separator: something");
    ASSERT_THROW(CSVController(0, a.random<std::string>(), yaml), InvalidInputException);
    boost::replace_all(yaml, "separator: something", "separator: semicolon");
    ASSERT_NO_THROW(CSVController(0, a.random<std::string>(), yaml));
    ASSERT_EQ(';', CSVController(0, a.random<std::string>(), yaml).yaml.separator);
}

TEST_F(CSVControllerTest, can_parse_time_shift_to_match_tstart)
{
    std::string yaml = test_yaml();
    ASSERT_TRUE(CSVController(0, a.random<std::string>(), yaml).yaml.shift_time_column);
    boost::replace_all(
        yaml,
        "shift time column to match tstart: true",
        "shift time column to match tstart: false");
    ASSERT_FALSE(CSVController(0, a.random<std::string>(), yaml).yaml.shift_time_column);
    boost::replace_all(
        yaml,
        "shift time column to match tstart: false",
        "shift time column to match tstart: foo");
    ASSERT_THROW(CSVController(0, a.random<std::string>(), yaml), InvalidInputException);
}

TEST_F(CSVControllerTest, can_parse_commands)
{
    auto commands = CSVController(0, a.random<std::string>(), test_yaml()).yaml.commands;
    ASSERT_EQ(2, commands.size());
    ASSERT_EQ(commands["port side propeller(beta)"], "beta_co");
    ASSERT_EQ(commands["port side propeller(rpm)"], "rpm_co");
}

TEST_F(CSVControllerTest, commands_should_be_correct)
{
    const CSVController controller(0, a.random<std::string>(), test_yaml());
    const std::vector<std::string> expected_commands = {
        "port side propeller(beta)",
        "port side propeller(rpm)"};
    ASSERT_EQ(expected_commands, controller.get_command_names());
}
