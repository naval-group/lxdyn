#!/bin/sh
# Pack a built closure into the release asset fetch.sh downloads, and print the line to
# paste into assets.sha256.
#
#   tools/deps/pack.sh native   ->  build/scratch/deps-assets/lxdyn-deps-native.tar.zst
#
# What goes in is what a consumer reads, and nothing else:
#
#   libxdyndeps_core.a, libxdyndeps_test.a   what build.zig links
#   install/include                          what build.zig compiles against
#   install/bin                              the host-matching flavor only -- build.zig runs *this* protoc
#                                            grpc_cpp_plugin, so a clone that only ever
#                                            fetched a closure cannot generate its sources
#                                            without them
#
# install/lib is deliberately absent: merge-deps.sh has already folded those thirty-odd
# archives into the two above, and they are most of the 11 GB build tree.
#
# Debug info is stripped. The asset exists to bootstrap a fresh clone and to feed CI, neither
# of which steps into boost, and it is a ~9x difference on every download. XDYN_PACK_STRIP=0
# keeps it, for anyone who does want to debug into the closure.
set -e
FLAVOR=${1:-x86_64-linux-gnu}
. "$(dirname "$0")/common.sh"

OUT=${XDYN_PACK_OUT:-$REPO/build/scratch/deps-assets}
ASSET=$OUT/lxdyn-deps-$FLAVOR.tar.zst
STRIP=${XDYN_PACK_STRIP:-1}

[ -f "$DEPS/libxdyndeps_core.a" ] || {
    echo "pack: no libxdyndeps_core.a under $DEPS -- build it first with 'mise run deps:$FLAVOR'" >&2
    exit 1
}

# The floor is what makes a published native asset linkable on Debian 10 / RHEL 8. Checked
# rather than trusted: the recipes pin it, but a closure built before the pin looks identical
# from the outside. See common.sh for why __isoc23_* is the tell.
#
# The symbol list is captured first and tested second, with no 2>/dev/null and no `|| true`. Read
# as one pipeline the check cannot fail: a missing llvm-nm, an unreadable archive and a clean one
# all produce no output, `grep -c` answers 0, and the gate opens. build-boost.sh carried the same
# construction and passed in a sandbox with no llvm in it -- an assertion that cannot
# run reporting success. This one guards what gets published.
if [ "$FLAVOR" = x86_64-linux-gnu ]; then
    syms=$(llvm-nm --undefined-only "$DEPS/libxdyndeps_core.a") || {
        echo "pack: cannot read $DEPS/libxdyndeps_core.a to check its glibc floor" >&2
        exit 1
    }
    stale=$(printf '%s\n' "$syms" | grep -c __isoc23_ || true)
    [ "$stale" -eq 0 ] || {
        echo "pack: $DEPS was built against a glibc newer than the 2.28 floor" >&2
        echo "      ($stale undefined __isoc23_* symbols; rebuild with 'mise run deps:x86_64-linux-gnu')" >&2
        exit 1
    }
fi

# llvm-strip, never binutils' strip. binutils is built for one architecture: on the aarch64
# and Windows archives it reports "Unable to recognise the architecture" for every member and
# still exits 0, which packs a 264 MB aarch64 asset where 50 MB was expected and gives no
# other sign. llvm-strip handles every flavor, so it is required rather than probed for.
if [ "$STRIP" != 0 ]; then
    command -v llvm-strip > /dev/null 2>&1 || {
        echo "pack: llvm-strip not on PATH -- run inside 'nix develop', or XDYN_PACK_STRIP=0" >&2
        exit 1
    }
fi

mkdir -p "$OUT"
STAGE=$(mktemp -d "${TMPDIR:-/tmp}/xdyn-pack-XXXXXX")
trap 'rm -rf "$STAGE"' EXIT HUP INT TERM

mkdir -p "$STAGE/install"
cp "$DEPS/libxdyndeps_core.a" "$DEPS/libxdyndeps_test.a" "$STAGE/"
cp -a "$DEPS/install/include" "$STAGE/install/"
[ "$FLAVOR" = x86_64-linux-gnu ] && cp -a "$DEPS/install/bin" "$STAGE/install/"

# $DEPS is a /nix/store path whenever the closure came from `nix build`, and everything under one
# is read-only. -a preserves that, after which the stripper cannot write and even the exit trap
# cannot clean up. Making the *copy* writable is the point -- the original stays untouched.
chmod -R u+w "$STAGE"

if [ "$STRIP" != 0 ]; then
    before=$(stat -c %s "$STAGE/libxdyndeps_core.a")
    # -type f skips the symlinks (protoc -> protoc-<version>), which would be stripped twice.
    # Shebang files are skipped by hand: install/bin also holds h5cc and h5c++, which are
    # shell scripts, and llvm-strip rejects them. Anything else failing is a real failure and
    # is allowed to stop the pack -- no `|| true`, which is what hid the bug above.
    find "$STAGE" -type f \( -name '*.a' -o -perm -u+x \) -exec sh -c '
        for f do
            [ "$(head -c2 "$f")" = "#!" ] && continue
            llvm-strip --strip-debug "$f"
        done' _ {} +
    after=$(stat -c %s "$STAGE/libxdyndeps_core.a")
    # Trust the size, not the exit status. Every archive here is >80% debug info; anything
    # above half its original size means the stripper walked over it without doing anything.
    [ "$after" -lt $((before / 2)) ] || {
        echo "pack: libxdyndeps_core.a is $after bytes after stripping, was $before" >&2
        echo "      the stripper did not recognise its members -- asset not written" >&2
        exit 1
    }
fi

# -19 rather than the default -3. This is written once and downloaded many times, and zstd's
# decompression speed does not depend on the level: -19 costs the packer ~40 s per flavor and
# the fetcher nothing, for a third off. `--ultra -22 --long=27` takes another 21% at ~4 min,
# and still decompresses with a plain `zstd -d` -- set XDYN_PACK_ZSTD to it if that matters.
tar -C "$STAGE" -cf - . | zstd ${XDYN_PACK_ZSTD:--19} -T0 -q -f -o "$ASSET"

echo "$FLAVOR: $(du -h "$ASSET" | cut -f1) -> $ASSET"
echo "paste into tools/deps/assets.sha256:"
( cd "$OUT" && sha256sum "lxdyn-deps-$FLAVOR.tar.zst" )
