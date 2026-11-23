# AGENTS.md

## Rules

- No AI-generated prose in issues or pull requests. A human explains the problem and the fix; generated code is fine if a human can defend it.
- Never add `pkgs.boost`, `pkgs.grpc`, `pkgs.hdf5` or `pkgs.binutils` to the devShell. They are libstdc++ builds, and mixing them with libc++ causes link failures far from the cause. The C++ libraries come only from the dependency closure.
- `ssc/`, `proto/` and `xdyn_grpc/` are lxdyn's own code: fix defects there directly. `external/` is vendored third-party code: list any edit in that tree's `PROVENANCE.md`.
- Comments explain why, never what. No dates or changelog notes in comments.

## Before calling a change done

```sh
zig build test                 # unit tests, then SSC's run_ssc_tests
mise run format                # clang-format the C and C++; lint fails without it
mise run lint                  # tabs and clang-format in the C and C++, zig fmt on the build files
mise run python:lint           # if you touched Python
mise run python:test           # if you touched xdyn_wrapper_python/
mise run integration           # if you touched the CLI or the YAML parser
mise run integration:grpc      # if you touched proto/ or xdyn_grpc/
nix flake check                # what CI runs
```

Run one test with `mise run test -- --gtest_filter='Gravity*'`, or skip the long end-to-end ones with `--gtest_filter=-'*LONG*'`. The suites write their fixtures and outputs into the directory they run from, so never start `run_all_tests` from the repository root.

## Layout

| Path | What |
|---|---|
| `xdyn/` | one folder per module; [`xdyn/README.md`](xdyn/README.md) says what each holds, and the table in `tools/lint.sh` what each may include |
| `xdyn/force_models/` | one class per force model, with a static `model_name()` that is its YAML key |
| `xdyn/force_models/force_model_table.cpp` | every force model a YAML file can name but `grpc`; a new force model is one line here |
| `xdyn/{wind,current,wave}_models/*_model_table.cpp` | every wind, current and wave model a YAML file can name but `grpc`, and the wave spectra and spreadings; a new one is one line in its folder's table |
| `xdyn/simulator_api/` | YAML and meshes to a `Sim` and its controllers, the one module that depends on every model; `model_tables.cpp` asks the folder tables, then the gRPC models, which only it registers |
| `xdyn/core/` | the engine; it includes the model interfaces only, and no parser or file reader |
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
