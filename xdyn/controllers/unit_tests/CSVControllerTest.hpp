/*
 * CSVControllerTest.hpp
 *
 *  Created on: Mar 16, 2022
 *      Author: cady
 */

#ifndef CSVCONTROLLERTEST_HPP_
#define CSVCONTROLLERTEST_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>

#include "gtest/gtest.h"
#include "xdyn/controllers/TempFile.hpp"

class CSVControllerTest : public ::testing::Test {
protected:
    CSVControllerTest();
    std::string test_yaml() const;
    TempFile csv;
    ssc::random_data_generator::DataGenerator a;
};

#endif /* CSVCONTROLLERTEST_HPP_ */
