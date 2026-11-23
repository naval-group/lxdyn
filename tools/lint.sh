#!/bin/sh
# `mise run lint` and the flake's `lint` check: tabs, each module's includes and clang-format in the C and C++, zig fmt on the build files.
set -eu
cd "$(dirname "$0")/.."

if grep --recursive --include='*.cpp' --include='*.c' --include='*.hpp' --include='*.h' \
        --exclude-dir=ssc --exclude-dir=websocketpp \
        --exclude-dir=eigen3-hdf5 --exclude-dir=thirdparty -P "\t" xdyn/ ; then
    echo "Tabs found in the lines shown above."
    exit 1
fi
echo "Repo passed no-tabs check."

# Each module, then what its code may include from xdyn/: a module, or one header as module/Header.hpp; an unlisted module may include any.
# core/ builds a simulation from what it is handed, so it names the model interfaces only.
include_rules='
core           exceptions input mesh wind_models/WindModel.hpp current_models/UWCurrentModel.hpp wave_models/SurfaceElevationInterface.hpp wave_models/SurfaceElevationGrid.hpp wave_models/DiscreteDirectionalWaveSpectrum.hpp
input
yaml_parser    exceptions input
force_models   core exceptions hydro_db input mesh yaml_parser
wind_models    core exceptions hydro_db input mesh yaml_parser
current_models core exceptions hydro_db input mesh yaml_parser
wave_models    core exceptions hydro_db input mesh yaml_parser
'
library_code() {
    grep --recursive --include='*.cpp' --include='*.hpp' --include='*.h' --exclude-dir=unit_tests "$@"
}
module_of() {
    sed -E 's|^xdyn/([a-z_]+)/.*|\1|'
}
broken=$(echo "$include_rules" | while read -r module allowed; do
    [ -n "$module" ] || continue
    library_code -noE '#include "xdyn/[a-z_]+/[^"]+"' "xdyn/$module" \
        | awk -v self="$module" -v allowed="$allowed" '{
            header = $0; sub(/.*#include "xdyn\//, "", header); sub(/"$/, "", header)
            folder = header; sub(/\/.*/, "", folder)
            n = split(allowed, entries, " ")
            ok = folder == self
            for (i = 1; i <= n; i++) if (entries[i] == folder || entries[i] == header) ok = 1
            if (!ok) print
        }'
done)
if [ -n "$broken" ]; then
    echo "$broken"
    echo "The includes above break tools/lint.sh's include_rules."
    exit 1
fi

# What only some modules may include: its name, its pattern, the files it applies to, then those modules.
only_in() {
    name=$1 pattern=$2 files=$3 allowed=$4
    found=$(library_code -lE "$pattern" xdyn | grep -E "$files" | while read -r file; do
        case " $allowed " in *" $(echo "$file" | module_of) "*) ;; *) echo "$file" ;; esac
    done)
    if [ -n "$found" ]; then
        echo "$found"
        echo "The files above include $name; only $allowed may."
        exit 1
    fi
}
# The model folders sit below grpc/, so the gRPC models are registered above it.
only_in 'grpc/' '#include "xdyn/grpc/' '' 'grpc simulator_api executables'
only_in 'gRPC or protobuf headers' '#include [<"]((grpc|grpcpp|google/protobuf)/|[a-z_]+\.(grpc\.)?pb\.h)' '' 'grpc servers executables'
only_in 'yaml-cpp in a header' '#include [<"]yaml-cpp/' '\.h(pp)?$' 'yaml_parser'
if library_code -nE '#include [<"]yaml-cpp/' xdyn/core; then
    echo "core/ includes yaml-cpp above; it works on parsed input only."
    exit 1
fi
echo "Every module includes only what the include rules allow."

sh tools/format.sh --dry-run --Werror
echo "C and C++ passed clang-format."

# ast-check adds what AstGen rejects to what fmt parses, such as an unused local.
zig_files=$(find . \( -name .zig-cache -o -name zig-out -o -name build -o -name 'libcxx-*' -o -name host-tools \) -prune \
    -o -name '*.zig' -print)
zig fmt --check build.zig.zon $zig_files
for file in $zig_files; do zig ast-check "$file" > /dev/null; done
echo "Zig build files passed zig fmt and ast-check."
