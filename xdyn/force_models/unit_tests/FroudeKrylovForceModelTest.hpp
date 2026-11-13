/*
 * FroudeKrylovForceModelTest.hpp
 *
 *  Created on: Oct 3, 2014
 *      Author: cady
 */

#ifndef FROUDEKRYLOVFORCEMODELTEST_HPP_
#define FROUDEKRYLOVFORCEMODELTEST_HPP_

#include <memory>
#include <ssc/macros.hpp>
#include <ssc/random_data_generator/DataGenerator.hpp>

#include "gtest/gtest.h"
#include "xdyn/core/EnvironmentAndFrames.hpp"
#include "xdyn/environment_models/WaveModel.hpp"
#include "xdyn/external_data_structures/GeometricTypes3d.hpp"

class FroudeKrylovForceModelTest : public ::testing::Test {
protected:
    FroudeKrylovForceModelTest();
    VectorOfVectorOfPoints get_points() const;
    std::shared_ptr<WaveModel> get_wave_model() const;
    ssc::random_data_generator::DataGenerator a;
};

#endif /* FROUDEKRYLOVFORCEMODELTEST_HPP_ */
