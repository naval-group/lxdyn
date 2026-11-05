"""Facilitate the creation of a gRPC controllers in Python."""
import logging
import time
from collections.abc import Callable
from concurrent import futures
from dataclasses import dataclass

import grpc
import yaml

from .._proto.controller_pb2 import (
    ControllerResponse,
    ControllerStatesEuler,
    ControllerStatesQuaternion,
    SetParametersRequest,
    SetParametersResponse,
)
from .._proto.controller_pb2_grpc import ControllerServicer as GrpcControllerServicer
from .._proto.controller_pb2_grpc import add_ControllerServicer_to_server
from ..common import closest_match

SERVICE_NAME = "grpc-controller"

logging.basicConfig(
    format="%(asctime)s,%(msecs)d ["
    + SERVICE_NAME
    + "] - %(levelname)-4s [%(filename)s:%(lineno)d] %(message)s",
    datefmt="%d-%m-%Y:%H:%M:%S",
)
LOGGER = logging.getLogger(__name__)
LOGGER.setLevel(logging.INFO)


@dataclass
class StatesQuaternion:
    """Ship states history supplied to the controller. Using quaternions.

    Attributes:
        t (float): Simulation time (in seconds).
        x (float): Projection on axis X of the NED frame of the vector between
                   the origin of the NED frame and the origin of the BODY frame
                   In metres.
        y (float): Projection on axis Y of the NED frame of the vector between
                   the origin of the NED frame and the origin of the BODY frame
                   In metres.
        z (float): Projection on axis Z of the NED frame of the vector between
                   the origin of the NED frame and the origin of the BODY frame
                   In metres.
        u (float): Projection on axis X of the NED frame of the vector of the
                   ship's speed relative to the ground (BODY/NED). In m/s.
        v (float): Projection on axis Y of the NED frame of the vector of the
                   ship's speed relative to the ground (BODY/NED). In m/s.
        w (float): Projection on axis Z of the NED frame of the vector of the
                   ship's speed relative to the ground (BODY/NED). In m/s.
        p (float): Projection on axis X of the NED frame of the vector of the
                   ship's rotation speed relative to the ground (BODY/NED).
                   In rad/s.
        q (float): Projection on axis Y of the NED frame of the vector of the
                   ship's rotation speed relative to the ground (BODY/NED).
                   In rad/s.
        r (float): Projection on axis Z of the NED frame of the vector
                   of the ship's rotation speed relative to the ground
                   (BODY/NED). In rad/s.
        qr (float): Real part of the quaternion defining the rotation
                    from the NED frame to the ship's BODY frame.
        qi (float): First imaginary part of the quaternion defining the
                    rotation from the NED frame to the ship's BODY frame.
        qj (float): Second imaginary part of the quaternion defining the
                    rotation from the NED frame to the ship's BODY frame.
        qk (float): Third imaginary part of the quaternion defining the
                    rotation from the NED frame to the ship's BODY frame.
    """

    t: float = 0
    x: float = 0
    y: float = 0
    z: float = 0
    u: float = 0
    v: float = 0
    w: float = 0
    p: float = 0
    q: float = 0
    r: float = 0
    qr: float = 0
    qi: float = 0
    qj: float = 0
    qk: float = 0


@dataclass
class StatesEuler:
    """Ship states history supplied to the controller. Using Euler 3-2-1 angles

    Attributes:
        t (float): Simulation time (in seconds).
        x (float): Projection on axis X of the NED frame of the vector between
                   the origin of the NED frame and the origin of the BODY frame
                   In metres.
        y (float): Projection on axis Y of the NED frame of the vector between
                   the origin of the NED frame and the origin of the BODY frame
                   In metres.
        z (float): Projection on axis Z of the NED frame of the vector between
                   the origin of the NED frame and the origin of the BODY frame
                   In metres.
        u (float): Projection on axis X of the NED frame of the vector of the
                   ship's speed relative to the ground (BODY/NED). In m/s.
        v (float): Projection on axis Y of the NED frame of the vector of the
                   ship's speed relative to the ground (BODY/NED). In m/s.
        w (float): Projection on axis Z of the NED frame of the vector of the
                   ship's speed relative to the ground (BODY/NED). In m/s.
        p (float): Projection on axis X of the NED frame of the vector of the
                   ship's rotation speed relative to the ground (BODY/NED).
                   In rad/s.
        q (float): Projection on axis Y of the NED frame of the vector of the
                   ship's rotation speed relative to the ground (BODY/NED).
                   In rad/s.
        r (float): Projection on axis Z of the NED frame of the vector
                   of the ship's rotation speed relative to the ground
                   (BODY/NED). In rad/s.
        phi (float): First Euler angle in the convention [psi, theta', phi''],
                     i.e. using rotation matrix Rned2body = Rz(ψ).Ry(θ).Rx(ϕ)
                     See xdyn's documentation for details.
        theta (float): Second Euler angles in the convention [psi, theta',
                     phi''], i.e. using rotation matrix Rz(ψ).Ry(θ).Rx(ϕ)
                     See xdyn's documentation for details.
        psi (float): Third Euler angle in the convention [psi, theta', phi''],
                     i.e. using rotation matrix Rned2body = Rz(ψ).Ry(θ).Rx(ϕ)
                     See xdyn's documentation for details.
    """

    t: float = 0
    x: float = 0
    y: float = 0
    z: float = 0
    u: float = 0
    v: float = 0
    w: float = 0
    p: float = 0
    q: float = 0
    r: float = 0
    phi: float = 0
    theta: float = 0
    psi: float = 0


