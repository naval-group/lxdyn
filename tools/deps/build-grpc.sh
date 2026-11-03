#!/bin/sh
# The whole gRPC stack, including the abseil, protobuf, re2, c-ares, upb, boringssl and zlib it
# vendors. Every one of them exports mangled std:: symbols, so none may come from the host.
set -e
FLAVOR=${1:-x86_64-linux-gnu}
. "$(dirname "$0")/common.sh"

# The only step that needs the host tools: gRPC generates C++ from its own .proto files during
# its build and has to *run* protoc to do it. common.sh puts them on PATH; the check belongs here
# rather than there, because fetch-sources.sh and build-host-tools.sh both source common.sh
# before these exist -- the second one in order to produce them.
[ -x "$NATIVE_BIN/protoc" ] || {
  echo "$FLAVOR needs the host tools first: no protoc under $NATIVE_BIN" >&2
  echo "run 'tools/deps/build-host-tools.sh'" >&2
  exit 1
}

# protobuf 35 makes SerializeToString [[nodiscard]], and gRPC's reflection and channelz services
# drop its result.
export DEPS_NOWARN="$DEPS_NOWARN -Wno-unused-result"

# CMAKE_DISABLE_FIND_PACKAGE_systemd: gRPC picks up a host libsystemd if one is installed, and
# every binary in the closure then needs it at link time.
# The plugin toggles: xdyn generates C++ only, and the other six languages' plugins are a
# significant fraction of the build.
cmake_build "$SRC/grpc" grpc \
  -DgRPC_BUILD_TESTS=OFF \
  -DgRPC_INSTALL=ON \
  -DABSL_PROPAGATE_CXX_STD=ON \
  -DCMAKE_DISABLE_FIND_PACKAGE_systemd=ON \
  -DZLIB_BUILD_EXAMPLES=OFF \
  -DgRPC_BUILD_GRPC_CPP_PLUGIN=ON \
  -DgRPC_BUILD_GRPC_PYTHON_PLUGIN=OFF \
  -DgRPC_BUILD_GRPC_CSHARP_PLUGIN=OFF \
  -DgRPC_BUILD_GRPC_NODE_PLUGIN=OFF \
  -DgRPC_BUILD_GRPC_OBJECTIVE_C_PLUGIN=OFF \
  -DgRPC_BUILD_GRPC_PHP_PLUGIN=OFF \
  -DgRPC_BUILD_GRPC_RUBY_PLUGIN=OFF \
  $GRPC_EXTRA

# Captured, then tested, for the same reason as build-boost.sh's check.
syms=$(llvm-nm "$DEPS/install/lib/libgrpc++.a")
case $syms in
  *St3__1*) ;;
  *) echo "grpc: no libc++ mangling in the output" >&2; exit 1 ;;
esac
