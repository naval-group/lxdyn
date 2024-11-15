/*
 * MMGMMGRudderForceModelTest.cpp
 *
 *  Created on: Oct 11, 2024
 *      Author: fbickert
 */

#include "MMGRudderForceModelTest.hpp"
#include "xdyn/core/BodyBuilder.hpp"
#include "xdyn/force_models/MMGRudderForceModel.hpp"
#include "xdyn/test_data_generator/yaml_data.hpp"

#define _USE_MATH_DEFINE
#include <cmath>
#define PI M_PI

#define DEG2RAD (PI / 180.)

#define EPS 1E-6

#define BODY "body 1"

namespace ssc
{
    namespace random_data_generator
    {
        template <>
        MMGRudderForceModel::Yaml TypedScalarDataGenerator<MMGRudderForceModel::Yaml>::get() const
        {
            MMGRudderForceModel::Yaml ret;
            ret.Ar = random<double>();
            ret.b = random<double>();
            ret.xH_adim = random<double>();
            ret.lR_adim = random<double>();
            ret.tR = random<double>();
            ret.aH = random<double>();
            ret.gammaR = { random<double>(), random<double>() };
            ret.epsilon = random<double>();
            ret.kappaMmg = random<double>();
            ret.effective_aspect_ratio = random<double>();
            ret.position_of_the_rudder_frame_in_the_body_frame=YamlCoordinates(random<double>(),random<double>(),random<double>());

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
            ret.application_point = YamlCoordinates(random<double>(), random<double>(),random<double>());

            return ret;
        }
    }
}

MMGRudderForceModelTest::MMGRudderForceModelTest()
    : a(ssc::random_data_generator::DataGenerator(21212))
{
}

MMGRudderForceModelTest::~MMGRudderForceModelTest() {}

void MMGRudderForceModelTest::SetUp() {}

void MMGRudderForceModelTest::TearDown() {}

EnvironmentAndFrames MMGRudderForceModelTest::get_env()
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

TEST_F(MMGRudderForceModelTest, parser)
{
    /*
    The purpose of this test is to check that the input data are well parsed
    */

    const auto rudderModel_parser = MMGRudderForceModel::parse(test_data::MMGRudderAndPropeller());
    ASSERT_EQ("MMG port side propeller", rudderModel_parser.name);
    ASSERT_DOUBLE_EQ(-152.73, rudderModel_parser.position_of_propeller_frame.coordinates.x);
    ASSERT_DOUBLE_EQ(0, rudderModel_parser.position_of_propeller_frame.coordinates.y);
    ASSERT_DOUBLE_EQ(0, rudderModel_parser.position_of_propeller_frame.coordinates.z);
    ASSERT_EQ(BODY, rudderModel_parser.position_of_propeller_frame.frame);
    ASSERT_DOUBLE_EQ(0, rudderModel_parser.position_of_propeller_frame.angle.phi);
    ASSERT_DOUBLE_EQ(0, rudderModel_parser.position_of_propeller_frame.angle.theta);
    ASSERT_DOUBLE_EQ(0, rudderModel_parser.position_of_propeller_frame.angle.psi);
    ASSERT_DOUBLE_EQ(0.35, rudderModel_parser.wake_coefficient);
    ASSERT_DOUBLE_EQ(0.22, rudderModel_parser.thrust_deduction_factor);
    ASSERT_DOUBLE_EQ(9.86, rudderModel_parser.diameter);
    ASSERT_DOUBLE_EQ(0.2931, rudderModel_parser.k0);
    ASSERT_DOUBLE_EQ(-0.2753, rudderModel_parser.k1);
    ASSERT_DOUBLE_EQ(-0.1385, rudderModel_parser.k2);
    ASSERT_DOUBLE_EQ(2.0, rudderModel_parser.C1);
    ASSERT_DOUBLE_EQ(1.1, rudderModel_parser.C2[0]);
    ASSERT_DOUBLE_EQ(1.6, rudderModel_parser.C2[1]);
    ASSERT_DOUBLE_EQ(0, rudderModel_parser.application_point.x);
    ASSERT_DOUBLE_EQ(0, rudderModel_parser.application_point.y);
    ASSERT_DOUBLE_EQ(0, rudderModel_parser.application_point.z);
    ASSERT_DOUBLE_EQ(112.5, rudderModel_parser.Ar);
    ASSERT_DOUBLE_EQ(15.8, rudderModel_parser.b);
    ASSERT_DOUBLE_EQ(-0.464, rudderModel_parser.xH_adim);
    ASSERT_DOUBLE_EQ(-0.71, rudderModel_parser.lR_adim);
    ASSERT_DOUBLE_EQ(0.387, rudderModel_parser.tR);
    ASSERT_DOUBLE_EQ(0.312, rudderModel_parser.aH);
    ASSERT_DOUBLE_EQ(0.395, rudderModel_parser.gammaR[0]);
    ASSERT_DOUBLE_EQ(0.640, rudderModel_parser.gammaR[1]);
    ASSERT_DOUBLE_EQ(1.09, rudderModel_parser.epsilon);
    ASSERT_DOUBLE_EQ(0.5, rudderModel_parser.kappaMmg);
    ASSERT_DOUBLE_EQ(1.827, rudderModel_parser.effective_aspect_ratio);
    ASSERT_DOUBLE_EQ(-160, rudderModel_parser.position_of_the_rudder_frame_in_the_body_frame.x);
    ASSERT_DOUBLE_EQ(0, rudderModel_parser.position_of_the_rudder_frame_in_the_body_frame.y);
    ASSERT_DOUBLE_EQ(0, rudderModel_parser.position_of_the_rudder_frame_in_the_body_frame.z);
    ASSERT_DOUBLE_EQ(320, rudderModel_parser.Lpp);
}

