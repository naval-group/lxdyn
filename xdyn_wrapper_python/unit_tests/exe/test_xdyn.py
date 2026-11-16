"""
Unit test for XDyn
"""
import unittest
from pathlib import Path

from xdyn import ErrorReporter, XdynCommandLineArguments, run

from ..fixtures import path


class XDynTest(unittest.TestCase):
    """Test class for XDyn"""

    def test_xdyn_no_argument(self):
        input_data = XdynCommandLineArguments()
        error_outputter = ErrorReporter()
        res = run(input_data, error_outputter)
        self.assertTrue(res)

    def test_xdyn_falling_ball_example(self):
        input_data = XdynCommandLineArguments()
        input_data.solver = "rk4"
        input_data.yaml_filenames = [path("tutorials/tutorial_01_falling_ball.yml")]
        input_data.initial_timestep = 0.1
        input_data.tstart = 0.0
        input_data.tend = 1.0
        error_outputter = ErrorReporter()
        res = run(input_data, error_outputter)
        self.assertTrue(res)
        with open("falling_ball.csv", encoding="utf-8") as fid:
            lines = fid.readlines()
        self.assertEqual(len(lines), 1 + 11)
        for extension in (".csv", ".json", ".h5"):
            res_filename = f"falling_ball{extension}"
            self.assertTrue(Path(res_filename).exists())


if __name__ == "__main__":

    unittest.main()
