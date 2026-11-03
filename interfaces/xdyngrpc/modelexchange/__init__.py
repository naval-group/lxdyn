"""gRPC model exchange."""
from typing import Dict, Union

import grpc

from .._proto import model_exchange_pb2_grpc
from .._proto.model_exchange_pb2 import ModelExchangeRequestEuler, ModelExchangeRequestQuaternion


class ModelExchangeEuler:
    """Launch model exchange steps. Wrapper arround gRPC."""

    def __init__(self, server_url: str):
        xdyn_channel = grpc.insecure_channel(server_url)
        self.xdyn_stub = model_exchange_pb2_grpc.ModelExchangeStub(xdyn_channel)
        self.request = ModelExchangeRequestEuler()

    def dx_dt(
        self, state: Dict[str, float], requested_output
    ) -> Dict[str, Union[float, Dict[str, float]]]:
        """Evaluate the derivative step."""
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
        self.request.requested_output[:] = requested_output
        res = self.xdyn_stub.dx_dt_euler_321(self.request)
        return {
            "t": res.d_dt.t,
            "x": res.d_dt.x,
            "y": res.d_dt.y,
            "z": res.d_dt.z,
            "u": res.d_dt.u,
            "v": res.d_dt.v,
            "w": res.d_dt.w,
            "p": res.d_dt.p,
            "q": res.d_dt.q,
            "r": res.d_dt.r,
            "qr": res.d_dt.qr,
            "qi": res.d_dt.qi,
            "qj": res.d_dt.qj,
            "qk": res.d_dt.qk,
            "phi": res.d_dt.phi,
            "theta": res.d_dt.theta,
            "psi": res.d_dt.psi,
            "extra_observations": res.extra_observations,
        }


class ModelExchangeQuaternion:
    """Launch model exchange steps. Wrapper arround gRPC."""

    def __init__(self, server_url: str):
        xdyn_channel = grpc.insecure_channel(server_url)
        self.xdyn_stub = model_exchange_pb2_grpc.ModelExchangeStub(xdyn_channel)
        self.request = ModelExchangeRequestQuaternion()

    def dx_dt(
        self, state: Dict[str, float], requested_output
    ) -> Dict[str, Union[float, Dict[str, float]]]:
        """Evaluate the derivative step."""
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
        self.request.requested_output[:] = requested_output
        res = self.xdyn_stub.dx_dt_quaternion(self.request)
        return {
            "t": res.d_dt.t,
            "x": res.d_dt.x,
            "y": res.d_dt.y,
            "z": res.d_dt.z,
            "u": res.d_dt.u,
            "v": res.d_dt.v,
            "w": res.d_dt.w,
            "p": res.d_dt.p,
            "q": res.d_dt.q,
            "r": res.d_dt.r,
            "qr": res.d_dt.qr,
            "qi": res.d_dt.qi,
            "qj": res.d_dt.qj,
            "qk": res.d_dt.qk,
            "phi": res.d_dt.phi,
            "theta": res.d_dt.theta,
            "psi": res.d_dt.psi,
            "extra_observations": res.extra_observations,
        }