TEST_F(MMGRudderForceModelTest, check_name)
{
    /*
    The purpose of this test is to check that the ForceModel name is correct.
    */

    // Create environnement
    EnvironmentAndFrames env = get_env();
    // Create MMGRudderForceModel object
    const MMGRudderForceModel rudderModel(a.random<MMGRudderForceModel::Yaml>(), "", env);
    ASSERT_EQ("MMG propeller+rudder", rudderModel.model_name());
}

TEST_F(MMGRudderForceModelTest, get_angle_of_attack)
{
    /*
    The purpose of this test is to check the non-regression of the function get_angle_of_attack
    (same test than in RudderForceModel).
    */

    const MMGRudderForceModel::RudderModel rudderModel(a.random<MMGRudderForceModel::Yaml>(),
                                                       a.random<double>());
    ASSERT_DOUBLE_EQ(-1, rudderModel.get_angle_of_attack(1, 2));
    ASSERT_DOUBLE_EQ(1, rudderModel.get_angle_of_attack(2, 1));
}

TEST_F(MMGRudderForceModelTest, get_fluid_angle)
{
    /*
    The purpose of this test is to check the non-regression of the function get_fluid_angle (same
    test than in RudderForceModel).
    */

    const MMGRudderForceModel::RudderModel rudderModel(a.random<MMGRudderForceModel::Yaml>(),
                                                       a.random<double>());
    ssc::kinematics::Point V;
    V.x() = 1;
    V.y() = 2;
    V.z() = a.random<double>();
    ASSERT_DOUBLE_EQ(1.1071487177940904, rudderModel.get_fluid_angle(V));
}

TEST_F(MMGRudderForceModelTest, get_Ar)
{
    /*
    The purpose of this test is to check the non-regression of the function get_Ar which computes
    the rudder area both inside and outside the propeller wake
    */

    const MMGRudderForceModel::RudderModel rudderModel(
        MMGRudderForceModel::parse(test_data::MMGRudderAndPropeller()), a.random<double>());
    ASSERT_DOUBLE_EQ(112.5, rudderModel.get_Ar());
}

