"""
Unit test for RadiationDampingForceModel
"""

import os
import unittest

import numpy as np
from xdyn.core import BodyStates, EnvironmentAndFrames
from xdyn.core.io import TypeOfQuadrature
from xdyn.exceptions import InvalidInputException
from xdyn.force import RadiationDampingForceModel
from xdyn.ssc.random import DataGenerator

from ..fixtures import read


def record(states: BodyStates, t: float, value: float):
    states.u.record(t, value)
    states.v.record(t, value)
    states.w.record(t, value)
    states.p.record(t, value)
    states.q.record(t, value)
    states.r.record(t, value)


def record_sine(
    states: BodyStates, t_start: float, T: float, n_periods: int, n_steps: int
) -> float:
    t_end = t_start + T * n_periods
    dt = (t_end - t_start) / n_steps
    for i in range(n_steps + 1):
        t = t_start + i * dt
        record(states, t, np.sin(2 * np.pi * (t - t_start) / T))
    return t_end


class RadiationDampingForceModelTest(unittest.TestCase):
    """Test class for RadiationDampingForceModel"""

    def setUp(self):
        self.rng = DataGenerator(666)
        hdb_filename = "test_ship.hdb"
        with open(hdb_filename, "w", encoding="utf-8") as fid:
            fid.write(read("tutorials/test_ship.hdb"))

    def tearDown(self) -> None:
        hdb_filename = "test_ship.hdb"
        os.remove(hdb_filename)

    def test_can_parse(self):
        """Check that parse function produces a valid RadiationDampingForceModelTest object"""
        data = RadiationDampingForceModel.parse(read("yaml/radiation_damping.yml")).yaml
        self.assertEqual("test_ship.hdb", data.hdb_filename)
        self.assertEqual(50, data.nb_of_points_for_retardation_function_discretization)
        self.assertEqual(30, data.omega_max)
        self.assertEqual(0, data.omega_min)
        self.assertTrue(data.output_Br_and_K)
        self.assertEqual(10, data.tau_max)
        self.assertEqual(0.2094395, data.tau_min)
        self.assertEqual(TypeOfQuadrature.CLENSHAW_CURTIS, data.type_of_quadrature_for_convolution)
        self.assertEqual(TypeOfQuadrature.SIMPSON, data.type_of_quadrature_for_cos_transform)
        self.assertEqual(0.696, data.calculation_point_in_body_frame.x)
        self.assertEqual(0, data.calculation_point_in_body_frame.y)
        self.assertEqual(1.418, data.calculation_point_in_body_frame.z)
        self.assertTrue(data.forward_speed_correction)

    def test_cannot_specify_both_hdb_and_precal_r_files(self):
        invalid_yaml = """
        model: radiation damping
        hdb: test_ship.hdb
        raodb: test_ship.ini
        type of quadrature for cos transform: simpson
        type of quadrature for convolution: clenshaw-curtis
        nb of points for retardation function discretization: 50
        omega min: {value: 0, unit: rad/s}
        omega max: {value: 30, unit: rad/s}
        tau min: {value: 0.2094395, unit: s}
        tau max: {value: 10, unit: s}
        output Br and K: true
        forward speed correction: true
        calculation point in body frame:
            x: {value: 0.696, unit: m}
            y: {value: 0, unit: m}
            z: {value: 1.418, unit: m}
        """

        expected_msg = (
            "When using the radiation force model, you cannot specify "
            "both the 'hdb' and 'raodb' in the YAML, as xdyn would not know which "
            "one to use to retrieve radiation damping coefficients: you should remove"
            " either 'hdb' or 'raodb' from the YAML file."
        )
        with self.assertRaises(InvalidInputException) as pcm:
            RadiationDampingForceModel.parse(invalid_yaml, False)
        self.assertTrue(expected_msg in str(pcm.exception), str(pcm.exception))

    def test_should_be_able_to_use_precal_r_files_only(self):
        valid_yaml = """
        model: radiation damping
        raodb: test_ship.ini
        type of quadrature for cos transform: simpson
        type of quadrature for convolution: clenshaw-curtis
        nb of points for retardation function discretization: 50
        omega min: {value: 0, unit: rad/s}
        omega max: {value: 30, unit: rad/s}
        tau min: {value: 0.2094395, unit: s}
        tau max: {value: 10, unit: s}
        output Br and K: true
        forward speed correction: true
        calculation point in body frame:
            x: {value: 0.696, unit: m}
            y: {value: 0, unit: m}
            z: {value: 1.418, unit: m}
        """
        RadiationDampingForceModel.parse(valid_yaml, False)

    def test_need_one_of_precal_r_or_hdb(self):
        invalid_yaml = """
        model: radiation damping
        type of quadrature for cos transform: simpson
        type of quadrature for convolution: clenshaw-curtis
        nb of points for retardation function discretization: 50
        omega min: {value: 0, unit: rad/s}
        omega max: {value: 30, unit: rad/s}
        tau min: {value: 0.2094395, unit: s}
        tau max: {value: 10, unit: s}
        output Br and K: true
        forward speed correction: true
        calculation point in body frame:
            x: {value: 0.696, unit: m}
            y: {value: 0, unit: m}
            z: {value: 1.418, unit: m}
        """
        expected_msg = (
            "When using the radiation force model, you must use "
            "*either* the 'hdb' key in the YAML file (to read the radiation "
            "damping matrix coefficients from an HDB file) *or* 'raodb' "
            "(if you wish to use the outputs of PRECAL_R): xdyn couldn't "
            "find either in the YAML file"
        )
        with self.assertRaises(InvalidInputException) as pcm:
            RadiationDampingForceModel.parse(invalid_yaml, False)
        self.assertTrue(expected_msg in str(pcm.exception), str(pcm.exception))

    def test_precal_r_filename_is_read_properly(self):
        valid_yaml = """
        model: radiation damping
        raodb: test_ship.ini
        type of quadrature for cos transform: simpson
        type of quadrature for convolution: clenshaw-curtis
        nb of points for retardation function discretization: 50
        omega min: {value: 0, unit: rad/s}
        omega max: {value: 30, unit: rad/s}
        tau min: {value: 0.2094395, unit: s}
        tau max: {value: 10, unit: s}
        output Br and K: true
        forward speed correction: true
        calculation point in body frame:
            x: {value: 0.696, unit: m}
            y: {value: 0, unit: m}
            z: {value: 1.418, unit: m}
        """
        precal_r_filename = RadiationDampingForceModel.parse(
            valid_yaml, False
        ).yaml.precal_r_filename
        self.assertEqual("test_ship.ini", precal_r_filename)

    def test_can_use_data_from_precal_r(self):
        yaml = """
        model: radiation damping
        raodb: data.raodb.ini
        type of quadrature for cos transform: simpson
        type of quadrature for convolution: trapezoidal
        nb of points for retardation function discretization: 50
        omega min: {value: 0, unit: rad/s}
        omega max: {value: 30, unit: rad/s}
        tau min: {value: 0.2094395, unit: s}
        tau max: {value: 10, unit: s}
        output Br and K: false
        forward speed correction: false
        calculation point in body frame:
            x: {value: 0.696, unit: m}
            y: {value: 0, unit: m}
            z: {value: 1.418, unit: m}
        """
        precalr_file = "data.raodb.ini"
        with open(precalr_file, "w", encoding="utf-8") as fid:
            fid.write(read("tutorials/ONRT_SIMMAN.raodb.ini"))
        data = RadiationDampingForceModel.parse(yaml)
        env = EnvironmentAndFrames()
        body_name = self.rng.random_string()()
        F = RadiationDampingForceModel(data, body_name, env)
        states = BodyStates(100)
        states.name = body_name
        T = 10
        record_sine(states, t_start=0, T=T, n_periods=10, n_steps=100)
        Frad = F.get_force(states, 0, env)
        self.assertNotEqual(0, Frad.X())
        os.remove(precalr_file)


if __name__ == "__main__":
    unittest.main()
