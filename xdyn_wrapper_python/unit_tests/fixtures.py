"""Test inputs: the documents under the repository's test_data/, and meshes built in code."""

from pathlib import Path

import numpy as np
from xdyn.core import BodyBuilder, BodyPtr
from xdyn.core.io import YamlRotation

# Relative to this file, so a wheel tested from a clean venv finds them too.
TEST_DATA = Path(__file__).resolve().parents[2] / "test_data"

Mesh = list[list[np.ndarray]]


def path(name: str) -> str:
    return str(TEST_DATA / name)


def read(name: str) -> str:
    return (TEST_DATA / name).read_text(encoding="utf-8")


def two_triangles() -> Mesh:
    return [
        [np.array([0.0, 0.0, 0.0]), np.array([4.0, 0.0, 0.0]), np.array([2.0, 2.0, 0.0])],
        [np.array([0.0, 0.0, 0.0]), np.array([2.0, -1.0, 0.0]), np.array([4.0, 0.0, 0.0])],
    ]


def unit_cube() -> Mesh:
    vertices = {
        1: (-0.5, -0.5, 0.5),
        2: (0.5, -0.5, 0.5),
        3: (0.5, 0.5, 0.5),
        4: (-0.5, 0.5, 0.5),
        5: (0.5, -0.5, -0.5),
        6: (-0.5, -0.5, -0.5),
        7: (-0.5, 0.5, -0.5),
        8: (0.5, 0.5, -0.5),
    }
    facets = [
        (1, 2, 3), (1, 3, 4), (5, 6, 7), (5, 7, 8), (6, 1, 4), (6, 4, 7),
        (2, 5, 8), (2, 8, 3), (6, 5, 2), (6, 2, 1), (4, 3, 8), (4, 8, 7),
    ]  # fmt: skip
    return [[np.array(vertices[i]) for i in facet] for facet in facets]


def get_body(name: str, mesh: Mesh | None = None) -> BodyPtr:
    rot = YamlRotation("angle", ["z", "y'", "x''"])
    return BodyBuilder(rot).build_for_test(
        name, two_triangles() if mesh is None else mesh, 0, 0.0, rot, 0.0, False
    )
