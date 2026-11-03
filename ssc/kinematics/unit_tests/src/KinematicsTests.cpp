/*
 * KinematicsTests.cpp
 *
 * \date 1 avr. 2014
 * \author cec
 */

#include "KinematicsTests.hpp"
#include "ssc/kinematics/Kinematics.hpp"
#include "random_kinematics.hpp"
#include "ssc/kinematics/KinematicsException.hpp"
#include "ssc/kinematics/rotation_matrix_builders.hpp"
#include "transform_double_equal.hpp"

#define EPS 1E-13

using namespace ssc::kinematics;

KinematicsTests::KinematicsTests() : a(ssc::random_data_generator::DataGenerator(122))
{
}

KinematicsTests::~KinematicsTests()
{
}

void KinematicsTests::SetUp()
{
}

void KinematicsTests::TearDown()
{
}

TEST_F(KinematicsTests, can_add_a_transform_to_a_kinematics_object)
{
    ssc::kinematics::Kinematics k;
    const auto bTa = random_transform(a, a.random<std::string>(), a.random<std::string>());
    k.add(bTa);
}

TEST_F(KinematicsTests, can_retrieve_a_transform)
{
    ssc::kinematics::Kinematics k;
    const std::string from_frame = a.random<std::string>();
    const std::string to_frame = a.random<std::string>();
    const auto bTa = random_transform(a, from_frame, to_frame);
    k.add(bTa);
    const auto transform = k.get(from_frame, to_frame);
    ASSERT_TRUE(double_equal(transform, bTa));
}

TEST_F(KinematicsTests, can_retrieve_inverse_transform)
{
    //! [KinematicsTests get_example]
    ssc::kinematics::Kinematics k;
    const std::string from_frame = a.random<std::string>();
    const std::string to_frame = a.random<std::string>();
    const auto aTb = random_transform(a, from_frame, to_frame);
    k.add(aTb);
    const auto bTa = k.get(to_frame, from_frame);
    //! [KinematicsTests get_example]
    //! [KinematicsTests get_example output]
    ASSERT_TRUE(double_equal(identity(from_frame), aTb*bTa, EPS));
    ASSERT_TRUE(double_equal(identity(to_frame), bTa*aTb, EPS));
    //! [KinematicsTests get_example output]
}

TEST_F(KinematicsTests, throws_if_transform_is_not_computable)
{
    ssc::kinematics::Kinematics k;
    ASSERT_THROW(k.get(a.random<std::string>(),a.random<std::string>()), KinematicsException);
}

TEST_F(KinematicsTests, can_compute_a_transformation_if_necessary_and_feasible)
{
    ssc::kinematics::Kinematics k;

    const auto aTb = random_transform(a, "A", "B");
    const auto aTc = random_transform(a, "A", "C");
    const auto bTd = random_transform(a, "B", "D");
    const auto cTe = random_transform(a, "C", "E");
    const auto cTf = random_transform(a, "C", "F");
    const auto dTg = random_transform(a, "D", "G");
    const auto dTh = random_transform(a, "D", "H");
    const auto dTi = random_transform(a, "D", "I");
    const auto eTd = random_transform(a, "E", "D");

    k.add(aTb);
    k.add(aTc);
    k.add(bTd);
    k.add(cTe);
    k.add(cTf);
    k.add(dTg);
    k.add(dTh);
    k.add(dTi);
    k.add(eTd);

    const auto eTh = k.get("E", "H");

    ASSERT_TRUE(double_equal(eTh, eTd * dTh, EPS));
}

TEST_F(KinematicsTests, can_compute_the_inverse_of_a_composite_transform)
{
    ssc::kinematics::Kinematics k;

    const auto aTb = random_transform(a, "A", "B");
    const auto bTc = random_transform(a, "B", "C");
    k.add(aTb);
    k.add(bTc);

    k.get("A", "C"); // This will build the composite transform A->C
    ASSERT_NO_THROW(k.get("C", "A"));

    const auto cTa = k.get("C", "A");
    ASSERT_TRUE(double_equal(identity("A"), aTb*bTc*cTa, EPS));
}

