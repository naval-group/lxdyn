"""Facilitate the creation of a gRPC force model in Python."""
import logging
from concurrent import futures
from functools import partial
from typing import Any

import grpc
import yaml

from .._proto.force_pb2 import (
    FilteredStatesAndConvention,
    ForceResponse,
    RequiredWaveInformationResponse,
    SetForceParameterResponse,
    States,
    WaveInformation,
)
from .._proto.force_pb2_grpc import ForceServicer as GrpcForceServicer
from .._proto.force_pb2_grpc import add_ForceServicer_to_server
from ..common import closest_match
from .grpc_force_types_marshalling import from_grpc_results_from_potential_theory

SERVICE_NAME = "grpc-force"

logging.basicConfig(
    format="%(asctime)s,%(msecs)d ["
    + SERVICE_NAME
    + "] - %(levelname)-4s [%(filename)s:%(lineno)d] %(message)s",
    datefmt="%d-%m-%Y:%H:%M:%S",
)
LOGGER = logging.getLogger(__name__)
LOGGER.setLevel(logging.INFO)


NOT_IMPLEMENTED = " is not implemented in this model."


class AbstractForceModel:
    """Derive from this class to implement a gRPC force model for xdyn."""

    def get_parameters(self) -> dict[str, Any]:
        """
        Return a dictionary with all parameters needed by xdyn, with keys:

        Returns
        -------
        dict
            Should contain the following fields:
            - `max_history_length` (float): How far back (in seconds) should the
              history values in ForceRequest go?
            - `needs_wave_outputs` (bool): Should the force model be queried at
              each time step using the 'required_wave_information' rpc method
              to know what wave information it requires?
            - `required_commands` (List[str]): List of commands needed by this
              model, without the model name (e.g. ['beta1', 'beta2'])
            - `frame` (str): Reference frame from which we define the
              reference in which the forces and torques are expressed.
            - `x` (float): Position along the x-axis of 'frame' of the point of
              application of the force.
            - `y` (float): Position along the y-axis of 'frame' of the point of
              application of the force.
            - `z` (float): Position along the z-axis of 'frame' of the point of
              application of the force.
            - `phi` (float): First Euler angle defining the rotation from
              'frame' to the reference frame in which the forces and torques
              are expressed. Depends on the angle convention chosen in the
              'rotations convention' section of xdyn's input file. See xdyn's
              documentation for details.
            - `theta` (float): Second Euler angle defining the rotation from
              'frame' to the reference frame in which the forces and torques
              are expressed. Depends on the angle convention chosen in the
              'rotations convention' section of xdyn's input file. See xdyn's
              documentation for details.
            - `psi` (float): Third Euler angle defining the rotation from
              'frame' to the reference frame in which the forces and torques
              are expressed. Depends on the angle convention chosen in the
              'rotations convention' section of xdyn's input file.
              See xdyn's documentation for details.
        """
        raise NotImplementedError("get_parameters" + NOT_IMPLEMENTED)

    def required_wave_information(
        self, _t: float, _x: float, _y: float, _z: float, /
    ) -> dict[str, Any]:
        """Give the list of wave data needed by this model.

        Only called if set_parameters returns True.

        Parameters
        ----------
        t : double
            Simulation time (in seconds)
        x : double
            Projection on axis X of the NED frame of the vector between the
            origin of the NED frame and the origin of the BODY frame
        y : double
            Projection on axis Y of the NED frame of the vector between the
            origin of the NED frame and the origin of the BODY frame
        z : double
            Projection on axis Z of the NED frame of the vector between the
            origin of the NED frame and the origin of the BODY frame

        Returns
        -------
        dict
            Should contain the following fields:
            - elevations (dict): Each element should contain
                - x (list of doubles) Points at which the force model requires
                  wave elevations (projection on X-axis, in meters, expressed
                  in the Earth-centered, Earth-fixed North-East-Down reference
                  frame.
                - y (list of doubles) Points at which the force model requires
                  wave elevations (projection on Y-axis, in meters, expressed
                  in the Earth-centered, Earth-fixed North-East-Down reference
                  frame.
                - t (double) Time at which the force model requires wave
                  elevations (in seconds)
            - dynamic_pressures (dict): Each element should contain
                - x (list of doubles) Points at which the force model requires
                  dynamic pressures (projection on X-axis, in meters, expressed
                  in the Earth-centered, Earth-fixed North-East-Down reference
                  frame.
                - y (list of doubles) Points at which the force model requires
                  dynamic pressures (projection on Y-axis, in meters, expressed
                  in the Earth-centered, Earth-fixed North-East-Down reference
                  frame.
                - z (list of doubles) Points at which the force model requires
                  dynamic pressures (projection on Z-axis, in meters, expressed
                  in the Earth-centered, Earth-fixed North-East-Down reference
                  frame.
                - t (double) Time at which the force model requires dynamic
                  pressures (in seconds)
            - orbital_velocities (dict): Each element should contain
                - x (list of doubles) Points at which the force model requires
                  orbital velocities (projection on X-axis, in meters,
                  expressed in the Earth-centered, Earth-fixed North-East-Down
                  reference frame.
                - y (list of doubles) Points at which the force model requires
                  orbital velocities (projection on Y-axis, in meters,
                  expressed in the Earth-centered, Earth-fixed North-East-Down
                  reference frame.
                - z (list of doubles) Points at which the force model requires
                  orbital velocities (projection on Z-axis, in meters,
                  expressed in the Earth-centered, Earth-fixed North-East-Down
                  reference frame.
                - t (double) Time at which the force model requires orbital
                  velocities (in seconds)
            - spectrum (dict or None): If it's a dict, should contain:
                - x (double) Projection on the X axis of the NED frame of the
                  position at which we want the linearized wave spectrum
                - y (double) Projection on the Y axis of the NED frame of the
                  position at which we want the linearized wave spectrum
                - t (double) Simulation time at which we want the linearized
                  wave spectrum
        """
        return dict()

    def force(
        self,
        _states: States,
        _commands: dict[str, float],
        _wave_information: WaveInformation,
        _filtered_states: FilteredStatesAndConvention,
        /,
    ):
        """Calculate the force & torque.

        Parameters
        ----------
        - states : dict
            Should contain the following fields:
            - t (list of doubles): dates at which the state values are given.
              max(t)-min(t) <= max_history_length returned by set_parameters.
            - x (list of doubles): Value of x for each t. x[0] is the
              current (at t) projection on axis X of the NED frame of the
              vector between the origin of the NED frame and the origin of the
              BODY frame.
            - y (list of doubles): Value of y for each t. y[0] is the
              current (at t) projection on axis Y of the NED frame of the
              vector between the origin of the NED frame and the origin of the
              BODY frame.
            - z (list of doubles): Value of z for each t. z[0] is the
              current (at t) projection on axis Z of the NED frame of the
              vector between the origin of the NED frame and the origin of the
              BODY frame.
            - u (list of doubles): Value of u for each t. u[0] is the current
              (at t) projection on axis X of the NED frame of the vector of the
              ship's speed relative to the ground (BODY/NED).
            - v (list of doubles): Value of v for each t. v[0] is the current
              (at t) projection on axis Y of the NED frame of the vector of the
              ship's speed relative to the ground (BODY/NED).
            - w (list of doubles): Value of w for each t. w[0] is the current
              (at t) projection on axis Z of the NED frame of the vector of the
              ship's speed relative to the ground (BODY/NED).
            - p (list of doubles): Value of p for each t. p[0] is the
              current (at t) of the projection on axis X of the NED frame of
              the vector of the ship's rotation speed relative to the ground
              (BODY/NED).
            - q (list of doubles): Value of q for each t. q[0] is the
              current (at t) of the projection on axis Y of the NED frame of
              the vector of the ship's rotation speed relative to the ground
              (BODY/NED).
            - r (list of doubles): Value of r for each t. r[0] is the
              current (at t) of the projection on axis Z of the NED frame of
              the vector of the ship's rotation speed relative to the ground
              (BODY/NED).
            - qr (list of doubles): Value of qr for each t. qr[0] is the
              current (at t) value of the real part of the quaternion defining
              the rotation from the NED frame to the ship's BODY frame.
            - qi (list of doubles): Value of qi for each t. qi[0] is the
              current (at t) value of the first imaginary part of the
              quaternion defining the rotation from the NED frame to the ship's
              BODY frame.
            - qj (list of doubles): Value of qj for each t. qj[0] is the
              current (at t) value of the second imaginary part of the
              quaternion defining the rotation from the NED frame to the ship's
              BODY frame.
            - qk (list of doubles): Value of qk for each t. qk[0] is the
              current (at t) value of the third imaginary part of the
              quaternion defining the rotation from the NED frame to the ship's
              BODY frame.
            - phi (list of doubles): Value of phi for each t. phi[0] is the
              current value (at t) of one of the three Euler angles, calculated
              using the angle convention chosen in the 'rotations convention'
              section of xdyn's input file. See xdyn's documentation for details.
            - theta (list of doubles): Value of theta for each t. theta[0] is
              the current value (at t) of one of the three Euler angles, calcualted
              using the angle convention chosen in the 'rotations convention'
              section of xdyn's input file. See xdyn's documentation for details.
            - psi (list of doubles): Value of psi for each t. psi[0] is the
              current value (at t) of one of the three Euler angles, calculated
              using the angle convention chosen in the 'rotations convention'
              section of xdyn's input file. See xdyn's documentation for
              details.
            - rotations_convention (string): Angle convention chosen in xdyn's
              YAML file. Use it to check the convention is what you are
              expecting! Format: ["psi", "theta'", "phi''"].
        - commands (dict): contains the current (at t) values of all the
          commands available to all the controlled forces of the simulation.
          The name of the commands is: model_name(command_name) e.g.
          PropRudd(rpm)
        - wave_information (dict): wave information requested by the force
          model (rpc method 'required_wave_information'). Should contain the
          following fields:
            - elevations (dict):
              Should contain the following fields:
            - dynamic_pressures (dict):
              Should contain the following fields:
            - orbital_velocities (dict): Should contain the following fields:
              - vx (list of doubles): Projection on the X-axis of the
                Earth-centered, Earth-fixed North-East-Down reference frame of
                the velocity of each wave partical relative to the ground. In
                meters per second. Same size and ordering as input from rpc
                'required_wave_information' (orbital_velocities).
              - vy (list of doubles): Projection on the Y-axis of the
                Earth-centered, Earth-fixed North-East-Down reference frame of
                the velocity of each wave partical relative to the ground. In
                meters per second. Same size and ordering as input from rpc
                'required_wave_information' (orbital_velocities).
              - vz (list of doubles): Projection on the Z-axis of the
                Earth-centered, Earth-fixed North-East-Down reference frame of
                the velocity of each wave partical relative to the ground. In
                meters per second. Same size and ordering as input from rpc
                'required_wave_information' (orbital_velocities).
            - spectrum (dict): Dictionary with the following fields that are
              all lists of floats with the same number of elements:
                - a (list of floats): Amplitudes for each ray. In m.
                - omega (list of floats): Angular frequencies the spectrum was
                  discretized at. (In rad/s).
                - psi (list of floats): Directions between 0 & 2pi the spatial
                  spreading was discretized at. (In rad).
                  0° is for waves coming from the South and propagating to the North.
                  90° is for waves coming from the West and propagating to the East.
                - k (list of floats): Discretized wave number for each
                  frequency (should therefore be the same size as omega).
                  (In rad/m).
                - phase (list of floats): Random phases. (In rad).
        - filtered_states : class
            Should contain the following fields:
            - x (float): Current filtered projection on axis X of the NED frame of the
              vector between the origin of the NED frame and the origin of the
              BODY frame.
            - y (float): Current filtered projection on axis Y of the NED frame of the
              vector between the origin of the NED frame and the origin of the
              BODY frame.
            - z (float): Current filtered projection on axis Z of the NED frame of the
              vector between the origin of the NED frame and the origin of the
              BODY frame.
            - u (float): Current filtered projection on axis X of the NED frame
              of the vector of the ship's speed relative to the ground
              (BODY/NED).
            - v (float): Current filtered projection on axis Y of the NED frame
              of the vector of the ship's speed relative to the ground
              (BODY/NED).
            - w (float): Current filtered projection on axis Z of the NED frame
              of the vector of the ship's speed relative to the ground
              (BODY/NED).
            - p (float): Current filtered projection on axis X of the NED frame
              of the vector of the ship's rotation speed relative to the ground
              (BODY/NED).
            - q (float): Current filtered projection on axis Y of the NED frame
              of the vector of the ship's rotation speed relative to the ground
              (BODY/NED).
            - r (float): Current filtered projection on axis Z of the NED frame
              of the vector of the ship's rotation speed relative to the ground
              (BODY/NED).
            - phi (float): Current filtered value of one of the three Euler
              angles, calculated using the angle convention chosen in the
              'rotations convention' section of xdyn's input file. See xdyn's
              documentation for details.
            - theta (float): Current filtered value of one of the three Euler
              angles, calculated using the angle convention chosen in the
              'rotations convention' section of xdyn's input file. See xdyn's
              documentation for details.
            - psi (float): Current filtered value of one of the three Euler
              angles, calculated using the angle convention chosen in the
              'rotations convention' section of xdyn's input file. See xdyn's
              documentation for details.
            - rotations_convention (string): Angle convention chosen in xdyn's
              YAML file. Use it to check the convention is what you are
              expecting! Format: ["psi", "theta'", "phi''"].


        Returns
        -------
        dict
            Should contain the following fields:
            - Fx (double) Projection of the force acting on "BODY" on the
              X-axis of the body frame (in N).
            - Fy (double) Projection of the force acting on "BODY" on the
              Y-axis of the body frame (in N).
            - Fz (double) Projection of the force acting on "BODY" on the
              Z-axis of the body frame (in N).
            - Mx (double) Projection of the torque acting on "BODY" on the
              X-axis of the body frame (in N.m).
            - My (double) Projection of the torque acting on "BODY" on the
              Y-axis of the body frame (in N.m).
            - Mz (double) Projection of the torque acting on "BODY" on the
              Z-axis of the body frame (in N.m).
            - extra_observations (dict string -> double): Anything we wish
              to serialize. Specific to each force model. Not taken into
              account in the numerical integration & not available to other
              force or environment models.

        """
        raise NotImplementedError("force" + NOT_IMPLEMENTED)


