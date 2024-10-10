/*
 * RudderForceModelTest.cpp
 *
 *  Created on: Feb 2, 2015
 *      Author: cady
 */
#include "RudderForceModelTest.hpp"
#include "xdyn/force_models/RudderForceModel.hpp"
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
        template <> RudderForceModel::Yaml TypedScalarDataGenerator<RudderForceModel::Yaml>::get() const
        {
            RudderForceModel::Yaml ret;
            ret.Ar = random<double>().between(0,1e5);
            ret.b = random<double>();
            ret.blade_area_ratio = random<double>();
            ret.diameter = random<double>();
            ret.drag_coeff = random<double>();
            ret.effective_aspect_ratio_factor = random<double>();
            ret.lift_coeff = random<double>();
            return ret;
        }
    }
}

RudderForceModelTest::RudderForceModelTest() : a(ssc::random_data_generator::DataGenerator(21212))
{
}

RudderForceModelTest::~RudderForceModelTest()
{
}

void RudderForceModelTest::SetUp()
{
}

void RudderForceModelTest::TearDown()
{
}

EnvironmentAndFrames RudderForceModelTest::get_env()
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

TEST_F(RudderForceModelTest, angle_of_attack)
{
//! [RudderForceModelTest get_alpha_example]
    RudderForceModel::RudderModel riw(a.random<RudderForceModel::Yaml>(),a.random<double>(),a.random<double>());
//! [RudderForceModelTest get_alpha_example]
//! [RudderForceModelTest get_alpha_example output]
    ASSERT_DOUBLE_EQ(-1,riw.get_angle_of_attack(1, 2));
    ASSERT_DOUBLE_EQ(1,riw.get_angle_of_attack(2, 1));
//! [RudderForceModelTest get_alpha_example output]
}

TEST_F(RudderForceModelTest, get_Cd)
{
    RudderForceModel::Yaml parameters = a.random<RudderForceModel::Yaml>();
    parameters.Ar = 3;
    parameters.b = 4;
    parameters.effective_aspect_ratio_factor = 0.5;
    RudderForceModel::RudderModel riw(parameters,1024,2);

    const double Vs = 6;
    const double Cl = 7;
    ASSERT_DOUBLE_EQ(6.5028916553907399, riw.get_Cd(Vs, Cl));
}

TEST_F(RudderForceModelTest, get_Cl)
{
    RudderForceModel::Yaml parameters = a.random<RudderForceModel::Yaml>();
    parameters.Ar = 3;
    parameters.b = 4;
    parameters.effective_aspect_ratio_factor = 0.5;
    RudderForceModel::RudderModel riw(parameters,1024,2);
    const double alpha = 0.4;
    ASSERT_DOUBLE_EQ(1.0985577009852809, riw.get_Cl(alpha));
}

TEST_F(RudderForceModelTest, get_lift)
{
    RudderForceModel::Yaml parameters = a.random<RudderForceModel::Yaml>();
    parameters.Ar = 10;
    parameters.lift_coeff = 2;
    RudderForceModel::RudderModel riw(parameters,1024,a.random<double>());

    const double Vs = 12;
    const double Cl = 1.3;
    const double alpha = PI/4;
    const double Ar = 1.4;
    ASSERT_DOUBLE_EQ(189766.19029849127, riw.get_lift(Vs,Cl,alpha,Ar));
}

TEST_F(RudderForceModelTest, get_drag)
{
    RudderForceModel::Yaml parameters = a.random<RudderForceModel::Yaml>();
    parameters.Ar = 10;
    parameters.drag_coeff = 2;
    RudderForceModel::RudderModel riw(parameters,1024,a.random<double>());

    const double Vs = 12;
    const double Cl = 1.3;
    const double Ar = 1.4;
    ASSERT_DOUBLE_EQ(268369.91999999998, riw.get_drag(Vs,Cl,Ar));
}

TEST_F(RudderForceModelTest, get_force)
{
    RudderForceModel::RudderModel riw(a.random<RudderForceModel::Yaml>(),a.random<double>(),a.random<double>());
    const double drag = 10;
    const double lift = 200;
    const double angle = -PI/3;
    ssc::kinematics::Vector6d v = riw.get_force(drag, lift, angle);
    ASSERT_DOUBLE_EQ(-91.339745962155646, v(0));
    ASSERT_DOUBLE_EQ(178.20508075688772, v(1));
    ASSERT_DOUBLE_EQ(0, v(2));
    ASSERT_DOUBLE_EQ(0, v(3));
    ASSERT_DOUBLE_EQ(0, v(4));
    ASSERT_DOUBLE_EQ(0, v(5));
}

