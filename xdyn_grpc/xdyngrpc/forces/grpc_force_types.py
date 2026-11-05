"""Image of the "force" gRPC types."""

from dataclasses import dataclass, field

import numpy as np


@dataclass
class States:
    """Ship states history supplied to the controller. Using quaternions.

    Attributes:
        t (List float):     Simulation time (in seconds).
        x (List float):     Projection on axis X of the NED frame of the vector between
                            the origin of the NED frame and the origin of the BODY frame
                            In metres.
        y (List float):     Projection on axis Y of the NED frame of the vector between
                            the origin of the NED frame and the origin of the BODY frame
                            In metres.
        z (List float):     Projection on axis Z of the NED frame of the vector between
                            the origin of the NED frame and the origin of the BODY frame
                            In metres.
        u (List float):     Projection on axis X of the NED frame of the vector of the
                            ship's speed relative to the ground (BODY/NED). In m/s.
        v (List float):     Projection on axis Y of the NED frame of the vector of the
                            ship's speed relative to the ground (BODY/NED). In m/s.
        w (List float):     Projection on axis Z of the NED frame of the vector of the
                            ship's speed relative to the ground (BODY/NED). In m/s.
        p (List float):     Projection on axis X of the NED frame of the vector of the
                            ship's rotation speed relative to the ground (BODY/NED).
                            In rad/s.
        q (List float):     Projection on axis Y of the NED frame of the vector of the
                            ship's rotation speed relative to the ground (BODY/NED).
                            In rad/s.
        r (List float):     Projection on axis Z of the NED frame of the vector
                            of the ship's rotation speed relative to the ground
                            (BODY/NED). In rad/s.
        qr (List float):    Real part of the quaternion defining the rotation
                            from the NED frame to the ship's BODY frame.
        qi (List float):    First imaginary part of the quaternion defining the
                            rotation from the NED frame to the ship's BODY frame.
        qj (List float):    Second imaginary part of the quaternion defining the
                            rotation from the NED frame to the ship's BODY frame.
        qk (List float):    Third imaginary part of the quaternion defining the
                            rotation from the NED frame to the ship's BODY frame.
        phi (List float):   First Euler angle defining the rotation from 'frame'
                            to the reference frame in which the forces and torques
                            are expressed. Depends on the angle convention chosen
                            in the 'rotations convention' section of xdyn's input
                            file. See xdyn's documentation for details.
        theta (List float): Second Euler angle defining the rotation from 'frame'
                            to the reference frame in which the forces and torques
                            are expressed. Depends on the angle convention chosen
                            in the 'rotations convention' section of xdyn's input
                            file. See xdyn's documentation for details.
        psi (List float):   Third Euler angle defining the rotation from 'frame'
                            to the reference frame in which the forces and torques
                            are expressed. Depends on the angle convention chosen
                            in the 'rotations convention' section of xdyn's input
                            file. See xdyn's documentation for details.
        rotation_convention (str): Angle convention chosen in xdyn's YAML file.
                                   Use it to check the convention is what you
                                   are expecting!
                                   Format: ["psi", "theta'", "phi''"]
    """

    t: list[float] = field(default_factory=list)
    x: list[float] = field(default_factory=list)
    y: list[float] = field(default_factory=list)
    z: list[float] = field(default_factory=list)
    u: list[float] = field(default_factory=list)
    v: list[float] = field(default_factory=list)
    w: list[float] = field(default_factory=list)
    p: list[float] = field(default_factory=list)
    q: list[float] = field(default_factory=list)
    r: list[float] = field(default_factory=list)
    qr: list[float] = field(default_factory=list)
    qi: list[float] = field(default_factory=list)
    qj: list[float] = field(default_factory=list)
    qk: list[float] = field(default_factory=list)
    phi: list[float] = field(default_factory=list)
    theta: list[float] = field(default_factory=list)
    psi: list[float] = field(default_factory=list)
    rotation_convention: str = ""


