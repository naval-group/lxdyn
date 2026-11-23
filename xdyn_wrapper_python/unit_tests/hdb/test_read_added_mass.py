"""
Unit test for read_added_mass
"""

import unittest

from xdyn.core import YamlDynamics6x6Matrix
from xdyn.hdb import HDBParser, read_added_mass

from ..fixtures import path


class ReadAddedMassTest(unittest.TestCase):
    """Test class for read_added_mass"""

    def test_reads_the_rows_from_the_hdb_file(self):
        added_mass = YamlDynamics6x6Matrix()
        added_mass.read_from_file = True
        added_mass.hdb_filename = path("hdb/test_ship.hdb")
        rows = read_added_mass(added_mass)
        expected = HDBParser.from_file(path("hdb/test_ship.hdb")).get_added_mass()
        self.assertFalse(rows.read_from_file)
        self.assertEqual(list(expected[0]), rows.row_1)
        self.assertEqual(list(expected[5]), rows.row_6)

    def test_keeps_the_rows_it_is_given(self):
        added_mass = YamlDynamics6x6Matrix()
        added_mass.row_1 = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0]
        self.assertEqual(added_mass.row_1, read_added_mass(added_mass).row_1)
