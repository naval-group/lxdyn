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
LIBRARY = Path(os.environ["XDYN_FMU_LIBRARY"])
STATES = ["x", "y", "z", "u", "v", "w", "p", "q", "r", "qr", "qi", "qj", "qk"]
RPM = 100 * 2 * np.pi / 60  # rad/s, as test_data::dummy_controllers_and_commands_for_propulsion...


@pytest.fixture(scope="module")
def fmu(tmp_path_factory):
    path = tmp_path_factory.mktemp("fmu") / "propelled_ship.fmu"
    subprocess.run(
        [
            sys.executable,
            str(HERE.parent / "make_fmu.py"),
            str(HERE / "propelled_ship.yml"),
            "-o",
            str(path),
            "--library",
            str(LIBRARY),
            "--dt",
            "0.1",
        ],
        check=True,
    )
    return path


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
    s.setReal([refs["propeller(rpm)"], refs["propeller(P/D)"]], [rpm, 1.064935])
    for k in range(steps):
        s.doStep(currentCommunicationPoint=k * h, communicationStepSize=h)
    state = s.getReal([refs[n] for n in STATES])
    s.terminate()
    s.freeInstance()
    return state


def test_fmpy_finds_nothing_wrong(fmu):
    assert validate_fmu(str(fmu)) == []


def test_variables_come_from_the_model(fmu):
    variables = {v.name: v for v in read_model_description(fmu).modelVariables}
    assert variables["propeller(rpm)"].causality == "input"
    assert variables["propeller(P/D)"].causality == "input"
    assert variables["z"].causality == "output"
    assert float(variables["z"].start) == 1  # the YAML's initial position
    assert float(variables["xdyn_dt"].start) == 0.1


def test_the_propeller_pushes_the_ship_ahead(fmu):
    slow = run(fmu, 20, rpm=RPM / 2)[STATES.index("u")]
    fast = run(fmu, 20)[STATES.index("u")]
    assert 0 < slow < fast


def test_reset_state_imposes_the_next_state(fmu):
    s, refs = slave(fmu)
    s.setReal([refs["propeller(rpm)"], refs["propeller(P/D)"]], [RPM, 1.064935])
    imposed = {"x": 100.0, "y": -50.0, "z": 1.0, "qr": 1.0}
    s.setReal([refs[f"reset_{n}"] for n in imposed], list(imposed.values()))
    s.setBoolean([refs["reset_state"]], [True])
    s.doStep(currentCommunicationPoint=0, communicationStepSize=0.1)
    x, y = s.getReal([refs["x"], refs["y"]])
    s.terminate()
    s.freeInstance()
    # From there, the propeller moves the ship by a fraction of a millimetre in one step.
    assert x == pytest.approx(100, abs=1e-2)
    assert y == pytest.approx(-50, abs=1e-2)


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
        start_values={"propeller(rpm)": RPM},
        output=["u"],
    )
    assert result["u"][-1] > 0