@dataclass
class SetForceParameterResponse:
    """Return type of set_parameters.

    Attributes
        max_history_length (float):   How far back (in seconds) should the history values in
                                      ForceRequest go?
        needs_wave_outputs (bool):    Should the force model be queried at each time step using the
                                      'required_wave_information' rpc method to know what wave
                                      information it requires?
        commands (List[str]):         List of commands needed by this model, without the model name
                                      (e.g. ['beta1', 'beta2'])
        frame (string):               Reference frame from which we define the reference in which
                                      the forces and torques are expressed.
        x (float):                    Position along the x-axis of 'frame' of the point of
                                      application of the force.
        y (float):                    Position along the y-axis of 'frame' of the point of
                                      application of the force.
        z (float):                    Position along the z-axis of 'frame' of the point of
                                      application of the force.
        phi (float):                  First Euler angle defining the rotation from 'frame' to the
                                      reference frame in which the forces and torques are expressed.
                                      Depends on the angle convention chosen in the 'rotations
                                      convention' section of xdyn's input file. See xdyn's
                                      documentation for details.
        theta (float):                Second Euler angle defining the rotation from 'frame' to the
                                      reference frame in which the forces and torques are expressed.
                                      Depends on the angle convention chosen in the 'rotations
                                      convention' section of xdyn's input file. See xdyn's
                                      documentation for details.
        psi (float):                  Third Euler angle defining the rotation from 'frame' to the
                                      reference frame in which the forces and torques are expressed.
                                      Depends on the angle convention chosen in the 'rotations
                                      convention' section of xdyn's input file. See xdyn's
                                      documentation for details.
    """

    max_history_length: float = 0
    needs_wave_outputs: bool = False
    commands: list[str] = field(default_factory=list)
    frame: str = ""
    x: float = 0
    y: float = 0
    z: float = 0
    phi: float = 0
    theta: float = 0
    psi: float = 0


@dataclass
class XYZTGrid:
    """Wave elevation grid.

    Attributes
        x (List[float]): Positions (in meters) at which we want the value of interest. Projected on
                         the X-axis of the Earth-centered, Earth-fixed North-East-Down reference
                         frame. Should be the same size as y and z.
        y (List[float]): Positions (in meters) at which we want the value of interest. Projected on
                         the Y-axis of the Earth-centered, Earth-fixed North-East-Down reference
                         frame. Should be the same size as x and z.
        z (List[float]): Positions (in meters) at which we want the value of interest. Projected on
                         the Z-axis of the Earth-centered, Earth-fixed North-East-Down reference
                         frame. Should be the same size as x and y.
        t (float):       Simulation time (in seconds). All values are calculated at that instant.
                         The documentation of each waves model should describe how the phases are
                         defined.

    """

    x: list[float] = field(default_factory=list)
    y: list[float] = field(default_factory=list)
    z: list[float] = field(default_factory=list)
    t: float = 0


@dataclass
class XYTGrid:
    """Wave elevation grid.

    Attributes
        x (List[float]): Positions (in meters) at which we want the value of interest. Projected on
                         the X-axis of the Earth-centered, Earth-fixed North-East-Down reference
                         frame. Should be the same size as y and z.
        y (List[float]): Positions (in meters) at which we want the value of interest. Projected on
                         the Y-axis of the Earth-centered, Earth-fixed North-East-Down reference
                         frame. Should be the same size as x and z.
        t (float):       Simulation time (in seconds). All values are calculated at that instant.
                         The documentation of each waves model should describe how the phases are
                         defined.

    """

    x: list[float] = field(default_factory=list)
    y: list[float] = field(default_factory=list)
    t: float = 0


@dataclass
class OrbitalVelocitiesResponse:
    """Orbital velocity grid.

    Attributes
        x (List[float]): Positions (in meters) at which the orbital velocity was computed. Projected
                         on the X-axis of the Earth-centered, Earth-fixed North-East-Down reference
                         frame. Should be the same size as y and z.
        y (List[float]): Positions (in meters) at which the orbital velocity was computed. Projected
                         on the Y-axis of the Earth-centered, Earth-fixed North-East-Down reference
                         frame. Should be the same size as x and z.
        z (List[float]): Positions (in meters) at which the orbital velocity was computed. Projected
                         on the Z-axis of the Earth-centered, Earth-fixed North-East-Down reference
                         frame. Should be the same size as x and y.
        t (float):       Simulation time (in seconds). All values are calculated at that instant.
                         The documentation of each waves model should describe how the phases are
                         defined.
        vx (List[float]): Projection on the X-axis of the Earth-centered, Earth-fixed
                          North-East-Down reference frame of the velocity of each partical.
                          In meters per second. Should be the same size as x, y, z, vy and vz.
        vy (List[float]): Projection on the Y-axis of the Earth-centered, Earth-fixed
                          North-East-Down reference frame of the velocity of each partical.
                          In meters per second. Should be the same size as x, y, z, vx and vz.
        vz (List[float]): Projection on the Z-axis of the Earth-centered, Earth-fixed
                          North-East-Down reference frame of the velocity of each partical.
                          In meters per second. Should be the same size as x, y, z, vx and vy.
    """

    x: list[float] = field(default_factory=list)
    y: list[float] = field(default_factory=list)
    z: list[float] = field(default_factory=list)
    t: float = 0
    vx: list[float] = field(default_factory=list)
    vy: list[float] = field(default_factory=list)
    vz: list[float] = field(default_factory=list)


@dataclass
class Spectrum:
    """Linear wave spectrum. All attributes are list of float that should have the same length

    Attributes
        a (List[float]):                      Amplitudes of each ray (in m).
        omega (List[float]):                  Angular frequencies the spectrum was discretized at for each ray (in rad/s).
        psi (List[float]):                    Directions between 0 & 2pi of each ray (in rad).
        k (List[float]):                      Discretized wave numbers (should therefore be the same size as omega) for each ray (in rad/m).
        phase (List[List[float]]):            Random phases for each ray (in rad).
    """

    a: list[float] = field(default_factory=list)
    psi: list[float] = field(default_factory=list)
    omega: list[float] = field(default_factory=list)
    k: list[float] = field(default_factory=list)
    phase: list[list[float]] = field(default_factory=list)


@dataclass
class ForceResponse:
    """Return type of the gRPC method 'force'.

    Attributes
        Fx (float):                           Projection of the force acting on "BODY" on the
                                              X-axis of the body frame.
        Fy (float):                           Projection of the force acting on "BODY" on the
                                              Y-axis of the body frame.
        Fz (float):                           Projection of the force acting on "BODY" on the
                                              Z-axis of the body frame.
        Mx (float):                           Projection of the torque acting on "BODY" on the
                                              X-axis of the body frame, expressed at the origin
                                              of the BODY frame (center of gravity).
        My (float):                           Projection of the torque acting on "BODY" on the
                                              Y-axis of the body frame, expressed at the origin
                                              of the BODY frame (center of gravity).
        Mz (float):                           Projection of the torque acting on "BODY" on the
                                              Z-axis of the body frame, expressed at the origin
                                              of the BODY frame (center of gravity).
        extra_observations (Dict[str,float]): Anything we wish to serialize. Specific to each
                                              force model.
    """

    Fx: float = 0
    Fy: float = 0
    Fz: float = 0
    Mx: float = 0
    My: float = 0
    Mz: float = 0
    extra_observations: dict[str, float] = field(default_factory=dict)


@dataclass
class DynamicPressuresResponse:
    """Dynamic pressure grid.

    Attributes
        x (List[float]):    Positions (in meters) at which the orbital velocity was computed.
                            Projected on the X-axis of the Earth-centered, Earth-fixed
                            North-East-Down reference frame. Should be the same size as y and z.
        y (List[float]):    Positions (in meters) at which the orbital velocity was computed.
                            Projected on the Y-axis of the Earth-centered, Earth-fixed
                            North-East-Down reference frame. Should be the same size as x and z.
        z (List[float]):    Positions (in meters) at which the orbital velocity was computed.
                            Projected on the Z-axis of the Earth-centered, Earth-fixed
                            North-East-Down reference frame. Should be the same size as x and y.
        t (float):          Simulation time (in seconds). All values are calculated at that
                            instant. The documentation of each waves model should describe how
                            the phases are defined.
        pdyn (List[float]): Dynamic pressure (in Pascal) for each (x,y,z) value at t. Should be
                            the same size as x, y and z.
    """

    x: list[float] = field(default_factory=list)
    y: list[float] = field(default_factory=list)
    z: list[float] = field(default_factory=list)
    t: float = 0
    pdyn: list[float] = field(default_factory=list)


@dataclass
class WaveInformation:
    """This is the wave information the gRPC force model requested from xdyn.

    Attributes
        elevations (XYZTGrid):                            Elevations computed for this force model
        dynamic_pressures (DynamicPressuresResponse):     Dynamic pressures computed for this force
                                                          model
        orbital_velocities (OrbitalVelocitiesResponse):   Orbital velocities computed for this
                                                          force model
        spectrum (Spectrum):                          Spectrum computed for this force model
    """

    elevations: XYZTGrid = field(default_factory=XYZTGrid)
    dynamic_pressures: DynamicPressuresResponse = field(default_factory=DynamicPressuresResponse)
    orbital_velocities: OrbitalVelocitiesResponse = field(default_factory=OrbitalVelocitiesResponse)
    spectrum: Spectrum = field(default_factory=Spectrum)


