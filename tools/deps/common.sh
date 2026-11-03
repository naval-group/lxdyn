# Sourced by the recipes; does nothing on its own. The caller sets FLAVOR first.
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../.." && pwd)

FLAVOR=${FLAVOR:-x86_64-linux-gnu}

# Every flavor names an explicit target, including the native one. That is what makes the build
# hermetic: with no -target, zig treats the compile as native and honours CPATH, which inside
# `nix develop` means glibc 2.42 plus every libstdc++ C++ library in the flake's buildInputs.
# An explicit target switches zig to its own bundled headers and nothing else.
#
# glibc 2.28 is the floor because it is the oldest still in service — Debian 10, RHEL 8,
# Ubuntu 18.04. It also fixes what a published closure can link against: build against a newer
# glibc and its headers redirect to symbols (__isoc23_strtol and friends) that older systems
# do not have, so the final link fails on the user's machine rather than here.
# A flavor IS its target triple. Two-thirds of one does not identify a closure: x86_64-linux-gnu
# and x86_64-linux-musl differ only in the libc, which is exactly what a closure carries. The
# directory, the release asset, the mise task and build.zig's default all use this name.
case $FLAVOR in
  x86_64-linux-gnu)
    ZIG_TARGET=x86_64-linux-gnu.2.28
    B2_ARGS="architecture=x86 address-model=64"
    CMAKE_SYSTEM=Linux CMAKE_PROCESSOR=x86_64
    ;;
  x86_64-linux-musl)
    ZIG_TARGET=x86_64-linux-musl
    B2_ARGS="architecture=x86 address-model=64"
    CMAKE_SYSTEM=Linux CMAKE_PROCESSOR=x86_64
    ;;
  aarch64-linux-musl)
    ZIG_TARGET=aarch64-linux-musl
    B2_ARGS="architecture=arm address-model=64"
    CMAKE_SYSTEM=Linux CMAKE_PROCESSOR=aarch64
    ;;
  x86_64-windows-gnu)
    ZIG_TARGET=x86_64-windows-gnu
    B2_ARGS="target-os=windows threadapi=win32 architecture=x86 address-model=64"
    CMAKE_SYSTEM=Windows CMAKE_PROCESSOR=AMD64
    # The pure-C boringssl: its Windows assembly is NASM, untried with zig's linker, and the
    # fallback costs nothing for xdyn's localhost gRPC.
    GRPC_EXTRA=-DOPENSSL_NO_ASM=ON
    # gRPC and c-ares pin their own _WIN32_WINNT under MinGW (0x600, 0x0602) over zig's 0x0a00,
    # and gRPC puts the MSVC-only __declspec(empty_bases) on any Windows compiler. The rest is
    # HDF5's Windows code: DWORD printed with %u, which is the same width there, and its stdio
    # driver, which xdyn never selects, seeking with MinGW's 32-bit off_t.
    NOWARN_TARGET="-Wno-macro-redefined -Wno-unknown-attributes -Wno-unknown-pragmas -Wno-undef
      -Wno-shadow -Wno-format -Wno-gnu-zero-variadic-macro-arguments -Wno-bad-function-cast
      -Wno-sign-conversion -Wno-unused-parameter -Wno-implicit-int-float-conversion
      -Wno-shorten-64-to-32"
    ;;
  host-tools)
    # Not a closure flavor: the protoc and grpc_cpp_plugin the *build machine* runs. musl so they
    # come out static and need no interpreter; `uname -m` so nothing here is x86_64-only.
    #
    # No CMAKE_SYSTEM on purpose, and this is the bootstrap: naming one turns CMAKE_CROSSCOMPILING
    # on, whereupon gRPC hunts for a host protoc instead of building one -- which is exactly what
    # this step exists to produce.
    ZIG_TARGET=$(uname -m)-linux-musl
    ;;
  *)
    echo "unknown flavor '$FLAVOR' -- expected one of:" >&2
    echo "  x86_64-linux-gnu  x86_64-linux-musl  aarch64-linux-musl  x86_64-windows-gnu" >&2
    echo "A macOS flavor is deliberately absent: it needs a real Apple SDK, because gRPC's" >&2
    echo "cf_engine and abseil's timezone lookup use CoreFoundation. See docs." >&2
    exit 1
    ;;
esac
export ZIG_TARGET

