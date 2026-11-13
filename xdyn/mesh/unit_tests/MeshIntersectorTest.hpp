#ifndef MESH_INTERSECTOR_TEST_HPP
#define MESH_INTERSECTOR_TEST_HPP

#include <ssc/random_data_generator.hpp>

#include "gtest/gtest.h"
#include "xdyn/mesh/MeshIntersector.hpp"

class MeshIntersectorTest : public ::testing::Test {
protected:
    MeshIntersectorTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif
