#!/bin/sh
# `mise run lint` and the flake's `lint` check: tabs, core/'s includes and clang-format in the C and C++, zig fmt on the build files.
set -eu
cd "$(dirname "$0")/.."

if grep --recursive --include='*.cpp' --include='*.c' --include='*.hpp' --include='*.h' \
        --exclude-dir=ssc --exclude-dir=websocketpp \
        --exclude-dir=eigen3-hdf5 --exclude-dir=thirdparty -P "\t" xdyn/ ; then
    echo "Tabs found in the lines shown above."
    exit 1
fi
echo "Repo passed no-tabs check."

# core/ builds a simulation from models it is handed, so it names their interfaces only.
if grep --recursive --include='*.cpp' --include='*.hpp' --exclude-dir=unit_tests -nE \
        '#include "xdyn/(force_models|simulator_api|observers|servers|wind_models|current_models|wave_models)/' xdyn/core \
    | grep -vE '"xdyn/(wind_models/WindModel|current_models/UWCurrentModel|wave_models/(SurfaceElevationInterface|SurfaceElevationGrid|DiscreteDirectionalWaveSpectrum))\.hpp"'; then
    echo "core/ includes the models above; it may include only their interfaces."
    exit 1
fi
echo "core/ includes only the model interfaces."

sh tools/format.sh --dry-run --Werror
echo "C and C++ passed clang-format."

# ast-check adds what AstGen rejects to what fmt parses, such as an unused local.
zig_files=$(find . \( -name .zig-cache -o -name zig-out -o -name build -o -name 'libcxx-*' -o -name host-tools \) -prune \
    -o -name '*.zig' -print)
zig fmt --check build.zig.zon $zig_files
for file in $zig_files; do zig ast-check "$file" > /dev/null; done
echo "Zig build files passed zig fmt and ast-check."
