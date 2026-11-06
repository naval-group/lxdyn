"""The SDK's controllers, called directly rather than over gRPC."""
# pylint: disable=missing-function-docstring

import numpy as np
import pytest
import yaml
from xdyngrpc.controllers import controller, pid_controller, state_feedback_controller


def get_yaml_pid(dt: float) -> str:
    """Generate the YAML expected by the PID controller."""
    return yaml.dump(
        {
            "command": "beta",
            "type": "PID",
            "dt": dt,
            "setpoint": "psi_co",
            "state weights": {"x": 2, "phi": 2, "p": -1},
            "gains": {"Kp": -1, "Ki": 0, "Kd": -1},
            "integral saturation": 1.0,
        }
    )


def get_yaml_state_feedback(dt: float) -> str:
    """Generate the YAML expected by the state feedback controller."""
    return yaml.dump(
        {
            "commands": ["delta", "beta_1", "beta_2"],
            "dt": dt,
            "setpoints bind": {"phi": "phi_co", "psi": "psi_co"},
            "input states": ["v", "p", "r", "phi", "psi"],
            "K matrix": [
                [0.1, -0.2, 0.3, -0.4, 0.5],
                [-1.0, 2.0, -3.0, 4.0, -5.0],
                [10.0, -20.0, 30.0, -40.0, 50.0],
            ],
        }
    )


def get_yaml_state_feedback_integral_state(dt: float) -> str:
    """Generate the YAML expected by the state feedback controller with an integral state."""
    return yaml.dump(
        {
            "commands": ["delta", "beta_1", "beta_2"],
            "dt": dt,
            "setpoints bind": {"phi": "phi_co", "psi": "psi_co"},
            "input states": ["v", "p", "r", "phi", "psi"],
            "integral states with saturation": {"phi": None, "psi": -0.01},
            "K matrix": [
                [0.1, -0.2, 0.3, -0.4, 0.5, -0.6, 0.7],
                [-1.0, 2.0, -3.0, 4.0, -5.0, 6.0, -7.0],
                [10.0, -20.0, 30.0, -40.0, 50.0, -60.0, 70.0],
            ],
        }
    )


def test_can_call_pid_controller():
    dt = 2.3
    pid_controller.PIDController(get_yaml_pid(dt), dt)


def test_should_get_the_right_number_of_inputs():
    dt = 2.3
    pid = pid_controller.PIDController(get_yaml_pid(dt), dt)
    assert pid.get_setpoint_names() == ["psi_co"]


def test_can_get_plant_output():
    dt = 2.3
    pid = pid_controller.PIDController(get_yaml_pid(dt), dt)
    states = controller.StatesEuler()
    states.x = 1.3
    states.phi = 2.7
    states.p = -2
    assert pid.get_plant_output(states) == 10


def test_can_get_pid_commands():
    dt = 0.5
    Kp = 2.5
    Ki = 0.1
    Kd = 0.314
    pid_config = yaml.dump(
        {
            "command": "beta",
            "type": "PID",
            "dt": dt,
            "setpoint": "rpm_co",
            "state weights": {"x": 1, "y": -1},
            "gains": {"Kp": Kp, "Ki": Ki, "Kd": Kd},
        }
    )
    pid = pid_controller.PIDController(pid_config, dt)

    # First time step
    error = 0.05
    rpm_co = 5
    states = controller.StatesEuler(
        x=2 * rpm_co, y=rpm_co + error, z=300.0, u=1.0, v=2.0, w=3.0, p=4.0, q=5.0, r=6.0
    )
    dstates_dt = controller.StatesEuler()

    first_expected_command = Kp * error
    assert (
        abs(
            pid.get_commands_euler_321(states, dstates_dt, [rpm_co])["beta"]
            - first_expected_command
        )
        < 1e-6
    )

    # Second time step
    error2 = -0.03
    rpm_co = 1.5
    states.x = 2 * rpm_co
    states.y = rpm_co + error2
    second_expected_command = Kp * error2 + Ki * error2 * dt + Kd * (error2 - error) / dt
    assert (
        abs(
            pid.get_commands_euler_321(states, dstates_dt, [rpm_co])["beta"]
            - second_expected_command
        )
        < 1e-6
    )

    # Third time step
    error3 = 0.01
    rpm_co = 5.5
    states.x = 2 * rpm_co
    states.y = rpm_co + error3
    third_expected_command = (
        Kp * error3 + Ki * (error2 * dt + error3 * dt) + Kd * (error3 - error2) / dt
    )

    assert (
        abs(
            pid.get_commands_euler_321(states, dstates_dt, [rpm_co])["beta"]
            - third_expected_command
        )
        < 1e-6
    )