TEST_F(MMGRudderForceModelTest, get_Fn)
{
    /*
    The purpose of this test is to check the non-regression of the function get_Fn which computes
    the gradient lift coefficient of the rudder force inside or outside the propeller wake
    */

    MMGRudderForceModel::Yaml parameters = a.random<MMGRudderForceModel::Yaml>();
    parameters.effective_aspect_ratio = 1.827;
    const MMGRudderForceModel::RudderModel rudderModel(parameters, 1025);
    ASSERT_DOUBLE_EQ(0.5 * 1025 * 100 * 100 * 2.7469977924944815 * 0.5,
                     rudderModel.get_Fn(100, 10, 30 * DEG2RAD));
}

TEST_F(MMGRudderForceModelTest, get_force_subfunction)
{
    /*
    The purpose of this test is to check the non-regression of the function
    RudderForceget::get_force which computes the total rudder tensor inside or outside the propeller
    wake, given a certain value of Fn and rudder angle.

    With a positive rudder angle, the rudder side force is orientated toward the negative y and thus
    the yaw moment is positive.
    */

    const MMGRudderForceModel::RudderModel rudderModel(
        MMGRudderForceModel::parse(test_data::MMGRudderAndPropeller()), a.random<double>());

    ssc::kinematics::Vector6d force = rudderModel.get_force(1000, 30 * DEG2RAD);

    ASSERT_NEAR(-(1 - 0.387) * 1000 * 0.5, force(0), EPS);
    ASSERT_NEAR(-(1 + 0.312) * 1000 * 0.8660254037844386, force(1), EPS);
    ASSERT_NEAR(0, force(2), EPS);
    ASSERT_NEAR(0, force(3), EPS);
    ASSERT_NEAR(0, force(4), EPS);
    ASSERT_NEAR(-1000 * 0.8660254037844386 * 0.312 * (160 - 148.48), force(5), EPS);
}

TEST_F(MMGRudderForceModelTest, get_vr_betaR_pos)
{
    /*
    The purpose of this test is to check the non-regression of the function get_vr which computes
    the effective lateral fluid velocity at the rudder location, with an effective leeway angle
    $\beta_R$ slightly larger than 0.
    */

    // Define a 10m/s ship speed with a +30° drift angle in MMG ref (i.e. v<0)
    BodyStates states;
    const double t = 0;
    states.u.record(t, 10 * 0.8660254037844386);
    states.v.record(t, -5);
    // Add a yaw velocity in rad/s
    states.r.record(t, -0.023); // $\beta_R$>0 <=> r>-0.023045720756967376

    const MMGRudderForceModel::RudderModel rudderModel(
        MMGRudderForceModel::parse(test_data::MMGRudderAndPropeller()), a.random<double>());

    double vr = rudderModel.get_vr(states.u(), states.v(), states.r());
    ASSERT_NEAR(10 * 0.64 * (PI / 6 - 0.71 * 0.023 * 320 / 10),
                     vr,EPS); // gamma_R=0.64 as $beta_R$>0
}

TEST_F(MMGRudderForceModelTest, get_vr_betaR_neg)
{
    /*
    The purpose of this test is to check the non-regression of the function get_vr which computes
    the effective lateral fluid velocity at the rudder location, with an effective leeway angle
    $\beta_R$ slightly smaller than 0.
    */

    // Define a 10m/s ship speed with a +30° drift angle in MMG ref (i.e. v<0)
    BodyStates states;
    const double t = 0;
    states.u.record(t, 10 * 0.8660254037844386);
    states.v.record(t, -5);
    // Add a yaw velocity in rad/s
    states.r.record(t, -0.025); // $\beta_R$>0 <=> r>-0.023045720756967376

    const MMGRudderForceModel::RudderModel rudderModel(
        MMGRudderForceModel::parse(test_data::MMGRudderAndPropeller()), a.random<double>());

    double vr = rudderModel.get_vr(states.u(), states.v(), states.r());
    ASSERT_NEAR(10 * 0.395 * (PI / 6 - 0.71 * 0.025 * 320 / 10),
                     vr,EPS); // gamma_R=0.395 as $beta_R$<0
}

