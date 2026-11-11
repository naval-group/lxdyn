/*
 * CSVControllerTest.hpp
 *
 *  Created on: Mar 16, 2022
 *      Author: cady
 */

#ifndef CSVCONTROLLERTEST_HPP_
#define CSVCONTROLLERTEST_HPP_

#include "xdyn/listeners_and_controllers/TempFile.hpp"
#include <ssc/random_data_generator/DataGenerator.hpp>
#include "gtest/gtest.h"

class CSVControllerTest : public ::testing::Test
{
    protected:
        CSVControllerTest();
        ~CSVControllerTest() override;
        void SetUp() override;
        void TearDown() override;
        std::string test_yaml() const;
        TempFile csv;
        ssc::random_data_generator::DataGenerator a;
};

#endif  /* CSVCONTROLLERTEST_HPP_ */
