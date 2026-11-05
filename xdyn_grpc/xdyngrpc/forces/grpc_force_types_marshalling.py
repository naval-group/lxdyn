"""Converts to & from gRPC force types."""


import numpy as np

from .._proto import force_pb2, wave_types_pb2
from .grpc_force_types import (
    DynamicPressuresResponse,
    ForceResponse,
    OrbitalVelocitiesResponse,
    RequiredWaveInformationResponse,
    ResultsFromPotentialTheory,
    Spectrum,
    SpectrumRequest,
    States,
    WaveInformation,
    WrenchMatrices,
    XYTGrid,
    XYZTGrid,
)


def to_grpc_states(states: States) -> force_pb2.States:
    """Convert Python dataclass to protobuf."""
    ret = force_pb2.States()
    ret.t[:] = states.t
    ret.x[:] = states.x
    ret.y[:] = states.y
    ret.z[:] = states.z
    ret.u[:] = states.u
    ret.v[:] = states.v
    ret.w[:] = states.w
    ret.p[:] = states.p
    ret.q[:] = states.q
    ret.r[:] = states.r
    ret.phi[:] = states.phi
    ret.theta[:] = states.theta
    ret.psi[:] = states.psi
    return ret


def to_grpc_xyztgrid(xyzt_grid: XYZTGrid) -> wave_types_pb2.XYZTGrid:
    """Convert Python XYZTGrid to gRPC XYZTGrid."""
    ret = wave_types_pb2.XYZTGrid()
    ret.x[:] = xyzt_grid.x
    ret.y[:] = xyzt_grid.y
    ret.z[:] = xyzt_grid.z
    ret.t = xyzt_grid.t
    return ret


def to_grpc_dynamic_pressure_response(
    dp: DynamicPressuresResponse,
) -> wave_types_pb2.DynamicPressuresResponse:
    """Convert Python DynamicPressuresResponse to gRPC DynamicPressuresResponse."""
    ret = wave_types_pb2.DynamicPressuresResponse()
    ret.x[:] = dp.x
    ret.y[:] = dp.y
    ret.z[:] = dp.z
    ret.t = dp.t
    ret.pdyn[:] = dp.pdyn
    return ret


def to_grpc_orbital_velocities(
    vo: OrbitalVelocitiesResponse,
) -> wave_types_pb2.OrbitalVelocitiesResponse:
    """Convert Python OrbitalVelocitiesResponse to gRPC OrbitalVelocitiesResponse."""
    ret = wave_types_pb2.OrbitalVelocitiesResponse()
    ret.x[:] = vo.x
    ret.y[:] = vo.y
    ret.z[:] = vo.z
    ret.t = vo.t
    ret.vx[:] = vo.vx
    ret.vy[:] = vo.vy
    ret.vz[:] = vo.vz
    return ret


def to_grpc_spectrum(spectrum: Spectrum) -> wave_types_pb2.SpectrumResponse:
    """Convert Python Spectrum to gRPC SpectrumResponse."""
    ret = wave_types_pb2.SpectrumResponse()
    ret.a[:] = spectrum.a
    ret.psi[:] = spectrum.psi
    ret.omega[:] = spectrum.omega
    ret.k[:] = spectrum.k
    ret.phase[:] = spectrum.phase
    return ret


def to_grpc_wave_information(wave_information: WaveInformation) -> force_pb2.WaveInformation:
    """Convert Python WaveInformation to gRPC WaveInformation."""
    ret = force_pb2.WaveInformation()
    ret.elevations.CopyFrom(to_grpc_xyztgrid(wave_information.elevations))
    ret.dynamic_pressures.CopyFrom(
        to_grpc_dynamic_pressure_response(wave_information.dynamic_pressures)
    )
    ret.orbital_velocities.CopyFrom(to_grpc_orbital_velocities(wave_information.orbital_velocities))
    ret.spectrum.CopyFrom(to_grpc_spectrum(wave_information.spectrum))
    return ret


def from_grpc_force_response(force_response: force_pb2.ForceResponse) -> ForceResponse:
    """Convert gRPC ForceResponse to Python ForceResponse."""
    ret = ForceResponse()
    ret.Fx = force_response.Fx
    ret.Fy = force_response.Fy
    ret.Fz = force_response.Fz
    ret.Mx = force_response.Mx
    ret.My = force_response.My
    ret.Mz = force_response.Mz
    for key in force_response.extra_observations:
        ret.extra_observations[key] = force_response.extra_observations[key]
    return ret


