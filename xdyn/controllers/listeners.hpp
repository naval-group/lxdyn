/*
 * listeners.hpp
 *
 *  Created on: Oct 21, 2014
 *      Author: cady
 */

#ifndef LISTENERS_HPP_
#define LISTENERS_HPP_

#include <ssc/data_source.hpp>

struct YamlTimeSeries;

/**  \brief Reads data from YAML & builds an interpolation table per command.
 *  \returns DataSource used to retrieve the commands of the controlled forces models at each
 * instant
 *  \snippet controllers/unit_tests/listenersTest.cpp listenersTest
 * listen_to_file_example
 */
ssc::data_source::DataSource make_command_listener(const std::vector<YamlTimeSeries>& commands);

/**  \brief Reads data from YAML & builds an interpolation table per setpoint.
 *  \returns DataSource used to retrieve the setpoints of the controlled forces models at each
 * instant
 *  \snippet controllers/unit_tests/listenersTest.cpp listenersTest
 * listen_to_file_example
 */
void add_setpoints_listener(
    ssc::data_source::DataSource& ds,
    const std::vector<YamlTimeSeries>& setpoints);

#endif /* LISTENERS_HPP_ */
