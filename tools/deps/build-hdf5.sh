#!/bin/sh
# No HL and no tools -- xdyn calls H5:: directly -- and no compression filters, which would pull
# the host's zlib and szip into a closure that is otherwise self-contained.
set -e
FLAVOR=${1:-x86_64-linux-gnu}
. "$(dirname "$0")/common.sh"

cmake_build "$SRC/hdf5" hdf5 \
  -DHDF5_BUILD_CPP_LIB=ON \
  -DHDF5_BUILD_HL_LIB=OFF \
  -DHDF5_BUILD_TOOLS=OFF \
  -DHDF5_BUILD_EXAMPLES=OFF \
  -DHDF5_BUILD_UTILS=OFF \
  -DBUILD_TESTING=OFF \
  -DHDF5_ENABLE_ZLIB_SUPPORT=OFF \
  -DHDF5_ENABLE_SZIP_SUPPORT=OFF