def from_grpc_added_mass_matrix(added_mass_matrix: force_pb2.AddedMassMatrix) -> np.ndarray:
    """Convert gRPC AddedMassMatrix to Python Numpy array."""
    ret = np.zeros(shape=(6, 6))
    ret[0][0] = added_mass_matrix.Ma_11
    ret[0][1] = added_mass_matrix.Ma_12
    ret[0][2] = added_mass_matrix.Ma_13
    ret[0][3] = added_mass_matrix.Ma_14
    ret[0][4] = added_mass_matrix.Ma_15
    ret[0][5] = added_mass_matrix.Ma_16
    ret[1][0] = added_mass_matrix.Ma_21
    ret[1][1] = added_mass_matrix.Ma_22
    ret[1][2] = added_mass_matrix.Ma_23
    ret[1][3] = added_mass_matrix.Ma_24
    ret[1][4] = added_mass_matrix.Ma_25
    ret[1][5] = added_mass_matrix.Ma_26
    ret[2][0] = added_mass_matrix.Ma_31
    ret[2][1] = added_mass_matrix.Ma_32
    ret[2][2] = added_mass_matrix.Ma_33
    ret[2][3] = added_mass_matrix.Ma_34
    ret[2][4] = added_mass_matrix.Ma_35
    ret[2][5] = added_mass_matrix.Ma_36
    ret[3][0] = added_mass_matrix.Ma_41
    ret[3][1] = added_mass_matrix.Ma_42
    ret[3][2] = added_mass_matrix.Ma_43
    ret[3][3] = added_mass_matrix.Ma_44
    ret[3][4] = added_mass_matrix.Ma_45
    ret[3][5] = added_mass_matrix.Ma_46
    ret[4][0] = added_mass_matrix.Ma_51
    ret[4][1] = added_mass_matrix.Ma_52
    ret[4][2] = added_mass_matrix.Ma_53
    ret[4][3] = added_mass_matrix.Ma_54
    ret[4][4] = added_mass_matrix.Ma_55
    ret[4][5] = added_mass_matrix.Ma_56
    ret[5][0] = added_mass_matrix.Ma_61
    ret[5][1] = added_mass_matrix.Ma_62
    ret[5][2] = added_mass_matrix.Ma_63
    ret[5][3] = added_mass_matrix.Ma_64
    ret[5][4] = added_mass_matrix.Ma_65
    ret[5][5] = added_mass_matrix.Ma_66
    return ret


def get_grpc_array_size(array: force_pb2.Array) -> tuple[int, int]:
    """Return the size of the gRPC Array in a form suitable for Numpy."""
    nx = len(array.line)
    ny = 0
    for line in array.line:
        ny = max(ny, len(line.element))
    return (nx, ny)


def from_grpc_array(array: force_pb2.Array) -> np.ndarray:
    """Convert gRPC Array to Python Numpy 2D-array."""
    ret = np.zeros(get_grpc_array_size(array))
    i = 0
    for line in array.line:
        j = 0
        for element in line.element:
            ret[i][j] = element
            j += 1
        i += 1
    return ret


def from_grpc_wrench_matrices(wrench_matrices: force_pb2.WrenchMatrices) -> WrenchMatrices:
    """Convert gRPC WrenchMatrices to Python WrenchMatrices."""
    return WrenchMatrices(
        X=from_grpc_array(wrench_matrices.X),
        Y=from_grpc_array(wrench_matrices.Y),
        Z=from_grpc_array(wrench_matrices.Z),
        K=from_grpc_array(wrench_matrices.K),
        M=from_grpc_array(wrench_matrices.M),
        N=from_grpc_array(wrench_matrices.N),
    )