@dataclass
class SpectrumRequest:
    """Defines the point at which we want a linearized wave spectrum.

    Attributes

    x (float) : Positions (in meters) at which we want the equivalent linear
                spectrum. Projected on the X-axis of the Earth-centered,
                Earth-fixed North-East-Down reference frame.
    y (float) : Positions (in meters) at which we want the equivalent linear
                spectrum. Projected on the Y-axis of the Earth-centered,
                Earth-fixed North-East-Down reference frame.
    t (float) : Simulation time (in seconds). The linearization is valid at
                that instant. The documentation of each waves model should
                describe how the phases are defined.
    """

    x: float = 0
    y: float = 0
    t: float = 0


@dataclass
class RequiredWaveInformationResponse:
    """Returned by gRPC method 'required_wave_information'. Called by xdyn so
    the force model can specify the wave information it needs.

    Attributes

        elevations (XYTGrid)               : Points at which the force model requires wave
                                             elevations. Updated at each timestep if
                                             needs_wave_outputs was set to true.
        dynamic_pressures (XYZTGrid)       : Points at which the force model requires dynamic
                                             pressures. Updated at each timestep if
                                             needs_wave_outputs was set to true.
        orbital_velocities (XYZTGrid)      : Points at which the force model requires orbital
                                             velocities. Updated at each timestep if
                                             needs_wave_outputs was set to true.
        spectrum (SpectrumRequest)         : Point at which we want to linearize the spectrum (if
                                             necessary)
        angular_frequencies_for_rao (bool) : Does the force model need the angular frequencies?
        directions_for_rao (bool)          : Does the force model need the spatial discretization?
        need_spectrum (bool)               : Do we want to linearize the spectrum at the 'spectrum'
                                             point? If set to False, 'spectrum' should be ignored.
                                             This is to avoid using an optional value.
    """

    elevations: XYTGrid = field(default_factory=XYTGrid)
    dynamic_pressures: XYZTGrid = field(default_factory=XYZTGrid)
    orbital_velocities: XYZTGrid = field(default_factory=XYZTGrid)
    spectrum: SpectrumRequest = field(default_factory=SpectrumRequest)
    angular_frequencies_for_rao: bool = False
    directions_for_rao: bool = False
    need_spectrum: bool = False


@dataclass
class WrenchMatrices:
    """Store force RAOs, by angular frequency (line) and incidence (column).

    Attributes

      - X (np.ndarray) : RAO for the X axis. Unit depends on the context (usually either in
                         N/(m².s) for module or radians for phase).
      - Y (np.ndarray) : RAO for the Y axis. Unit depends on the context (usually either in
                         N/(m².s) for module or radians for phase).
      - Z (np.ndarray) : RAO for the Z axis. Unit depends on the context (usually either in
                         N/(m².s) for module or radians for phase).
      - K (np.ndarray) : RAO for the K axis. Unit depends on the context (usually either in
                         N/(m².s) for module or radians for phase).
      - M (np.ndarray) : RAO for the M axis. Unit depends on the context (usually either in
                         N/(m².s) for module or radians for phase).
      - N (np.ndarray) : RAO for the N axis. Unit depends on the context (usually either in
                         N/(m².s) for module or radians for phase).
    """

    X: np.ndarray = field(default_factory=lambda: np.empty([0, 0]))
    Y: np.ndarray = field(default_factory=lambda: np.empty([0, 0]))
    Z: np.ndarray = field(default_factory=lambda: np.empty([0, 0]))
    K: np.ndarray = field(default_factory=lambda: np.empty([0, 0]))
    M: np.ndarray = field(default_factory=lambda: np.empty([0, 0]))
    N: np.ndarray = field(default_factory=lambda: np.empty([0, 0]))


