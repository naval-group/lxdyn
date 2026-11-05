#!/usr/bin/env python3
"""Packs a built extension module into the lxdyn wheel, with the metadata from pyproject.toml.

    make_wheel.py xdyn.cpython-314-x86_64-linux-gnu.so --version 26.11 --out dist/
                  --pyproject pyproject.toml --readme README.md --license LICENSE.md
                  [--platform manylinux_2_28_x86_64]

zig builds the module, so there is nothing for a build backend to do: this writes the zip,
METADATA, WHEEL and RECORD directly. The interpreter tag comes from the module's file name.
"""

import argparse
import base64
import hashlib
import re
import tomllib
import zipfile
from pathlib import Path

# Store files are dated 1970, which zip cannot hold; a fixed date also makes the wheel reproducible.
ZIP_DATE = (1980, 1, 1, 0, 0, 0)


def interpreter_tag(module: Path) -> str:
    m = re.fullmatch(r"xdyn\.cpython-(\d+)-[^.]+\.so", module.name)
    if not m:
        raise SystemExit(f"{module.name}: expected xdyn.cpython-<version>-<platform>.so")
    return f"cp{m[1]}"


def metadata(project: dict, version: str, readme: str, license_name: str) -> str:
    lines = [
        "Metadata-Version: 2.4",
        f"Name: {project['name']}",
        f"Version: {version}",
        f"Summary: {project['description']}",
        *(f"Author-email: {a['name']} <{a['email']}>" for a in project["authors"]),
        f"License-Expression: {project['license']}",
        f"License-File: {license_name}",
        *(f"Classifier: {c}" for c in project["classifiers"]),
        f"Requires-Python: {project['requires-python']}",
        *(f"Requires-Dist: {d}" for d in project["dependencies"]),
        *(f"Project-URL: {k}, {v}" for k, v in project["urls"].items()),
        "Description-Content-Type: text/markdown",
    ]
    return "\n".join(lines) + "\n\n" + readme


def record_line(name: str, data: bytes) -> str:
    digest = base64.urlsafe_b64encode(hashlib.sha256(data).digest()).rstrip(b"=").decode()
    return f"{name},sha256={digest},{len(data)}"


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("module", type=Path)
    parser.add_argument("--version", required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--pyproject", type=Path, required=True)
    parser.add_argument("--readme", type=Path, required=True)
    parser.add_argument("--license", type=Path, required=True)
    parser.add_argument("--platform", default="manylinux_2_28_x86_64")
    args = parser.parse_args()

    project = tomllib.loads(args.pyproject.read_text())["project"]
    name = project["name"]
    python = interpreter_tag(args.module)
    tag = f"{python}-{python}-{args.platform}"
    dist_info = f"{name}-{args.version}.dist-info"
    license_name = args.license.name

    files = {
        args.module.name: args.module.read_bytes(),
        f"{dist_info}/METADATA": metadata(
            project, args.version, args.readme.read_text(), license_name
        ).encode(),
        f"{dist_info}/WHEEL": (
            "Wheel-Version: 1.0\nGenerator: lxdyn make_wheel.py\n"
            f"Root-Is-Purelib: false\nTag: {tag}\n"
        ).encode(),
        f"{dist_info}/licenses/{license_name}": args.license.read_bytes(),
    }
    record = [record_line(n, d) for n, d in files.items()] + [f"{dist_info}/RECORD,,"]
    files[f"{dist_info}/RECORD"] = ("\n".join(record) + "\n").encode()

    args.out.mkdir(parents=True, exist_ok=True)
    wheel = args.out / f"{name}-{args.version}-{tag}.whl"
    with zipfile.ZipFile(wheel, "w", zipfile.ZIP_DEFLATED) as z:
        for n, d in files.items():
            info = zipfile.ZipInfo(n, ZIP_DATE)
            info.external_attr = (0o755 if n.endswith(".so") else 0o644) << 16
            info.compress_type = zipfile.ZIP_DEFLATED
            z.writestr(info, d)
    print(wheel)


if __name__ == "__main__":
    main()
