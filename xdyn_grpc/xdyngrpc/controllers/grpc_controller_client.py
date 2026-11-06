"""gRPC sample client for controllers."""

import logging
from collections.abc import Mapping

from .._proto import controller_pb2, controller_pb2_grpc
from .controller import StatesEuler

SERVICE_NAME = "controller-client"

logging.basicConfig(
    format="%(asctime)s,%(msecs)d ["
    + SERVICE_NAME
    + "] - %(levelname)-4s [%(filename)s:%(lineno)d] %(message)s",
    datefmt="%d-%m-%Y:%H:%M:%S",
)
LOGGER = logging.getLogger(__name__)
LOGGER.setLevel(logging.INFO)


def to_grpc_states(states: StatesEuler) -> controller_pb2.ControllerStatesEuler:
    """Convert Python dataclass to protobuf."""
    ret = controller_pb2.ControllerStatesEuler()
    ret.t = states.t
    ret.x = states.x
    ret.y = states.y
    ret.z = states.z
    ret.u = states.u
    ret.v = states.v
    ret.w = states.w
    ret.p = states.p
    ret.q = states.q
    ret.r = states.r
    ret.phi = states.phi
    ret.theta = states.theta
    ret.psi = states.psi
    return ret


class Controller:
    """This class wraps the gRPC calls for use with "normal" python types."""

    def __init__(self, channel, parameters: str, t0: float = 0):
        """Constructor."""
        self.stub = controller_pb2_grpc.ControllerStub(channel)
        self.date_of_first_callback = None
        self.setpoint_names = None
        self.angle_representation = None
        self.set_parameters(parameters, t0)

    def set_parameters(self, yaml_string: str, t0: float) -> None:
        """Set the controller's YAML parameters."""
        request = controller_pb2.SetParametersRequest(parameters=yaml_string, t0=t0)
        response = self.stub.set_parameters(request)
        self.date_of_first_callback = response.date_of_first_callback
        self.setpoint_names = response.setpoint_names
        self.angle_representation = response.angle_representation

    def get_commands_euler_321(
        self,
        states: StatesEuler,
        dstates_dt: StatesEuler,
        setpoints: list[float],
    ) -> tuple[Mapping[str, float], float]:
        """Calculates the controller outputs (the commands), and when to call it next."""
        request = controller_pb2.ControllerRequestEuler()
        grpc_states = to_grpc_states(states)
        grpc_dstates_dt = to_grpc_states(dstates_dt)
        request.states.CopyFrom(grpc_states)
        request.dstates_dt.CopyFrom(grpc_dstates_dt)
        request.setpoints[:] = setpoints
        response = self.stub.get_commands_euler_321(request)
        return (response.commands, response.next_call)
