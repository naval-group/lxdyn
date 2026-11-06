"""Conversions between the gRPC messages and the Python dataclasses."""

from xdyngrpc._proto import force_pb2
from xdyngrpc.forces.grpc_force_types_marshalling import from_grpc_required_wave_information


def test_required_wave_information_carries_every_field():
    response = force_pb2.RequiredWaveInformationResponse()
    response.elevations.x[:] = [1, 2]
    response.elevations.y[:] = [3, 4]
    response.elevations.t = 5
    response.dynamic_pressures.z[:] = [7]
    response.orbital_velocities.x[:] = [9]
    response.spectrum.x = 1.5
    response.need_spectrum = True

    info = from_grpc_required_wave_information(response)

    assert info.elevations.x == [1, 2]
    assert info.elevations.y == [3, 4]
    assert info.elevations.t == 5
    assert info.dynamic_pressures.z == [7]
    assert info.orbital_velocities.x == [9]
    assert info.spectrum.x == 1.5
    assert info.need_spectrum
