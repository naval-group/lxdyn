#!/bin/sh
# Print the install prefix the mise tasks pass to `zig build --prefix`: build/<triple>.
#
#   sh tools/build-dir.sh                      -> /path/to/repo/build/x86_64-linux-gnu
#   sh tools/build-dir.sh x86_64-windows-gnu   -> /path/to/repo/build/x86_64-windows-gnu
#
# The default is the native target as build.zig resolves it, in *zig's* names rather than
# uname's (uname says arm64/Darwin, zig says aarch64/macos). build.zig pins gnu on Linux, so
# a musl host still builds -gnu by default.
set -eu

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)

if [ $# -gt 0 ]; then
    printf '%s' "$repo/build/$1"
    exit 0
fi

case $(uname -m) in
    x86_64 | amd64)  arch=x86_64 ;;
    arm64 | aarch64) arch=aarch64 ;;
    *)               arch=$(uname -m) ;;
esac

case $(uname -s) in
    Darwin) printf '%s' "$repo/build/$arch-macos-none" ;;
    *)      printf '%s' "$repo/build/$arch-linux-gnu" ;;
esac
