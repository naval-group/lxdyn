"""gRPC sample client for wave models."""

import logging
from typing import Any

from ..._proto import wave_grpc_pb2_grpc, wave_types_pb2

SERVICE_NAME = "waves-client"

logging.basicConfig(
    format="%(asctime)s,%(msecs)d ["
    + SERVICE_NAME
    + "] - %(levelname)-4s [%(filename)s:%(lineno)d] %(message)s",
    datefmt="%d-%m-%Y:%H:%M:%S",
)
LOGGER = logging.getLogger(__name__)
LOGGER.setLevel(logging.INFO)


class Waves:
    """This class wraps the gRPC calls for use with plain Python types."""

    def __init__(self, channel, parameters: str | None = None):
        """This constructor initializes the wave models (runs set_parameters)."""
        self.stub = wave_grpc_pb2_grpc.WavesStub(channel)
        if parameters is not None:
            response = self.set_parameters(parameters)
            if response:
                LOGGER.error("Error connecting to wave server: %s", response)

    def set_parameters(self, parameters: str):
        """Set the wave model's YAML parameters."""
        request = wave_types_pb2.SetParameterRequest(parameters=parameters)
        return self.stub.set_parameters(request).error_message

    def elevations(self, xyts) -> list[dict[str, float]]:
        """Compute wave elevations on a grid."""
        # Need to disable this Pylint warning because it doesn't play well with Protobuf
        # pylint: disable=no-member
        xytgrid = wave_types_pb2.XYTGrid()
        xytgrid.t = xyts["t"]
        for xyt in xyts["points"]:
            xytgrid.x.append(xyt[0])
            xytgrid.y.append(xyt[1])
        xyztgrid = self.stub.elevations(xytgrid)
        return list(
            map(
                lambda x, y, z: {"x": x, "y": y, "z": z, "t": xyztgrid.t},
                xyztgrid.x,
                xyztgrid.y,
                xyztgrid.z,
            )
        )

    def dynamic_pressures(self, xyzts: dict[str, Any]) -> list[dict[str, float]]:
        """Compute the wave dynamic pressure on a grid."""
        # Need to disable this Pylint warning because it doesn't play well with Protobuf
        # pylint: disable=no-member
        xyztgrid = wave_types_pb2.XYZTGrid()
        xyztgrid.t = xyzts["t"]
        for xyz in xyzts["points"]:
            xyztgrid.x.append(xyz[0])
            xyztgrid.y.append(xyz[1])
            xyztgrid.z.append(xyz[2])
        response = self.stub.dynamic_pressures(xyztgrid)
        return list(
            map(
                lambda x, y, z, pdyn: {"x": x, "y": y, "z": z, "t": response.t, "pdyn": pdyn},
                response.x,
                response.y,
                response.z,
                response.pdyn,
            )
        )

    def orbital_velocities(self, xyzts: dict[str, Any]) -> list[dict[str, float]]:
        """Compute the wave particles orbital velocity on a grid."""
        # Need to disable this Pylint warning because it doesn't play well with Protobuf
        # pylint: disable=no-member
        xyztgrid = wave_types_pb2.XYZTGrid()
        xyztgrid.t = xyzts["t"]
        for xyz in xyzts["points"]:
            xyztgrid.x.append(xyz[0])
            xyztgrid.y.append(xyz[1])
            xyztgrid.z.append(xyz[2])
        response = self.stub.orbital_velocities(xyztgrid)
        return list(
            map(
                lambda x, y, z, vx, vy, vz: {
                    "x": x,
                    "y": y,
                    "z": z,
                    "t": response.t,
                    "vx": vx,
                    "vy": vy,
                    "vz": vz,
                },
                response.x,
                response.y,
                response.z,
                response.vx,
                response.vy,
                response.vz,
            )
        )

    def spectrum(self, t: float = 0.0, x: float = 0.0, y: float = 0.0) -> dict[str, list[float]]:
        """Get a linear wave spectrum at a given point in space-time.

        For an Airy wave server, spectrum is independent from t, x and y.
        """
        request = wave_types_pb2.SpectrumRequest()
        request.t = t
        request.x = x
        request.y = y
        response = self.stub.spectrum(request)
        return {
            "a": list(response.a),
            "omega": list(response.omega),
            "psi": list(response.psi),
            "k": list(response.k),
            "phase": list(response.phase),
        }

