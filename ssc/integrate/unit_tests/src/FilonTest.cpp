/*
 * FilonTest.cpp
 *
 * \date Dec 10, 2014
 * \author cec
 */


#include "FilonTest.hpp"
#include "ssc/integrate/Filon.hpp"

#define _USE_MATH_DEFINE
#include <cmath>
#define PI M_PI
#define EPS (1E-6)


FilonTest::FilonTest() : a(ssc::random_data_generator::DataGenerator(1221))
{
}

FilonTest::~FilonTest()
{
}

void FilonTest::SetUp()
{
}

void FilonTest::TearDown()
{
}

extern "C" double dci(double *x);
extern "C" double dsi(double *x);

/*
TEST_F(FilonTest, example2)
{
    ssc::integrate::Filon I([](const double omega){return omega<=0 ? 0 : log(omega);}, 10*PI);
    double ten_pi = 10*PI;
    const double expected = - dsi(&ten_pi) / ten_pi;

    ASSERT_NEAR(expected, I.integrate_f(0,1,EPS), 1E-5);
}*/

TEST_F(FilonTest, example)
{
    ssc::integrate::Filon I([](const double omega){return exp(-omega/2);}, 100);
    ASSERT_NEAR(4.783810813E-5, I.integrate_f(0,2*PI,EPS), EPS);
}

TEST_F(FilonTest, radiation_damping_1)
{
    const auto Br = [](const double omega){return 0.5*(0.1/(0.01+(0.5-omega)*(0.5-omega))+0.1/(0.01+(0.5+omega)*(0.5+omega)));};
    const auto Kr = [](const double tau){return exp(-0.1*tau)*cos(0.5*tau);};

    ssc::integrate::Filon I(Br, 1);


    const std::vector<double> taus = {0.5, 1, 1.5, 2, 1.5, 3};
    for (size_t i = 0 ; i < taus.size() ; ++i)
    {
        const double tau = taus.at(i);
        ASSERT_NEAR(Kr(tau), 2./PI*I.compute_for(tau,0,50,EPS), 1E-4) << "tau = " << tau;
    }
}

TEST_F(FilonTest, radiation_damping_2)
{
    const auto Br = [](const double omega){return (2+5*omega*omega)/((2-6*omega+5*omega*omega)*(2+6*omega+5*omega*omega));};
    const auto Kr = [](const double tau){return exp(-0.2*tau)*cos(0.6*tau);};

    ssc::integrate::Filon I(Br);


    const std::vector<double> taus = {0.5, 1, 1.5, 2, 1.5, 3};
    for (size_t i = 0 ; i < taus.size() ; ++i)
    {
        const double tau = taus.at(i);
        ASSERT_NEAR(Kr(tau), 2./PI*I.compute_for(tau,0,50,EPS), 1E-4) << "tau = " << tau;
    }
}

