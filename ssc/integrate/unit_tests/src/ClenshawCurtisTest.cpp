/*
 * IntegrateOscillatoryTest.cpp
 *
 * \date Dec 9, 2014
 * \author cec
 */


#include "ClenshawCurtisTest.hpp"
#include "ssc/integrate/ClenshawCurtisCosine.hpp"
#include "ssc/integrate/ClenshawCurtisSine.hpp"

#define _USE_MATH_DEFINE
#include <cmath>
#define PI M_PI
#define EPS (1E-6)

ClenshawCurtisTest::ClenshawCurtisTest() : a(ssc::random_data_generator::DataGenerator(54212))
{
}

ClenshawCurtisTest::~ClenshawCurtisTest()
{
}

void ClenshawCurtisTest::SetUp()
{
}

void ClenshawCurtisTest::TearDown()
{
}

extern "C" double dci(double *x);

TEST_F(ClenshawCurtisTest, example)
{
//! [IntegrateOscillatoryTest example]
    ssc::integrate::ClenshawCurtisSine I([](const double omega){return omega<=0 ? 0 : log(omega);}, 10*PI);
//! [IntegrateOscillatoryTest example]
//! [IntegrateOscillatoryTest expected output]
    //const double expected = -0.1281316; // - ( gamma + log(10*PI) - ci(10*pi) ) / (10*pi)
    const double gamma = 0.57721566490153286060651209;
    double ten_pi = 10*PI;
    const double expected = - ( gamma + log(10*PI) - dci(&ten_pi) ) / (10*PI);

    ASSERT_NEAR(expected, I.integrate_f(0,1,EPS), EPS);
//! [IntegrateOscillatoryTest expected output]
}

TEST_F(ClenshawCurtisTest, sin)
{
    ssc::integrate::ClenshawCurtisSine I([](const double omega){return sin(omega);}, 1);
    const double expected = 0.2726756432935796;
    ASSERT_NEAR(expected, I.integrate_f(0,1,1E-3), EPS);
}

TEST_F(ClenshawCurtisTest, radiation_damping)
{
//    const auto Br1 = [](const double omega){return 0.5*(0.1/(0.01+(0.5-omega)*(0.5-omega))+0.1/(0.01+(0.5+omega)*(0.5+omega)));};
//    const auto Kr1 = [](const double tau){return exp(-0.1*tau)*cos(0.5*tau);};
    const auto Br2 = [](const double omega){return (2+5*omega*omega)/((2-6*omega+5*omega*omega)*(2+6*omega+5*omega*omega));};
    const auto Kr2 = [](const double tau){return exp(-0.2*tau)*cos(0.6*tau);};

    ssc::integrate::ClenshawCurtisCosine I(Br2, 1);


    const std::vector<double> taus = {0, 0.5, 1, 1.5, 2, 1.5, 3};
    for (size_t i = 0 ; i < taus.size() ; ++i)
    {
        const double tau = taus.at(i);
        ASSERT_NEAR(Kr2(tau), 2./PI*I.compute_for(tau,0,4,EPS), 0.1) << "tau = " << tau;
    }
}