class ForceServicer(GrpcForceServicer):
    """Implements the gRPC methods defined in force.proto."""

    def __init__(self, model_class: type[AbstractForceModel]):
        """Constructor.

        Parameters
        ----------
        model_class : Implementation of an AbstractForceModel
            Implements the scalar force model to use.

        """
        self.wave_information_required: bool = False
        self.model_class: type[AbstractForceModel] = model_class
        self.model: dict[str, AbstractForceModel] = {}
        self.required_commands: dict[str, list[str]] = {}

    def set_parameters(self, request, context) -> SetForceParameterResponse:
        """Set the parameters of self.model.

        Parameters
        ----------
        request : SetParameterRequest
            Defined in force.proto.
        context : grpc._server._Context
            gRPC context

        Returns
        -------
        dict
            Should contain the following fields:
            - max_history_length (double): How far back (in seconds) should the
              history values in ForceRequest go?
            - needs_wave_outputs (bool): Should the wave model be queried at
              each time step using the 'required_wave_information' rpc method
              to know what wave information it requires?
            - commands (repeated string): List of commands needed by this
              model, without the model name (e.g. ['beta1', 'beta2'])
            - frame (string): Reference frame from which we define the
              reference in which the forces and torques are expressed.
            - x (double): Position along the x-axis of 'frame' of the point of
              application of the force.
            - y (double): Position along the y-axis of 'frame' of the point of
              application of the force.
            - z (double): Position along the z-axis of 'frame' of the point of
              application of the force.
            - phi (double): First Euler angle defining the rotation from
              'frame' to the reference frame in which the forces and torques
              are expressed. Depends on the angle convention chosen in the
              'rotations convention' section of xdyn's input file. See xdyn's
              documentation for details.
            - theta (double): Second Euler angle defining the rotation from
              'frame' to the reference frame in which the forces and torques
              are expressed. Depends on the angle convention chosen in the
              'rotations convention' section of xdyn's input file. See xdyn's
              documentation for details.
            - psi (double): Third Euler angle defining the rotation from
              'frame' to the reference frame in which the forces and torques
              are expressed. Depends on the angle convention chosen in the
              'rotations convention' section of xdyn's input file.
              See xdyn's documentation for details.

        """
        LOGGER.info("Received parameters: %s", request.parameters)
        response = SetForceParameterResponse()
        try:
            pot_theory = from_grpc_results_from_potential_theory(
                request.results_from_potential_theory
            )
            instance = self.model_class(request.parameters, request.body_name, pot_theory)
            self.model[request.instance_name] = instance
            out = instance.get_parameters()
            self.required_commands[request.instance_name] = out["required_commands"]
            response.max_history_length = out["max_history_length"]
            response.needs_wave_outputs = out["needs_wave_outputs"]
            response.frame = out["frame"]
            response.x = out["x"]
            response.y = out["y"]
            response.z = out["z"]
            response.phi = out["phi"]
            response.theta = out["theta"]
            response.psi = out["psi"]
            response.commands[:] = out["required_commands"]
            self.wave_information_required = response.needs_wave_outputs
        except KeyError as exception:
            match = closest_match(
                list(yaml.safe_load(request.parameters)), str(exception).replace("'", "")
            )
            context.set_details("Unable to find key " + str(exception) + " in the YAML. " + match)
            context.set_code(grpc.StatusCode.INVALID_ARGUMENT)
        return response

    def to_xdyn_command_name(self, instance_name: str, command: str) -> str:
        """Convert a command name to a form recognizable by xdyn."""
        return instance_name + "(" + command + ")"

    def get_command(
        self, available_commands: dict[str, float], instance_name: str, command: str
    ) -> float:
        """Get command from xdyn or throw an exception."""
        formatted_command = self.to_xdyn_command_name(instance_name, command)
        if formatted_command not in available_commands:
            raise KeyError(
                "Command '"
                + formatted_command
                + "' was not provided. Got ["
                + ",".join(available_commands)
                + "]"
            )
        return available_commands[formatted_command]

    def force(self, request, context) -> ForceResponse:
        """Marshall force model's arguments from gRPC."""
        response = ForceResponse()
        required_commands = self.required_commands[request.instance_name]
        get_command_value = partial(self.get_command, request.commands, request.instance_name)
        commands = {command: get_command_value(command) for command in required_commands}
        out = self.model[request.instance_name].force(
            request.states, commands, request.wave_information, request.filtered_states
        )
        response.Fx = out["Fx"]
        response.Fy = out["Fy"]
        response.Fz = out["Fz"]
        response.Mx = out["Mx"]
        response.My = out["My"]
        response.Mz = out["Mz"]
        response.extra_observations.update(out["extra_observations"])
        return response

    def required_wave_information(self, request, context) -> RequiredWaveInformationResponse:
        response = RequiredWaveInformationResponse()
        if not self.wave_information_required:
            return response
        required_wave_information = self.model[request.instance_name].required_wave_information(
            request.t, request.x, request.y, request.z
        )
        if "elevations" in required_wave_information:
            response.elevations.x[:] = required_wave_information["elevations"]["x"]
            response.elevations.y[:] = required_wave_information["elevations"]["y"]
            response.elevations.t = required_wave_information["elevations"]["t"]
        if "dynamic_pressures" in required_wave_information:
            response.dynamic_pressures.x[:] = required_wave_information["dynamic_pressures"]["x"]
            response.dynamic_pressures.y[:] = required_wave_information["dynamic_pressures"]["y"]
            response.dynamic_pressures.z[:] = required_wave_information["dynamic_pressures"]["z"]
            response.dynamic_pressures.t = required_wave_information["dynamic_pressures"]["t"]
        if "orbital_velocities" in required_wave_information:
            response.orbital_velocities.x[:] = required_wave_information["orbital_velocities"]["x"]
            response.orbital_velocities.y[:] = required_wave_information["orbital_velocities"]["y"]
            response.orbital_velocities.z[:] = required_wave_information["orbital_velocities"]["z"]
            response.orbital_velocities.t = required_wave_information["orbital_velocities"]["t"]
        if "spectrum" in required_wave_information:
            response.need_spectrum = True
            response.spectrum.x = required_wave_information["spectrum"]["x"]
            response.spectrum.y = required_wave_information["spectrum"]["y"]
            response.spectrum.t = required_wave_information["spectrum"]["t"]
        return response


def start(model: type[AbstractForceModel], port: int = 9002) -> tuple[grpc.Server, int]:
    """Start the gRPC server and return it with the port it bound, which port 0 picks."""
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    add_ForceServicer_to_server(ForceServicer(model), server)
    bound_port = server.add_insecure_port(f"[::]:{port}")
    server.start()
    return server, bound_port


def serve(model: type[AbstractForceModel], port: int = 9002):
    """Launch the gRPC server and block until interrupted."""
    server, _ = start(model, port)
    try:
        server.wait_for_termination()
    except KeyboardInterrupt:
        server.stop(0)
