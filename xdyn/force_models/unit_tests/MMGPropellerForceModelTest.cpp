/*
 * MMGPropellerTest.cpp
 *
 *  Created on: Oct 3, 2024
 *      Author: fbickert
 */

#include "MMGPropellerForceModelTest.hpp"
#include "MMGPropellerForceModel.hpp"
#include "xdyn/exceptions/NumericalErrorException.hpp"
#include "xdyn/test_data_generator/yaml_data.hpp"
#include "xdyn/core/BodyBuilder.hpp"

#include "gmock/gmock.h"
using ::testing::_;
using ::testing::ElementsAreArray;

#define _USE_MATH_DEFINE
#include <cmath>
#define PI M_PI

#define EPS 1E-6

#define BODY "body 1"


MMGPropellerForceModelTest::MMGPropellerForceModelTest() : a(ssc::random_data_generator::DataGenerator(9876))
{
}

MMGPropellerForceModelTest::~MMGPropellerForceModelTest()
{
}

void MMGPropellerForceModelTest::SetUp()
{
}

void MMGPropellerForceModelTest::TearDown()
{
}

TEST_F(MMGPropellerForceModelTest, parser)
{
    /*
    The purpose of this test is to check that the input data are well parsed
    */
    const auto k = MMGPropellerForceModel::parse(test_data::MMGPropeller());
    ASSERT_EQ("port side propeller", k.name);
    ASSERT_DOUBLE_EQ(0, k.position_of_propeller_frame.angle.phi);
    ASSERT_DOUBLE_EQ(0, k.position_of_propeller_frame.angle.theta);
    ASSERT_DOUBLE_EQ(0, k.position_of_propeller_frame.angle.psi);
    ASSERT_DOUBLE_EQ(-171, k.position_of_propeller_frame.coordinates.x);
    ASSERT_DOUBLE_EQ(0, k.position_of_propeller_frame.coordinates.y);
    ASSERT_DOUBLE_EQ(12.6, k.position_of_propeller_frame.coordinates.z);
    ASSERT_EQ("mesh(body 1)", k.position_of_propeller_frame.frame);
    ASSERT_DOUBLE_EQ(1, k.relative_rotative_efficiency);
    ASSERT_TRUE(k.rotating_clockwise);
    ASSERT_DOUBLE_EQ(0.22, k.thrust_deduction_factor);
    ASSERT_DOUBLE_EQ(0.35, k.wake_coefficient);
    ASSERT_DOUBLE_EQ(9.86, k.diameter);
    ASSERT_DOUBLE_EQ(0.2931,k.k0);
    ASSERT_DOUBLE_EQ(-0.2753,k.k1);
    ASSERT_DOUBLE_EQ(-0.1385,k.k2);
}

TEST_F(MMGPropellerForceModelTest, check_wake_factor_calculation)
{
    /*
    The purpose of this test is to check that the polymorphism is well respected when computing the wake factor
    */
    // Load and modify yaml input
    auto input = MMGPropellerForceModel::parse(test_data::MMGPropeller());
    // Create environnement
    EnvironmentAndFrames env;
    env.rho = 1024;
    env.rot = YamlRotation("angle", {"z","y'","x''"});
    // Create propeller model 
    const MMGPropellerForceModel propModel(input, "", env);

    ASSERT_EQ("MMG propeller", propModel.model_name());

    // Define one state
    BodyStates states;
    // Define a drifting behaviour of 30°
    states.u.record(0, 1.7320508075688772);
    states.v.record(0, -1.);

    double wake_fac=propModel.get_wake_factor(states);

    ASSERT_DOUBLE_EQ(1-(1-0.35)*(1+0.25*(1-30*PI/180)),wake_fac);
}

TEST_F(MMGPropellerForceModelTest, check_kt_calculation)
{
    /*
    The purpose of this test is to check that K_T is well computed
    */
    // Load and modify yaml input
    auto input = MMGPropellerForceModel::parse(test_data::MMGPropeller());
    // Create environnement
    EnvironmentAndFrames env;
    env.rho = 1024;
    env.rot = YamlRotation("angle", {"z","y'","x''"});
    // Create propeller model 
    const MMGPropellerForceModel propModel(input, "", env);

    // Define one state
    BodyStates states;
    // Define a drifting behaviour of 30°
    states.u.record(0, 1.7320508075688772*0.5);
    states.v.record(0, -0.5);
    // Define a command rpm
    std::map<std::string,double> commands;
    
    double J=0.5;
    double kt=propModel.get_Kt(commands,J);

    ASSERT_DOUBLE_EQ(0.2931-0.2753*0.5-0.1385*0.25,kt);
}

TEST_F(MMGPropellerForceModelTest, force)
{
    /*
    The purpose of this test is to check the non-regression of the output tensor
    */
    // Load and modify yaml input
    auto input = MMGPropellerForceModel::parse(test_data::MMGPropeller());
    // Create environnement
    EnvironmentAndFrames env;
    env.rho = 1024;
    env.rot = YamlRotation("angle", {"z","y'","x''"});
    env.k = ssc::kinematics::KinematicsPtr(new ssc::kinematics::Kinematics());
    env.k->add(ssc::kinematics::Transform(ssc::kinematics::Point("NED"), "mesh(" BODY ")"));
    env.k->add(ssc::kinematics::Transform(ssc::kinematics::Point("NED"), BODY));
    // Create body
    BodyPtr b(BodyBuilder(env.rot).build(BODY, VectorOfVectorOfPoints(), 0, 0, env.rot, true));
    // Create propeller model 
    const MMGPropellerForceModel w(input, b->get_name(), env);

    // Define one state
    std::vector<double> s = {1,2,3,4,5,6,0,0,0,1,0,0,0};
    auto states = b->get_states();
    // Define a drifting behaviour of 30°
    states.u.record(0, 1.7320508075688772);
    states.v.record(0, -1);
    b->update_kinematics(s, env.k);
    // Define a command rpm
    std::map<std::string,double> commands;
    commands["rpm"] = 5*(2*PI);

    Wrench propeller_wrench=w.get_force(states, a.random<double>(), env, commands);

    // K_T=0.28597392441489244419
    ASSERT_NEAR((1-0.22)*1024*25*9.86*9.86*9.86*9.86*0.285973924414892, propeller_wrench.X(),EPS);
    ASSERT_DOUBLE_EQ(0, propeller_wrench.Y());
    ASSERT_DOUBLE_EQ(0, propeller_wrench.Z());
    ASSERT_DOUBLE_EQ(0, propeller_wrench.K());
    ASSERT_DOUBLE_EQ(0, propeller_wrench.M());
    ASSERT_DOUBLE_EQ(0, propeller_wrench.N());
}
