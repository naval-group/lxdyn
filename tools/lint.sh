#!/bin/sh
# `mise run lint` and the flake's `lint` check: no tabs in xdyn's C and C++, zig fmt over the build files.
set -eu
cd "$(dirname "$0")/.."

if grep --recursive --include='*.cpp' --include='*.c' --include='*.hpp' --include='*.h' \
        --exclude-dir=ssc --exclude-dir=websocketpp \
        --exclude-dir=eigen3-hdf5 --exclude-dir=thirdparty -P "\t" xdyn/ ; then
    echo "Tabs found in the lines shown above."
    exit 1
fi
echo "Repo passed no-tabs check."

# ast-check adds what AstGen rejects to what fmt parses, such as an unused local.
zig_files=$(find . \( -name .zig-cache -o -name zig-out -o -name build -o -name 'libcxx-*' -o -name host-tools \) -prune \
    -o -name '*.zig' -print)
zig fmt --check build.zig.zon $zig_files
for file in $zig_files; do zig ast-check "$file" > /dev/null; done
echo "Zig build files passed zig fmt and ast-check."
