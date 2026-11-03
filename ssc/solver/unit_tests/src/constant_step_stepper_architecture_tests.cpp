/*
 * constant_step_stepper_arcitecture_tests.cpp
 *
 * \date 21 mars 2014
 *      Author: cec
 */

#include "constant_step_stepper_architecture_tests.hpp"
#include "ODEMocks.hpp"
#include "ssc/solver/solve.hpp"

using ::testing::_;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::StrictMock;
using namespace ssc::solver;

constant_step_stepper_architecture_tests::constant_step_stepper_architecture_tests()
    : a(ssc::random_data_generator::DataGenerator(12212))
{
}

constant_step_stepper_architecture_tests::~constant_step_stepper_architecture_tests() {}

void constant_step_stepper_architecture_tests::SetUp() {}

void constant_step_stepper_architecture_tests::TearDown() {}

TEST_F(
    constant_step_stepper_architecture_tests,
    collaborators_should_be_called_in_the_right_sequence_for_constant_step_steppers_event_detected)
{
    StrictMock<ODEMocks> mock;
    SystemWithMock sys(mock);
    ObserverWithMock observer(mock);
    SchedulerWithMock scheduler(mock);
    EventHandlerWithMock event_handler(mock);
    ExplicitStepperWithMock stepper(mock);

    ON_CALL(mock, get_time()).WillByDefault(Return(a.random<double>()()));
    ON_CALL(mock, detected_state_events()).WillByDefault(Return(true)); // Event detected

    InSequence
        seq; // Name is irrelevant: all expectations in its scope are expected to occur in sequence
    EXPECT_CALL(mock, get_time()).RetiresOnSaturation();
    EXPECT_CALL(mock, initialize_system_outputs_before_first_observation()).RetiresOnSaturation();
    EXPECT_CALL(mock, collect_available_serializations(_, _, _)).RetiresOnSaturation();
    EXPECT_CALL(mock, check_variables_to_serialize_are_available()).RetiresOnSaturation();
    EXPECT_CALL(mock, observe_after_solver_step(_, _, _)).RetiresOnSaturation();
    EXPECT_CALL(mock, has_more_time_events()).RetiresOnSaturation();
    EXPECT_CALL(mock, get_time()).RetiresOnSaturation();
    EXPECT_CALL(mock, model_function(_, _, _)).RetiresOnSaturation();
    EXPECT_CALL(mock, observe_before_solver_step(_, _, _)).RetiresOnSaturation();
    EXPECT_CALL(mock, flush()).RetiresOnSaturation();
    EXPECT_CALL(mock, get_step()).RetiresOnSaturation();
    EXPECT_CALL(mock, do_step(_, _, _, _)).RetiresOnSaturation();
    EXPECT_CALL(mock, force_states(_, _)).RetiresOnSaturation();
    EXPECT_CALL(mock, detected_state_events()).RetiresOnSaturation();
    EXPECT_CALL(mock, locate_event()).RetiresOnSaturation();
    EXPECT_CALL(mock, run_event_actions()).RetiresOnSaturation();
    EXPECT_CALL(mock, advance_to_next_time_event()).RetiresOnSaturation();
    EXPECT_CALL(mock, get_time()).RetiresOnSaturation();
    EXPECT_CALL(mock, observe_after_solver_step(_, _, _)).RetiresOnSaturation();
    EXPECT_CALL(mock, has_more_time_events()).RetiresOnSaturation();
    EXPECT_CALL(mock, get_time()).RetiresOnSaturation();
    EXPECT_CALL(mock, model_function(_, _, _)).RetiresOnSaturation();
    EXPECT_CALL(mock, observe_before_solver_step(_, _, _)).RetiresOnSaturation();
    EXPECT_CALL(mock, flush()).RetiresOnSaturation();

    solve_for_constant_step<ExplicitStepperWithMock, SystemWithMock, ObserverWithMock>(sys, observer, stepper,
                                                                                       scheduler, event_handler, {});
}

TEST_F(
    constant_step_stepper_architecture_tests,
    collaborators_should_be_called_in_the_right_sequence_for_constant_step_steppers_no_event_detected)
{
    StrictMock<ODEMocks> mock;
    SystemWithMock sys(mock);
    ObserverWithMock observer(mock);
    SchedulerWithMock scheduler(mock);
    EventHandlerWithMock event_handler(mock);
    ExplicitStepperWithMock stepper(mock);

    ON_CALL(mock, get_time()).WillByDefault(Return(a.random<double>()()));
    ON_CALL(mock, detected_state_events()).WillByDefault(Return(false)); // No event detected

    InSequence
        seq; // Name is irrelevant: all expectations in its scope are expected to occur in sequence
    EXPECT_CALL(mock, get_time()).RetiresOnSaturation();
    EXPECT_CALL(mock, initialize_system_outputs_before_first_observation()).RetiresOnSaturation();
    EXPECT_CALL(mock, collect_available_serializations(_, _, _)).RetiresOnSaturation();
    EXPECT_CALL(mock, check_variables_to_serialize_are_available()).RetiresOnSaturation();
    EXPECT_CALL(mock, observe_after_solver_step(_, _, _)).RetiresOnSaturation();
    EXPECT_CALL(mock, has_more_time_events()).RetiresOnSaturation();
    EXPECT_CALL(mock, get_time()).RetiresOnSaturation();
    EXPECT_CALL(mock, model_function(_, _, _)).RetiresOnSaturation();
    EXPECT_CALL(mock, observe_before_solver_step(_, _, _)).RetiresOnSaturation();
    EXPECT_CALL(mock, flush()).RetiresOnSaturation();
    EXPECT_CALL(mock, get_step()).RetiresOnSaturation();
    EXPECT_CALL(mock, do_step(_, _, _, _)).RetiresOnSaturation();
    EXPECT_CALL(mock, force_states(_, _)).RetiresOnSaturation();
    EXPECT_CALL(mock, detected_state_events()).RetiresOnSaturation();
    EXPECT_CALL(mock, advance_to_next_time_event()).RetiresOnSaturation();
    EXPECT_CALL(mock, get_time()).RetiresOnSaturation();
    EXPECT_CALL(mock, observe_after_solver_step(_, _, _)).RetiresOnSaturation();
    EXPECT_CALL(mock, has_more_time_events()).RetiresOnSaturation();
    EXPECT_CALL(mock, get_time()).RetiresOnSaturation();
    EXPECT_CALL(mock, model_function(_, _, _)).RetiresOnSaturation();
    EXPECT_CALL(mock, observe_before_solver_step(_, _, _)).RetiresOnSaturation();
    EXPECT_CALL(mock, flush()).RetiresOnSaturation();

    solve_for_constant_step<ExplicitStepperWithMock, SystemWithMock, ObserverWithMock>(sys, observer, stepper,
                                                                                       scheduler, event_handler, {});
}
