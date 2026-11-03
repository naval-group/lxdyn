#!/bin/sh
# The no-tabs check over xdyn's own C and C++: `mise run lint`, and the flake's `lint` check.
set -eu
cd "$(dirname "$0")/.."

if grep --recursive --include='*.cpp' --include='*.c' --include='*.hpp' --include='*.h' \
        --exclude-dir=ssc --exclude-dir=websocketpp \
        --exclude-dir=eigen3-hdf5 --exclude-dir=thirdparty -P "\t" xdyn/ ; then
    echo "Tabs found in the lines shown above."
    exit 1
fi
echo "Repo passed no-tabs check."