TEST_F(RudderForceModelTest, get_wrench)
{
    RudderForceModel::Yaml parameters;
    parameters.Ar = 10;
    parameters.drag_coeff = 2;
    parameters.lift_coeff = 5;
    parameters.b = 99;
    parameters.effective_aspect_ratio_factor = 2.3;
    RudderForceModel::RudderModel riw(parameters,1024,0.75);
    const double area = 1.467;
    ssc::kinematics::Vector6d v = riw.get_wrench(3,4,0.5,area);
    ASSERT_DOUBLE_EQ(-2021.4412785509464, v(0));
    ASSERT_DOUBLE_EQ(1757.2988992064641, v(1));
    ASSERT_DOUBLE_EQ(0, v(2));
    ASSERT_DOUBLE_EQ(0, v(3));
    ASSERT_DOUBLE_EQ(0, v(4));
    ASSERT_DOUBLE_EQ(0, v(5));
}

TEST_F(RudderForceModelTest, get_Ar)
{
    RudderForceModel::Yaml parameters = a.random<RudderForceModel::Yaml>();
    RudderForceModel::RudderModel riw(parameters,a.random<double>(),a.random<double>());
    const double CTh = a.random<double>().between(-1,1000);
    const auto ar = riw.get_Ar(CTh);
    //ar.outside_wake = parameters.Ar-ar.in_wake;
    ASSERT_SMALL_RELATIVE_ERROR(parameters.Ar, ar.in_wake+ar.outside_wake, 1E-10);
}

TEST_F(RudderForceModelTest, get_Ar2)
{
    RudderForceModel::Yaml parameters = a.random<RudderForceModel::Yaml>();
    parameters.Ar = 10;
    parameters.b = 4;
    parameters.position_of_propeller_frame.coordinates.x = 0;
    parameters.position_of_propeller_frame.coordinates.y = 0;
    parameters.position_of_propeller_frame.coordinates.z = 0;
    parameters.position_of_the_rudder_frame_in_the_body_frame.x = 2.47;
    parameters.position_of_the_rudder_frame_in_the_body_frame.y = 0;
    parameters.position_of_the_rudder_frame_in_the_body_frame.z = 0;
    parameters.diameter = 1.67;
    const double CTh = 5.3;
    RudderForceModel::RudderModel riw(parameters,a.random<double>(),a.random<double>());
    const auto ar = riw.get_Ar(CTh);
    ASSERT_DOUBLE_EQ(3.5404447215261827, ar.in_wake);
    ASSERT_DOUBLE_EQ(6.4595552784738173, ar.outside_wake);
}

TEST_F(RudderForceModelTest, get_Vs)
{
    RudderForceModel::Yaml parameters = a.random<RudderForceModel::Yaml>();
    parameters.diameter = 3.6;
    RudderForceModel::RudderModel riw(parameters,1024,a.random<double>());
    const auto vs = riw.get_vs(1.5,12,6);
    ASSERT_DOUBLE_EQ(14.250875681004253, vs.in_wake.v.norm());
    ASSERT_DOUBLE_EQ(13.416407864998739, vs.outside_wake.v.norm());
}

TEST_F(RudderForceModelTest, get_fluid_angle)
{
    const RudderForceModel::RudderModel riw(a.random<RudderForceModel::Yaml>(),a.random<double>(),a.random<double>());
    RudderForceModel::InOutWake<ssc::kinematics::Point> V;
    V.in_wake.x() = 1;
    V.in_wake.y() = 2;
    V.in_wake.z() = a.random<double>();
    V.outside_wake.x() = -4;
    V.outside_wake.y() = -4;
    V.outside_wake.z() = a.random<double>();
    const auto vs = riw.get_fluid_angle(V);
    ASSERT_DOUBLE_EQ(1.1071487177940904, vs.in_wake);
    ASSERT_DOUBLE_EQ(-3*PI/4, vs.outside_wake);
}

