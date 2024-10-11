/*
 * MMGMMGRudderForceModelTest.cpp
 *
 *  Created on: Oct 11, 2024
 *      Author: fbickert
 */

#include "MMGRudderForceModelTest.hpp"
#include "xdyn/force_models/MMGRudderForceModel.hpp"
#include "env_for_tests.hpp"
#include "xdyn/test_data_generator/yaml_data.hpp"
#include "xdyn/core/BodyBuilder.hpp"

#define _USE_MATH_DEFINE
#include <cmath>
#define PI M_PI

#define DEG2RAD (PI/180.)

#define EPS 1E-6

#define BODY "body 1"

namespace ssc
{
    namespace random_data_generator
    {
        template <> MMGRudderForceModel::Yaml TypedScalarDataGenerator<MMGRudderForceModel::Yaml>::get() const
        {
            MMGRudderForceModel::Yaml ret;
            ret.Ar = random<double>().between(0,1e5);
            ret.b = random<double>();
            ret.xH = random<double>();
            ret.lR = random<double>();
            ret.tR = random<double>();
            ret.aH = random<double>();
            ret.gammaR[0] = random<double>();
            ret.gammaR[1] = random<double>();
            ret.epsilon = random<double>();
            ret.kappaMmg = random<double>();
            ret.effective_aspect_ratio = random<double>();

            return ret;
        }
    }
}

MMGRudderForceModelTest::MMGRudderForceModelTest():  a(ssc::random_data_generator::DataGenerator(21212))
{
}

MMGRudderForceModelTest::~MMGRudderForceModelTest()
{
}

void MMGRudderForceModelTest::SetUp()
{
}

void MMGRudderForceModelTest::TearDown()
{
}

EnvironmentAndFrames MMGRudderForceModelTest::get_env()
{
    EnvironmentAndFrames env;
    env.rho = 1024;
    env.g = 9.81;
    env.rot = YamlRotation("angle", {"z","y'","x''"});
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
    ASSERT_DOUBLE_EQ(152.73, rudderModel_parser.position_of_propeller_frame.coordinates.x);
    ASSERT_DOUBLE_EQ(0, rudderModel_parser.position_of_propeller_frame.coordinates.y);
    ASSERT_DOUBLE_EQ(15, rudderModel_parser.position_of_propeller_frame.coordinates.z);
    ASSERT_EQ(BODY, rudderModel_parser.position_of_propeller_frame.frame);
    ASSERT_DOUBLE_EQ(0, rudderModel_parser.position_of_propeller_frame.angle.phi);
    ASSERT_DOUBLE_EQ(0, rudderModel_parser.position_of_propeller_frame.angle.theta);
    ASSERT_DOUBLE_EQ(0, rudderModel_parser.position_of_propeller_frame.angle.psi);
    ASSERT_DOUBLE_EQ(0.35, rudderModel_parser.wake_coefficient);
    ASSERT_DOUBLE_EQ(1, rudderModel_parser.relative_rotative_efficiency);
    ASSERT_DOUBLE_EQ(0.22, rudderModel_parser.thrust_deduction_factor);
    ASSERT_DOUBLE_EQ(9.86, rudderModel_parser.diameter);
    ASSERT_DOUBLE_EQ(0.2931, rudderModel_parser.k0);
    ASSERT_DOUBLE_EQ(-0.2753, rudderModel_parser.k1);
    ASSERT_DOUBLE_EQ(-0.1385, rudderModel_parser.k2);
    ASSERT_DOUBLE_EQ(112.5, rudderModel_parser.Ar);
    ASSERT_DOUBLE_EQ(15.8,rudderModel_parser.b);
    ASSERT_DOUBLE_EQ(-148.48,rudderModel_parser.xH);
    ASSERT_DOUBLE_EQ(-227.2,rudderModel_parser.lR);
    ASSERT_DOUBLE_EQ(0.387,rudderModel_parser.tR);
    ASSERT_DOUBLE_EQ(0.312,rudderModel_parser.aH);
    ASSERT_DOUBLE_EQ(0.395,rudderModel_parser.gammaR[0]);
    ASSERT_DOUBLE_EQ(0.640,rudderModel_parser.gammaR[1]);
    ASSERT_DOUBLE_EQ(1.09,rudderModel_parser.epsilon);
    ASSERT_DOUBLE_EQ(0.5,rudderModel_parser.kappaMmg);
    ASSERT_DOUBLE_EQ(8.652,rudderModel_parser.effective_aspect_ratio);
    ASSERT_DOUBLE_EQ(-171.1, rudderModel_parser.position_of_the_rudder_frame_in_the_body_frame.x);
    ASSERT_DOUBLE_EQ(0, rudderModel_parser.position_of_the_rudder_frame_in_the_body_frame.y);
    ASSERT_DOUBLE_EQ(12.6, rudderModel_parser.position_of_the_rudder_frame_in_the_body_frame.z);
}

