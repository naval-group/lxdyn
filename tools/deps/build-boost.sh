#!/bin/sh
# b2 rather than CMake: Boost's release tarball ships no supported CMake build for the compiled
# libraries. The cross flavors need their binfmt_misc handler registered, because b2 runs its
# configure probes -- qemu-user for aarch64, wine for win. `nix develop .#cross` provides both.
set -e
FLAVOR=${1:-x86_64-linux-gnu}
. "$(dirname "$0")/common.sh"

cd "$SRC/boost_1_92_0"

# One toolset name per flavor, from common.sh's B2_TAG -- the flavor with its dashes stripped,
# because b2 splits `toolset=name-version` on the first dash. b2 keys its object directories
# off the toolset, so a shared name would have the flavors overwrite each other's objects.
#
# The zig prefix is load-bearing, not decoration: b2 ships a builtin toolset called clang-win
# (clang-cl against MSVC), so `using clang : win` loads tools/clang-win.jam and fails demanding
# clang-cl.exe. Any flavor named after a platform b2 knows would collide the same way.
cat > "$BUILD/user-config.jam" <<JAM
using clang : zig$B2_TAG : $HERE/bin/zig-cxx-boost : <archiver>$HERE/bin/zig-ar <ranlib>$HERE/bin/zig-ranlib ;
JAM

./b2 --user-config="$BUILD/user-config.jam" toolset=clang-zig$B2_TAG \
  link=static runtime-link=shared threading=multi variant=release cxxstd=17 \
  $B2_ARGS --build-dir="$BUILD/boost" \
  --with-program_options --with-filesystem --with-regex --with-thread \
  --with-random --with-chrono --with-system --with-date_time --with-atomic \
  --prefix="$DEPS/install" -j"$(nproc)" install

# A wrapper b2 failed to pick up leaves a libstdc++ build that links against libc++ code and
# then breaks at the first std::string crossing the boundary. b2 names its archives .lib under
# target-os=windows, so match the stem rather than the extension.
#
# Asserted positively, the way build-grpc.sh does, and with no 2>/dev/null. The negative form --
# "fail if __cxx11 appears" -- passes when llvm-nm is absent, when the glob matches nothing, and
# when the archive is unreadable, because all three produce no output for grep to find. It did
# exactly that in a build sandbox with no llvm in it: an assertion that cannot
# run reporting success. libboost_filesystem carries 95 St3__1 symbols when the wrapper takes.
# Captured, then tested: `llvm-nm | grep -q` stops reading at the first match, which aborts
# llvm-nm with a broken pipe wherever SIGPIPE is ignored, as in the Nix sandbox.
syms=$(llvm-nm "$DEPS/install/lib/"libboost_filesystem.*)
case $syms in
  *St3__1*) ;;
  *) echo "boost: no libc++ mangling in the output -- the zig wrapper did not take" >&2; exit 1 ;;
esac
