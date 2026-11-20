# Contributing to lxdyn

This document explains how to contribute to the lxdyn project.

## Contribution Philosophy
lxdyn aims to foster research, experimentation, and interoperability in the field of multi-agent simulation.

A LOTUSim Governance Committee, established within Naval Group, oversees the overall coherence of the project’s developments.
In the future, the committee may include external personas (key partners or major contributors).

The committee publishes a roadmap highlighting priority topics and features, based on:
- Internal needs within Naval Group,
- Partner priorities,
- And community-driven contributions.

This roadmap is updated every six months and available in the Project Roadmap section.

New ideas that enrich lxdyn are always welcome, even if they are not listed on the roadmap.

- For roadmap-related questions, please contact the [Product Owner](mailto:lotusim_support@naval-group.com).

The LOTUSim Governance Committee ensures:

- Technical consistency of the project core,
- Validation of contributions,
- Release publication in alignment with the roadmap.

Before proposing a contribution, please first open an issue (see the Technical Specification section).

## Types of Contributions
You can contribute in several ways:

- New features – e.g. new algorithmic modules for platform guidance.
- New models – for platforms, sensors, or environments.
- Enhancements – improving existing functions or models.
- Data contributions – use cases, simulation scenarios, or datasets.
- Bug fixes – code corrections, refactoring, or readability/maintainability improvements.

Please follow this process to propose changes:

1. Create an Issue

    - If none exists, open an issue on GitHub.
    - Use one of the following labels: new_feature, new_model, enhancement, data, or bugfix.
    - Clearly describe your proposal, including objectives, use cases, scientific background, and references if relevant.

2. Announce Your Work

    - In the issue comments, indicate that you plan to work on it.
    - Briefly describe your proposed solution or approach.

3. Fork and Clone the Repository

    - Fork lxdyn and clone it locally.
    - Implement your changes; [Development](#development) explains how to build and test them.

4. Push to a Dedicated Branch

    - Push your changes to your fork.
    - Use a dedicated branch (e.g., feature/my-feature) to facilitate feedback and updates.

5. Submit a Pull Request

    - Open a pull request.
    - Clearly reference the related issue and describe the implemented changes.

## Review and Integration

All pull requests are reviewed by maintainers and, if relevant, by the lxdyn Governance Committee.
Feedback may include requests for clarification, documentation, or code improvements.

Once approved, your contribution will be merged into the main branch and included in the next release according to the project roadmap.

## Development

You need [Nix](https://nixos.org/download/) with flakes enabled (`experimental-features = nix-command flakes` in `~/.config/nix/nix.conf`). The devShell provides zig, mise, uv and the rest of the tools, and the prebuilt C++ dependencies; no host compiler is used. With [direnv](https://direnv.net/), `direnv allow` replaces `nix develop`. Every `mise run` stops outside the devShell, so no host tool stands in for a pinned one.

```bash
git clone https://github.com/naval-group/lxdyn.git && cd lxdyn
nix develop                    # the tools, protoc and the prebuilt C++ dependencies, ~35 MB once
mise run zig:build             # binaries in build/x86_64-linux-gnu/bin
```

`mise run zig:build` also writes `compile_commands.json` at the root, which clangd reads; the devShell's clangd matches zig's clang.

The C++ dependencies (Boost, gRPC, HDF5, yaml-cpp, GoogleTest) are prebuilt against zig's libc++ rather than taken from the host.

Run a tutorial from the build tree:

```bash
mise run demos && cd demos
mise run xdyn -- tutorial_01_falling_ball.yml --dt 0.1 --tend 10 -o out.csv
```

### Tests

```bash
zig build test                 # unit tests
mise run integration           # CLI scenarios
mise run integration:grpc      # gRPC and JSON protocol scenarios
mise run python:test           # Python bindings
mise run tidy                  # clang-tidy, with the checks .clang-tidy lists
mise run cross                 # cross targets under qemu and wine, in `nix develop .#cross`
nix flake check                # everything CI runs
```

### Cross-compiling

lxdyn cross-compiles to Linux (x86_64, aarch64) and Windows (x86_64) from a single host:

```bash
nix develop .#cross            # + the cross closures, qemu and wine
zig build -Dtarget=aarch64-linux-musl --prefix build/aarch64-linux-musl
```

### Debugging

```bash
mise run zig:debug             # -O0 -g for lxdyn's code, dependencies stay optimized
mise run gdb -- run_all_tests --gtest_filter='Gravity*'
valgrind $(sh tools/build-dir.sh)-debug/bin/run_all_tests --gtest_filter='Gravity*'
```

### Container image

`mise run deploy:image` packages already-built and tested binaries without recompiling them. `mise run deploy:test` smoke-tests the result.