TEST_F(KinematicsTests, can_compute_identity)
{
    ssc::kinematics::Kinematics k;
    k.add(random_transform(a, "A", "B"));
    const auto aTa = k.get("A", "A");
    ASSERT_TRUE(double_equal(identity("A"), aTa, EPS));
}

TEST_F(KinematicsTests, can_add_same_transform_several_times)
{
    ssc::kinematics::Kinematics k;
    const auto aTb = random_transform(a, "A", "B");
    k.add(aTb);
    k.add(aTb);
    k.add(aTb);
    k.add(aTb);
}

TEST_F(KinematicsTests, should_throw_when_computing_transform_between_two_disjoint_frames)
{
    ssc::kinematics::Kinematics k;
    k.add(random_transform(a, "A", "B"));
    k.add(random_transform(a, "B", "C"));
    k.add(random_transform(a, "E", "D"));
    ASSERT_THROW(k.get("A", "E"), KinematicsException);
}

TEST_F(KinematicsTests, should_throw_when_adding_tranform_from_a_frame_to_itself) // Bug detected in IRT simulator
{
    ssc::kinematics::Kinematics k;
    const auto aTa = random_transform(a, "A", "A");
    ASSERT_THROW(k.add(aTa), KinematicsException);
}


TEST_F(KinematicsTests, can_compose_two_transformations)
{
    ssc::kinematics::Kinematics k;
    const double pi = 3.141592653589793238462643383279502884;
    const RotationMatrix rot_z_45deg = ssc::kinematics::rot(0, 0, 1, pi / 4);
    const auto aTb = Transform(Point("A", 3, 5, 7), "B");
    const auto bTc = Transform(Point("B", 0, 0, 0), rot_z_45deg, "C");
    const auto aTc_expected = Transform(Point("A", 3, 5, 7), rot_z_45deg, "C");
    k.add(aTb);
    k.add(bTc);
    auto aTc = k.get("A", "C");
    ASSERT_TRUE(double_equal(aTc_expected, aTc, EPS));
    aTc.swap();

    const auto origin_of_c_in_a = aTc * Point("C");
    ASSERT_EQ("A", origin_of_c_in_a.get_frame());
    ASSERT_NEAR(3, origin_of_c_in_a.x(), EPS);
    ASSERT_NEAR(5, origin_of_c_in_a.y(), EPS);
    ASSERT_NEAR(7, origin_of_c_in_a.z(), EPS);

    const auto point_x1_of_c_in_a = aTc * Point("C", 1, 0, 0);
    ASSERT_EQ("A", point_x1_of_c_in_a.get_frame());
    ASSERT_NEAR(3 + sqrt(2.0) / 2.0, point_x1_of_c_in_a.x(), EPS);
    ASSERT_NEAR(5 + sqrt(2.0) / 2.0, point_x1_of_c_in_a.y(), EPS);
    ASSERT_NEAR(7, point_x1_of_c_in_a.z(), EPS);

    const auto point_y1_of_c_in_a = aTc * Point("C", 0, 1, 0);
    ASSERT_EQ("A", point_y1_of_c_in_a.get_frame());
    ASSERT_NEAR(3 - sqrt(2.0) / 2.0, point_y1_of_c_in_a.x(), EPS);
    ASSERT_NEAR(5 + sqrt(2.0) / 2.0, point_y1_of_c_in_a.y(), EPS);
    ASSERT_NEAR(7, point_y1_of_c_in_a.z(), EPS);

    const auto point_z1_of_c_in_a = aTc * Point("C", 0, 0, 1);
    ASSERT_EQ("A", point_z1_of_c_in_a.get_frame());
    ASSERT_NEAR(3, point_z1_of_c_in_a.x(), EPS);
    ASSERT_NEAR(5, point_z1_of_c_in_a.y(), EPS);
    ASSERT_NEAR(8, point_z1_of_c_in_a.z(), EPS);
}