TEST_F(RudderForceModelTest, parser)
{
    const auto w = RudderForceModel::parse(test_data::rudder());
    ASSERT_DOUBLE_EQ(0.5, w.blade_area_ratio);
    ASSERT_EQ("port side propeller", w.name);
    ASSERT_EQ(3, w.number_of_blades);
    ASSERT_DOUBLE_EQ(0, w.position_of_propeller_frame.angle.phi);
    ASSERT_DOUBLE_EQ(-10*DEG2RAD, w.position_of_propeller_frame.angle.theta);
    ASSERT_DOUBLE_EQ(-1*DEG2RAD, w.position_of_propeller_frame.angle.psi);
    ASSERT_DOUBLE_EQ(-4, w.position_of_propeller_frame.coordinates.x);
    ASSERT_DOUBLE_EQ(-2, w.position_of_propeller_frame.coordinates.y);
    ASSERT_DOUBLE_EQ(2, w.position_of_propeller_frame.coordinates.z);
    ASSERT_EQ("mesh(body 1)", w.position_of_propeller_frame.frame);
    ASSERT_DOUBLE_EQ(1, w.relative_rotative_efficiency);
    ASSERT_TRUE(w.rotating_clockwise);
    ASSERT_DOUBLE_EQ(0.7, w.thrust_deduction_factor);
    ASSERT_DOUBLE_EQ(0.9, w.wake_coefficient);
    ASSERT_DOUBLE_EQ(2, w.diameter);
    ASSERT_DOUBLE_EQ(2.2, w.Ar);
    ASSERT_DOUBLE_EQ(2,w.b);
    ASSERT_DOUBLE_EQ(1,w.drag_coeff);
    ASSERT_DOUBLE_EQ(2.1,w.lift_coeff);
    ASSERT_DOUBLE_EQ(1.7,w.effective_aspect_ratio_factor);
    ASSERT_DOUBLE_EQ(-5.1, w.position_of_the_rudder_frame_in_the_body_frame.x);
    ASSERT_DOUBLE_EQ(-2, w.position_of_the_rudder_frame_in_the_body_frame.y);
    ASSERT_DOUBLE_EQ(2, w.position_of_the_rudder_frame_in_the_body_frame.z);
}

TEST_F(RudderForceModelTest, force_and_torque)
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
    const RudderForceModel rudder(RudderForceModel::parse(test_data::rudder()), b->get_name(), env);
    
    // Check model name
    ASSERT_EQ("propeller+rudder", rudder.model_name());
    
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

    const auto F = rudder.get_force(states, t, env, commands);

    ASSERT_DOUBLE_EQ(1991600.7989408118, F.X());
    ASSERT_DOUBLE_EQ(1835490.3295199818, F.Y());
    ASSERT_DOUBLE_EQ(414940.19127081765, F.Z());
    ASSERT_DOUBLE_EQ(-2750648.9323577448, F.K());
    ASSERT_DOUBLE_EQ(48012.755709036814, F.M());
    ASSERT_DOUBLE_EQ(-2549303.5889394642, F.N());
}

TEST_F(RudderForceModelTest, force_and_torque_rudder_alone)
{
    /*
    This test checks the non-regression of the rudder tensor independently from the propeller tensor.
    */

    // Create environnement
    EnvironmentAndFrames env = get_env();
    // Create body
    BodyPtr b(BodyBuilder(env.rot).build(BODY, VectorOfVectorOfPoints(), 0, 0, env.rot, true));
    auto states = b->get_states();
    // Create rudder force model
    const RudderForceModel rudder(RudderForceModel::parse(test_data::rudder()), b->get_name(), env);

    // Define body velocities
    const double t = 24;
    states.u.record(t, 3);
    states.v.record(t, 4);
    states.w.record(t, 5);

    // Create commands
    std::map<std::string,double> commands;
    commands["rpm"] = 200;//defaut unit is rad/s
    commands["P/D"] = 1.2;
    commands["beta"] = PI/6;
    const double prop_thrust=1024*(10000/PI/PI)*16*0.479798653;
    const double cos_theta_cos_psi = 0.9846577620214009;// with theta=-10° and psi=-1° from input data

    const auto rudder_force = rudder.get_rudder_force(states, t, env, commands, prop_thrust * cos_theta_cos_psi);// rudder forces and moments in propeller frame expressed at the propeller location

    ASSERT_DOUBLE_EQ(-361272.67620997917, rudder_force(0));
    ASSERT_DOUBLE_EQ(1876488.4373155038, rudder_force(1));
    ASSERT_DOUBLE_EQ(0, rudder_force(2));
    ASSERT_DOUBLE_EQ(0, rudder_force(3));
    ASSERT_DOUBLE_EQ(0, rudder_force(4));
    ASSERT_DOUBLE_EQ(-1.1*1876488.4373155038, rudder_force(5));//there is -1.1m from the propeller to the rudder
}

TEST_F(RudderForceModelTest, force_and_torque_with_phi_angle)
{
    /*
    This test checks that the rudder and propeller tensor is unchanged if the propeller frame is rotated by an angle phi around its x-axis.
    */

    std::string input=test_data::rudder();
    // Create environnement
    EnvironmentAndFrames env = get_env();
    // Create body
    BodyPtr b(BodyBuilder(env.rot).build(BODY, VectorOfVectorOfPoints(), 0, 0, env.rot, true));
    auto states = b->get_states();
    // Create rudder force model without phi angle
    const RudderForceModel rudder_nophi(RudderForceModel::parse(input), b->get_name(), env);
    // Create rudder force model with phi angle
    std::string old_str="    phi: {value:  0";
    std::string new_str="    phi: {value: 90";
    input.replace(input.find(old_str), old_str.length(), new_str);
    const RudderForceModel rudder_phi(RudderForceModel::parse(input), b->get_name(), env);

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
