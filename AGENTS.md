# AGENTS.md

## Rules

- No AI-generated prose in issues or pull requests. A human explains the problem and the fix; generated code is fine if a human can defend it.
- Never add `pkgs.boost`, `pkgs.grpc`, `pkgs.hdf5` or `pkgs.binutils` to the devShell. They are libstdc++ builds, and mixing them with libc++ causes link failures far from the cause. The C++ libraries come only from the dependency closure.
- `ssc/`, `proto/` and `xdyn_grpc/` are lxdyn's own code: fix defects there directly. `external/` is vendored third-party code: list any edit in that tree's `PROVENANCE.md`.
- Comments explain why, never what. No dates or changelog notes in comments.

## Before calling a change done

```sh
zig build test                 # unit tests, then SSC's run_ssc_tests
mise run lint                  # no tabs in xdyn/
mise run python:lint           # if you touched Python
mise run python:test           # if you touched xdyn_wrapper_python/
mise run integration           # if you touched the CLI or the YAML parser
mise run integration:grpc      # if you touched proto/ or xdyn_grpc/
nix flake check                # what CI runs
```

Run one test with `mise run zig:build && $(sh tools/build-dir.sh)/bin/run_all_tests --gtest_filter='Gravity*'`, or skip the long end-to-end ones with `--gtest_filter=-'*LONG*'`.

## Layout

| Path | What |
|---|---|
| `xdyn/force_models/` | one class per force model, with a static `model_name()` that is its YAML key |
| `xdyn/environment_models/` | waves, spectra, current, wind |
| `xdyn/yaml_parser/` | input parsing |
| `xdyn/observers_and_api/simulator_api.cpp` | `can_parse<...>()` registers every model; a new model goes here |
| `xdyn/<module>/unit_tests/` | that module's GoogleTest suite |
| `xdyn/executables/` | `xdyn`, the gRPC/websocket servers and the example generators |
| `ssc/` | SSC: units, interpolation, kinematics, solvers, websocket |
| `proto/`, `xdyn_grpc/` | the gRPC API and the Python package for writing models against it |
| `xdyn_wrapper_python/`, `xdyn_wrapper_fmi/` | the pybind11 module and the FMU |
| `integration_tests/` | CLI, gRPC, JSON and deploy scenarios |
| `tools/deps/` | closure build scripts and `assets.sha256` |

`build.zig` globs its sources, so a new `.cpp` under `xdyn/<module>/` or `xdyn/<module>/unit_tests/` needs no registration. Each component has its own `build.zig` next to its sources. `zig build gen` writes the gRPC stubs, SSC's umbrella headers and the git SHA into the zig cache, so never commit generated files.

## Traps

- **`zig build` installs into `zig-out/`, which nothing reads.** Use `mise run zig:build` (`--prefix build/<triple>`).
- **`zig build test` installs nothing.** The integration lanes depend on `zig:build` for that reason: without it every scenario exits 127.
- **Debug builds use `-Ddebug` (`mise run zig:debug`), never `-Doptimize=Debug`.** `mise run gdb` and `.gdbinit` expect the former.
- **Never check a cross target with `zig build test -Dtarget=...`.** On a host with `binfmt_misc` handlers it passes without qemu or wine. Use `mise run cross` in `nix develop .#cross`.
- **`integration_tests/xdyn/Makefile` has a hardcoded `all:` list.** A new scenario does not run until it has a target.
- **`build.zig` stops on a fetched closure that `tools/deps/assets.sha256` no longer names.** Re-fetch with `mise run deps:fetch <triple>`.
- **The deploy image is `FROM scratch` with static musl binaries.** It has no shell, so `--entrypoint sh` fails, and no CA certificates.
- **A release bumps `build.zig.zon` to the tag first** (`26.11` is `26.11.0` there), because Nix cannot see tags. The release workflows refuse a mismatch.

## The dependency closure

C++ dependencies are prebuilt per target triple into `./libcxx-<triple>` (`-Ddeps=` or `$XDYN_DEPS_<TRIPLE>`). protoc and grpc_cpp_plugin go in `./host-tools` (`-Ddeps-host=` or `$XDYN_DEPS_HOST`). `build.zig` locates a closure and never builds one.

```sh
mise run bootstrap                       # host tools + the native closure
mise run deps:fetch aarch64-linux-musl   # another target
nix build .#libcxx-deps-x86_64-linux-gnu # build one hermetically, ~20 min
```

Triples: `x86_64-linux-gnu`, `x86_64-linux-musl`, `aarch64-linux-musl`, `x86_64-windows-gnu`. `mise tasks` lists everything else.
