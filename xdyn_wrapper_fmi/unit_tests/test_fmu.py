"""The FMU as an importer sees it, through FMPy: XDYN_FMU_LIBRARY names the library to pack."""

import os
import subprocess
import sys
import threading
from pathlib import Path

import numpy as np
import pytest
from fmpy import read_model_description, simulate_fmu
from fmpy.fmi2 import FMU2Slave
from fmpy.validation import validate_fmu

HERE = Path(__file__).resolve().parent
REPO = HERE.parent.parent
LIBRARY = Path(os.environ["XDYN_FMU_LIBRARY"])
STATES = ["x", "y", "z", "u", "v", "w", "p", "q", "r", "qr", "qi", "qj", "qk"]
SHIP = "TestShip."
RPM_INPUT, P_D_INPUT = "TestShip.propeller.rpm", "TestShip.propeller.P_D"
RPM = 100 * 2 * np.pi / 60  # rad/s, as test_data::dummy_controllers_and_commands_for_propulsion...


# Radiation damping reads the ship's past velocities, up to tau max back. xdyn reads the HDB
# file from the working directory, not from the asset root: the path is absolute.
RADIATION_DAMPING = f"""\
      - model: radiation damping
        hdb: {REPO / "validation" / "test_ship.hdb"}
        type of quadrature for cos transform: simpson
        type of quadrature for convolution: simpson
        nb of points for retardation function discretization: 50
        omega min: {{value: 0, unit: rad/s}}
        omega max: {{value: 30, unit: rad/s}}
        tau min: {{value: 0.2094395, unit: s}}
        tau max: {{value: 10, unit: s}}
        output Br and K: false
        calculation point in body frame:
            x: {{value: 0.696, unit: m}}
            y: {{value: 0, unit: m}}
            z: {{value: 1.418, unit: m}}
"""


def pack(yaml, path, *options):
    subprocess.run(
        [
            sys.executable,
            str(HERE.parent / "make_fmu.py"),
            str(yaml),
            "-o",
            str(path),
            "--library",
            str(LIBRARY),
            "--dt",
            "0.1",
            *options,
        ],
        check=True,
    )
    return path


@pytest.fixture(scope="module")
def fmu(tmp_path_factory):
    return pack(HERE / "propelled_ship.yml", tmp_path_factory.mktemp("fmu") / "propelled_ship.fmu")


def slave(fmu, name="ship"):
    description = read_model_description(fmu)
    unzipped = Path(fmu).with_suffix("")
    if not unzipped.exists():
        from fmpy import extract

        extract(fmu, unzipdir=unzipped)
    s = FMU2Slave(
        guid=description.guid,
        unzipDirectory=unzipped,
        modelIdentifier=description.coSimulation.modelIdentifier,
        instanceName=name,
    )
    s.instantiate()
    s.setupExperiment(startTime=0)
    s.enterInitializationMode()
    s.exitInitializationMode()
    refs = {v.name: v.valueReference for v in description.modelVariables}
    return s, refs


def run(fmu, steps, rpm=RPM, h=0.5):
    s, refs = slave(fmu)
    s.setReal([refs[RPM_INPUT], refs[P_D_INPUT]], [rpm, 1.064935])
    for k in range(steps):
        s.doStep(currentCommunicationPoint=k * h, communicationStepSize=h)
    state = s.getReal([refs[SHIP + n] for n in STATES])
    s.terminate()
    s.freeInstance()
    return state


def test_fmpy_finds_nothing_wrong(fmu):
    assert validate_fmu(str(fmu)) == []


def test_variables_come_from_the_model(fmu):
    description = read_model_description(fmu)
    assert description.variableNamingConvention == "structured"
    variables = {v.name: v for v in description.modelVariables}
    assert variables[RPM_INPUT].causality == "input"
    assert variables[RPM_INPUT].description == "propeller(rpm)"
    assert variables[P_D_INPUT].causality == "input"
    assert variables["TestShip.z"].causality == "output"
    assert float(variables["TestShip.z"].start) == 1  # the YAML's initial position
    assert float(variables["xdyn.dt"].start) == 0.1
    assert "[psi, theta', phi'']" in variables["TestShip.phi"].description


def test_variables_are_in_si_units(fmu):
    variables = {v.name: v for v in read_model_description(fmu).modelVariables}
    assert (variables[RPM_INPUT].unit, variables[RPM_INPUT].displayUnit) == ("rad/s", "rpm")
    assert variables[P_D_INPUT].unit is None
    assert (variables["TestShip.psi"].unit, variables["TestShip.psi"].displayUnit) == ("rad", "deg")


def test_the_solver_is_an_enumeration(fmu):
    description = read_model_description(fmu)
    solver = next(v for v in description.modelVariables if v.name == "xdyn.solver")
    assert solver.type == "Enumeration"
    assert [item.name for item in solver.declaredType.items] == ["euler", "rk4", "rkck"]
    assert int(solver.start) == 2


def test_names_become_structured_name_parts(tmp_path):
    yaml = tmp_path / "ship.yml"
    text = (HERE / "propelled_ship.yml").read_text()
    yaml.write_text(text.replace("name: propeller", "name: prop-1"))
    fmu = pack(yaml, tmp_path / "ship.fmu")
    variables = {v.name for v in read_model_description(fmu).modelVariables}
    assert {"TestShip.prop_1.rpm", "TestShip.prop_1.P_D"} <= variables


