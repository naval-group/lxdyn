"""The module and its wheel name the same release: the flake names one, build.zig the other."""

import unittest
from importlib import metadata

import xdyn


class VersionTest(unittest.TestCase):
    def test_module_version_is_the_wheel_version(self):
        try:
            wheel = metadata.version("lxdyn")
        except metadata.PackageNotFoundError:
            self.skipTest("xdyn comes from the build tree, not an installed wheel")
        self.assertEqual(xdyn.__version__, wheel)

    def test_commit_is_stamped(self):
        self.assertTrue(xdyn.__commit__)
