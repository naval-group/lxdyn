"""The SDK's force client against the harmonic oscillator, served in-process."""

import grpc
import pytest
from xdyngrpc.forces import force, grpc_force_types
from xdyngrpc.forces.grpc_force_client import ForceModel
from xdyngrpc.forces.harmonic_oscillator import HarmonicOscillator

INSTANCE_NAME = "parametric oscillator"
PARAMETERS = """
    name: parametric oscillator
    model: grpc
    k: 60
    c: 1
"""


@pytest.fixture(scope="module")
def channel():
    server, port = force.start(HarmonicOscillator, port=0)
    with grpc.insecure_channel(f"localhost:{port}") as channel:
        yield channel
    server.stop(0)


def states_at_rest(t: float) -> grpc_force_types.States:
    states = grpc_force_types.States()
    states.t = [t]
    for name in (
        "x",
        "y",
        "z",
        "u",
        "v",
        "w",
        "p",
        "q",
        "r",
        "qi",
        "qj",
        "qk",
        "phi",
        "theta",
        "psi",
    ):
        setattr(states, name, [0])
    states.qr = [1]
    return states


def test_force_at_rest_is_zero_and_carries_the_extra_observations(channel):
    model = ForceModel(channel, PARAMETERS, "body name", INSTANCE_NAME)
    t = 123456
    res = model.force(
        states_at_rest(t), {"parametric oscillator(omega)": 1.0}, grpc_force_types.WaveInformation()
    )
    for component in ("Fx", "Fy", "Fz", "Mx", "My", "Mz"):
        assert getattr(res, component) == pytest.approx(0, abs=1e-10), component
    assert res.extra_observations["k"] == 2
    assert res.extra_observations["harmonic_oscillator_time"] == t


def test_force_follows_the_parameters_sent_by_the_client(channel):
    model = ForceModel(channel, PARAMETERS, "body name", INSTANCE_NAME)
    states = states_at_rest(0)
    states.x = [0.5]
    states.u = [2]
    res = model.force(
        states, {"parametric oscillator(omega)": 1.0}, grpc_force_types.WaveInformation()
    )
    assert res.Fx == pytest.approx(-(60 * 0.5 + 1 * 2))
