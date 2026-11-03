#!/bin/sh
# googletest through its own CMake build, for every flavor. zig-cc names the -target, which
# drops CPATH from the search path: gmock cannot pick up a shell's gtest headers over its own.
#
# The *_main archives this installs are dropped at merge time: run_all_tests defines its own.
set -e
FLAVOR=${1:-x86_64-linux-gnu}
. "$(dirname "$0")/common.sh"

cmake_build "$SRC/googletest" googletest -DBUILD_GMOCK=ON -DINSTALL_GTEST=ON
