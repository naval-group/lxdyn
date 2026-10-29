#!/usr/bin/env python3
"""Packs an xdyn model into an FMI 2.0 FMU (Co-Simulation, and Model Exchange where it can).

    make_fmu.py model.yml [more.yml ...] -o ship.fmu --library build/<triple>/fmu/xdyn_fmu.so
                [--binary win64=build/x86_64-windows-gnu/fmu/xdyn_fmu.dll]
                [--assets-root DIR --asset path/in/root ...] [--dt 0.1] [--solver rk4]

The YAML files are concatenated into resources/model.yml, as xdyn reads several files given on
its command line. resources/ is the asset root the FMU builds the model against: each --asset
is copied there from --assets-root under the relative path the YAML names it by.

modelDescription.xml comes from the library itself (xdyn_fmu_model_description), so the
variables are those xdyn finds in the model. --library must therefore run here; --binary adds
the libraries of other platforms beside it.
"""

import argparse
import ctypes
import platform
import shutil
import sys
import tempfile
import zipfile
from pathlib import Path

EXTENSIONS = {"linux64": "so", "win64": "dll"}


def host_platform():
    return {"Linux": "linux64", "Windows": "win64"}[platform.system()]


def model_description(library, yaml_path, assets_root, model_name, dt, solver):
    lib = ctypes.CDLL(str(library))
    lib.xdyn_fmu_model_description.restype = ctypes.c_char_p
    lib.xdyn_fmu_model_description.argtypes = [
        ctypes.c_char_p,
        ctypes.c_char_p,
        ctypes.c_char_p,
        ctypes.c_double,
        ctypes.c_char_p,
    ]
    lib.xdyn_fmu_error.restype = ctypes.c_char_p
    xml = lib.xdyn_fmu_model_description(
        str(yaml_path).encode(), str(assets_root).encode(), model_name.encode(), dt, solver.encode()
    )
    if xml is None:
        sys.exit(f"make_fmu: xdyn rejects the model: {lib.xdyn_fmu_error().decode()}")
    return xml.decode()


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("yaml", nargs="+", type=Path)
    parser.add_argument("-o", "--output", type=Path, required=True)
    parser.add_argument("--library", type=Path, required=True, help="xdyn_fmu for this machine")
    parser.add_argument(
        "--binary",
        action="append",
        default=[],
        metavar="PLATFORM=PATH",
        help="xdyn_fmu for another platform (linux64, win64)",
    )
    parser.add_argument(
        "--assets-root", type=Path, default=Path("."), help="where the --asset paths start"
    )
    parser.add_argument(
        "--asset", action="append", default=[], help="a file the YAML names (mesh, data)"
    )
    parser.add_argument("--dt", type=float, default=0.1, help="xdyn's integration step, s")
    parser.add_argument("--solver", choices=["euler", "rk4", "rkck"], default="rk4")
    args = parser.parse_args()

    binaries = {host_platform(): args.library}
    for item in args.binary:
        name, _, path = item.partition("=")
        if name not in EXTENSIONS or not path:
            parser.error(f"--binary {item}: expected one of {sorted(EXTENSIONS)}=PATH")
        binaries[name] = Path(path)

    with tempfile.TemporaryDirectory() as tmp:
        root = Path(tmp)
        resources = root / "resources"
        resources.mkdir()
        (resources / "model.yml").write_text("\n".join(p.read_text() for p in args.yaml))
        for asset in args.asset:
            (resources / asset).parent.mkdir(parents=True, exist_ok=True)
            shutil.copy(args.assets_root / asset, resources / asset)
        (resources / "parameters.txt").write_text(f"dt {args.dt!r}\nsolver {args.solver}\n")
        (root / "modelDescription.xml").write_text(
            model_description(
                args.library,
                resources / "model.yml",
                resources,
                args.output.stem,
                args.dt,
                args.solver,
            )
        )
        for name, path in binaries.items():
            (root / "binaries" / name).mkdir(parents=True)
            shutil.copy(path, root / "binaries" / name / f"xdyn_fmu.{EXTENSIONS[name]}")
        with zipfile.ZipFile(args.output, "w", zipfile.ZIP_DEFLATED) as fmu:
            for f in sorted(root.rglob("*")):
                if f.is_file():
                    fmu.write(f, f.relative_to(root))
    print(f"{args.output}: {', '.join(sorted(binaries))}")


if __name__ == "__main__":
    main()
