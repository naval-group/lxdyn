#!/bin/sh
# clang-format over the first-party C and C++; arguments go to clang-format: `-i`, or `--dry-run --Werror`.
# rapidjson and the FMI headers are vendored in place and keep their upstream's format.
set -eu
cd "$(dirname "$0")/.."

find xdyn ssc xdyn_wrapper_python xdyn_wrapper_fmi integration_tests \
    \( -path ssc/json/rapidjson -o -path xdyn_wrapper_fmi/fmi2 \) -prune \
    -o -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.c' -o -name '*.h' \) -print \
    | xargs -P "$(nproc)" -n 32 clang-format "$@"