def get_number_of_frequencies(frequency_matrix: force_pb2.FrequencyMatrix) -> int:
    """Returns the max number of elements in FrequencyMatrix, to initialize a np.ndarray."""
    n = max(0, len(frequency_matrix.Ma_11))
    n = max(n, len(frequency_matrix.Ma_12))
    n = max(n, len(frequency_matrix.Ma_13))
    n = max(n, len(frequency_matrix.Ma_14))
    n = max(n, len(frequency_matrix.Ma_15))
    n = max(n, len(frequency_matrix.Ma_16))
    n = max(n, len(frequency_matrix.Ma_21))
    n = max(n, len(frequency_matrix.Ma_22))
    n = max(n, len(frequency_matrix.Ma_23))
    n = max(n, len(frequency_matrix.Ma_24))
    n = max(n, len(frequency_matrix.Ma_25))
    n = max(n, len(frequency_matrix.Ma_26))
    n = max(n, len(frequency_matrix.Ma_31))
    n = max(n, len(frequency_matrix.Ma_32))
    n = max(n, len(frequency_matrix.Ma_33))
    n = max(n, len(frequency_matrix.Ma_34))
    n = max(n, len(frequency_matrix.Ma_35))
    n = max(n, len(frequency_matrix.Ma_36))
    n = max(n, len(frequency_matrix.Ma_41))
    n = max(n, len(frequency_matrix.Ma_42))
    n = max(n, len(frequency_matrix.Ma_43))
    n = max(n, len(frequency_matrix.Ma_44))
    n = max(n, len(frequency_matrix.Ma_45))
    n = max(n, len(frequency_matrix.Ma_46))
    n = max(n, len(frequency_matrix.Ma_51))
    n = max(n, len(frequency_matrix.Ma_52))
    n = max(n, len(frequency_matrix.Ma_53))
    n = max(n, len(frequency_matrix.Ma_54))
    n = max(n, len(frequency_matrix.Ma_55))
    n = max(n, len(frequency_matrix.Ma_56))
    n = max(n, len(frequency_matrix.Ma_61))
    n = max(n, len(frequency_matrix.Ma_62))
    n = max(n, len(frequency_matrix.Ma_63))
    n = max(n, len(frequency_matrix.Ma_64))
    n = max(n, len(frequency_matrix.Ma_65))
    n = max(n, len(frequency_matrix.Ma_66))
    return n


def from_grpc_frequency_matrix(frequency_matrix: force_pb2.FrequencyMatrix) -> np.ndarray:
    """Convert gRPC FrequencyMatrix to Python Numpy 3D array."""
    ret = np.zeros(shape=(6, 6, get_number_of_frequencies(frequency_matrix)))
    ret[0][0][:] = np.array(frequency_matrix.Ma_11)
    ret[0][1][:] = np.array(frequency_matrix.Ma_12)
    ret[0][2][:] = np.array(frequency_matrix.Ma_13)
    ret[0][3][:] = np.array(frequency_matrix.Ma_14)
    ret[0][4][:] = np.array(frequency_matrix.Ma_15)
    ret[0][5][:] = np.array(frequency_matrix.Ma_16)
    ret[1][0][:] = np.array(frequency_matrix.Ma_21)
    ret[1][1][:] = np.array(frequency_matrix.Ma_22)
    ret[1][2][:] = np.array(frequency_matrix.Ma_23)
    ret[1][3][:] = np.array(frequency_matrix.Ma_24)
    ret[1][4][:] = np.array(frequency_matrix.Ma_25)
    ret[1][5][:] = np.array(frequency_matrix.Ma_26)
    ret[2][0][:] = np.array(frequency_matrix.Ma_31)
    ret[2][1][:] = np.array(frequency_matrix.Ma_32)
    ret[2][2][:] = np.array(frequency_matrix.Ma_33)
    ret[2][3][:] = np.array(frequency_matrix.Ma_34)
    ret[2][4][:] = np.array(frequency_matrix.Ma_35)
    ret[2][5][:] = np.array(frequency_matrix.Ma_36)
    ret[3][0][:] = np.array(frequency_matrix.Ma_41)
    ret[3][1][:] = np.array(frequency_matrix.Ma_42)
    ret[3][2][:] = np.array(frequency_matrix.Ma_43)
    ret[3][3][:] = np.array(frequency_matrix.Ma_44)
    ret[3][4][:] = np.array(frequency_matrix.Ma_45)
    ret[3][5][:] = np.array(frequency_matrix.Ma_46)
    ret[4][0][:] = np.array(frequency_matrix.Ma_51)
    ret[4][1][:] = np.array(frequency_matrix.Ma_52)
    ret[4][2][:] = np.array(frequency_matrix.Ma_53)
    ret[4][3][:] = np.array(frequency_matrix.Ma_54)
    ret[4][4][:] = np.array(frequency_matrix.Ma_55)
    ret[4][5][:] = np.array(frequency_matrix.Ma_56)
    ret[5][0][:] = np.array(frequency_matrix.Ma_61)
    ret[5][1][:] = np.array(frequency_matrix.Ma_62)
    ret[5][2][:] = np.array(frequency_matrix.Ma_63)
    ret[5][3][:] = np.array(frequency_matrix.Ma_64)
    ret[5][4][:] = np.array(frequency_matrix.Ma_65)
    ret[5][5][:] = np.array(frequency_matrix.Ma_66)
    return ret


