#!/bin/sh
# Debug an xdyn executable from the -Ddebug build.
#
#   sh tools/gdb.sh run_all_tests --gtest_filter='Foo*'
#   mise run gdb -- run_all_tests --gtest_filter='Foo*'
#
# The explicit -x is the point of this wrapper: gdb declines to auto-load a repository
# .gdbinit unless the directory is on its auto-load safe-path, so the catchthrow helper
# would silently not be there.
#
# XDYN_TRIPLE selects the build tree (default: the native one).
set -e
REPO=$(cd "$(dirname "$0")/.." && pwd)
cd "$REPO"

TRIPLE="${XDYN_TRIPLE:-x86_64-linux-gnu}"
BIN_DIR="build/$TRIPLE-debug/bin"

if [ $# -eq 0 ]; then
  echo "usage: sh tools/gdb.sh <executable> [args...]" >&2
  echo "available in $BIN_DIR:" >&2
  ls "$BIN_DIR" 2>/dev/null | sed 's/^/  /' >&2 || echo "  (none — run: mise run zig:debug)" >&2
  exit 2
fi

exe="$1"
shift

if [ ! -x "$BIN_DIR/$exe" ]; then
  echo "tools/gdb.sh: $BIN_DIR/$exe not found." >&2
  echo "              build it first with: mise run zig:debug" >&2
  echo "              (set \$XDYN_TRIPLE to debug a cross build)" >&2
  exit 1
fi

exec gdb -q -x .gdbinit --args "$BIN_DIR/$exe" "$@"
