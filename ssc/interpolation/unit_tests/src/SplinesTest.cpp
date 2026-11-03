/*
 * SplineTest.cpp
 *
 * \date 8 juin 2012, 17:07:12
 * \author cec
 */

#include "SplinesTest.hpp"
#include "ssc/macros/extra_test_assertions.hpp"
#include "ssc/interpolation/NaturalSplines.hpp"
#include "ssc/interpolation/VectorOfEquallySpacedNumbers.hpp"
#include "ssc/interpolation/VectorOfEquallySpacedNumbersException.hpp"
#include "ssc/interpolation/InterpolatorException.hpp"
#include "ssc/interpolation/SplinesException.hpp"
#include "ssc/interpolation/SplineVariableStep.hpp"
#include "ssc/interpolation/PiecewiseConstantVariableStep.hpp"

#include <cmath>
#include <algorithm>
#define PI 4.*atan(1.)
#define EPS 1E-8

using namespace ssc::interpolation;

namespace ssc
{
    namespace random_data_generator
    {
        template<> VectorOfEquallySpacedNumbers get_min_bound<VectorOfEquallySpacedNumbers>();
        template<> VectorOfEquallySpacedNumbers get_max_bound<VectorOfEquallySpacedNumbers>();
    }
}



SplinesTest::SplinesTest() : a(ssc::random_data_generator::DataGenerator(316497))
{
}

SplinesTest::~SplinesTest()
{
}

void SplinesTest::SetUp()
{
}

void SplinesTest::TearDown()
{
}

TEST_F(SplinesTest, example)
{
//! [SplineTest example]
    std::vector<double> y(4,0);
    y[0] = 0;
    y[1] = 9;
    y[2] = 36;
    y[3] = 81;
    NaturalSplines spline(0,9,y);
//! [SplineTest example]
//! [SplineTest expected output]
    ASSERT_DOUBLE_EQ(9,spline.f(3));
    ASSERT_DOUBLE_EQ(5.4,spline.df(3));
    ASSERT_DOUBLE_EQ(2.4,spline.df(3,2));
//! [SplineTest expected output]
}

TEST_F(SplinesTest, value_at_the_interpolation_points_should_be_the_same_as_that_given_in_the_input_data)
{
    for (size_t i = 0 ; i < 100 ; ++i)
    {
        const VectorOfEquallySpacedNumbers x(0,6,70);
        const std::vector<double> y = a.random_vector_of<double>().of_size(x.size());
        NaturalSplines spline(x.get_min(),x.get_max(),y);
        for (size_t j = 0 ; j < x.size() ; ++j)
        {
            ASSERT_SMALL_RELATIVE_ERROR(y.at(j), spline.f(x.at(j)),EPS);
        }
    }
}

TEST_F(SplinesTest, interpolated_value_should_lie_between_the_value_of_the_spline_at_the_surrounding_knots)
{
    const VectorOfEquallySpacedNumbers x(10,16,7);
    std::vector<double> y(7,0);
    y[0] = 12;
    y[1] = 23;
    y[2] = 34;
    y[3] = 45;
    y[4] = 56;
    y[5] = 67;
    y[6] = 78;
    NaturalSplines spline(x.get_min(),x.get_max(),y);
    for (size_t i = 0 ; i < x.size()-1 ; ++i)
    {
        const double x0 = a.random<double>().between(x.at(i),x.at(i+1));
        const double lower_bound = std::min(y.at(i),y.at(i+1));
        const double upper_bound = std::max(y.at(i),y.at(i+1));
        ASSERT_GE(spline.f(x0),lower_bound);
        ASSERT_LE(spline.f(x0),upper_bound);
    }
}

TEST_F(SplinesTest, constructor_should_throw_an_exception_if_there_are_fewer_than_two_points_and_xmin_doesnt_equal_xmax)
{
    const std::vector<double> an_empty_vector = std::vector<double>();
    const std::vector<double> a_vector_with_one_element = a.random_vector_of<double>().of_size(1);
    ASSERT_THROW(NaturalSplines(0, 1,an_empty_vector),InterpolatorException);
    ASSERT_THROW(NaturalSplines(0, 1,a_vector_with_one_element),VectorOfEquallySpacedNumbersException);
    NaturalSplines(0, 0,a_vector_with_one_element);
}