# Warnings in the dependencies' own sources, each read and none ours to fix. zig-cc and zig-cxx
# append these after the project's flags, so its -Wall/-Wextra cannot turn them back on. Named
# one by one rather than -w, which also discards -Werror= diagnostics and so changes the result
# of the configure probes that rely on them -- and so that a new kind still shows after a bump.
export DEPS_NOWARN="-Wno-deprecated-declarations -Wno-cast-qual -Wno-c11-extensions
  -Wno-incompatible-pointer-types-discards-qualifiers -Wno-enum-enum-conversion
  -Wno-uninitialized-const-pointer ${NOWARN_TARGET-}"

# b2 keys its object directories off the toolset name, so each flavor needs its own -- but the
# triple's dashes cannot go in one: b2 splits `toolset=name-version` on the first dash. Strip
# them. (The `zig` prefix is separate and also load-bearing; see build-boost.sh.)
B2_TAG=$(echo "$FLAVOR" | tr -d -- '-')

# Sources are shared by every flavor; only the install tree and the merged archives are per-flavor.
# Keeping build trees out of $DEPS leaves it holding exactly what gets published.
SRC=${XDYN_DEPS_SRC:-$REPO/libcxx-src}
DEPS=${XDYN_DEPS:-$REPO/libcxx-$FLAVOR}
BUILD=$SRC/build/$FLAVOR
mkdir -p "$SRC" "$BUILD" "$DEPS/install/lib" "$DEPS/install/include"

# Checked here rather than three components in, when cmake is missing and the error is a
# shell's. `nix develop` alone carries neither cmake nor ninja: both went with the CMake lane
# and nothing in `zig build` wants them. They live in the
# .#deps shell, together with the emulators b2's cross configure probes need.
#
# llvm-nm is in the list because build-boost.sh and build-grpc.sh assert on its output, and it
# was undeclared for as long as these recipes have existed -- every shell that ever ran them
# happened to carry it. In a sandbox that did not, the omission surfaced five steps in, on the
# fourth build, and only because gRPC's assertion is written so that it fails when it cannot
# run. Boost's was not, and passed. A tool an assertion depends on belongs in this list.
for tool in cmake ninja zig llvm-nm; do
  command -v "$tool" > /dev/null 2>&1 || {
    echo "tools/deps needs '$tool', which this shell does not provide." >&2
    echo "Building a closure needs:  nix develop .#deps" >&2
    exit 1
  }
done

# cmake and b2 both exec these by name.
PATH=$HERE/bin:$PATH

# gRPC find_program()s a HOST protoc and grpc_cpp_plugin for its own build-time codegen, and
# they must come from the same v1.85.1 tree so the gencode matches the libprotobuf it is compiled
# against -- a system protoc fails the version assertion the generated sources carry.
#
# Keyed on CMAKE_SYSTEM rather than on the flavor name: a host protoc is wanted exactly when CMake
# believes it is cross-compiling, which is the condition that makes gRPC go looking for one. Every
# flavor now sets CMAKE_SYSTEM, so every flavor takes this branch; only build-host-tools.sh, which
# produces what is read here, does not.
if [ -n "$CMAKE_SYSTEM" ]; then
  # $XDYN_DEPS_HOST names the host tools, built by build-host-tools.sh. It used to name the native
  # *closure*, which made that closure a build dependency of the other three and assumed an x86_64
  # glibc build host; the host tools are static and per-machine, so it no longer does either.
  NATIVE_BIN=${XDYN_DEPS_HOST:-$REPO/host-tools}/install/bin
  # Prepended, never asserted here. Two callers source this file before the tools exist --
  # fetch-sources.sh, which does not need them, and build-host-tools.sh, which exists to create
  # them -- so an assertion at this point fails the bootstrap on its way to bootstrapping.
  # build-grpc.sh is the only step that needs them, and checks there.
  PATH=$NATIVE_BIN:$PATH
fi
export PATH

# find_program consults $CMAKE_PREFIX_PATH before $PATH, and `nix develop` exports one naming
# every C++ library in the flake, so gRPC would take the flake's protoc over the prepend above.
# A closure may not discover anything through the shell that happens to be running it.
unset CMAKE_PREFIX_PATH CMAKE_PROGRAM_PATH CMAKE_INCLUDE_PATH CMAKE_LIBRARY_PATH CMAKE_FRAMEWORK_PATH

