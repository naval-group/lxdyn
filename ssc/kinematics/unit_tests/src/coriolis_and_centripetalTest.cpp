/*
 * coriolis_and_centripetalTest.cpp
 *
 * \date Jun 3, 2014
 * \author cec
 */

#include "coriolis_and_centripetalTest.hpp"
#include "ssc/kinematics/coriolis_and_centripetal.hpp"

#define EPS 1E-13

using namespace ssc::kinematics;

coriolis_and_centripetalTest::coriolis_and_centripetalTest() : a(ssc::random_data_generator::DataGenerator(45455))
{
}

coriolis_and_centripetalTest::~coriolis_and_centripetalTest()
{
}

void coriolis_and_centripetalTest::SetUp()
{
}

void coriolis_and_centripetalTest::TearDown()
{
}

TEST_F(coriolis_and_centripetalTest, if_omega_is_zero_force_should_be_zero)
{
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const Matrix6x6 M = Matrix6x6::Random();
        const Eigen::Vector3d uvw = Eigen::Vector3d::Random();
        const Eigen::Vector3d pqr = Eigen::Vector3d::Constant(0);
        const Wrench F = coriolis_and_centripetal(a.random<Point>(), M, uvw, pqr);
        ASSERT_NEAR(0., F.force(0), EPS);
        ASSERT_NEAR(0., F.force(1), EPS);
        ASSERT_NEAR(0., F.force(2), EPS);
    }
}

TEST_F(coriolis_and_centripetalTest, if_M_is_identity_torque_should_be_zero)
{
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const Matrix6x6 M = Matrix6x6::Identity();
        const Eigen::Vector3d uvw = Eigen::Vector3d::Random();
        const Eigen::Vector3d pqr = Eigen::Vector3d::Constant(0);
        const Wrench F = coriolis_and_centripetal(a.random<Point>(), M, uvw, pqr);
        ASSERT_NEAR(0., F.torque(0),EPS);
        ASSERT_NEAR(0., F.torque(1),EPS);
        ASSERT_NEAR(0., F.torque(2),EPS);
    }
}

TEST_F(coriolis_and_centripetalTest, if_M_is_just_ones_force_and_torque_have_simple_expressions)
{
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const Matrix6x6 M = Matrix6x6::Constant(1);
        const Eigen::Vector3d uvw = Eigen::Vector3d::Random();
        const Eigen::Vector3d pqr = Eigen::Vector3d::Random();
        const Wrench F = coriolis_and_centripetal(a.random<Point>(), M, uvw, pqr);
        const double u = uvw(0);
        const double v = uvw(1);
        const double w = uvw(2);
        const double p = pqr(0);
        const double q = pqr(1);
        const double r = pqr(2);
        const double u_v_w_p_q_r = u+v+w+p+q+r;
        ASSERT_NEAR(u_v_w_p_q_r*(r-q), F.force(0),EPS);
        ASSERT_NEAR(u_v_w_p_q_r*(p-r), F.force(1),EPS);
        ASSERT_NEAR(u_v_w_p_q_r*(q-p), F.force(2),EPS);
        ASSERT_NEAR(u_v_w_p_q_r*(w-v+r-q), F.torque(0),EPS);
        ASSERT_NEAR(u_v_w_p_q_r*(u-w+p-r), F.torque(1),EPS);
        ASSERT_NEAR(u_v_w_p_q_r*(v-u+q-p), F.torque(2),EPS);
    }
}