TEST_F(SplinesTest, should_be_able_to_build_an_empty_spline)
{
    NaturalSplines s;
    ASSERT_EQ(0, s.f(a.random<double>()));
    ASSERT_EQ(0, s.df(a.random<double>()));
    ASSERT_EQ(0, s.df(a.random<double>(),2));
}

TEST_F(SplinesTest, should_be_able_to_assign_a_spline)
{
    std::vector<double> y(4,0);
    y[0] = 0;
    y[1] = 9;
    y[2] = 36;
    y[3] = 81;
    NaturalSplines spline(0,9,y);
    ASSERT_EQ(9,spline.f(3));
    NaturalSplines empty;
    spline = empty;
    ASSERT_EQ(0,spline.f(a.random<double>()));
}

TEST_F(SplinesTest, bug1_index_is_incorrect_when_value_is_much_greater_than_max_bound)
{
    std::vector<double> y(4,0);
    y[0] = 0;
    y[1] = 9;
    y[2] = 36;
    y[3] = 81;
    NaturalSplines spline(0,4,y);
    ASSERT_NO_THROW(spline.f(a.random<double>().greater_than(123456789)));
}

TEST_F(SplinesTest, bug1_index_is_incorrect_when_value_is_much_lower_than_min_bound)
{
    std::vector<double> y(4,0);
    y[0] = 0;
    y[1] = 9;
    y[2] = 36;
    y[3] = 81;
    NaturalSplines spline(0,4,y);
    ASSERT_NO_THROW(spline.f(a.random<double>().no().greater_than(-123456789)));
}

TEST_F(SplinesTest, bug_2_range_check_exception_with_certain_inputs)
{
    std::vector<double> y(10,0);
    y[0] = 1.0000;
    y[1] = 0.9397;
    y[2] = 0.7660;
    y[3] = 0.5000;
    y[4] = 0.1736;
    y[5] = -0.1736;
    y[6] = -0.5000;
    y[7] = -0.7660;
    y[8] = -0.9397;
    y[9] = -1.0000;
    const double x0 = -1e-16;
    const double xmin = -PI;
    const double xmax = 0;
    NaturalSplines spline(xmin, xmax,y);

    ASSERT_NO_THROW(spline.f(x0));
    ASSERT_NO_THROW(spline.df(x0));
    ASSERT_NO_THROW(spline.df(x0,2));
}

TEST_F(SplinesTest, should_be_able_to_retrieve_parabolic_coefficients)
{
    std::vector<double> v(4,0);
    v[0] = 0;
    v[1] = 1;
    v[2] = 4;
    v[3] = 3;
    NaturalSplines s(0,3,v);
    ASSERT_DOUBLE_EQ(0,s.f(0));
    ASSERT_DOUBLE_EQ(0.6250000000e-1,s.f(0.25));
    ASSERT_DOUBLE_EQ(0.2,s.f(0.5));
    ASSERT_DOUBLE_EQ(.4875000000,s.f(0.75));
    ASSERT_DOUBLE_EQ(1,s.f(1));
    ASSERT_DOUBLE_EQ(1.768750000,s.f(1.25));
    ASSERT_DOUBLE_EQ(2.65,s.f(1.5));
    ASSERT_DOUBLE_EQ(3.45625000,s.f(1.75));
    ASSERT_DOUBLE_EQ(4,s.f(2));
    ASSERT_DOUBLE_EQ(4.14375000,s.f(2.25));
    ASSERT_DOUBLE_EQ(3.95,s.f(2.5));
    ASSERT_DOUBLE_EQ(3.53125000,s.f(2.75));
    ASSERT_DOUBLE_EQ(3,s.f(3));

    auto coeffs2 = s.get_parabolic_coefficients();
    ASSERT_EQ(3, coeffs2.size());
    ASSERT_DOUBLE_EQ(3*0.8,coeffs2.at(0).a);
    ASSERT_DOUBLE_EQ(0,coeffs2.at(0).b);
    ASSERT_DOUBLE_EQ(1./5.,coeffs2.at(0).c);

    ASSERT_DOUBLE_EQ(3*(-2),coeffs2.at(1).a);
    ASSERT_DOUBLE_EQ(2*2.4,coeffs2.at(1).b);
    ASSERT_DOUBLE_EQ(2.6,coeffs2.at(1).c);

    ASSERT_DOUBLE_EQ(3*1.2,coeffs2.at(2).a);
    ASSERT_DOUBLE_EQ(2*(-3.6),coeffs2.at(2).b);
    ASSERT_DOUBLE_EQ(1.4,coeffs2.at(2).c);
}

