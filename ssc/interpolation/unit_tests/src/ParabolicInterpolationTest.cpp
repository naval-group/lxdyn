/*
 * ParabolicInterpolationTest.cpp
 *
 * \date 19 févr. 2013, 09:56:08
 * \author cec
 */

#include "ParabolicInterpolationTest.hpp"
#include "ssc/interpolation/ParabolicInterpolation.hpp"
#include "ssc/interpolation/ParabolicInterpolationException.hpp"
#include "ssc/interpolation/InterpolatorException.hpp"

using namespace ssc::interpolation;

ParabolicInterpolationTest::ParabolicInterpolationTest() : a(ssc::random_data_generator::DataGenerator(77))
{
}

ParabolicInterpolationTest::~ParabolicInterpolationTest()
{
}

void ParabolicInterpolationTest::SetUp()
{
}

void ParabolicInterpolationTest::TearDown()
{
}

namespace ssc
{
    namespace random_data_generator
    {
        template <> ParabolicCoefficients ssc::random_data_generator::TypedScalarDataGenerator<ParabolicCoefficients>::get() const
        {
            ParabolicCoefficients ret(random<double>(),random<double>(),random<double>());
            return ret;
        }

        template<> ParabolicCoefficients get_max_bound<ParabolicCoefficients>()
        {
            return ParabolicCoefficients(0,0,0);
        }

        template<> ParabolicCoefficients get_min_bound<ParabolicCoefficients>()
        {
            return ParabolicCoefficients(0,0,0);
        }
    }
}

TEST_F(ParabolicInterpolationTest, example)
{
//! [ParabolicInterpolationTest example]
    std::vector<ParabolicCoefficients> v;
    v.push_back(ParabolicCoefficients(1,2,3));
    v.push_back(ParabolicCoefficients(4,5,6));
    v.push_back(ParabolicCoefficients(7,8,9));
    v.push_back(ParabolicCoefficients(10,11,12));
    v.push_back(ParabolicCoefficients(13,14,15));
    ParabolicInterpolation pi(0,5,v);
//! [ParabolicInterpolationTest example]
//! [ParabolicInterpolationTest expected output]
    double x = 0.5;
    double x_xi = x-0;
    ASSERT_DOUBLE_EQ(1*x_xi*x_xi+2*x_xi+3, pi.f(x));
    x = 1.7;
    x_xi = x-1;
    ASSERT_DOUBLE_EQ(4*x_xi*x_xi+5*x_xi+6, pi.f(x));
    x = 2.8;
    x_xi = x-2;
    ASSERT_DOUBLE_EQ(7*x_xi*x_xi+8*x_xi+9, pi.f(x));
//! [ParabolicInterpolationTest expected output]
}

TEST_F(ParabolicInterpolationTest, derivative)
{
    std::vector<ParabolicCoefficients> coeffs;
    coeffs.push_back(ParabolicCoefficients(12./5.,0,1./5.));
    coeffs.push_back(ParabolicCoefficients(-6.,84./5.,-41./5.));
    coeffs.push_back(ParabolicCoefficients(18./5.,-108./5.,151./5.));
    ParabolicInterpolation pp(0, 3, coeffs);
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const double X = a.random<double>().between(0,1);
        ASSERT_DOUBLE_EQ(2.*(12./5.)*X,pp.df(X));
    }
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const double X = a.random<double>().between(1,2);
        ASSERT_DOUBLE_EQ((84./5.)-2.*6.*(X-1),pp.df(X));
    }
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const double X = a.random<double>().between(2,3);
        ASSERT_DOUBLE_EQ(-(108./5.)+2.*(18./5.)*(X-2),pp.df(X));
    }
}

TEST_F(ParabolicInterpolationTest, should_throw_if_coeffs_has_too_few_elements)
{
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const double xmin = a.random<double>();
        const double xmax = a.random<double>().greater_than(xmin);
        const size_t n = a.random<size_t>().no().greater_than(1);
        ASSERT_THROW(ParabolicInterpolation(xmin, xmax, a.random_vector_of<ParabolicCoefficients>().of_size(n)), ParabolicInterpolationException);
    }
}

TEST_F(ParabolicInterpolationTest, should_throw_if_xmin_greater_than_xmax)
{
    const size_t N = 100;
    for (size_t i = 0 ; i < N ; ++i)
    {
        const double xmax = a.random<double>();
        const double xmin = a.random<double>().greater_than(xmax);
        const size_t n = a.random<size_t>().greater_than(1).but().no().greater_than(N);
        ASSERT_THROW(ParabolicInterpolation(xmin, xmax, a.random_vector_of<ParabolicCoefficients>().of_size(n)), InterpolatorException);
    }
}

TEST_F(ParabolicInterpolationTest, should_not_throw_if_xmin_equals_xmax)
{
    const size_t N = 100;
    for (size_t i = 0 ; i < N ; ++i)
    {
        const double xmin = a.random<double>();
        const double xmax = xmin;
        const size_t n = a.random<size_t>().greater_than(1).but().no().greater_than(N);
        ASSERT_NO_THROW(ParabolicInterpolation(xmin, xmax, a.random_vector_of<ParabolicCoefficients>().of_size(n)));
    }
}

TEST_F(ParabolicInterpolationTest, range_problem_detected_in_EONAV)
{
    std::vector<ParabolicCoefficients> coeffs;
    coeffs.push_back(ParabolicCoefficients(12./5.,0,1./5.));
    coeffs.push_back(ParabolicCoefficients(-6.,84./5.,-41./5.));
    coeffs.push_back(ParabolicCoefficients(18./5.,-108./5.,151./5.));
    ParabolicInterpolation pp(0, 3, coeffs);
    ASSERT_NO_THROW(pp.f(3));
    ASSERT_NO_THROW(pp.df(3));
    ASSERT_NO_THROW(pp.df(3,2));
}
