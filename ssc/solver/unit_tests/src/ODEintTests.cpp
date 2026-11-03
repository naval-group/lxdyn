/*
 * ODEintTests.cpp
 *
 * \date 7 mars 2014
 *      Author: cec
 */

#include "ODEintTests.hpp"

#include <functional> // Otherwise CLang issues an error (bind.hpp:36:14: error: ‘std::bind’ has not been declared)
#include "ssc/solver/Observers.hpp"
#include "ssc/solver/Scheduler.hpp"
#include "ssc/solver/solve.hpp"
#include "test_systems.hpp"

#include <boost/numeric/odeint/stepper/euler.hpp>
#include <boost/numeric/odeint/stepper/runge_kutta4.hpp>
#include <boost/numeric/odeint/stepper/runge_kutta_cash_karp54.hpp>

#include <sstream>

using namespace ssc::solver;

ODEintTests::ODEintTests() : a(ssc::random_data_generator::DataGenerator(452251321))
{
}

ODEintTests::~ODEintTests()
{
}

void ODEintTests::SetUp()
{
}

void ODEintTests::TearDown()
{
}

typedef boost::numeric::odeint::euler<StateType> EulerStepper;
typedef boost::numeric::odeint::runge_kutta4<StateType> RK4Stepper;
typedef boost::numeric::odeint::runge_kutta_cash_karp54<StateType> RKCK;

TEST_F(ODEintTests, linear)
{
//! [ODEintTests example]
    StateType x0(2,0);
    SimpleODE system(x0);
    double t0 = 0.0 , tend = 10.0 , dt = 1;

    VectorObserver observer;
    ssc::solver::Scheduler scheduler(t0, tend, dt);
    quicksolve<EulerStepper,SimpleODE>(system, scheduler, observer);
//! [ODEintTests example]

//! [ODEintTests expected output]
    const std::vector<std::pair<double,double> > v = observer.get();
    const size_t N = (size_t)floor((tend-t0)/dt+0.5)+1;
    ASSERT_EQ(N,v.size());
    for (size_t i = 0 ; i < N ; ++i)
    {
        const double t = t0 + dt*((double)i);
        ASSERT_EQ(t, v.at(i).first);
        ASSERT_DOUBLE_EQ(2*t, v.at(i).second);
    }
//! [ODEintTests expected output]
}

TEST_F(ODEintTests, linear_with_adaptive_RK_Cash_Karp)
{
    StateType x0(2,0);
    SimpleODE system(x0);
    double t0 = 0.0 , tend = 10.0 , dt = 1;

    VectorObserver observer;
    ssc::solver::Scheduler scheduler(t0, tend, dt);
    quicksolve<RKCK>(system, scheduler, observer);

    const std::vector<std::pair<double,double> > v = observer.get();
    ASSERT_EQ(11,v.size());
    for (size_t i = 0 ; i <= 10 ; ++i)
    {
        ASSERT_EQ(i, v.at(i).first);
        ASSERT_DOUBLE_EQ(2*double(i), v.at(i).second);
    }
}

TEST_F(ODEintTests, quadratic_with_euler)
{
    StateType x0(2,0);
    QuadraticODE system(x0);
    const size_t N = 10;
    double t0 = 0.0 , tend = N , dt = 1;

    VectorObserver observer;
    ssc::solver::Scheduler scheduler(t0, tend, dt);
    quicksolve<EulerStepper>(system, scheduler, observer);

    const std::vector<std::pair<double,double> > v = observer.get();
    ASSERT_EQ(N+1,v.size());
    for (size_t i = 0 ; i <= N ; ++i)
    {
        ASSERT_EQ(i, v.at(i).first);
        ASSERT_DOUBLE_EQ(double(i)*(double(i)-1), v.at(i).second);
    }
}

TEST_F(ODEintTests, quadratic_with_RK4)
{
    StateType x0(2,0);
    QuadraticODE system(x0);
    const size_t N = 10;
    double t0 = 0.0 , tend = N , dt = 1;

    VectorObserver observer;
    ssc::solver::Scheduler scheduler(t0, tend, dt);
    quicksolve<RK4Stepper>(system, scheduler, observer);

    const std::vector<std::pair<double,double> > v = observer.get();
    ASSERT_EQ(N+1,v.size());
    for (size_t i = 0 ; i <= N ; ++i)
    {
        ASSERT_EQ(i, v.at(i).first);
        ASSERT_DOUBLE_EQ(double(i*i), v.at(i).second);
    }
}