def test_names_that_collide_are_rejected(tmp_path):
    text = (HERE / "propelled_ship.yml").read_text()
    propeller = text[text.index("      - name: propeller") :]
    yaml = tmp_path / "ship.yml"
    yaml.write_text(
        text.replace("name: propeller", "name: prop-1")
        + propeller.replace("name: propeller", "name: prop_1")
    )
    with pytest.raises(subprocess.CalledProcessError):
        pack(yaml, tmp_path / "ship.fmu")


def test_the_propeller_pushes_the_ship_ahead(fmu):
    slow = run(fmu, 20, rpm=RPM / 2)[STATES.index("u")]
    fast = run(fmu, 20)[STATES.index("u")]
    assert 0 < slow < fast


def test_reset_state_imposes_the_next_state(fmu):
    s, refs = slave(fmu)
    s.setReal([refs[RPM_INPUT], refs[P_D_INPUT]], [RPM, 1.064935])
    imposed = {"x": 100.0, "y": -50.0, "z": 1.0, "qr": 1.0}
    s.setReal([refs[f"TestShip.reset_{n}"] for n in imposed], list(imposed.values()))
    s.setBoolean([refs["TestShip.reset_state"]], [True])
    s.doStep(currentCommunicationPoint=0, communicationStepSize=0.1)
    x, y = s.getReal([refs["TestShip.x"], refs["TestShip.y"]])
    # From there, the propeller moves the ship by a fraction of a millimetre in one step.
    assert x == pytest.approx(100, abs=1e-2)
    assert y == pytest.approx(-50, abs=1e-2)
    # Still true, as a master re-sets its inputs before each step: no second reset.
    for k in range(1, 20):
        s.setBoolean([refs["TestShip.reset_state"]], [True])
        s.doStep(currentCommunicationPoint=k * 0.1, communicationStepSize=0.1)
    assert s.getReal([refs["TestShip.x"]])[0] > x
    s.terminate()
    s.freeInstance()


def radiating_ship(tmp_path):
    yaml = tmp_path / "radiating_ship.yml"
    yaml.write_text(
        (HERE / "propelled_ship.yml")
        .read_text()
        .replace("    external forces:\n", "    external forces:\n" + RADIATION_DAMPING, 1)
    )
    return yaml


def test_the_history_goes_on_from_one_step_to_the_next(tmp_path):
    fmu = pack(radiating_ship(tmp_path), tmp_path / "radiating_ship.fmu")
    # The same xdyn steps either way: a history cut at each communication step would make the
    # radiation force, hence the state, depend on the step size.
    assert run(fmu, 5, h=1.0) == pytest.approx(run(fmu, 50, h=0.1), rel=1e-9, abs=1e-12)


def test_instances_in_parallel_do_not_share_state(fmu):
    alone = run(fmu, 10)
    results = [None, None]

    def one(i):
        results[i] = run(fmu, 10)

    threads = [threading.Thread(target=one, args=(i,)) for i in range(2)]
    for t in threads:
        t.start()
    for t in threads:
        t.join()
    assert results[0] == alone
    assert results[1] == alone


def test_simulate_fmu_runs_it_end_to_end(fmu):
    result = simulate_fmu(
        str(fmu),
        stop_time=5,
        output_interval=0.5,
        start_values={RPM_INPUT: RPM, P_D_INPUT: 1.064935},
        output=["TestShip.u"],
    )
    assert result["TestShip.u"][-1] > 0


def test_model_exchange_follows_co_simulation(fmu):
    def final(mode, **options):
        result = simulate_fmu(
            str(fmu),
            fmi_type=mode,
            stop_time=20,
            output_interval=0.5,
            start_values={RPM_INPUT: RPM, P_D_INPUT: 1.064935},
            output=["TestShip.x", "TestShip.u", "der(TestShip.u)", "TestShip.psi"],
            **options,
        )
        return result[-1]

    cs = final("CoSimulation")
    me = final("ModelExchange", solver="CVode", relative_tolerance=1e-8)
    # xdyn's RK4 at 0.1 s against CVode: the same trajectory, to the integrators' accuracy.
    assert me["TestShip.u"] == pytest.approx(cs["TestShip.u"], rel=1e-5)
    assert me["TestShip.x"] == pytest.approx(cs["TestShip.x"], rel=1e-5)
    assert me["der(TestShip.u)"] == pytest.approx(cs["der(TestShip.u)"], rel=1e-3)
    assert me["TestShip.psi"] == pytest.approx(cs["TestShip.psi"], abs=1e-9)


def test_model_exchange_declares_its_structure(fmu):
    description = read_model_description(fmu)
    assert description.modelExchange is not None
    names = {v: v.name for v in description.modelVariables}
    assert [names[d.variable] for d in description.derivatives][:2] == [
        "der(TestShip.x)",
        "der(TestShip.y)",
    ]
    x = next(o for o in description.outputs if o.variable.name == "TestShip.x")
    assert [d.name for d in x.dependencies] == ["TestShip.x"]
    euler = next(o for o in description.outputs if o.variable.name == "TestShip.phi")
    assert [d.name for d in euler.dependencies] == [f"TestShip.{n}" for n in STATES[9:]]


def test_no_model_exchange_for_a_model_with_history(tmp_path):
    fmu = pack(radiating_ship(tmp_path), tmp_path / "radiating_ship.fmu")
    description = read_model_description(fmu)
    assert description.modelExchange is None
    assert not any(v.name.startswith("der(") for v in description.modelVariables)
    # Co-Simulation alone: the outputs are end-of-step states, no input reaches them.
    assert all(o.dependencies == [] for o in description.outputs)
