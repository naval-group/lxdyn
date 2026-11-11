/*
 * AssetPathTest.hpp
 */

#ifndef ASSETPATHTEST_HPP_
#define ASSETPATHTEST_HPP_

#include "gtest/gtest.h"

class AssetPathTest : public ::testing::Test
{
    protected:
        AssetPathTest();
        ~AssetPathTest() override;
        void SetUp() override;
        void TearDown() override;
};

#endif /* ASSETPATHTEST_HPP_ */
