"""Integrates a scalar wave model in a gRPC server."""

import logging
import time
from concurrent import futures

import grpc
import yaml

from ..._proto import wave_grpc_pb2_grpc, wave_types_pb2
from ...common import closest_match

SERVICE_NAME = "waves-server"

logging.basicConfig(
    format="%(asctime)s,%(msecs)d ["
    + SERVICE_NAME
    + "] - %(levelname)-4s [%(filename)s:%(lineno)d] %(message)s",
    datefmt="%d-%m-%Y:%H:%M:%S",
)
LOGGER = logging.getLogger(__name__)
LOGGER.setLevel(logging.INFO)


NOT_IMPLEMENTED = "is not implemented in this model."


class AbstractWaveModel:
    """Defines a (scalar) wave model.

    Vectorization is done by WavesServicer in module 'waves'.
    """

    def set_parameters(self, parameters: str):
        """Initialize the wave model with YAML parameters.

        Parameters
        ----------
        parameters : string
            YAML string containing the parameters of this model. Can be empty.

        Returns
        -------
        Nothing

        """
        raise NotImplementedError("set_parameters " + NOT_IMPLEMENTED)

    def elevation(self, x: float, y: float, t: float) -> float:
        """Calculate the elevations of the free surface at any point in time.

        Parameters
        ----------
        x : float
            Position (in meters) at which we want the elevation. Projected on
            the X-axis of the Earth-centered, Earth-fixed North-East-Down
            reference frame.
        y : float
            Position (in meters) at which we want the elevation. Projected on
            the Y-axis of the Earth-centered, Earth-fixed North-East-Down
            reference frame.
        t : float
            Simulation time (in seconds). All values are calculated at that
            instant. The documentation of each waves model should describe how
            the phases are defined.

        Returns
        -------
        float
            Free surface height along the Z-axis (oriented downwards) in
            meters.

        """
        raise NotImplementedError("elevation " + NOT_IMPLEMENTED)

    def dynamic_pressure(self, x: float, y: float, z: float, t: float) -> float:
        """Calculate the dynamic pressure due to waves.

        Parameters
        ----------
        x : float
            Position (in meters) at which we want the elevation. Projected on
            the X-axis of the Earth-centered, Earth-fixed North-East-Down
            reference frame.
        y : float
            Position (in meters) at which we want the elevation. Projected on
            the Y-axis of the Earth-centered, Earth-fixed North-East-Down
            reference frame.
        z : float
            Position (in meters) at which we want the elevation. Projected on
            the Z-axis of the Earth-centered, Earth-fixed North-East-Down
            reference frame.
        t : float
            Simulation time (in seconds). All values are calculated at that
            instant. The documentation of each waves model should describe how
            the phases are defined.

        Returns
        -------
        float
            Dynamic pressure (in Pascal) at (x,y,z,t).

        """
        raise NotImplementedError("dynamic_pressure " + NOT_IMPLEMENTED)

    def orbital_velocity(self, x: float, y: float, z: float, t: float) -> dict[str, float]:
        """Calculate the orbital velocity of the wave particles.

        Parameters
        ----------
        x : float
            Position (in meters) at which we want the elevation. Projected on
            the X-axis of the Earth-centered, Earth-fixed North-East-Down
            reference frame.
        y : float
            Position (in meters) at which we want the elevation. Projected on
            the Y-axis of the Earth-centered, Earth-fixed North-East-Down
            reference frame.
        z : float
            Position (in meters) at which we want the elevation. Projected on
            the Z-axis of the Earth-centered, Earth-fixed North-East-Down
            reference frame.
        t : float
            Simulation time (in seconds). All values are calculated at that
            instant. The documentation of each waves model should describe how
            the phases are defined.

        Returns
        -------
        dict
            Should contain the following fields:
            - vx (float): Projection on the X-axis of the Earth-centered,
                          Earth-fixed North-East-Down reference frame of the
                          velocity of each wave partical relative to the
                          ground. In meters per second.
            - vy (float): Projection on the Y-axis of the Earth-centered,
                          Earth-fixed North-East-Down reference frame of the
                          velocity of each wave partical relative to the
                          ground. In meters per second.
            - vz (float): Projection on the Z-axis of the Earth-centered,
                          Earth-fixed North-East-Down reference frame of the
                          velocity of each wave partical relative to the
                          ground. In meters per second.

        """
        raise NotImplementedError("orbital_velocity " + NOT_IMPLEMENTED)

    def spectrum(self, x: float, y: float, t: float, /):
        """Linear spectrum that can be used by xdyn's diffraction module.

        Parameters
        ----------
        x : float
            Position (in meters) at which we want the (linearized) spectrum.
            Projected on the X-axis of the Earth-centered, Earth-fixed
            North-East-Down reference frame.
        y : float
            Position (in meters) at which we want the (linearized) spectrum.
            Projected on the Y-axis of the Earth-centered, Earth-fixed
            North-East-Down reference frame.
        t : float
            Simulation time (in seconds). Date at which we want the
            (linearized) spectrum.

        Returns
        -------
        dict with the following fields that are all lists of floats with the
        same number of elements:
            - a (list of floats): Amplitudes for each ray. In m.
            - omega (list of floats): Angular frequencies the spectrum was
              discretized at. (In rad/s).
            - psi (list of floats): Directions between 0 & 2pi the spatial
              spreading was discretized at. (In rad).
            - k (list of floats): Discretized wave number for each
              frequency (should therefore be the same size as omega).
              (In rad/m).
            - phase (list of floats): Random phases. (In rad).

        """
        raise NotImplementedError("spectrum " + NOT_IMPLEMENTED)


