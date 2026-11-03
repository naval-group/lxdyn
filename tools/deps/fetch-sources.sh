#!/bin/sh
# Fetch every upstream source once into libcxx-src/, shared by every flavor.
set -e
FLAVOR=${1:-x86_64-linux-gnu}
. "$(dirname "$0")/common.sh"

clone() {  # $1 = url, $2 = tag, $3 = directory
  [ -d "$SRC/$3" ] || git clone --depth 1 --branch "$2" "$1" "$SRC/$3"
}

# yaml-cpp used to come from an external/ submodule instead, so that the closure and the CMake
# lane compiled the same sources. Deleting that lane left the closure as yaml-cpp's only
# consumer, so the submodule bought nothing and cost something: a git submodule is invisible to
# a Nix flake source, which would have left the one dependency Nix could not pin.
clone https://github.com/jbeder/yaml-cpp yaml-cpp-0.9.0 yaml-cpp
clone https://github.com/google/googletest v1.18.0 googletest
clone https://github.com/HDFGroup/hdf5 2.2.0 hdf5

# gRPC vendors abseil, protobuf, re2, c-ares, upb, boringssl and zlib. The closure builds all of
# them rather than resolving any from the host, so the submodules are not optional.
[ -d "$SRC/grpc" ] || git clone --depth 1 --branch v1.85.1 \
  --recurse-submodules --shallow-submodules https://github.com/grpc/grpc "$SRC/grpc"

[ -d "$SRC/boost_1_92_0" ] || \
  curl -fsSL https://archives.boost.io/release/1.92.0/source/boost_1_92_0.tar.bz2 | tar xj -C "$SRC"

# b2 is bootstrapped here rather than in build-boost.sh: the source tree is shared, so
# bootstrapping per flavor would have the flavors race. b2 is a host tool -- it reads Jamfiles
# and spawns compilers, and nothing it links ever reaches the closure.
#
# Built by zig, and deliberately NOT through bootstrap.sh: that script clears CXX before
# probing the host for a compiler and passes nothing through, so no variable can steer it.
# The engine's own build.sh takes --cxx. Without this the closure build silently needs a host
# C++ compiler nobody declared -- the devShell has none at all, so b2 was being built
# by whatever /usr/bin happened to offer, and the build simply failed on a machine with none.
#
# musl, so b2 comes out statically linked and runs on any Linux of this architecture. It
# outlives the shell that made it: $SRC is shared across flavors and can be shared between
# clones, so a b2 tied to one machine's glibc would be a trap.
[ -x "$SRC/boost_1_92_0/b2" ] || (
  cd "$SRC/boost_1_92_0/tools/build/src/engine"
  # Through `sh`, not ./build.sh: its shebang is `#!/usr/bin/env sh`, and a build sandbox
  # has no /usr/bin/env -- the failure is "not found" for a script that plainly exists.
  sh ./build.sh clang --cxx="zig c++ -target $(uname -m)-linux-musl" > /dev/null
  cp b2 "$SRC/boost_1_92_0/b2"
)
