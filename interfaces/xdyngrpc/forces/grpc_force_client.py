"""gRPC sample client for forces."""

import logging
from typing import Dict

from .._proto import force_pb2, force_pb2_grpc
from .grpc_force_types import (
    ForceResponse,
    RequiredWaveInformationResponse,
    SetForceParameterResponse,
    States,
    WaveInformation,
)
from .grpc_force_types_marshalling import (
    from_grpc_force_response,
    from_grpc_required_wave_information,
    to_grpc_states,
    to_grpc_wave_information,
)

SERVICE_NAME = "force-client"

logging.basicConfig(
    format="%(asctime)s,%(msecs)d ["
    + SERVICE_NAME
    + "] - %(levelname)-4s [%(filename)s:%(lineno)d] %(message)s",
    datefmt="%d-%m-%Y:%H:%M:%S",
)
LOGGER = logging.getLogger(__name__)
LOGGER.setLevel(logging.INFO)


class ForceModel:
    """This client class wraps the gRPC calls for use with "normal" python types."""

    def __init__(self, channel, parameters: str, body_name: str, instance_name: str):
        """Constructor."""
        self.stub = force_pb2_grpc.ForceStub(channel)
        self.date_of_first_callback = None
        self.setpoint_names = None
        self.angle_representation = None
        self.has_extra_observations = None
        self.instance_name = instance_name
        self.set_parameters(parameters, body_name, instance_name)

    def set_parameters(
        self, parameters: str, body_name: str, instance_name: str
    ) -> SetForceParameterResponse:
        """Set the force model's YAML parameters.

        parameters    : str
                        YAML string containing the parameters to set for this particular model.
        body_name     : str
                        Name of the body this force model is acting upon. Useful, eg., if you want
                        to express the force & torque in the body's reference frame.
        instance_name : str
                        Name of the instance of this force model. Useful, eg., if you need to use
                        the same model multiple times in the same simulation, with different
                        parameters. Eg. for a propeller model you might want a port instance & a
                        starboard one, differing in their position.
        """
        request = force_pb2.SetForceParameterRequest(
            parameters=parameters, body_name=body_name, instance_name=instance_name
        )
        response = self.stub.set_parameters(request)
        ret = SetForceParameterResponse()
        ret.max_history_length = response.max_history_length
        ret.needs_wave_outputs = response.needs_wave_outputs
        ret.commands = response.commands
        ret.frame = response.frame
        ret.x = response.x
        ret.y = response.y
        ret.z = response.z
        ret.phi = response.phi
        ret.theta = response.theta
        ret.psi = response.psi
        return ret

    def force(
        self, states: States, commands: Dict[str, float], wave_information: WaveInformation
    ) -> ForceResponse:
        """Calculates the force model's outputs.

        Parameters
        ----------
        states :           States
                           Ship states history (covering at least max_history_length)
        commands :         Dict[str, float]
                           All commands known by xdyn at this timestep
        wave_information : WaveInformation
                           Wave information that was requested by the force model
        """
        request = force_pb2.ForceRequest()
        grpc_states = to_grpc_states(states)
        request.states.CopyFrom(grpc_states)
        for key, value in commands.items():
            request.commands[key] = value
        grpc_wave_information = to_grpc_wave_information(wave_information)
        request.wave_information.CopyFrom(grpc_wave_information)
        request.instance_name = self.instance_name
        response = self.stub.force(request)
        return from_grpc_force_response(response)

    def required_wave_information(
        self, t: float, x: float, y: float, z: float, instance_name: str
    ) -> RequiredWaveInformationResponse:
        """Ask the force model what wave information it needs.

        Parameters
        ----------
        t :             float
                        Simulation time (in seconds).
        x :             float
                        Projection on axis X of the NED frame of the vector between the origin of
                        the NED frame and the origin of the BODY frame
        y :             float
                        Projection on axis Y of the NED frame of the vector between the origin of
                        the NED frame and the origin of the BODY frame
        z :             float
                        Projection on axis Z of the NED frame of the vector between the origin of
                        the NED frame and the origin of the BODY frame
        instance_name : str
                        Name of the instance of this force model. Useful, eg., if you need to use
                        the same model multiple times in the same simulation, with different
                        parameters. Eg. for a propeller model you might want a port instance & a
                        starboard one, differing in their position.

        """
        request = force_pb2.RequiredWaveInformationRequest()
        request.t = t
        request.x = x
        request.y = y
        request.z = z
        request.instance_name = instance_name
        response = self.stub.required_wave_information(request)
        return from_grpc_required_wave_information(response)
