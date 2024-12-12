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
            ret.C0 = random<double>();
            ret.C1 = random<double>();
            ret.C2 = { random<double>(), random<double>() };
            ret.application_point = YamlCoordinates(random<double>(), random<double>(),random<double>());
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

TEST_F(MMGPropellerForceModelTest, wrapToPi)
{
    /*
    The purpose of this test is to check that the function wrapToPi returns an angle value between ]-PI,PI]
    */

    // ASSERT_NEAR is used instead of ASSERT_DOUBLE_EQ because of the function fmod which is used in wrapToPi
    ASSERT_NEAR(PI/3, MMGPropellerForceModel::wrapToPi(PI/3),EPS);
    ASSERT_NEAR(PI, MMGPropellerForceModel::wrapToPi(PI),EPS);
    ASSERT_NEAR(PI, MMGPropellerForceModel::wrapToPi(-PI),EPS);
    ASSERT_NEAR(-2*PI/3, MMGPropellerForceModel::wrapToPi(-2*PI/3),EPS);    
    ASSERT_NEAR(PI/3, MMGPropellerForceModel::wrapToPi(PI/3+2*PI),EPS);
    ASSERT_NEAR(-2*PI/3, MMGPropellerForceModel::wrapToPi(PI/3+PI),EPS);
    ASSERT_NEAR(2*PI/3, MMGPropellerForceModel::wrapToPi(-PI/3+PI),EPS);
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
    ASSERT_DOUBLE_EQ(12.6, k.position_of_propeller_frame.coordinates.z);
    ASSERT_EQ("mesh(body 1)", k.position_of_propeller_frame.frame);
    ASSERT_DOUBLE_EQ(0.22, k.thrust_deduction_factor);
    ASSERT_DOUBLE_EQ(0.35, k.wake_coefficient);
    ASSERT_DOUBLE_EQ(9.86, k.diameter);
    ASSERT_DOUBLE_EQ(0.2931,k.k0);
    ASSERT_DOUBLE_EQ(-0.2753,k.k1);
    ASSERT_DOUBLE_EQ(-0.1385,k.k2);
    ASSERT_DOUBLE_EQ(0.,k.C0);
    ASSERT_DOUBLE_EQ(2.0,k.C1);
    ASSERT_DOUBLE_EQ(1.1,k.C2[0]);
    ASSERT_DOUBLE_EQ(1.6,k.C2[1]);
    ASSERT_DOUBLE_EQ(0, k.application_point.x);
    ASSERT_DOUBLE_EQ(0, k.application_point.y);
    ASSERT_DOUBLE_EQ(0, k.application_point.z);
}

TEST_F(MMGPropellerForceModelTest, name)
{
    EnvironmentAndFrames env=get_env();
    const MMGPropellerForceModel propModel(a.random<MMGPropellerForceModel::Yaml>(), "", env);
    ASSERT_EQ("MMG propeller", propModel.model_name());
}

TEST_F(MMGPropellerForceModelTest, check_propeller_longitudinal_location_in_MMG_frame)
{
    /*
    The purpose of this test is to check that the propeller longitudinal position in MMG frame is correctly computed.
    */
    // Create environnement
    EnvironmentAndFrames env = get_env();
    // Create body
    BodyPtr b(BodyBuilder(env.rot).build(BODY, VectorOfVectorOfPoints(), 0, 0, env.rot, true));
    // Get input data
    MMGPropellerForceModel::Yaml input=MMGPropellerForceModel::parse(test_data::MMGPropeller());
    // Create propeller model 
    const MMGPropellerForceModel propModel(input, b->get_name(), env);

    // Case 1: Check the longitudinal position in case the body frame and the MMG frame are the same
    ASSERT_EQ(-160,propModel.get_propeller_position_in_MMG_frame().x());
    ASSERT_EQ(0,propModel.get_propeller_position_in_MMG_frame().y());
    ASSERT_EQ(12.6,propModel.get_propeller_position_in_MMG_frame().z());

    // Case 2: Check the longitudinal position in case the body frame and the MMG frame are not the same
    input.application_point.x=-10;
    input.application_point.y=20;
    input.application_point.z=5;
    const MMGPropellerForceModel propModel2(input, b->get_name(), env);
    ASSERT_EQ(-150,propModel2.get_propeller_position_in_MMG_frame().x());
    ASSERT_EQ(-20,propModel2.get_propeller_position_in_MMG_frame().y());
    ASSERT_EQ(7.6,propModel2.get_propeller_position_in_MMG_frame().z());

    // Case 3: Check the longitudinal position in case the body frame and the MMG frame are not the same (opposites values than for the previous test)
    input.application_point.x=+10;
    input.application_point.y=-20;
    input.application_point.z=-5;
    const MMGPropellerForceModel propModel3(input, b->get_name(), env);
    ASSERT_EQ(-170,propModel3.get_propeller_position_in_MMG_frame().x());
    ASSERT_EQ(20,propModel3.get_propeller_position_in_MMG_frame().y());
    ASSERT_EQ(17.6,propModel3.get_propeller_position_in_MMG_frame().z());
}