TEST_F(MMGRudderForceModelTest, get_vs)
{
    /*
    The purpose of this test is to check the non-regression of the function get_vs which computes
    the fluid velocity at the rudder location both inside and outside the propeller wake
    */

    MMGRudderForceModel::Yaml parameters = a.random<MMGRudderForceModel::Yaml>();
    parameters.diameter = 1;
    parameters.b = 4;
    parameters.kappaMmg = 0.5;
    parameters.epsilon = 0.8;
    const MMGRudderForceModel::RudderModel rudderModel(parameters, 1025);
    const auto vs = rudderModel.get_vs(1.5, 10, 2);
    ASSERT_DOUBLE_EQ(
        8 * sqrt(0.25 * (1 + 0.5 * (sqrt(2.5) - 1)) * (1 + 0.5 * (sqrt(2.5) - 1)) + 0.75),
        vs.v.x());
    ASSERT_DOUBLE_EQ(2, vs.v.y());
    ASSERT_DOUBLE_EQ(0, vs.v.z());
}

TEST_F(MMGRudderForceModelTest, get_wrench_double)
{
    /*
    The purpose of this test is to check the non-regression of the function
    get_wrench which computes the rudder tensor inside or outside the propeller wake, given a rudder velocity, a rudder area and a rudder angle.
    */

    const MMGRudderForceModel::RudderModel rudderModel(MMGRudderForceModel::parse(test_data::MMGRudderAndPropeller()), 1025);
    // Define a 10 m/s fluid velocity with a -5° angle at rudder location
    Eigen::Vector3d Vrud_vec(10 * cos(5 * DEG2RAD), -10 * sin(5 * DEG2RAD), 0);
    const ssc::kinematics::Point Vrud("", Vrud_vec);
    /*
    With a 30° inflow angle of attack at the rudder location, a 100m2 rudder area and a 10m/s fluid velocity: Fn=7039181.843267109
    In order to have a 30° fluid angle of attack at the rudder location, we define a 25° rudder angle
    */
    ssc::kinematics::Vector6d force = rudderModel.get_wrench(25*DEG2RAD, Vrud, 100);
    ASSERT_NEAR(-(1 - 0.387) * 7039181.843267109*sin(25*DEG2RAD), force(0), EPS);
    ASSERT_NEAR(-(1 + 0.312) * 7039181.843267109 * cos(25*DEG2RAD), force(1), EPS);
    ASSERT_NEAR(0, force(2), EPS);
    ASSERT_NEAR(0, force(3), EPS);
    ASSERT_NEAR(0, force(4), EPS);
    ASSERT_NEAR(-7039181.843267109 * cos(25*DEG2RAD) * 0.312 * (160- 148.48), force(5), EPS);
}

TEST_F(MMGRudderForceModelTest, force_and_torque)
{
    /*
    This test checks the non-regression of the propeller + rudder tensor.
    */

    // Create environnement
    EnvironmentAndFrames env = get_env();
    // Create body
    BodyPtr b(BodyBuilder(env.rot).build(BODY, VectorOfVectorOfPoints(), 0, 0, env.rot, true));
    auto states = b->get_states();
    // Create rudder force model
    const MMGRudderForceModel rudderModel(MMGRudderForceModel::parse(test_data::MMGRudderAndPropeller()), b->get_name(), env);

    // Define body velocities
    const double t = 0;
    states.u.record(0, 8);
    states.v.record(0, 1);
    states.r.record(0, 0.01);
    // Create commands
    std::map<std::string, double> commands;
    commands["rpm"] = 70*2*PI/60; //70 rpm = 70*2*PI/60 rad/s
    commands["P/D"] = 1.2;//useless for MMG propeller but necessary for AbstractWageningen
    commands["beta"] = 10*DEG2RAD;

    const auto rudderTensor = rudderModel.get_force(states, t, env, commands);

    // We compare the two tensors which are both expressed in body frame
    ASSERT_NEAR(1068003.9043270403, rudderTensor.X(), EPS);
    ASSERT_NEAR(2868941.6534986021, rudderTensor.Y(), EPS);
    ASSERT_NEAR(0, rudderTensor.Z(), EPS);
    ASSERT_NEAR(0, rudderTensor.K(), EPS);
    ASSERT_NEAR(0, rudderTensor.M(), EPS);
    ASSERT_NEAR(-12997705.174082102, rudderTensor.N(), EPS);
}

