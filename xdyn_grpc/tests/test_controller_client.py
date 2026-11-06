"""The SDK's controller client against a PID controller, served in-process."""

import grpc
import pytest
from xdyngrpc.controllers import controller
from xdyngrpc.controllers.controller import StatesEuler
from xdyngrpc.controllers.grpc_controller_client import Controller
from xdyngrpc.controllers.pid_controller import PIDController


@pytest.fixture(scope="module")
def channel():
    server, port = controller.start(PIDController, port=0)
    with grpc.insecure_channel(f"localhost:{port}") as channel:
        yield channel
    server.stop(0)


def test_pid_commands_over_three_time_steps(channel):
    yaml_parameters = """
        command: beta
        dt: 0.5
        setpoint: rpm_co
        state weights:
            x: 1
            y: -1
        gains:
            Kp: 2.5
            Ki: 0.1
            Kd: 0.314
    """
    Kp = 2.5
    Ki = 0.1
    Kd = 0.314
    dt = 0.5
    pid = Controller(channel, yaml_parameters)
    # First time step
    error = 0.05
    rpm_co = 5
    states = StatesEuler(
        x=2 * rpm_co,
        y=rpm_co + error,
        z=300.0,
        u=1.0,
        v=2.0,
        w=3.0,
        p=4.0,
        q=5.0,
        r=6.0,
    )
    dstates_dt = StatesEuler()

    first_expected_command = Kp * error
    assert (
        abs(
            pid.get_commands_euler_321(states, dstates_dt, [rpm_co])[0]["beta"]
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
            pid.get_commands_euler_321(states, dstates_dt, [rpm_co])[0]["beta"]
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
            pid.get_commands_euler_321(states, dstates_dt, [rpm_co])[0]["beta"]
            - third_expected_command
        )
        < 1e-6
    )
