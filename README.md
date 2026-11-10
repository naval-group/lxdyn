# lxdyn

lxdyn is the LOTUSim fork of xdyn, a lightweight time-domain ship simulator. It models the dynamic behaviour of a ship at sea with its actuators, including some non-linear effects, by solving Newton's second law for a solid body under hydrodynamic forces.

Only the project is renamed: the executables, the Python module and the C++ library are still called `xdyn`, as the input files and the upstream documentation expect.

It was developed by SIREHNA through self-funded and collaborative projects, including the IRT Jules Verne's ["Bassin Numérique" project](https://www.irt-jules-verne.fr/wp-content/uploads/bassin-numerique.pdf).

(c) 2014-2015, [IRT Jules Verne](https://www.irt-jules-verne.fr/), [SIREHNA](https://www.sirehna.com/), [Naval Group](https://www.naval-group.com/en/), [Bureau Veritas](https://www.bureauveritas.fr/), [Hydrocean](https://marine-offshore.bureauveritas.com/bvsolutions), [STX France](https://chantiers-atlantique.com/en/), [LHEEA](https://lheea.ec-nantes.fr/) for the initial version.

(c) 2015-2022 [SIREHNA](https://www.sirehna.com/) & [Naval Group](https://www.naval-group.com/en/) for all subsequent versions.

The [user documentation](https://sirehna_naval_group.gitlab.io/sirehna/xdyn/) is in French, hosted upstream, and incomplete in places.

## Usage

All you need is [podman](https://podman.io/):

```bash
alias lxdyn='podman run --rm --userns=keep-id -v "$PWD:/data" ghcr.io/naval-group/lxdyn'
lxdyn --help
lxdyn my_simulation.yml --dt 0.1 --tend 10
```

The mount is where the results get written, and `--userns=keep-id` makes you the owner of them. Use `:26.8` instead of `:latest` to pin a release. The [command-line options](https://sirehna_naval_group.gitlab.io/sirehna/xdyn/#ligne-de-commande) are documented upstream.

With docker, replace `--userns=keep-id` with `-u $(id -u):$(id -g)`.

### Tutorials

The image ships the generator for the tutorial inputs:

```bash
mkdir -p demos && cd demos
podman run --rm --userns=keep-id -v "$PWD:/data" \
    --entrypoint /usr/bin/generate_yaml_example ghcr.io/naval-group/lxdyn .
lxdyn tutorial_01_falling_ball.yml --dt 0.1 --tend 1 -o tsv
```

These are documented upstream: [falling ball](https://gitlab.com/sirehna_naval_group/sirehna/xdyn/-/blob/master/doc/user_fr/tutorial_01.md), [hydrostatic](https://gitlab.com/sirehna_naval_group/sirehna/xdyn/-/blob/master/doc/user_fr/tutorial_02.md), [waves](https://gitlab.com/sirehna_naval_group/sirehna/xdyn/-/blob/master/doc/user_fr/tutorial_03.md), [propulsion](https://gitlab.com/sirehna_naval_group/sirehna/xdyn/-/blob/master/doc/user_fr/tutorial_06.md) and [gRPC wave model](https://gitlab.com/sirehna_naval_group/sirehna/xdyn/-/blob/master/doc/user_fr/tutorial_09.md). So are the [environment models](https://gitlab.com/sirehna_naval_group/sirehna/xdyn/-/blob/master/doc/user_fr/modeles_environnementaux.md) and the [force models](https://gitlab.com/sirehna_naval_group/sirehna/xdyn/-/blob/master/doc/user_fr/modeles_efforts.md).

## Development

You need [Nix](https://nixos.org/download/) with flakes enabled (`experimental-features = nix-command flakes` in `~/.config/nix/nix.conf`). The devShell provides zig, mise, uv and the rest of the tools, and no host compiler is used. With [direnv](https://direnv.net/), `direnv allow` replaces `nix develop`. Every `mise run` stops outside the devShell, so no host tool stands in for a pinned one.

```bash
git clone https://github.com/naval-group/lxdyn.git && cd lxdyn
nix develop
mise run bootstrap             # protoc + the prebuilt C++ dependencies, ~35 MB
mise run zig:build             # binaries in build/x86_64-linux-gnu/bin
```

The C++ dependencies (Boost, gRPC, HDF5, yaml-cpp, GoogleTest) are prebuilt against zig's libc++ rather than taken from the host. `nix develop .#deps` builds them from source instead, which takes a few hours.

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
mise run cross                 # cross targets under qemu and wine, in `nix develop .#cross`
nix flake check                # everything CI runs
```

### Cross-compiling

lxdyn cross-compiles to Linux (x86_64, aarch64) and Windows (x86_64) from a single host:

```bash
mise run deps:fetch aarch64-linux-musl
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

## Built with

* [Zig](https://ziglang.org/) — `zig cc` (clang + libc++) as the compiler, `build.zig` as the build system
* [Boost](https://www.boost.org/), [yaml-cpp](https://github.com/jbeder/yaml-cpp), [HDF5](https://www.hdfgroup.org/solutions/hdf5/), [Eigen](https://eigen.tuxfamily.org/), [gRPC](https://grpc.io/)
* SSC, in `ssc/` — units, interpolation, kinematics, websockets and exception handling

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md), [GOVERNANCE.md](GOVERNANCE.md) and the [code of conduct](CODE_OF_CONDUCT.md).

## Versioning

lxdyn follows [CalVer](https://calver.org/), `YY.MM.MICRO`: `26.8` is the August 2026 release. See the [tags](https://github.com/naval-group/lxdyn/tags).

## Authors

The main contributors to the original project are Charles-Edouard Cady, Guillaume Jacquenot, Léa Lincker and Moran Charlou. See the [contributor graph](https://github.com/naval-group/lxdyn/graphs/contributors) for everyone else.

## License

Eclipse Public License 2.0, see [LICENSE.md](LICENSE.md).