TEST_F(MMGRudderForceModelTest, force_and_torque_rudder_alone)
{
    /*
    This test checks the non-regression of the rudder tensor independently from the propeller
    tensor.
    */
    
    MMGRudderForceModel::Yaml input
        = MMGRudderForceModel::parse(test_data::MMGRudderAndPropeller());
    // Create environnement
    EnvironmentAndFrames env = get_env();
    // Create body
    BodyPtr b(BodyBuilder(env.rot).build(BODY, VectorOfVectorOfPoints(), 0, 0, env.rot, true));
    auto states = b->get_states();
    // Create rudder force model
    const MMGRudderForceModel rudderModel(MMGRudderForceModel::parse(test_data::MMGRudderAndPropeller()), b->get_name(), env);

    // Define body velocities
    const double t = 0;
    states.u.record(0, 8);
    states.v.record(0, 1);
    states.r.record(0, 0.01);
    // Create commands
    std::map<std::string, double> commands;
    commands["beta"] = PI/6;

    // Define a propeller thrust with rho=1024, K_T=0.5, n=70 rpm, t=0.2 and D=9
    const double prop_thrust=0.8*1024*pow(70/60,2)*pow(9,4)*0.5;

    const auto rudderTensor = rudderModel.get_rudder_force(states, t, env, commands,prop_thrust);

    ASSERT_NEAR(-1698918.4794772132, rudderTensor(0), EPS);
    ASSERT_NEAR(6298056.1487378832, rudderTensor(1), EPS);
    ASSERT_NEAR(0, rudderTensor(2), EPS);
    ASSERT_NEAR(0, rudderTensor(3), EPS);
    ASSERT_NEAR(0, rudderTensor(4), EPS);
    ASSERT_NEAR(17253601.625030234, rudderTensor(5), EPS);
}