def from_grpc_states_euler(states: ControllerStatesEuler) -> StatesEuler:
    """Convert protobuf states type to Python dataclass."""
    ret = StatesEuler()
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


def from_grpc_states_quaternion(
    states: ControllerStatesQuaternion,
) -> StatesQuaternion:
    """Convert protobuf states type to Python dataclass."""
    ret = StatesQuaternion()
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
    ret.qr = states.qr
    ret.qi = states.qi
    ret.qj = states.qj
    ret.qk = states.qk
    return ret


SERVICE_NAME = "grpc-controller"

logging.basicConfig(
    format="%(asctime)s,%(msecs)d ["
    + SERVICE_NAME
    + "] - %(levelname)-4s [%(filename)s:%(lineno)d] %(message)s",
    datefmt="%d-%m-%Y:%H:%M:%S",
)
LOGGER = logging.getLogger(__name__)
LOGGER.setLevel(logging.INFO)


def clip(value: float, min_sat: float, max_sat: float) -> float:
    min_ = min(min_sat, max_sat)
    if value < min_:
        return min_
    max_ = max(min_sat, max_sat)
    if value > max_:
        return max_
    return value


def saturate_command(
    command: float,
    previous_command: float,
    dt: float,
    pos_sat: float | None = None,
    spd_sat: float | None = None,
) -> float:
    """Saturates 'command' in position (pos_sat) and/or in speed (spd_sat).
    When saturating in position, command will be within +/-pos_sat. When saturating
    in speed, its derivative stays within +/-spd_sat."""
    if pos_sat is not None:
        command = clip(command, -pos_sat, pos_sat)
    if spd_sat is not None:
        command = clip(command, previous_command - dt * spd_sat, previous_command + dt * spd_sat)
    return command


NOT_IMPLEMENTED = " is not implemented in this model."


class AbstractControllerModel:
    """Derive from this class to implement a gRPC controller for xdyn."""

    def __init__(self, t0: float, dt: float):
        """Initialize the counters used to compute the next timestep."""
        self.t0: float = t0
        self.nb_of_calls: int = 0
        self.dt: float = dt

    def increment_nb_of_calls(self):
        """Register that a call to the controller has been made."""
        self.nb_of_calls += 1

    def get_date_of_next_callback(self) -> float:
        """Returns the date at which the controller should be called for the
        next.

        Returns
        -------
        - date_of_first_callback (float): Date at which the controller should
                                        be called for the first time.
        """
        return self.t0 + self.nb_of_calls * self.dt

    def get_angle_representation(self) -> str:
        """Which method should we call to get the commands computed by the controller?

        'QUATERNION' -> get_commands_quaternion
        'EULER_321' -> get_commands_euler_321
        """
        raise NotImplementedError("get_angle_representation" + NOT_IMPLEMENTED)

    def get_commands_quaternion(
        self, states: StatesQuaternion, dstates_dt: StatesQuaternion
    ) -> dict[str, float]:
        """Calculate the commands using quaternions instead of angles.

        Parameters
        ----------
        - states (StatesQuaternion): latest ship states
        - dstates_dt (StatesQuaternion): ship states derivative at the previous
                                         timestep

        Returns
        -------
        - commands (Dict[str,float]): commands used by xdyn's controlled forces.
          It's the controller's responsibility to ensure the names in the dict
          are recognized by xdyn. It can do that by asking the user for the
          right names via the YAML.
        """
        raise NotImplementedError("get_commands_quaternion" + NOT_IMPLEMENTED)

    def get_setpoint_names(self) -> list[str]:
        """Return the name(s) of the controller inputs (setpoints).

        These setpoints are given by the simulator

        Returns
        -------
        - setpoint_names (List[str]): names of the setpoints
        """
        raise NotImplementedError("get_setpoint_names" + NOT_IMPLEMENTED)

    def get_commands_euler_321(
        self,
        states: StatesEuler,
        dstates_dt: StatesEuler,
        setpoints: list[float],
    ) -> dict[str, float]:
        """Calculate the commands using angles in Rned2body = Rz(ψ).Ry(θ).Rx(ϕ)
        rotation convention

        Parameters
        ----------
        - states (StatesEuler): latest ship states
        - dstates_dt (StatesEuler): ship states derivative at the previous
                                    timestep
        - setpoints (Dict[str,float]): controller inputs (setpoints)

        Returns
        -------
        - commands (Dict[str,float]): commands used by xdyn's controlled forces
        """
        raise NotImplementedError("get_commands_euler_321" + NOT_IMPLEMENTED)

    def get_command_names(self) -> list[str]:
        """Return the name(s) of the controller outputs (commands).

        This will be used by the solver (e.g., xdyn) to set the value of the corresponding
        variables.
        The names are assumed to be in the same order as the numerical values computed by
        'get_commands*'

        Returns
        -------
        - commands (List[str]): commands computed by this controller
        """
        raise NotImplementedError("get_command_names" + NOT_IMPLEMENTED)