def test_integral_saturation():
    dt = 0.5
    Kp = 2.5
    Ki = 0.1
    Kd = 0.314 * 0.0
    int_sat = 0.01
    pid_config = yaml.dump(
        {
            "command": "beta",
            "type": "PID",
            "dt": dt,
            "setpoint": "rpm_co",
            "state weights": {"x": 1, "y": -1},
            "gains": {"Kp": Kp, "Ki": Ki, "Kd": Kd},
            "integral saturation": int_sat,
        }
    )
    pid = pid_controller.PIDController(pid_config, dt)

    # First time step
    error = 0.05
    rpm_co = 5
    states = controller.StatesEuler(
        x=2 * rpm_co, y=rpm_co + error, z=300.0, u=1.0, v=2.0, w=3.0, p=4.0, q=5.0, r=6.0
    )
    dstates_dt = controller.StatesEuler()

    first_expected_command = Kp * error
    assert (
        abs(
            pid.get_commands_euler_321(states, dstates_dt, [rpm_co])["beta"]
            - first_expected_command
        )
        < 1e-6
    )

    # Second time step
    error2 = -0.03
    rpm_co = 1.5
    states.x = 2 * rpm_co
    states.y = rpm_co + error2
    second_expected_command = Kp * error2 + Ki * (-int_sat) + Kd * (error2 - error) / dt

    assert (
        abs(
            pid.get_commands_euler_321(states, dstates_dt, [rpm_co])["beta"]
            - second_expected_command
        )
        < 1e-6
    )

    # Third time step
    error3 = 0.05
    rpm_co = 5.5
    states.x = 2 * rpm_co
    states.y = rpm_co + error3
    third_expected_command = Kp * error3 + Ki * int_sat + Kd * (error3 - error2) / dt

    assert (
        abs(
            pid.get_commands_euler_321(states, dstates_dt, [rpm_co])["beta"]
            - third_expected_command
        )
        < 1e-6
    )

    # Fourth time step
    error4 = -0.008
    rpm_co = 5.5
    states.x = 2 * rpm_co
    states.y = rpm_co + error4
    fourth_expected_command = (
        Kp * error4 + Ki * (int_sat + error4 * dt) + Kd * (error4 - error3) / dt
    )

    assert (
        abs(
            pid.get_commands_euler_321(states, dstates_dt, [rpm_co])["beta"]
            - fourth_expected_command
        )
        < 1e-6
    )

    # Check behaviour with a negative saturation value
    pid.integral_sat = -int_sat
    error5 = +0.01
    rpm_co = 5.5
    states.x = 2 * rpm_co
    states.y = rpm_co + error5
    fourth_expected_command = Kp * error5 + Ki * int_sat + Kd * (error5 - error4) / dt

    assert (
        abs(
            pid.get_commands_euler_321(states, dstates_dt, [rpm_co])["beta"]
            - fourth_expected_command
        )
        < 1e-6
    )

    # Check behaviour with a saturation value of 0.0
    pid.integral_sat = 0.0
    error6 = +0.01
    rpm_co = 5.5
    states.x = 2 * rpm_co
    states.y = rpm_co + error6
    fourth_expected_command = Kp * error6 + Ki * 0.0 + Kd * (error6 - error5) / dt

    assert (
        abs(
            pid.get_commands_euler_321(states, dstates_dt, [rpm_co])["beta"]
            - fourth_expected_command
        )
        < 1e-6
    )

    # Check position saturation
    pid.pos_saturation = 0.125
    pid.integral_sat = 0.0
    error7 = +10.0
    rpm_co = 5.5
    states.x = 2 * rpm_co
    states.y = rpm_co + error7
    saturated_expected_command = pid.pos_saturation

    assert (
        abs(
            pid.get_commands_euler_321(states, dstates_dt, [rpm_co])["beta"]
            - saturated_expected_command
        )
        < 1e-6
    )

    # Check speed saturation
    pid.spd_saturation = 0.1
    pid.integral_sat = 0.0
    error7 = -10.0
    rpm_co = 5.5
    states.x = 2 * rpm_co
    states.y = rpm_co + error7
    saturated_expected_command = pid.pos_saturation - pid.spd_saturation * pid.dt

    assert (
        abs(
            pid.get_commands_euler_321(states, dstates_dt, [rpm_co])["beta"]
            - saturated_expected_command
        )
        < 1e-6
    )


def test_state_feedback_wrong_sizes():
    dt = 2.3
    param = get_yaml_state_feedback(dt)
    param = param.replace("- beta_1\n", "")
    with pytest.raises(AssertionError) as e:
        state_feedback_controller.StateFeedbackController(param, dt)
    np.testing.assert_equal(
        e.value.args[0],
        "The number of rows in gain matrix K is not equal to "
        "the number of commands: K has 3 rows but there "
        "are 2 commands.",
    )

    param = get_yaml_state_feedback(dt)
    param = param.replace("- phi\n", "")
    with pytest.raises(AssertionError) as e:
        state_feedback_controller.StateFeedbackController(param, dt)
    np.testing.assert_equal(
        e.value.args[0],
        "The number of columns in gain matrix K is not equal to "
        "the number of input (and integral) states: K has 5 columns "
        "but there are 4 states.",
    )