TEST_F(MMGRudderForceModelTest, force_and_torque_rudder_alone_with_xG)
{
    /*
    This test checks that the displacement of the point of application or CoG is well taken into account.
    */
    
    // Case 1: reference test with the body frame origin, the MMG frame origin and the CoG located at the same point
    MMGRudderForceModel::Yaml input
        = MMGRudderForceModel::parse(test_data::MMGRudderAndPropeller());
    // Create environnement
    EnvironmentAndFrames env = get_env();
    // Create body
    BodyPtr b(BodyBuilder(env.rot).build(BODY, VectorOfVectorOfPoints(), 0, 0, env.rot, true));
    auto states = b->get_states();
    // Create rudder force model
    const MMGRudderForceModel rudderModel(MMGRudderForceModel::parse(test_data::MMGRudderAndPropeller()), b->get_name(), env);
    // Define body velocities
    const double t = 0;
    states.u.record(0, 8);
    states.v.record(0, 1);
    states.r.record(0, 0.01);
    // Create commands
    std::map<std::string, double> commands;
    commands["beta"] = PI/6;
    // Define a propeller thrust with rho=1024, K_T=0.5, n=70 rpm, t=0.2 and D=9
    const double prop_thrust=0.8*1024*pow(70/60,2)*pow(9,4)*0.5;
    // Compute rudder force
    const auto rudderTensor_case1 = rudderModel.get_rudder_force(states, t, env, commands,prop_thrust);

    // Case 2: the CoG is moved 10m backward from the body frame origin (which is still the same than the MMG origin), so that xG=-10
    // To keep the same vm value that case 1, we need v=0.9 so that vm=0.9+10*0.01=1
    states.v.record(0, 0.9);
    states.G=ssc::kinematics::Point(b->get_name(),{-10,2,3});
    // Compute rudder force
    const auto rudderTensor_case2 = rudderModel.get_rudder_force(states, t, env, commands,prop_thrust);

    ASSERT_NEAR(rudderTensor_case2(0), rudderTensor_case1(0), EPS);
    ASSERT_NEAR(rudderTensor_case2(1), rudderTensor_case1(1), EPS);
    ASSERT_NEAR(rudderTensor_case2(2), rudderTensor_case1(2), EPS);
    ASSERT_NEAR(rudderTensor_case2(3), rudderTensor_case1(3), EPS);
    ASSERT_NEAR(rudderTensor_case2(4), rudderTensor_case1(4), EPS);
    ASSERT_NEAR(rudderTensor_case2(5), rudderTensor_case1(5), EPS);

    // Case 3: the CoG is moved 10m backward from the body frame origin and the MMG frame origin is moved 10m forward the body frame origin, so that xG=-20.
    input.position_of_propeller_frame.coordinates.x+=10;
    input.application_point.x+=10;
    // To keep the same vm value that case 1, we need v=0.8 so that vm=0.8+20*0.01=1
    states.v.record(0, 0.8);
    // Create a new rudder force model with new inputs
    const MMGRudderForceModel rudderModel_new(input, b->get_name(), env);
    // Compute rudder force
    const auto rudderTensor_case3 = rudderModel_new.get_rudder_force(states, t, env, commands,prop_thrust);

    ASSERT_NEAR(rudderTensor_case3(0), rudderTensor_case1(0), EPS);
    ASSERT_NEAR(rudderTensor_case3(1), rudderTensor_case1(1), EPS);
    ASSERT_NEAR(rudderTensor_case3(2), rudderTensor_case1(2), EPS);
    ASSERT_NEAR(rudderTensor_case3(3), rudderTensor_case1(3), EPS);
    ASSERT_NEAR(rudderTensor_case3(4), rudderTensor_case1(4), EPS);
    ASSERT_NEAR(rudderTensor_case3(5), rudderTensor_case1(5), EPS);
}

TEST_F(MMGRudderForceModelTest, force_and_torque_with_phi)
{
    /*
        This test checks that the rudder and propeller tensor is unchanged if the propeller frame is
       rotated by an angle phi around its x-axis.
    */

    MMGRudderForceModel::Yaml input
        = MMGRudderForceModel::parse(test_data::MMGRudderAndPropeller());
    // Create environnement
    EnvironmentAndFrames env = get_env();
    // Create body
    BodyPtr b(BodyBuilder(env.rot).build(BODY, VectorOfVectorOfPoints(), 0, 0, env.rot, true));
    auto states = b->get_states();
    // Create rudder force model without phi angle
    const MMGRudderForceModel rudder_nophi(input, b->get_name(), env);
    // Create rudder force model with phi angle
    input.position_of_propeller_frame.angle.phi = PI / 2;
    const MMGRudderForceModel rudder_phi(input, b->get_name(), env);

    // Define body velocities
    const double t = 24;
    states.u.record(t, 3);
    states.v.record(t, 4);
    states.w.record(t, 5);

    // Create commands
    std::map<std::string, double> commands;
    commands["rpm"] = 200;
    commands["P/D"] = 1.2;
    commands["beta"] = PI / 6;

    const auto F_nophi = rudder_nophi.get_force(states, t, env, commands);
    const auto F_phi = rudder_phi.get_force(states, t, env, commands);

    // We compare the two tensors which are both expressed in body frame
    ASSERT_NEAR(F_phi.X(), F_nophi.X(), EPS);
    ASSERT_NEAR(F_phi.Y(), F_nophi.Y(), EPS);
    ASSERT_NEAR(F_phi.Z(), F_nophi.Z(), EPS);
    ASSERT_NEAR(F_phi.K(), F_nophi.K(), EPS);
    ASSERT_NEAR(F_phi.M(), F_nophi.M(), EPS);
    ASSERT_NEAR(F_phi.N(), F_nophi.N(), EPS);
}