TEST_F(SplinesTest, should_be_able_to_compute_position_of_minimum)
{
    for (size_t i = 0 ; i < 100 ; ++i)
    {
        const double xmin = a.random<double>();
        const double xmax = a.random<double>().greater_than(xmin);
        const double xmid = (xmin+xmax)/2.;
        const double ymin = a.random<double>().between(-1000,0);
        std::vector<double> y(11,0);
        y[0] = 5;
        y[1] = 4;
        y[2] = 3;
        y[3] = 2;
        y[4] = 1;
        y[5] = ymin;
        y[6] = 1;
        y[7] = 2;
        y[8] = 3;
        y[9] = 4;
        y[10] = 5;
        NaturalSplines s(xmin,xmax,y);
        const auto minimum = s.find_position_and_value_of_minimum();
        ASSERT_SMALL_RELATIVE_ERROR(xmid, minimum.first, EPS);
        ASSERT_SMALL_RELATIVE_ERROR(ymin, minimum.second, EPS);
    }
}

TEST_F(SplinesTest, bug_in_min_computation)
{
    const double xmin = -9.101;
    const double xmax = -xmin;
    std::vector<double> v(11);
    v[0] = 1481633085564.5;
    v[1] = 1086836315872.4;
    v[2] = 786986140477.4;
    v[3] = 582082559379.5;
    v[4] = 472125572578.9;
    v[5] = 457115180075.4;
    v[6] = 537051381869.0;
    v[7] = 711934177959.80126953;
    v[8] = 981763568347.75585938;
    v[9] = 1346539553032.8674316;
    v[10] = 1806262132015.1352539;
    NaturalSplines s(xmin,xmax,v);
    ASSERT_SMALL_RELATIVE_ERROR(-0.62150157748417023, s.find_position_and_value_of_minimum().first, EPS);
}

TEST_F(SplinesTest, bug2_in_min_computation)
{
    std::vector<double> y(11,0);
    y[0] = 15.11546161820128418;
    y[1] = 11.570972870142302246;
    y[2] = 9.1584638137532995605;
    y[3] = 7.8779344490342663574;
    y[4] = 7.7293847759852185059;
    y[5] = 8.7128147946061462402;
    y[6] = 10.828224504897049561;
    y[7] = 14.075613906857929688;
    y[8] = 18.45498300048878418;
    y[9] = 23.966331785789619141;
    y[10] = 30.60966026276043457;
    const double xmin = -9.1099999999999905498;
    const double xmax = -xmin;
    NaturalSplines s(xmin,xmax,y);

    ASSERT_NEAR(-2.49, s.find_position_and_value_of_minimum().first, 0.1);
}

double f(const double omega);
double f(const double omega)
{
    return 0.5*(0.1/(0.01+(0.5-omega)*(0.5-omega))+0.1/(0.01+(0.5+omega)*(0.5+omega)));
}

TEST_F(SplinesTest, bug_detected_in_simulator)
{
    std::vector<double> omega;
    std::vector<double> B;
    const double omega_min = 0;
    const double omega_max = 4;
    size_t N = 100;
    for (size_t i = 0 ; i < N ; ++i)
    {
        omega.push_back(omega_min + (omega_max-omega_min)*((double)i)/((double)(N-1)));
        B.push_back(f(omega.back()));
    }
    SplineVariableStep s(omega, B);
    PiecewiseConstantVariableStep<double> p(omega, B);
    ASSERT_DOUBLE_EQ(B.at(N-2), s.f(omega.at(N-2)));
    ASSERT_DOUBLE_EQ(B.at(N-2), p.f(omega.at(N-2)));
    ASSERT_DOUBLE_EQ(B.at(N-1), s.f(omega.at(N-1)));
    ASSERT_DOUBLE_EQ(B.at(N-1), p.f(omega.at(N-1)));
}
