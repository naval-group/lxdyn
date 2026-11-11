#ifndef STL_IO_HDF5TEST_HPP_
#define STL_IO_HDF5TEST_HPP_

#include "gtest/gtest.h"

class stl_io_hdf5Test : public ::testing::Test
{
    protected:
        stl_io_hdf5Test(){}
        ~stl_io_hdf5Test() override{}
        void SetUp() override{}
        void TearDown() override{}
};

#endif
