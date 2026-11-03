# AGENTS.md

Notes for automated coding agents working in this repository. Humans should read [README.md](README.md) and [CONTRIBUTING.md](CONTRIBUTING.md) first; this file only records the things that are easy to get wrong here.

## Agent requirements

We do not accept AI-generated prose in issues or pull requests. A human must explain what the problem is and why the proposed change solves it. Generated code is fine when a human can defend it; a generated description of that code is not. The same rule is stated in [CONTRIBUTING.md](CONTRIBUTING.md), which is where human contributors will find it.

Commit messages start with a gitmoji, then a summary line in the past tense, then a body explaining why the change was made and what it cost. Never replace something in a single commit: add the new thing, switch to it, then remove the old one, as three commits. Read the last few commits before writing one.

## What this is

lxdyn is the LOTUSim fork of xdyn, a naval hydrodynamics simulator of roughly 200k lines of C++. It is built by `zig cc` against libc++, from a single host, for four targets. There is no CMake, no autotools and no docker in any build lane. `Containerfile` is the deploy image and the only container recipe in the repository.

## Environment

Everything comes from Nix. `nix develop` provides zig, mise, uv, gdb, doxygen, llvm and pkg-config. It is a shell without a C or C++ compiler and without any C++ library, and that is deliberate: the compiler is zig and the libraries come from the dependency closure. Do not add `pkgs.boost`, `pkgs.grpc`, `pkgs.hdf5` or `pkgs.binutils` to it. Those are libstdc++ builds and mixing them with libc++ is an ABI error that surfaces as link failures far from the cause.

- `nix develop` builds and tests natively
- `nix develop .#cross` adds qemu and wine, to *run* the cross suites
- `nix develop .#deps` adds cmake, ninja and nasm, to *build* a dependency closure

## The dependency closure

The C++ dependencies are prebuilt into a closure per target triple. `build.zig` **locates** a closure, it never builds one. It is found via `-Ddeps=` or `$XDYN_DEPS_<TRIPLE>`, and defaults to `./libcxx-<triple>`.

```sh
mise run deps:fetch x86_64-linux-gnu     # download, hash-verified, about 35 MB
nix build .#libcxx-deps-x86_64-linux-gnu # or build one hermetically, about 20 minutes
```

Triples: `x86_64-linux-gnu`, `x86_64-linux-musl`, `aarch64-linux-musl`, `x86_64-windows-gnu`.

## Commands

```sh
mise run zig:build             # zig build --prefix build/<triple>
zig build test                 # native unit suite, 963 tests
mise run zig:debug             # -O0 -g, into build/<triple>-debug
zig build gen                  # the code generators alone
mise run cross                 # both cross suites under qemu and wine
nix flake check                # the three suites and the lint, hermetic: what CI runs
mise run integration           # CLI scenarios
mise run integration:grpc      # gRPC and JSON protocol scenarios
mise run deploy:image          # container image
mise run deploy:test           # deployment smoke test
mise run deploy:publish        # push it to ghcr.io, from a tagged and clean tree only
mise run python:test           # binding tests on 3.10
mise run python:matrix         # on 3.10, 3.13 and 3.15
mise run python:lint           # ruff check
mise run python:wheel          # wheel plus a clean-venv check
mise run fmu:test              # the FMU through FMPy
mise run fmu:wine              # its Windows build under wine, against Linux (`nix develop .#cross`)
```

## Traps

**`-Ddebug`, never `-Doptimize=Debug`.** The former is a debug build of lxdyn's own code with the closure left optimized. The latter is not the same thing and is not what the `.gdbinit` helpers or `mise run gdb` expect.

**`mise run setup` resets every submodule to its committed pin.** If you are testing a local checkout of a submodule, it will be discarded. Commit the pin bump first.

**Do not run `zig build test -Dtarget=...` directly to check a cross target.** On a host with `binfmt_misc` handlers registered, zig runs the foreign binary transparently and the suite passes without qemu or wine being involved, which makes the result a property of that machine. `mise run cross` names the emulators with `-fqemu` and `-fwine`.

**A bare `zig build` installs into `zig-out/`, which nothing reads.** The tasks, scripts and Makefiles look in `build/<triple>`, which is what `mise run zig:build` passes as `--prefix`.

**The integration lanes depend on `zig:build` on purpose.** `zig build test` compiles and runs the suite but installs nothing, so on a tree where only the unit tests have run, every scenario exits 127.

**`build.zig` globs its source lists.** New `.cpp` files under `xdyn/<module>/` and `xdyn/<module>/unit_tests/` register themselves, so there is no file list to update. Each component's build is a plain Zig file beside its sources — `xdyn/build.zig`, `external/build.zig`, `xdyn_wrapper_python/build.zig`, `xdyn_wrapper_fmi/build.zig` — imported by the root `build.zig`, which owns the shared `Ctx`.

## Submodules

`external/ssc` and `external/interfaces` are forks under the same organisation, pinned by commit. The policy is minimal divergence: change lxdyn to suit SSC rather than the other way round, unless there is a real reason. `external/stb` and `external/websocketpp` are upstream third parties.

## Style

Comments explain *why*, never *what*. The code is the truth, so prefer an explicit name over a comment restating the line below it. Do not put dates or changelog entries in comments; that is git's job.

Python is formatted and linted with ruff (`mise run python:lint`). C++ follows the committed `.clang-format`.
