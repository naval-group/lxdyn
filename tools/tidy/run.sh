#!/bin/sh
# Arguments go to run-clang-tidy: `-fix`, or `-checks=-*,<check>` to survey a check before adopting it.
set -eu
cd "$(dirname "$0")/../.."

case $(command -v clang-tidy) in
    /nix/store/*) ;;
    *) echo "tidy: '$(command -v clang-tidy || echo none)' is not the devShell's clang-tidy, pinned to zig's clang" >&2
       exit 1 ;;
esac
exec run-clang-tidy -quiet -p . "$@"
