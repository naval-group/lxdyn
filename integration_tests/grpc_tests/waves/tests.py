"""Integration tests for gRPC wave models."""
import os

from xdyngrpc.cosimulation import CosimulationEuler

# Was hardcoded to the compose service name "xdyn:9002".
XDYN_SERVER_URL = os.environ.get("xdyn_server_url", "localhost:9002")


def test_should_get_the_right_number_of_results_from_xdyn_in_cosim_mode():
    cosim = CosimulationEuler(XDYN_SERVER_URL)
    dt = 1
    state = {
        "t": 0,
        "x": 0,
        "y": 8,
        "z": 12,
        "u": 1,
        "v": 0,
        "w": 0,
        "p": 0,
        "q": 1,
        "r": 0,
        "phi": 0,
        "theta": 0,
        "psi": 0,
    }
    requested_output = []
    res = cosim.step(state, dt, requested_output)
    # Lengths only: tutorial_09's cube is unstable (plain xdyn gives NaN in U by t=0.2).
    assert len(res['t']) == 2
    assert len(res['x']) == 2
    assert len(res['y']) == 2
    assert len(res['z']) == 2
    assert len(res['u']) == 2
    assert len(res['v']) == 2
    assert len(res['w']) == 2
    assert len(res['p']) == 2
    assert len(res['q']) == 2
    assert len(res['r']) == 2
    assert len(res['qr']) == 2
    assert len(res['qi']) == 2
    assert len(res['qj']) == 2
    assert len(res['qk']) == 2
    assert len(res['phi']) == 2
    assert len(res['theta']) == 2
    assert len(res['psi']) == 2


def main():
    test_should_get_the_right_number_of_results_from_xdyn_in_cosim_mode()


if __name__ == "__main__":
    main()
