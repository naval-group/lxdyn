#!/bin/sh
# Fail unless the release tag matches build.zig.zon, where the flake reads xdyn's version.
#
#   sh tools/check-release-version.sh 26.11      -> ok against 26.11.0
#
# The tag is YY.M or YY.M.P; zig requires a full X.Y.Z, so 26.11 matches 26.11.0.
set -eu

tag=${1:?usage: check-release-version.sh <tag>}
repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
zon=$(sed -n 's/^ *\.version = "\(.*\)",$/\1/p' "$repo/build.zig.zon")

if [ "$zon" = "$tag" ] || [ "$zon" = "$tag.0" ]; then
    echo "build.zig.zon $zon matches tag $tag"
    exit 0
fi
echo "build.zig.zon says $zon but the tag is $tag: bump it, commit, and tag again" >&2
exit 1
