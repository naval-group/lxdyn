/*
 * MMGPropellerTest.cpp
 *
 *  Created on: Oct 3, 2024
 *      Author: fbickert
 */

#include "MMGPropellerForceModelTest.hpp"
#include "MMGPropellerForceModel.hpp"
#include "xdyn/test_data_generator/yaml_data.hpp"
#include "xdyn/core/BodyBuilder.hpp"

#define _USE_MATH_DEFINE
#include <cmath>
#define PI M_PI

#define EPS 1E-6

#define BODY "body 1"

namespace ssc
{
    namespace random_data_generator
    {
        template <>
        MMGPropellerForceModel::Yaml TypedScalarDataGenerator<MMGPropellerForceModel::Yaml>::get() const
        {
            MMGPropellerForceModel::Yaml ret;
            // Inherited inputs from AbstractWageningen
            ret.name = random<std::string>();
            // ret.position_of_propeller_frame
            ret.wake_coefficient = random<double>();
            ret.relative_rotative_efficiency = random<double>();
            ret.thrust_deduction_factor = random<double>();
            ret.rotating_clockwise = random<bool>();
            ret.diameter = random<double>();

            // Inherited inputs from MMGPropellerForceModel
            ret.k0 = random<double>();
            ret.k1 = random<double>();
            ret.k2 = random<double>();
            ret.C1 = random<double>();
            ret.C2 = { random<double>(), random<double>() };
            return ret;
        }
    }
}

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

EnvironmentAndFrames MMGPropellerForceModelTest::get_env()
{
    EnvironmentAndFrames env;
    env.rho = 1024;
    env.g = 9.81;
    env.rot = YamlRotation("angle", { "z", "y'", "x''" });
    env.k = ssc::kinematics::KinematicsPtr(new ssc::kinematics::Kinematics());
    env.k->add(ssc::kinematics::Transform(ssc::kinematics::Point("NED"), "mesh(" BODY ")"));
    env.k->add(ssc::kinematics::Transform(ssc::kinematics::Point("NED"), BODY));
    return env;
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
    ASSERT_DOUBLE_EQ(-160, k.position_of_propeller_frame.coordinates.x);
    ASSERT_DOUBLE_EQ(0, k.position_of_propeller_frame.coordinates.y);
    ASSERT_DOUBLE_EQ(0, k.position_of_propeller_frame.coordinates.z);
    ASSERT_EQ("mesh(body 1)", k.position_of_propeller_frame.frame);
    ASSERT_DOUBLE_EQ(1, k.relative_rotative_efficiency);
    ASSERT_DOUBLE_EQ(0.22, k.thrust_deduction_factor);
    ASSERT_DOUBLE_EQ(0.35, k.wake_coefficient);
    ASSERT_DOUBLE_EQ(9.86, k.diameter);
    ASSERT_DOUBLE_EQ(0.2931,k.k0);
    ASSERT_DOUBLE_EQ(-0.2753,k.k1);
    ASSERT_DOUBLE_EQ(-0.1385,k.k2);
    ASSERT_DOUBLE_EQ(2.0,k.C1);
    ASSERT_DOUBLE_EQ(1.1,k.C2[0]);
    ASSERT_DOUBLE_EQ(1.6,k.C2[1]);
}

TEST_F(MMGPropellerForceModelTest, name)
{
    EnvironmentAndFrames env=get_env();
    const MMGPropellerForceModel propModel(a.random<MMGPropellerForceModel::Yaml>(), "", env);
    ASSERT_EQ("MMG propeller", propModel.model_name());
}

TEST_F(MMGPropellerForceModelTest, check_longitudinal_location_in_body_frame)
{
    /*
    The purpose of this test is to check that the propeller longitudinal position in body frame is correctly computed.
    */
    // Create environnement
    EnvironmentAndFrames env = get_env();
    // Create body
    BodyPtr b(BodyBuilder(env.rot).build(BODY, VectorOfVectorOfPoints(), 0, 0, env.rot, true));
    // Create propeller model 
    const MMGPropellerForceModel propModel(MMGPropellerForceModel::parse(test_data::MMGPropeller()), b->get_name(), env);
    ASSERT_EQ(-160,propModel.get_longitudinal_position_in_body_frame());
}

TEST_F(MMGPropellerForceModelTest, check_wake_factor_calculation)
{
    /*
    The purpose of this test is to check that the polymorphism is well respected when computing the wake factor
    */
    // Create environnement
    EnvironmentAndFrames env = get_env();
    // Create body
    BodyPtr b(BodyBuilder(env.rot).build(BODY, VectorOfVectorOfPoints(), 0, 0, env.rot, true));
    // Create propeller model 
    const MMGPropellerForceModel propModel(MMGPropellerForceModel::parse(test_data::MMGPropeller()), b->get_name(), env);

    // Define one state
    BodyStates states;
    // Define a 30° drift angle with SOG=2 m/s
    states.u.record(0, 1.7320508075688772);
    states.v.record(0, -1.);

    // If r=0 $\beta_P=\beta$=PI/6>0 and C2=1.6
    states.r.record(0, 0);
    double wake_fac=propModel.get_wake_factor(states);
    ASSERT_DOUBLE_EQ(1-0.65*(1+(1-exp(-PI/3))*0.6),wake_fac);
    
    // If r=-PI/480 then $\beta_P$=0 (C2=1.6 but it is not used)
    states.r.record(0, -PI/480);
    wake_fac=propModel.get_wake_factor(states);
    ASSERT_DOUBLE_EQ(0.35,wake_fac);

    // If r=PI/480 then $\beta_P$=PI/3>0 and C2=1.6
    states.r.record(0, PI/480);
    wake_fac=propModel.get_wake_factor(states);
    ASSERT_DOUBLE_EQ(1-0.65*(1+(1-exp(-2*PI/3))*0.6),wake_fac);

    // If r=-PI/240 then $\beta_P$=-PI/6<0 and C2=1.1
    states.r.record(0, -PI/240);
    wake_fac=propModel.get_wake_factor(states);
    ASSERT_DOUBLE_EQ(1-0.65*(1+(1-exp(-PI/3))*0.1),wake_fac);

    // Define a -30° drift angle with SOG=2 m/s
    states.v.record(0, 1.);
    // If r=-PI/480 then $\beta_P$=-PI/3 and C2=1.1
    states.r.record(0, -PI/480);
    wake_fac=propModel.get_wake_factor(states);
    ASSERT_DOUBLE_EQ(1-0.65*(1+(1-exp(-2*PI/3))*0.1),wake_fac);
}

