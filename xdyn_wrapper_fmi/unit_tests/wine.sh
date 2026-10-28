#!/bin/sh
# The Windows library against the Linux one: both packed in one FMU, both stepped by
# run_steps.c (the Windows build under wine), the final states compared. Needs
# `nix develop .#cross` for wine, and both libraries built (mise run fmu:wine builds them).
set -eu

repo=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$repo"
work=build/scratch/fmu-wine
mkdir -p "$work"
python=build/venv/fmu/bin/python

"$python" xdyn_wrapper_fmi/make_fmu.py xdyn_wrapper_fmi/unit_tests/propelled_ship.yml \
    -o "$work/ship.fmu" --dt 0.1 \
    --library "$(sh tools/build-dir.sh)/fmu/xdyn_fmu.so" \
    --binary win64=build/x86_64-windows-gnu/fmu/xdyn_fmu.dll
"$python" -m zipfile -e "$work/ship.fmu" "$work/ship"

zig cc -O2 -I xdyn_wrapper_fmi/fmi2 xdyn_wrapper_fmi/unit_tests/run_steps.c -ldl \
    -o "$work/run_steps"
zig cc -O2 -target x86_64-windows-gnu -I xdyn_wrapper_fmi/fmi2 \
    xdyn_wrapper_fmi/unit_tests/run_steps.c -o "$work/run_steps.exe"

"$work/run_steps" "$repo/$work/ship" "$work/ship/binaries/linux64/xdyn_fmu.so" > "$work/linux.txt"
export WINEPREFIX="$repo/build/wine" WINEDEBUG=-all
wine "$work/run_steps.exe" "$(winepath -w "$repo/$work/ship" | tr '\\' /)" \
    "$(winepath -w "$repo/$work/ship/binaries/win64/xdyn_fmu.dll")" > "$work/windows.txt" 2> /dev/null

# Not bit for bit: the two C runtimes round some transcendental functions differently.
"$python" - "$work/linux.txt" "$work/windows.txt" <<'PY'
import math, sys
linux, windows = ([float(x) for x in open(p)] for p in sys.argv[1:])
bad = [(a, b) for a, b in zip(linux, windows) if not math.isclose(a, b, rel_tol=1e-12, abs_tol=1e-12)]
if len(linux) != 13 or len(windows) != 13 or bad:
    sys.exit(f"Linux and Windows differ: {list(zip(linux, windows))}")
print("Linux and Windows libraries agree on all 13 states")
PY