TEST_F(MMGPropellerForceModelTest, check_CoG_location_in_MMG_frame)
{
    /*
    The purpose of this test is to check that the CoG longitudinal coordinate in MMG fram is well computed.
    */
    // Create environnement
    EnvironmentAndFrames env = get_env();
    // Create body
    BodyPtr b(BodyBuilder(env.rot).build(BODY, VectorOfVectorOfPoints(), 0, 0, env.rot, true));
    // Get input data
    MMGPropellerForceModel::Yaml input=MMGPropellerForceModel::parse(test_data::MMGPropeller());
    // Create propeller model 
    const MMGPropellerForceModel propModel(input, b->get_name(), env);

    // Create body states
    BodyStates states;

    // Case 1: the body frame origin, the MMG frame origin and the CoG are identical
    states.G=ssc::kinematics::Point(b->get_name(),{0,0,0});
    ASSERT_EQ(0,propModel.get_CoG_position_in_MMG_frame(states).x());
    ASSERT_EQ(0,propModel.get_CoG_position_in_MMG_frame(states).y());
    ASSERT_EQ(0,propModel.get_CoG_position_in_MMG_frame(states).z());

    // Case 2: the body frame origin and the CoG are identical, the MMG frame is different
    input.application_point.x=-10;
    input.application_point.y=2;
    input.application_point.z=-5;
    const MMGPropellerForceModel propModel2(input, b->get_name(), env);
    ASSERT_EQ(10,propModel2.get_CoG_position_in_MMG_frame(states).x());
    ASSERT_EQ(-2,propModel2.get_CoG_position_in_MMG_frame(states).y());
    ASSERT_EQ(5,propModel2.get_CoG_position_in_MMG_frame(states).z());

    // Case 3: the body frame origin, the CoG are identical and the MMG frame origin are different
    states.G.v(0)=8;
    states.G.v(1)=5;
    states.G.v(2)=4;
    ASSERT_EQ(18,propModel2.get_CoG_position_in_MMG_frame(states).x());
    ASSERT_EQ(3,propModel2.get_CoG_position_in_MMG_frame(states).y());
    ASSERT_EQ(9,propModel2.get_CoG_position_in_MMG_frame(states).z());
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

    // Define a state with CoG at body origin
    BodyStates states;

    // Define a 30° drift angle with SOG= 2 m/s
    states.u.record(0, 1.7320508075688772);
    states.v.record(0, -1.);

    // FORMULA YASUKAWA : when C0=0, it is the formula (16) from Yasukawa & Yoshimura 2014 which is used

    // Case 1: If r=0 $\beta_P=\beta$=PI/6>0 and C2=1.6
    states.r.record(0, 0);
    double wake_fac=propModel.get_wake_factor(states);
    ASSERT_NEAR(1-0.65*(1+(1-exp(-PI/3))*0.6),wake_fac,EPS);
    
    // Case 2: If r=-PI/480 then $\beta_P$=0 (C2=1.6 but it is not used)
    states.r.record(0, -PI/480);
    wake_fac=propModel.get_wake_factor(states);
    ASSERT_NEAR(0.35,wake_fac,EPS);

    // Case 3: If r=PI/480 then $\beta_P$=PI/3>0 and C2=1.6
    states.r.record(0, PI/480);
    wake_fac=propModel.get_wake_factor(states);
    ASSERT_NEAR(1-0.65*(1+(1-exp(-2*PI/3))*0.6),wake_fac,EPS);

    // Case 4: If r=-PI/240 then $\beta_P$=-PI/6<0 and C2=1.1
    states.r.record(0, -PI/240);
    wake_fac=propModel.get_wake_factor(states);
    ASSERT_NEAR(1-0.65*(1+(1-exp(-PI/3))*0.1),wake_fac,EPS);

    // Case 5: Define a -30° drift angle with SOG= 2 m/s
    states.v.record(0, 1.);
    // If r=-PI/480 then $\beta_P$=-PI/3 and C2=1.1
    states.r.record(0, -PI/480);
    wake_fac=propModel.get_wake_factor(states);
    ASSERT_NEAR(1-0.65*(1+(1-exp(-2*PI/3))*0.1),wake_fac,EPS);

    // Case 6: Define a 90° drift angle so that the computed wake factor is negative and is therefore replaced by zero 
    states.u.record(0, 0);
    states.v.record(0, -2);
    states.r.record(0, 0);
    wake_fac=propModel.get_wake_factor(states);
    ASSERT_NEAR(0,wake_fac,EPS);

    // Case 7: Define a case with xG equivalent to the case 3
    states.u.record(0, 1.7320508075688772);
    states.v.record(0, -0.9);
    states.r.record(0, PI/480);
    states.G=ssc::kinematics::Point(b->get_name(),{48/PI,2,3});
    // In that case vm=-0.9-0.1=-1
    wake_fac=propModel.get_wake_factor(states);
    ASSERT_NEAR(1-0.65*(1+(1-exp(-2*PI/3))*0.6),wake_fac,EPS);

    // Case 8: Define a case with xG and z_G equivalent to case 7
    states.u.record(0, 1.7320508075688772);
    states.v.record(0, -0.9);
    states.r.record(0, PI/480);
    states.p.record(0, -PI/48);
    states.G=ssc::kinematics::Point(b->get_name(),{24/PI,2,2.4/PI});
    // In that case vm=-0.9-0.05-0.05=-1
    wake_fac=propModel.get_wake_factor(states);
    ASSERT_NEAR(1-0.65*(1+(1-exp(-2*(PI/3-12.6*PI/48/2)))*0.6),wake_fac,EPS);

    // Case 9: Define a case with body origin, MMG origin and CoG separated
    MMGPropellerForceModel::Yaml input=MMGPropellerForceModel::parse(test_data::MMGPropeller());
    // We define the body origin 10m forward from midship and 5m below waterline
    input.application_point.x=-10;
    input.application_point.z=5;
    // The propeller longitudinal location is then moved accordingly
    input.position_of_propeller_frame.coordinates.x=-170;
    input.position_of_propeller_frame.coordinates.z=17.6;
    // The CoG from case 7 is also moved accordingly
    states.G=ssc::kinematics::Point(b->get_name(),{24/PI-10,2,2.4/PI+5});
    // Create propeller model 
    MMGPropellerForceModel propModel3(input, b->get_name(), env);
    // In that case we still have vm=-0.9-0.05-0.05=-1
    wake_fac=propModel3.get_wake_factor(states);
    ASSERT_NEAR(1-0.65*(1+(1-exp(-2*(PI/3-12.6*PI/48/2)))*0.6),wake_fac,EPS);

    // FORMULA OKUDA 2019 : when C0=0, it is the formula from Okuda et. all 2019 which is used

    // Case 10: Verify Okuda formula 
    input=MMGPropellerForceModel::parse(test_data::MMGPropeller());
    input.C2[0] = 1;
    input.C2[1] = 1;
    input.C0 = -2.5;
    // Define a 30° drift angle with SOG= 2 m/s
    states.u.record(0, 1.7320508075688772);
    states.v.record(0, -1.);
    // No roll nor yaw motion
    states.p.record(0,0.);
    states.r.record(0,0.);
    // Create propeller model 
    MMGPropellerForceModel propModel4(input, b->get_name(), env);
    wake_fac=propModel4.get_wake_factor(states);
    ASSERT_NEAR(0.35*exp(-2.5*pow(PI/6,2)),wake_fac,EPS);
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
