/*
 * EnvironmentAndFramesTest.cpp
 *
 *  Created on: 17 déc. 2020
 *      Author: mcharlou2016
 */

#include "EnvironmentAndFramesTest.hpp"
#include "EnvironmentAndFrames.hpp"
#include "xdyn/environment_models/ConstantUWCurrentModel.hpp"
#include "xdyn/exceptions/InvalidInputException.hpp"

TEST_F(EnvironmentAndFramesTest, can_get_rho_air_when_initialized)
{
    EnvironmentAndFrames env;
    env.set_rho_air(1.225);
    ASSERT_DOUBLE_EQ(1.225,env.get_rho_air());
}

TEST_F(EnvironmentAndFramesTest, throws_if_trying_to_get_uninitialized_rho_air)
{
    EnvironmentAndFrames env;
    ASSERT_THROW(env.get_rho_air(),InvalidInputException);
}

TEST_F(EnvironmentAndFramesTest, no_current_model_means_no_current)
{
    EnvironmentAndFrames env;
    const Eigen::Vector3d current = env.get_UWCurrent(Eigen::Vector3d(1,2,3), 0);
    ASSERT_DOUBLE_EQ(0, current(0));
    ASSERT_DOUBLE_EQ(0, current(1));
    ASSERT_DOUBLE_EQ(0, current(2));
}

TEST_F(EnvironmentAndFramesTest, current_without_a_wave_model_measures_depth_from_z_zero)
{
    EnvironmentAndFrames env;
    ConstantUWCurrentModel::Input input;
    input.velocity = 2.;
    input.orientation = 0.;
    env.UWCurrent = UWCurrentModelPtr(new ConstantUWCurrentModel(input));
    // No wave model: querying it for the free surface would dereference a null pointer
    const Eigen::Vector3d below = env.get_UWCurrent(Eigen::Vector3d(0,0,10), 0);
    ASSERT_DOUBLE_EQ(2., below(0));
    const Eigen::Vector3d above = env.get_UWCurrent(Eigen::Vector3d(0,0,-10), 0);
    ASSERT_DOUBLE_EQ(0., above(0));
}