@dataclass
class ResultsFromPotentialTheory:
    """Store results of hydrodynamic potential theory code.

    Attributes

    - Ma (np.ndarray)                               : Asymptotic added mass matrix (at infinite
                                                      frequency) (in kg for $`1\\leq i,j\\leq 3`$,
                                                      in kg.m² for $`4\\leq i,j\\leq 6`$, and in
                                                      kg.m otherwise)
    - diffraction_module_tables (WrenchMatrices)    : Module of the diffraction forces (by angular
                                                      frequency a and incidence i M[a][i])
                                                      (in N/(m².s))
    - diffraction_phase_tables (WrenchMatrices)     : Phase of the diffraction forces (by angular
                                                      frequency a and incidence i M[a][i]), in
                                                      radians.
    - diffraction_module_periods (np.ndarray)       : Periods at which the
                                                      diffraction_module_tables are expressed
                                                      (in seconds)
    - diffraction_phase_periods (np.ndarray)        : Periods at which the diffraction_phase_tables
                                                      are expressed (in seconds)
    - diffraction_module_psis (np.ndarray)          : Incidences at which the
                                                      diffraction_module_tables are expressed
                                                      (in radian)
    - diffraction_phase_psis (np.ndarray)           : Incidences at which the
                                                      diffraction_phase_tables are expressed
                                                      (in radian)
    - froude_krylov_module_tables (WrenchMatrices)  : Module of the Froude-Krylov forces (by
                                                      angular frequency a and incidence i M[a][i]),
                                                      in N/(m².s).
    - froude_krylov_phase_tables (WrenchMatrices)   : Phase of the Froude-Krylov forces (by
                                                      angular frequency a and incidence i M[a][i]),
                                                      in radian.
    - froude_krylov_module_periods (np.ndarray)     : Periods at which the
                                                      froude_krylov_module_tables are expressed
                                                      (in seconds)
    - froude_krylov_phase_periods (np.ndarray)      : Periods at which the
                                                      froude_krylov_phase_tables are expressed
                                                      (in seconds)
    - froude_krylov_module_psis (np.ndarray)        : Incidences at which the
                                                      froude_krylov_module_tables are expressed
                                                      (in N/(m².s))
    - froude_krylov_phase_psis (np.ndarray)         : Incidences at which the
                                                      froude_krylov_phase_tables are expressed
                                                      (in radian)
    - angular_frequencies (np.ndarray)              : Angular frequencies at which added_mass_coeff
                                                      and radiation_damping_coeff are expressed.
                                                      In rad/s.
    - forward_speed (float)                         : Speed at which the potential theory code
                                                      was run. In m/s.
    - added_mass_coeff (np.ndarray)                 : Added mass matrix coefficients
                                                      (by angular frequency)
                                                      (in kg for $`1\\leq i,j\\leq 3`$, in kg.m²
                                                      for $`4\\leq i,j\\leq 6`$, and in kg.m
                                                      otherwise)
    - radiation_damping_coeff (np.ndarray)          : Radiation damping matrix coefficients
                                                      (by angular frequency)
    - wave_drift_force_tables (WrenchMatrices)      : Wave drift forces (by angular frequency a
                                                      and incidence i M[a][i]), in N/(m².s).
    - wave_drift_periods (np.ndarray)               : Periods at which the wave drift forces
                                                      (in seconds)
    - wave_drift_psis (np.ndarray)                  : Incidences at which the wave drift forces
                                                      are expressed (in rad)
    """

    Ma: np.ndarray = field(default_factory=lambda: np.zeros(shape=(6, 6)))
    diffraction_module_tables: WrenchMatrices = field(default_factory=WrenchMatrices)
    diffraction_phase_tables: WrenchMatrices = field(default_factory=WrenchMatrices)
    diffraction_module_periods: np.ndarray = field(default_factory=lambda: np.empty(shape=0))
    diffraction_phase_periods: np.ndarray = field(default_factory=lambda: np.empty(shape=0))
    diffraction_module_psis: np.ndarray = field(default_factory=lambda: np.empty(shape=0))
    diffraction_phase_psis: np.ndarray = field(default_factory=lambda: np.empty(shape=0))
    froude_krylov_module_tables: WrenchMatrices = field(default_factory=WrenchMatrices)
    froude_krylov_phase_tables: WrenchMatrices = field(default_factory=WrenchMatrices)
    froude_krylov_module_periods: np.ndarray = field(default_factory=lambda: np.empty(shape=0))
    froude_krylov_phase_periods: np.ndarray = field(default_factory=lambda: np.empty(shape=0))
    froude_krylov_module_psis: np.ndarray = field(default_factory=lambda: np.empty(shape=0))
    froude_krylov_phase_psis: np.ndarray = field(default_factory=lambda: np.empty(shape=0))
    angular_frequencies: np.ndarray = field(default_factory=lambda: np.empty(shape=0))
    forward_speed: float = np.nan
    added_mass_coeff: np.ndarray = field(default_factory=lambda: np.empty(shape=(6, 6, 0)))
    radiation_damping_coeff: np.ndarray = field(default_factory=lambda: np.empty(shape=(6, 6, 0)))
    wave_drift_force_tables: WrenchMatrices = field(default_factory=WrenchMatrices)
    wave_drift_periods: np.ndarray = field(default_factory=lambda: np.empty(shape=0))
    wave_drift_psis: np.ndarray = field(default_factory=lambda: np.empty(shape=0))