TEST_F(MMGPropellerForceModelTest, check_kt_calculation)
{
    /*
    The purpose of this test is to check that K_T is well computed
    */
    // Create environnement
    EnvironmentAndFrames env = get_env();
    // Create body
    BodyPtr b(BodyBuilder(env.rot).build(BODY, VectorOfVectorOfPoints(), 0, 0, env.rot, true));
    // Create propeller model 
    const MMGPropellerForceModel propModel(MMGPropellerForceModel::parse(test_data::MMGPropeller()), b->get_name(), env);

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
    // Create environnement
    EnvironmentAndFrames env = get_env();
    // Create body
    BodyPtr b(BodyBuilder(env.rot).build(BODY, VectorOfVectorOfPoints(), 0, 0, env.rot, true));
    // Create propeller model 
    const MMGPropellerForceModel propModel(MMGPropellerForceModel::parse(test_data::MMGPropeller()), b->get_name(), env);

    // Define a 30° drift angle with SOG=2 m/s
    auto states = b->get_states();
    states.u.record(0, 1.7320508075688772);
    states.v.record(0, -1);
    // If r=0 $\beta_P=\beta$=PI/6>0 and C2=1.6
    states.r.record(0, 0);
    // Define a command rpm
    std::map<std::string,double> commands;
    commands["rpm"] = 5*(2*PI);// in rad/s

    Wrench propeller_wrench=propModel.get_force(states, a.random<double>(), env, commands);

    // K_T=0.28422530415696801
    ASSERT_NEAR((1-0.22)*1024*25*pow(9.86,4)*0.28422530415696801, propeller_wrench.X(),EPS);
    ASSERT_DOUBLE_EQ(0, propeller_wrench.Y());
    ASSERT_DOUBLE_EQ(0, propeller_wrench.Z());
    ASSERT_DOUBLE_EQ(0, propeller_wrench.K());
    ASSERT_DOUBLE_EQ(0, propeller_wrench.M());
    ASSERT_DOUBLE_EQ(0, propeller_wrench.N());
}

TEST_F(MMGPropellerForceModelTest, J_outside_domain)
{
    /*
    The purpose of this test is to verify that the error message is well thrown if J is outside its validity domain
    */
    std::stringstream error;
    // Redirect cerr to our stringstream buffer or any other ostream
    std::streambuf* orig =std::cerr.rdbuf(error.rdbuf());

    // Create environnement
    EnvironmentAndFrames env = get_env();
    // Create body
    BodyPtr b(BodyBuilder(env.rot).build(BODY, VectorOfVectorOfPoints(), 0, 0, env.rot, true));
    // Create propeller model 
    const MMGPropellerForceModel propModel(MMGPropellerForceModel::parse(test_data::MMGPropeller()), b->get_name(), env);

    // Define a command rpm
    std::map<std::string,double> commands;

    //Check behaviour if J=0: no error thrown
    double kt=propModel.get_Kt(commands,0);
    ASSERT_DOUBLE_EQ(0.2931,kt);
    EXPECT_TRUE(error.str().empty());
    std::cout << error.str() << std::endl;
    error.str("");

    //Check behaviour if J<0: error thrown
    kt=propModel.get_Kt(commands,-0.001);
    ASSERT_DOUBLE_EQ(0.2931,kt);
    EXPECT_FALSE(error.str().empty());
    std::cout << error.str() << std::endl;
    error.str("");

    //Check behaviour if 0<=J<Jmax: no error thrown
    // Jmax=0.7679569206840643
    kt=propModel.get_Kt(commands,0.76795);
    EXPECT_TRUE(error.str().empty());
    std::cout << error.str() << std::endl;
    error.str("");

    //Check behaviour if J>Jmax: error thrown
    // Jmax=0.7679569206840643
    kt=propModel.get_Kt(commands,0.767957);
    ASSERT_NEAR(0,kt,EPS);
    EXPECT_FALSE(error.str().empty());
    std::cout << error.str() << std::endl;
    error.str("");

    // Check behaviour if n=0 => J=+Inf
    commands["rpm"] = 0;
    const double J=propModel.get_advance_ratio(commands,10);
    kt=propModel.get_Kt(commands,J);
    ASSERT_NEAR(0,kt,EPS);
    EXPECT_FALSE(error.str().empty());
    std::cout << error.str() << std::endl;

    std::cerr.rdbuf(orig);
}