class ControllerServicer(GrpcControllerServicer):
    """Implements the gRPC methods defined in controller.proto."""

    def __init__(self, controller_class):
        """Constructor.

        Parameters
        ----------
        controller_class : Implementation of AbstractControllerModel
            Implements the controller model to use.

        """
        self.controller_class = controller_class
        self.controller = {}
        self.setpoint_names = []

    def set_parameters(self, request: SetParametersRequest, context) -> SetParametersResponse:
        """Set the parameters of self.model.

        Parameters
        ----------
        request : SetParametersRequest
            Defined in controller.proto.
        context : grpc._server._Context
            gRPC context

        Returns
        -------
            - date_of_first_callback (double): usually t0

        """
        response = SetParametersResponse()
        try:
            self.controller = self.controller_class(request.parameters, request.t0)
            self.setpoint_names = self.controller.get_setpoint_names()
            response.setpoint_names[:] = self.setpoint_names
            response.date_of_first_callback = self.controller.get_date_of_next_callback()
            response.angle_representation = SetParametersResponse.AngleRepresentation.Value(
                self.controller.get_angle_representation()
            )
            response.dt = self.controller.dt
            self.controller.increment_nb_of_calls()
            response.command_names[:] = self.controller.get_command_names()
        except KeyError as exception:
            match = closest_match(
                list(yaml.safe_load(request.parameters)),
                str(exception).replace("'", ""),
            )
            error = f"Unable to find key {str(exception)} in the YAML {match}"
            context.set_details(error)
            context.set_code(grpc.StatusCode.INVALID_ARGUMENT)
            LOGGER.error(error)
        return response

    def get_commands_euler_321(self, request, context) -> ControllerResponse:
        """Euler-angle version of the controller."""
        return self.get_commands(
            self.controller.get_commands_euler_321, from_grpc_states_euler, request, context
        )

    def get_commands_quaternion(self, request, context) -> ControllerResponse:
        """Quaternion version of the controller."""
        return self.get_commands(
            self.controller.get_commands_quaternion, from_grpc_states_quaternion, request, context
        )

    def get_commands(
        self, callback: Callable, converter: Callable, request, context
    ) -> ControllerResponse:
        """Marshalls the controller's arguments from gRPC."""
        response = ControllerResponse()
        if len(request.setpoints) != len(self.setpoint_names):
            raise IndexError(
                "The controller needs "
                + str(len(self.setpoint_names))
                + " inputs (setpoints) but "
                + str(len(request.setpoints))
                + " were provided by the simulator."
            )
        response.next_call = self.controller.get_date_of_next_callback()
        commands = callback(
            converter(request.states), converter(request.dstates_dt), request.setpoints
        )
        for key, value in commands.items():
            response.commands[key] = value
        self.controller.increment_nb_of_calls()
        return response


_ONE_DAY_IN_SECONDS = 60 * 60 * 24


def serve(controller):
    """Launch the gRPC server."""
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    add_ControllerServicer_to_server(ControllerServicer(controller), server)
    server.add_insecure_port("[::]:9002")
    server.start()
    try:
        while True:
            time.sleep(_ONE_DAY_IN_SECONDS)
    except KeyboardInterrupt:
        server.stop(0)