# Every closure flavor takes the toolchain file, including the host-matching one. Setting
# CMAKE_SYSTEM_NAME turns CMAKE_CROSSCOMPILING on even when it names the host, whereupon gRPC hunts
# for a host protoc instead of building one -- which is now what we want, because the one it would
# have built is dynamically linked and cannot run in a build sandbox that has no /lib64. The cost
# is that CMake will not run its try_run probes: measured, HDF5 stops defining H5_HAVE__FLOAT16 and
# H5_LDOUBLE_TO_FLOAT16_CORRECT, matching the other three flavors, and xdyn uses neither.
#
# build-host-tools.sh is the one caller that sets no CMAKE_SYSTEM, and must not: it is the bootstrap.
if [ -n "$CMAKE_SYSTEM" ]; then
  TOOLCHAIN=$BUILD/toolchain.cmake
  sed -e "s|@SYSTEM@|$CMAKE_SYSTEM|" -e "s|@PROCESSOR@|$CMAKE_PROCESSOR|" \
      -e "s|@BIN@|$HERE/bin|" -e "s|@PREFIX@|$DEPS/install|" \
      "$HERE/toolchain.cmake.in" > "$TOOLCHAIN"
  CMAKE_TARGET="-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN"
else
  CMAKE_TARGET="-DCMAKE_C_COMPILER=$HERE/bin/zig-cc -DCMAKE_CXX_COMPILER=$HERE/bin/zig-cxx
    -DCMAKE_AR=$HERE/bin/zig-ar -DCMAKE_RANLIB=$HERE/bin/zig-ranlib"
fi

# CMAKE_POLICY_VERSION_MINIMUM: CMake 4 refuses the pre-3.5 cmake_minimum_required that
# HDF5 1.14, yaml-cpp and gRPC's bundled zlib still declare.
#
# CMAKE_INSTALL_LIBDIR: an explicit -target leaves CMAKE_LIBRARY_ARCHITECTURE empty, and
# GNUInstallDirs then falls back to lib64 on any 64-bit target. b2 installs to lib regardless,
# so leaving this unset splits the closure across two directories and merge-deps.sh silently
# merges Boost alone.
#
# -Wno-author: the deprecation notices CMake addresses to protobuf's and re2's own authors.
# -Wno-unused-cli: the same flags go to every project and flavor, and not each one reads
# every variable -- HDF5 has no GNUInstallDirs, and gRPC only looks for systemd on Linux.
CMAKE_COMMON="$CMAKE_TARGET -Wno-author -Wno-unused-cli \
 -DCMAKE_CXX_STANDARD=17 \
 -DCMAKE_BUILD_TYPE=Release \
 -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
 -DBUILD_SHARED_LIBS=OFF \
 -DCMAKE_INSTALL_LIBDIR=lib \
 -DCMAKE_INSTALL_PREFIX=$DEPS/install"

cmake_build() {  # $1 = source dir, $2 = build subdir name, $3... = extra -D flags
  cmake_src=$1 cmake_name=$2
  shift 2
  # A build directory remembers the source directory it was configured from, and refuses to be
  # reused for another one. That is a correct thing for CMake to do and an unhelpful thing to
  # hit: a dependency moving between external/ and libcxx-src/ is a repository change, not a
  # user error, and $BUILD outlives it because the source tree is shared between flavors. The
  # cache is disposable -- drop it rather than making everyone delete it by hand.
  if [ -f "$BUILD/$cmake_name/CMakeCache.txt" ] && ! grep -qxF \
       "CMAKE_HOME_DIRECTORY:INTERNAL=$cmake_src" "$BUILD/$cmake_name/CMakeCache.txt"; then
    echo "$cmake_name: cached source directory has moved -- reconfiguring from scratch"
    rm -rf "$BUILD/$cmake_name"
  fi
  # CMake records the archiver in CMakeFiles/<ver>/CMakeCXXCompiler.cmake at first compiler
  # detection and never revisits it, so a tree configured before the archiver was named keeps
  # the one it found on PATH. -DCMAKE_AR does not dislodge it: it lands as an UNINITIALIZED
  # cache entry that the normal variable set in that file shadows, which means CMakeCache.txt
  # reads back zig-ar while rules.ninja still runs the host's. Checking the cache would pass.
  # Check what ninja executes.
  if [ -f "$BUILD/$cmake_name/CMakeFiles/rules.ninja" ] && ! grep -qF \
       "$HERE/bin/zig-ar" "$BUILD/$cmake_name/CMakeFiles/rules.ninja"; then
    echo "$cmake_name: build tree predates the zig archiver -- reconfiguring from scratch"
    rm -rf "$BUILD/$cmake_name"
  fi
  cmake -S "$cmake_src" -B "$BUILD/$cmake_name" -G Ninja $CMAKE_COMMON "$@"
  cmake --build "$BUILD/$cmake_name" -j"$(nproc)"
  cmake --install "$BUILD/$cmake_name"
}