class WavesServicer(wave_grpc_pb2_grpc.WavesServicer):
    """Implements the gRPC methods defined in waves.proto."""

    def __init__(self, model):
        """Constructor.

        Parameters
        ----------
        model : AbstractWaveModel
            Implements the scalar wave model to use.

        """
        self.model = model

    def set_parameters(
        self, request: wave_types_pb2.SetParameterRequest, context
    ) -> wave_types_pb2.SetParameterResponse:
        """Set the parameters of self.model.

        Parameters
        ----------
        request : SetParameterRequest
            Defined in waves.proto.
        context : grpc._server._Context
            gRPC context

        Returns
        -------
        dict
            Should contain the following fields:
            - error_message (string): empty if everything went OK.

        """
        LOGGER.info("Received parameters: %s", request.parameters)
        try:
            self.model.set_parameters(request.parameters)
        except KeyError as exception:
            match = closest_match(
                list(yaml.safe_load(request.parameters)), str(exception).replace("'", "")
            )
            context.set_details("Unable to find key " + str(exception) + " in the YAML. " + match)
            context.set_code(grpc.StatusCode.INVALID_ARGUMENT)
            return wave_types_pb2.SetParameterResponse(error_message=repr(exception))
        except Exception as exception:
            context.set_details(repr(exception))
            context.set_code(grpc.StatusCode.INVALID_ARGUMENT)
            return wave_types_pb2.SetParameterResponse(error_message=repr(exception))
        return wave_types_pb2.SetParameterResponse(error_message="")

    def elevations(self, request: wave_types_pb2.XYTGrid, context) -> wave_types_pb2.XYZTGrid:
        """Get wave elevations from self.model.

        Parameters
        ----------
        request : XYTGrid
            Defined in waves.proto.
        context : grpc._server._Context
            gRPC context

        Returns
        -------
        XYZTGrid
            Defined in waves.proto.

        """
        LOGGER.info("Got elevation request")
        xys = map(lambda x, y: {"x": x, "y": y}, request.x, request.y)
        try:
            z_s = [self.model.elevation(xy["x"], xy["y"], request.t) for xy in xys]
        except NotImplementedError as exception:
            context.set_details(repr(exception))
            context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        except Exception as exception:
            context.set_details(repr(exception))
            context.set_code(grpc.StatusCode.UNKNOWN)
        response = wave_types_pb2.XYZTGrid()
        response.x[:] = request.x
        response.y[:] = request.y
        response.z[:] = z_s
        response.t = request.t
        return response

    def dynamic_pressures(
        self, request: wave_types_pb2.XYZTGrid, context
    ) -> wave_types_pb2.DynamicPressuresResponse:
        """Get dynamic pressure from self.model.

        Parameters
        ----------
        request : XYTZGrid
            Defined in waves.proto.
        context : grpc._server._Context
            gRPC context

        Returns
        -------
        DynamicPressuresResponse
            Defined in waves.proto.

        """
        LOGGER.info("Got dynamic pressure request")
        xyzs = map(lambda x, y, z: (x, y, z), request.x, request.y, request.z)
        try:
            pdyn = [self.model.dynamic_pressure(*xyz, request.t) for xyz in xyzs]
        except NotImplementedError as exception:
            context.set_details(repr(exception))
            context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        except Exception as exception:
            context.set_details(repr(exception))
            context.set_code(grpc.StatusCode.UNKNOWN)
        response = wave_types_pb2.DynamicPressuresResponse()
        response.x[:] = request.x
        response.y[:] = request.y
        response.z[:] = request.z
        response.t = request.t
        response.pdyn[:] = pdyn
        return response

    def orbital_velocities(
        self, request: wave_types_pb2.XYZTGrid, context
    ) -> wave_types_pb2.OrbitalVelocitiesResponse:
        """Get orbital velocities from self.model.

        Parameters
        ----------
        request : XYTZGrid
            Defined in waves.proto.
        context : grpc._server._Context
            gRPC context

        Returns
        -------
        OrbitalVelocitiesResponse
            Defined in waves.proto.

        """
        LOGGER.info("Got orbital velocities request")
        xyzs = map(lambda x, y, z: (x, y, z), request.x, request.y, request.z)
        try:
            vorbs = [self.model.orbital_velocity(*xyz, request.t) for xyz in xyzs]
        except NotImplementedError as exception:
            context.set_details(repr(exception))
            context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        except Exception as exception:
            context.set_details(repr(exception))
            context.set_code(grpc.StatusCode.UNKNOWN)
        response = wave_types_pb2.OrbitalVelocitiesResponse()
        response.x[:] = request.x
        response.y[:] = request.y
        response.z[:] = request.z
        response.t = request.t
        response.vx[:] = [vorb["vx"] for vorb in vorbs]
        response.vy[:] = [vorb["vy"] for vorb in vorbs]
        response.vz[:] = [vorb["vz"] for vorb in vorbs]
        return response

    def spectrum(
        self, request: wave_types_pb2.SpectrumRequest, context
    ) -> wave_types_pb2.SpectrumResponse:
        """Get spectrum from self.model.

        Parameters
        ----------
        request : SpectrumRequest
            Defined in waves.proto.
        context : grpc._server._Context
            gRPC context

        Returns
        -------
        SpectrumResponse
            Defined in waves.proto.

        """
        LOGGER.info("Got spectrum request")
        try:
            spectrum = self.model.spectrum(request.x, request.y, request.t)
        except NotImplementedError as exception:
            context.set_details(repr(exception))
            context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        except Exception as exception:
            context.set_details(repr(exception))
            context.set_code(grpc.StatusCode.UNKNOWN)
        response = wave_types_pb2.SpectrumResponse()
        response.a[:] = spectrum["a"]
        response.k[:] = spectrum["k"]
        response.omega[:] = spectrum["omega"]
        response.psi[:] = spectrum["psi"]
        response.phase[:] = spectrum["phase"]
        return response


def serve(model: AbstractWaveModel, port: int = 50051):
    """Launch the gRPC server."""
    one_day_in_seconds = 60 * 60 * 24
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    wave_grpc_pb2_grpc.add_WavesServicer_to_server(WavesServicer(model), server)
    server.add_insecure_port(f"[::]:{port}")
    server.start()
    try:
        while True:
            time.sleep(one_day_in_seconds)
    except KeyboardInterrupt:
        server.stop(0)
