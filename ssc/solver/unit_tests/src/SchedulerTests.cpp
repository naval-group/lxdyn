/*
 * SchedulerTests.cpp
 *
 * \date March 9 2021
 *      Author: LLI
 */

#include "SchedulerTests.hpp"

#include "ssc/solver/Scheduler.hpp"

SchedulerTests::SchedulerTests()
    : a(ssc::random_data_generator::DataGenerator(36))
{
}

SchedulerTests::~SchedulerTests() {}

void SchedulerTests::SetUp() {}

void SchedulerTests::TearDown() {}

TEST_F(SchedulerTests, can_initialize_scheduler_with_quarter_second_steps)
{
    const double t0 = 12.125;
    const double dt = 0.2;
    const double N = 40;
    ssc::solver::Scheduler scheduler(t0, t0 + N * dt, dt);

    ASSERT_EQ(t0, scheduler.get_time());
    ASSERT_EQ(true, scheduler.has_more_time_events());
    for (double i = 1; i <= N; ++i)
    {
        scheduler.advance_to_next_time_event();
        ASSERT_EQ(t0 + i * dt, scheduler.get_time());
    }
    scheduler.advance_to_next_time_event();
    ASSERT_EQ(false, scheduler.has_more_time_events());
}

TEST_F(SchedulerTests, can_initialize_scheduler_with_tend_not_a_multiple_of_dt)
{
    const double t0 = 12.125;
    const double dt = 0.2;
    const double N = 40;
    ssc::solver::Scheduler scheduler(t0, t0 + (N + 0.5) * dt, dt);

    ASSERT_EQ(t0, scheduler.get_time());
    ASSERT_EQ(true, scheduler.has_more_time_events());
    for (double i = 1; i <= N; ++i)
    {
        scheduler.advance_to_next_time_event();
        ASSERT_EQ(t0 + i * dt, scheduler.get_time());
    }
    scheduler.advance_to_next_time_event();
    ASSERT_EQ(t0 + (N + 0.5) * dt, scheduler.get_time());
    scheduler.advance_to_next_time_event();
    ASSERT_EQ(false, scheduler.has_more_time_events());
}

TEST_F(SchedulerTests, can_add_time_events_before_iterating_throught_scheduler)
{
    const double t0 = 12.125;
    const double dt = 0.2;
    const double N = 40;
    ssc::solver::Scheduler scheduler(t0, t0 + (N + 0.5) * dt, dt);

    scheduler.add_time_event(t0 - 0.1 * dt);
    scheduler.add_time_event(t0 + 2 * N * dt); // should not be added because > tend
    scheduler.add_time_event(t0 + (N / 2 + 0.5) * dt);

    scheduler.advance_to_next_time_event();
    ASSERT_EQ(t0 - 0.1 * dt, scheduler.get_time());

    for (double i = 1; i <= N / 2; ++i)
    {
        scheduler.advance_to_next_time_event();
        ASSERT_EQ(t0 + i * dt, scheduler.get_time());
    }
    scheduler.advance_to_next_time_event();
    ASSERT_EQ(t0 + (N / 2 + 0.5) * dt, scheduler.get_time());
    for (double i = N / 2 + 1; i <= N; ++i)
    {
        scheduler.advance_to_next_time_event();
        ASSERT_EQ(t0 + i * dt, scheduler.get_time());
    }
    scheduler.advance_to_next_time_event();
    ASSERT_EQ(t0 + (N + 0.5) * dt, scheduler.get_time());
    scheduler.advance_to_next_time_event();
    ASSERT_EQ(false, scheduler.has_more_time_events());
}

TEST_F(SchedulerTests, can_add_time_events_while_iterating_thought_scheduler)
{
    const double t0 = 12.125;
    const double dt = 0.2;
    const double N = 40;
    ssc::solver::Scheduler scheduler(t0, t0 + N * dt, dt);

    ASSERT_EQ(t0, scheduler.get_time());
    for (double i = 1; i <= N / 2; ++i)
    {
        scheduler.advance_to_next_time_event();
        ASSERT_EQ(t0 + i * dt, scheduler.get_time());
    }

    scheduler.add_time_event(t0 + (N / 2 + 0.5) * dt);
    scheduler.add_time_event(t0 + 2 * N * dt); // should not be added because > tend
    scheduler.add_time_event(t0);
    scheduler.add_time_event(t0 + (3 * N - 0.01) * dt); // should not be added because > tend

    scheduler.advance_to_next_time_event();
    ASSERT_EQ(t0, scheduler.get_time());
    scheduler.advance_to_next_time_event();
    ASSERT_EQ(t0 + (N / 2 + 0.5) * dt, scheduler.get_time());
    for (double i = N / 2 + 1; i <= N; ++i)
    {
        scheduler.advance_to_next_time_event();
        ASSERT_EQ(t0 + i * dt, scheduler.get_time());
    }
    scheduler.advance_to_next_time_event();
    ASSERT_EQ(false, scheduler.has_more_time_events());
}

TEST_F(SchedulerTests, cannot_add_a_time_event_already_in_the_scheduler)
{
    const double t0 = 12.125;
    const double dt = 0.2;
    const double N = 40;
    ssc::solver::Scheduler scheduler(t0, t0 + N * dt, dt);

    scheduler.add_time_event(t0 + 1);
    scheduler.add_time_event(t0 + N * dt);
    scheduler.add_time_event(t0 + (N / 4 + 0.00005) * dt);
    scheduler.add_time_event(t0 + (N / 2) * dt);
    scheduler.add_time_event(t0 + 1 - 0.0001 * dt);
    scheduler.add_time_event(t0 + 1 + 0.000999999 * dt);
    scheduler.add_time_event(t0 + (N - 0.00099) * dt);
    scheduler.add_time_event(t0 + (N + 0.001) * dt);

    ASSERT_EQ(t0, scheduler.get_time());
    ASSERT_EQ(true, scheduler.has_more_time_events());
    for (double i = 1; i <= N; ++i)
    {
        scheduler.advance_to_next_time_event();
        ASSERT_EQ(t0 + i * dt, scheduler.get_time());
    }
    scheduler.advance_to_next_time_event();
    ASSERT_EQ(false, scheduler.has_more_time_events());
}