TEST_F(MMGRudderForceModelTest, get_angle_of_attack)
{
    /*
    The purpose of this test is to check the non-regression of the function get_angle_of_attack (same test than in RudderForceModel).
    */
        MMGRudderForceModel::RudderModel rudderModel(a.random<MMGRudderForceModel::Yaml>(),a.random<double>());
        ASSERT_DOUBLE_EQ(-1,rudderModel.get_angle_of_attack(1, 2));
        ASSERT_DOUBLE_EQ(1,rudderModel.get_angle_of_attack(2, 1));
}

TEST_F(MMGRudderForceModelTest, get_fluid_angle)
{
    /*
    The purpose of this test is to check the non-regression of the function get_fluid_angle (same test than in RudderForceModel).
    */
    const MMGRudderForceModel::RudderModel rudderModel(a.random<MMGRudderForceModel::Yaml>(),a.random<double>());
    MMGRudderForceModel::InOutWake<ssc::kinematics::Point> V;
    V.in_wake.x() = 1;
    V.in_wake.y() = 2;
    V.in_wake.z() = a.random<double>();
    V.outside_wake.x() = -4;
    V.outside_wake.y() = -4;
    V.outside_wake.z() = a.random<double>();
    ASSERT_DOUBLE_EQ(1.1071487177940904, rudderModel.get_fluid_angle(V.in_wake));
    ASSERT_DOUBLE_EQ(-3*PI/4, rudderModel.get_fluid_angle(V.outside_wake));
}

TEST_F(MMGRudderForceModelTest, get_Ar)
{
    /*
    The purpose of this test is to check the non-regression of the function get_Ar
    */
    const MMGRudderForceModel::RudderModel rudderModel(MMGRudderForceModel::parse(test_data::MMGRudderAndPropeller()),a.random<double>());
    ASSERT_DOUBLE_EQ(9.86/15.8*112.5, rudderModel.get_Ar().in_wake);
    ASSERT_DOUBLE_EQ(5.94/15.8*112.5, rudderModel.get_Ar().outside_wake);

}


TEST_F(MMGRudderForceModelTest, get_Fn)
{
    /*
    The purpose of this test is to check the non-regression of the function get_Fn
    */
    const MMGRudderForceModel::RudderModel rudderModel(MMGRudderForceModel::parse(test_data::MMGRudderAndPropeller()),a.random<double>());
    ASSERT_DOUBLE_EQ(0.5*1025*100*100*4.864865162355531*0.5,rudderModel.get_Fn(1025,100,10,30*DEG2RAD));
}

TEST_F(MMGRudderForceModelTest, get_force)
{

}

TEST_F(MMGRudderForceModelTest, get_wrench)
{

}


TEST_F(MMGRudderForceModelTest, get_Vs)
{

}

TEST_F(MMGRudderForceModelTest, force_and_torque)
{
    /*
    This test checks the non-regression of the propeller + rudder tensor.
    */
}

TEST_F(MMGRudderForceModelTest, force_and_torque_rudder_alone)
{
    /*
    This test checks the non-regression of the rudder tensor independently from the propeller tensor.
    */
}

TEST_F(MMGRudderForceModelTest, force_and_torque_with_phi)
{
/*
    This test checks that the rudder and propeller tensor is unchanged if the propeller frame is rotated by an angle phi around its x-axis.
    */

    MMGRudderForceModel::Yaml input=MMGRudderForceModel::parse(test_data::MMGRudderAndPropeller());
    // Create environnement
    EnvironmentAndFrames env = get_env();
    // Create body
    BodyPtr b(BodyBuilder(env.rot).build(BODY, VectorOfVectorOfPoints(), 0, 0, env.rot, true));
    auto states = b->get_states();
    // Create rudder force model without phi angle
    const MMGRudderForceModel rudder_nophi(input, b->get_name(), env);
    // Create rudder force model with phi angle
    input.position_of_propeller_frame.angle.phi=PI/2;
    const MMGRudderForceModel rudder_phi(input, b->get_name(), env);

    // Define body velocities
    const double t = 24;
    states.u.record(t, 3);
    states.v.record(t, 4);
    states.w.record(t, 5);

    // Create commands
    std::map<std::string,double> commands;
    commands["rpm"] = 200;
    commands["P/D"] = 1.2;
    commands["beta"] = PI/6;

    const auto F_nophi = rudder_nophi.get_force(states, t, env, commands);
    const auto F_phi = rudder_phi.get_force(states, t, env, commands);

    // We compare the two tensors which are both expressed in body frame
    ASSERT_NEAR(F_phi.X(), F_nophi.X(),EPS);
    ASSERT_NEAR(F_phi.Y(), F_nophi.Y(),EPS);
    ASSERT_NEAR(F_phi.Z(), F_nophi.Z(),EPS);
    ASSERT_NEAR(F_phi.K(), F_nophi.K(),EPS);
    ASSERT_NEAR(F_phi.M(), F_nophi.M(),EPS);
    ASSERT_NEAR(F_phi.N(), F_nophi.N(),EPS);
}