def test_state_feedback_wrong_integral_state():
    dt = 2.3
    param = get_yaml_state_feedback(dt)
    param = param + "integral states with saturation:\n  gamma: null\n  delta: 0.08"
    with pytest.raises(AssertionError) as e:
        state_feedback_controller.StateFeedbackController(param, dt)
    np.testing.assert_equal(
        e.value.args[0],
        "The following states need to be integrated by the controller "
        "(they are in 'integral states with saturation') but they couldn't "
        "be found in 'input states': either remove them from "
        "'integral states with saturation' or add them to 'input states':"
        " ['gamma', 'delta']",
    )


def test_state_feedback_get_plant_output():
    dt = 2.3
    param = get_yaml_state_feedback(dt)
    sf = state_feedback_controller.StateFeedbackController(param, dt)
    states = controller.StatesEuler()
    states.x = 1.3
    states.y = -2.3
    states.v = 4.52
    states.p = -2
    states.r = 3
    states.phi = 2.7
    states.psi = -8.9
    states.theta = 0.55
    plant_output = sf.get_plant_output(states)

    assert len(plant_output) == len(sf.input_states)

    np.testing.assert_equal(
        plant_output, np.array([states.v, states.p, states.r, states.phi, states.psi])
    )


def test_state_feedback_get_state_setpoints():
    dt = 2.3
    param = get_yaml_state_feedback(dt)
    sf = state_feedback_controller.StateFeedbackController(param, dt)
    setpoints = {"phi_co": 0.25, "psi_co": -5.6}
    state_setpoint = sf.get_state_setpoints(setpoints)

    assert len(state_setpoint) == len(sf.input_states)

    np.testing.assert_equal(
        state_setpoint, np.array([0.0, 0.0, 0.0, setpoints["phi_co"], setpoints["psi_co"]])
    )


def test_state_feedback_get_commands():
    dt = 2.3
    param = get_yaml_state_feedback(dt)
    sf = state_feedback_controller.StateFeedbackController(param, dt)

    # First time step
    states = controller.StatesEuler(
        x=2.0, y=3.0, z=300.0, u=1.0, v=2.0, w=3.0, p=4.0, q=5.0, r=6.0, phi=7.0, theta=8.0, psi=9.0
    )
    dstates_dt = controller.StatesEuler()

    setpoints = [4.0, -1.0]
    exp_command = -sf.K @ (
        np.array([states.v, states.p, states.r, states.phi, states.psi])
        - np.array([0.0, 0.0, 0.0, setpoints[0], setpoints[1]])
    )
    command = sf.get_commands_euler_321(states, dstates_dt, setpoints)

    np.testing.assert_equal(list(command.keys()), ["delta", "beta_1", "beta_2"])
    np.testing.assert_equal(list(command.values()), exp_command)

    # Tests saturation
    sf.pos_saturation = [4.0, None, 120.0]
    sf.spd_saturation = [None, 2.0, None]

    states = controller.StatesEuler(
        x=2.0,
        y=3.0,
        z=300.0,
        u=1.0,
        v=2.0,
        w=3.0,
        p=-40.0,
        q=5.0,
        r=6.0,
        phi=7.0,
        theta=8.0,
        psi=9.0,
    )
    exp_command = [-4.0, exp_command[1] + 2.0 * sf.dt, -120.0]

    command = sf.get_commands_euler_321(states, dstates_dt, setpoints)
    np.testing.assert_equal(list(command.values()), exp_command)


def test_state_feedback_get_commands_with_integral():
    dt = 0.1
    param = get_yaml_state_feedback_integral_state(dt)
    sf = state_feedback_controller.StateFeedbackController(param, dt)

    # First time step
    states = controller.StatesEuler(
        x=2.0, y=3.0, z=300.0, u=1.0, v=2.0, w=3.0, p=4.0, q=5.0, r=6.0, phi=7.0, theta=8.0, psi=9.0
    )
    dstates_dt = controller.StatesEuler()

    setpoints = [5.0, -1.0]
    int_phi = 0.2
    int_psi = 0.01
    exp_command = -sf.K @ (
        np.array([states.v, states.p, states.r, states.phi, states.psi, int_phi, int_psi])
        - np.array([0.0, 0.0, 0.0, setpoints[0], setpoints[1], 0.0, 0.0])
    )
    command = sf.get_commands_euler_321(states, dstates_dt, setpoints)

    np.testing.assert_equal(list(command.keys()), ["delta", "beta_1", "beta_2"])
    np.testing.assert_equal(list(command.values()), exp_command)
