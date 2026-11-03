"""gRPC cosimulation."""

import grpc

from .._proto import cosimulation_pb2_grpc
from .._proto.cosimulation_pb2 import CosimulationRequestEuler, CosimulationRequestQuaternion


class CosimulationEuler:
    """Launch cosimulation steps. Wrapper arround gRPC."""

    def __init__(self, server_url: str):
        xdyn_channel = grpc.insecure_channel(server_url)
        self.xdyn_stub = cosimulation_pb2_grpc.CosimulationStub(xdyn_channel)
        self.request = CosimulationRequestEuler()

    def step(self, state, Dt, requested_output):
        """Run a cosimulation step."""
        self.request.states.t[:] = [state["t"]]
        self.request.states.x[:] = [state["x"]]
        self.request.states.y[:] = [state["y"]]
        self.request.states.z[:] = [state["z"]]
        self.request.states.u[:] = [state["u"]]
        self.request.states.v[:] = [state["v"]]
        self.request.states.w[:] = [state["w"]]
        self.request.states.p[:] = [state["p"]]
        self.request.states.q[:] = [state["q"]]
        self.request.states.r[:] = [state["r"]]
        self.request.states.phi[:] = [state["phi"]]
        self.request.states.theta[:] = [state["theta"]]
        self.request.states.psi[:] = [state["psi"]]
        self.request.Dt = Dt
        self.request.requested_output[:] = requested_output
        res = self.xdyn_stub.step_euler_321(self.request)
        ret = {
            "t": res.all_states.t,
            "x": res.all_states.x,
            "y": res.all_states.y,
            "z": res.all_states.z,
            "u": res.all_states.u,
            "v": res.all_states.v,
            "w": res.all_states.w,
            "p": res.all_states.p,
            "q": res.all_states.q,
            "r": res.all_states.r,
            "qr": res.all_states.qr,
            "qi": res.all_states.qi,
            "qj": res.all_states.qj,
            "qk": res.all_states.qk,
            "phi": res.all_states.phi,
            "theta": res.all_states.theta,
            "psi": res.all_states.psi,
            "extra_observations": {},
        }
        for key, vector in res.extra_observations.items():
            ret["extra_observations"][key] = vector.value
        return ret


class CosimulationQuaternion:
    """Launch cosimulation steps. Wrapper arround gRPC."""

    def __init__(self, server_url: str):
        xdyn_channel = grpc.insecure_channel(server_url)
        self.xdyn_stub = cosimulation_pb2_grpc.CosimulationStub(xdyn_channel)
        self.request = CosimulationRequestQuaternion()

    def step(self, state, Dt, requested_output):
        """Run a cosimulation step."""
        self.request.states.t[:] = [state["t"]]
        self.request.states.x[:] = [state["x"]]
        self.request.states.y[:] = [state["y"]]
        self.request.states.z[:] = [state["z"]]
        self.request.states.u[:] = [state["u"]]
        self.request.states.v[:] = [state["v"]]
        self.request.states.w[:] = [state["w"]]
        self.request.states.p[:] = [state["p"]]
        self.request.states.q[:] = [state["q"]]
        self.request.states.r[:] = [state["r"]]
        self.request.states.qr[:] = [state["qr"]]
        self.request.states.qi[:] = [state["qi"]]
        self.request.states.qj[:] = [state["qj"]]
        self.request.states.qk[:] = [state["qk"]]
        self.request.Dt = Dt
        self.request.requested_output[:] = requested_output
        res = self.xdyn_stub.step_quaternion(self.request)
        ret = {
            "t": res.all_states.t,
            "x": res.all_states.x,
            "y": res.all_states.y,
            "z": res.all_states.z,
            "u": res.all_states.u,
            "v": res.all_states.v,
            "w": res.all_states.w,
            "p": res.all_states.p,
            "q": res.all_states.q,
            "r": res.all_states.r,
            "qr": res.all_states.qr,
            "qi": res.all_states.qi,
            "qj": res.all_states.qj,
            "qk": res.all_states.qk,
            "phi": res.all_states.phi,
            "theta": res.all_states.theta,
            "psi": res.all_states.psi,
            "extra_observations": {},
        }
        for key, vector in res.extra_observations.items():
            ret["extra_observations"][key] = vector.value
        return ret
