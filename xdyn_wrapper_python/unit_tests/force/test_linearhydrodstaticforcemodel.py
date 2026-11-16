"""
Unit test for LinearHydrostaticForceModel
"""
import unittest

import numpy as np
from xdyn.force import LinearHydrostaticForceModel

from ..fixtures import read


class LinearHydrostaticForceModelTest(unittest.TestCase):
    """Test class for LinearHydrostaticForceModel"""

    def test_parser(self):
        data = LinearHydrostaticForceModel.parse(read("yaml/linear_hydrostatics.yml"))
        self.assertEqual(-2, data.z_eq)
        self.assertAlmostEqual(1 * np.pi / 180.0, data.theta_eq, delta=1e-12)
        self.assertAlmostEqual(-3 * np.pi / 180.0, data.phi_eq, delta=1e-12)
        self.assertTrue(np.allclose(data.K1, [1, 0, 0]))
        self.assertTrue(np.allclose(data.K2, [0, 1, 0]))
        self.assertTrue(np.allclose(data.K3, [0, 0, 1]))


if __name__ == "__main__":

    unittest.main()
