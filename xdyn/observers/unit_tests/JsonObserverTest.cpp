#include "JsonObserverTest.hpp"

#include <ssc/solver/steppers.hpp>

#include "JsonObserver.hpp"
#include "ListOfObservers.hpp"
#include "xdyn/core/unit_tests/test_data.hpp"
#include "xdyn/simulator_api/simulator_api.hpp"
#include "xdyn/yaml_parser/parse_output.hpp"

JsonObserverTest::JsonObserverTest() : a(ssc::random_data_generator::DataGenerator(666)) {}

TEST_F(JsonObserverTest, should_be_able_to_create_an_observer)
{
    const double dt = 1E-1;
    const double tend = 10;
    auto sys = get_system(test_data::read("tutorials/tutorial_01_falling_ball.yml"), 0);
    const auto yaml = parse_output(test_data::read("tutorials/tutorial_01_falling_ball.yml"));
    {
        ssc::solver::Scheduler scheduler(0, tend, dt);
        ListOfObservers observers(yaml);
        ssc::solver::quicksolve<ssc::solver::EulerStepper>(sys, scheduler, observers);
    }
    for (const auto& output : yaml) {
        if (output.format == "json") {
            if (not output.filename.empty()) {
                // EXPECT_EQ(0,remove(output.filename.c_str()));
            }
        }
    }
}

TEST_F(JsonObserverTest, should_be_able_to_create_an_observer_with_waves)
{
    const double dt = 1E-1;
    const double tend = 10;
    auto sys = get_system(test_data::read("tutorials/tutorial_01_falling_ball.yml"), 0);
    const auto yaml = parse_output(test_data::read("tutorials/tutorial_01_falling_ball.yml"));
    {
        ssc::solver::Scheduler scheduler(0, tend, dt);
        ListOfObservers observers(yaml);
        ssc::solver::quicksolve<ssc::solver::EulerStepper>(sys, scheduler, observers);
    }
    for (const auto& output : yaml) {
        if (output.format == "json") {
            if (not output.filename.empty()) {
                // EXPECT_EQ(0,remove(output.filename.c_str()));
            }
        }
    }
}
