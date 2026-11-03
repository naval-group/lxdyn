"""Integration tests for the harmonic oscillator"""

import logging

import grpc

from . import grpc_force_types
from .grpc_force_client import ForceModel

SERVICE_NAME = "force-client"

logging.basicConfig(
    format="%(asctime)s,%(msecs)d ["
    + SERVICE_NAME
    + "] - %(levelname)-4s [%(filename)s:%(lineno)d] %(message)s",
    datefmt="%d-%m-%Y:%H:%M:%S",
)
LOGGER = logging.getLogger(__name__)
LOGGER.setLevel(logging.INFO)


def get_yaml(address: str, instance_name: str) -> str:
    """Build a YAML string suitable for the gRPC harmonic oscillator."""
    return f"""
            name: {instance_name}
            model: grpc
            url: {address}
            k: 60
            c: 1
        """


def test_harmonic_oscillator(address: str):
    """Launch the server & run some gRPC calls.

    Contains commands for a ForceModel gRPC server.
    """
    LOGGER.info(f"Testing force model at {address}")
    with grpc.insecure_channel(address) as channel:
        instance_name = "parametric oscillator"
        force = ForceModel(channel, get_yaml(address, instance_name), "body name", instance_name)
        t = 123456
        states = grpc_force_types.States()
        states.t = [t]
        states.x = [0]
        states.y = [0]
        states.z = [0]
        states.u = [0]
        states.v = [0]
        states.w = [0]
        states.p = [0]
        states.q = [0]
        states.r = [0]
        states.qr = [1]
        states.qi = [0]
        states.qj = [0]
        states.qk = [0]
        states.phi = [0]
        states.theta = [0]
        states.psi = [0]
        commands = {"parametric oscillator(omega)": 1}
        res = force.force(states, commands, grpc_force_types.WaveInformation())
        eps = 1e-10
        assert abs(res.Fx - 0) < eps, f"Expecting Fx close to 0, but got Fx={res.Fx}"
        assert abs(res.Fy - 0) < eps, f"Expecting Fy close to 0, but got Fy={res.Fy}"
        assert abs(res.Fz - 0) < eps, f"Expecting Fz close to 0, but got Fz={res.Fz}"
        assert abs(res.Mx - 0) < eps, f"Expecting Mx close to 0, but got Mx={res.Mx}"
        assert abs(res.My - 0) < eps, f"Expecting My close to 0, but got My={res.My}"
        assert abs(res.Mz - 0) < eps, f"Expecting Mz close to 0, but got Mz={res.Mz}"
        assert res.extra_observations["k"] == 2
        assert res.extra_observations["harmonic_oscillator_time"] == t
        LOGGER.info("OK")


if __name__ == "__main__":
    LOGGER.info(":!!!!Starting gRPC client")
    test_harmonic_oscillator("force-model:9002")
    LOGGER.info("All done!")
