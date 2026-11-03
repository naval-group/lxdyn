#!/bin/sh
# Merge the closure into two archives:
#   libxdyndeps_core.a  everything except gtest/gmock -- linked by every target
#   libxdyndeps_test.a  gtest and gmock -- linked by the test runner only
# The abseil/gRPC graph is circular and lld resolves on demand inside a single archive. Getting
# thirty separate .a files into a working order is the alternative.
set -e
FLAVOR=${1:-x86_64-linux-gnu}
. "$(dirname "$0")/common.sh"

LIB=$DEPS/install/lib

mri() {  # $1 = archive to write, $2... = archives to absorb
  out=$1
  shift
  rm -f "$out"
  { echo "create $out"
    for a in "$@"; do echo "addlib $a"; done
    echo save
    echo end
  } | zig ar -M
}

# *.lib as well as *.a: under target-os=windows b2 names its static archives .lib, in the same
# ar format. *.dll.a is excluded because the bundled zlib builds a DLL unconditionally, and
# merging its import library would bind every executable to a DLL at runtime.
# *_main is excluded on both sides: it defines a main() that collides with every executable's
# own, and which flavors install one differs.
core=$(ls "$LIB"/*.a "$LIB"/*.lib 2>/dev/null \
       | grep -vE 'libg(test|mock)(_main)?\.(a|lib)$' \
       | grep -v '\.dll\.a$')

mri "$DEPS/libxdyndeps_core.a" $core
mri "$DEPS/libxdyndeps_test.a" "$LIB/libgtest.a" "$LIB/libgmock.a"

# Captured, then tested, for the same reason as build-boost.sh's check.
members=$(zig ar t "$DEPS/libxdyndeps_core.a")
case $members in
  *gtest* | *gmock*) echo "merge: gtest leaked into the core archive" >&2; exit 1 ;;
esac
echo "$FLAVOR core: $(printf '%s\n' "$members" | wc -l) members"