def from_grpc_results_from_potential_theory(
    res: force_pb2.ResultsFromPotentialTheory,
) -> ResultsFromPotentialTheory:
    """Convert gRPC ResultsFromPotentialTheory to Python ResultsFromPotentialTheory."""
    return ResultsFromPotentialTheory(
        Ma=from_grpc_added_mass_matrix(res.Ma),
        diffraction_module_tables=from_grpc_wrench_matrices(res.diffraction_module_tables),
        diffraction_phase_tables=from_grpc_wrench_matrices(res.diffraction_phase_tables),
        diffraction_module_periods=np.array(res.diffraction_module_periods),
        diffraction_phase_periods=np.array(res.diffraction_phase_periods),
        diffraction_module_psis=np.array(res.diffraction_module_psis),
        diffraction_phase_psis=np.array(res.diffraction_phase_psis),
        froude_krylov_module_tables=from_grpc_wrench_matrices(res.froude_krylov_module_tables),
        froude_krylov_phase_tables=from_grpc_wrench_matrices(res.froude_krylov_phase_tables),
        froude_krylov_module_periods=np.array(res.froude_krylov_module_periods),
        froude_krylov_phase_periods=np.array(res.froude_krylov_phase_periods),
        froude_krylov_module_psis=np.array(res.froude_krylov_module_psis),
        froude_krylov_phase_psis=np.array(res.froude_krylov_phase_psis),
        angular_frequencies=np.array(res.angular_frequencies),
        forward_speed=res.forward_speed,
        added_mass_coeff=from_grpc_frequency_matrix(res.added_mass_coeff),
        radiation_damping_coeff=from_grpc_frequency_matrix(res.radiation_damping_coeff),
        wave_drift_force_tables=from_grpc_wrench_matrices(res.wave_drift_force_tables),
        wave_drift_periods=np.array(res.wave_drift_periods),
        wave_drift_psis=np.array(res.wave_drift_psis),
    )


def from_grpc_xyt_grid(xyt_grid: wave_types_pb2.XYTGrid) -> XYTGrid:
    """Convert gRPC XYTGrid to Python XYTGrid."""
    return XYTGrid(x=list(xyt_grid.x), y=list(xyt_grid.y), t=xyt_grid.t)


def from_grpc_xyzt_grid(xyzt_grid: wave_types_pb2.XYZTGrid) -> XYZTGrid:
    """Convert gRPC XYZTGrid to Python XYZTGrid."""
    return XYZTGrid(x=list(xyzt_grid.x), y=list(xyzt_grid.y), z=list(xyzt_grid.z), t=xyzt_grid.t)


def from_grpc_spectrum(spectrum: wave_types_pb2.SpectrumRequest) -> SpectrumRequest:
    """Convert gRPC SpectrumRequest to Python SpectrumRequest."""
    ret = SpectrumRequest()
    ret.x = spectrum.x
    ret.y = spectrum.y
    ret.t = spectrum.t
    return ret


def from_grpc_required_wave_information(
    required_wave_information: force_pb2.RequiredWaveInformationResponse,
) -> RequiredWaveInformationResponse:
    """Convert gRPC RequiredWaveInformationResponse to Python RequiredWaveInformationResponse."""
    return RequiredWaveInformationResponse(
        elevations=from_grpc_xyt_grid(required_wave_information.elevations),
        dynamic_pressures=from_grpc_xyzt_grid(required_wave_information.dynamic_pressures),
        orbital_velocities=from_grpc_xyzt_grid(required_wave_information.orbital_velocities),
        spectrum=from_grpc_spectrum(required_wave_information.spectrum),
        need_spectrum=required_wave_information.need_spectrum,
    )
