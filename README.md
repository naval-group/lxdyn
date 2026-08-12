# lxdyn

lxdyn is the LOTUSim fork of xdyn, a lightweight time-domain ship simulator modelling the dynamic behaviour of a ship at sea, with its actuators, including some non-linear aspects of that behaviour and featuring a customizable maneuvring model.
It simulates the mechanical behaviour of a solid body in a fluid environment by solving Newton's second law of motion, taking hydrodynamic forces into account.

The name is the only thing the fork renames. The executables, the Python module and the C++ library are all still called `xdyn`, because that is what the input files, the tutorials and the upstream documentation refer to.

It was developed by SIREHNA through both self-funded projects and various collaborative projects, including the IRT Jules Verne's ["Bassin Numérique" project](https://www.irt-jules-verne.fr/wp-content/uploads/bassin-numerique.pdf).

(c) 2014-2015, [IRT Jules Verne](https://www.irt-jules-verne.fr/), [SIREHNA](https://www.sirehna.com/), [Naval Group](https://www.naval-group.com/en/), [Bureau Veritas](https://www.bureauveritas.fr/), [Hydrocean](https://marine-offshore.bureauveritas.com/bvsolutions), [STX France](https://chantiers-atlantique.com/en/), [LHEEA](https://lheea.ec-nantes.fr/) for the initial version.

(c) 2015-2022 [SIREHNA](https://www.sirehna.com/) & [Naval Group](https://www.naval-group.com/en/) for all subsequent versions.

**Disclaimer**: the [user documentation](https://sirehna_naval_group.gitlab.io/sirehna/xdyn/) is written in French, is hosted by the upstream project, and is not part of this repository. It is incomplete in places and inaccuracies may remain.

## Using lxdyn

You need [podman](https://podman.io/). Nothing else: no compiler, no toolchain, no Python.

```bash
podman run --rm ghcr.io/naval-group/lxdyn --help
```

That is `:latest`. Every release is also published under its own tag, so `ghcr.io/naval-group/lxdyn:26.8` pins one.

### Running a simulation

lxdyn reads a YAML file and writes its results next to it, so the container needs your current directory mounted. Define this once and the rest of the page is readable:

```bash
alias lxdyn='podman run --rm --userns=keep-id -v "$PWD:/data" ghcr.io/naval-group/lxdyn'
```

The three flags are not decoration. `-v "$PWD:/data"` mounts your working directory over the image's `/data`, without which the results are written inside the container and disappear with it. `--userns=keep-id` maps your user into the container, without which the output files come out owned by an unrelated high UID that you then cannot delete. `--rm` deletes the container, not the image, once it exits.

Now run a simulation:

```bash
lxdyn my_simulation.yml --dt 0.1 --tend 10
```

All available options are listed in [the documentation](https://sirehna_naval_group.gitlab.io/sirehna/xdyn/#ligne-de-commande).

### The tutorials

The tutorial input files are generated rather than stored, so that they cannot drift away from the code that reads them. The generators ship inside the image:

```bash
mkdir -p demos && cd demos
podman run --rm --userns=keep-id -v "$PWD:/data" \
    --entrypoint /usr/bin/generate_yaml_example ghcr.io/naval-group/lxdyn .
lxdyn tutorial_01_falling_ball.yml --dt 0.1 --tend 1 -o tsv
```

That writes `falling_ball.csv`, `.h5` and `.json` beside itself. `-o tsv` prints the results to the terminal instead.

The tutorials themselves are documented upstream:

- [Falling ball](https://gitlab.com/sirehna_naval_group/sirehna/xdyn/-/blob/master/doc/user_fr/tutorial_01.md)
- [Hydrostatic](https://gitlab.com/sirehna_naval_group/sirehna/xdyn/-/blob/master/doc/user_fr/tutorial_02.md)
- [Waves](https://gitlab.com/sirehna_naval_group/sirehna/xdyn/-/blob/master/doc/user_fr/tutorial_03.md)
- [Propulsion](https://gitlab.com/sirehna_naval_group/sirehna/xdyn/-/blob/master/doc/user_fr/tutorial_06.md)
- [gRPC wave model](https://gitlab.com/sirehna_naval_group/sirehna/xdyn/-/blob/master/doc/user_fr/tutorial_09.md)

The **environment models** are described in detail [here](https://gitlab.com/sirehna_naval_group/sirehna/xdyn/-/blob/master/doc/user_fr/modeles_environnementaux.md), and the **force models** [here](https://gitlab.com/sirehna_naval_group/sirehna/xdyn/-/blob/master/doc/user_fr/modeles_efforts.md).

### What the image is

`FROM scratch`, holding nothing but the binaries. They are built for `x86_64-linux-musl`, which links libc statically on top of the libc++ and third-party libraries that were already static, so there is no loader to find and nothing underneath them to provide.

The consequence worth knowing: there is no shell in the image, so `podman run --entrypoint sh` is not a way to look around, and there are no CA certificates.

### Using docker instead

docker works, with one difference: it has no `--userns=keep-id`, so use `-u $(id -u):$(id -g)` to get output files owned by you.

```bash
alias lxdyn='docker run --rm -u $(id -u):$(id -g) -v "$PWD:/data" ghcr.io/naval-group/lxdyn'
```

## Building from source

Everything below is for developing lxdyn. If you only want to run simulations, the section above is all you need.

### Prerequisites

[Nix](https://nixos.org/download/) with flakes enabled, and nothing else. The `flake.nix` at the repository root pins zig, mise, uv and every other tool the build needs, so no compiler and no C++ library has to be installed on the host.

Flakes are not on by default. Either add this to `~/.config/nix/nix.conf`:

```
experimental-features = nix-command flakes
```

or use [direnv](https://direnv.net/), which the committed `.envrc` sets up for you. `direnv allow` once per clone, and entering the directory loads the devShell, so the `nix develop` below becomes optional.

The C++ dependencies, Boost, gRPC, HDF5, yaml-cpp and GoogleTest, are deliberately *not* system packages: they are built against zig's libc++ into a closure of their own, so the build does not depend on what the host distribution ships. You can download a prebuilt closure in about 35 MB, or build one from source in a few hours.

```bash
nix develop
mise run bootstrap             # submodules, SSC umbrella headers, and the closure (~35 MB)
```

`bootstrap` is `mise run setup` plus `mise run deps:fetch x86_64-linux-gnu`, and it is re-runnable: it leaves an existing closure alone. To build a closure from source instead of downloading it, use `nix develop .#deps`, the shell that adds the cmake and ninja the recipes need and the emulators Boost's cross configure probes run, then `mise run deps:x86_64-linux-gnu`. It takes hours.

### Building

```bash
zig build
```

The binaries can then be found in `build/<target>/bin`, so `build/x86_64-linux-gnu/bin` on a typical Linux host. Codegen runs as part of the build; there is no configure step, and no separate install step either.

Run one straight from the build tree. The executable is `xdyn`:

```bash
$(sh tools/build-dir.sh)/bin/xdyn my_simulation.yml --dt 0.1 --tend 10
```

Cross-compiling needs nothing but the matching closure:

```bash
mise run deps:fetch aarch64-linux-musl
zig build -Dtarget=aarch64-linux-musl
```

The supported targets are `x86_64-linux-gnu`, `x86_64-linux-musl`, `aarch64-linux-musl` and `x86_64-windows-gnu`.

### Building the container image

`mise run deploy:image` does no compiling. It stages already-built, already-tested binaries into `build/scratch/deploy/` first, so the image ships exactly what the test suite ran:

```bash
mise run deploy:image
```

`mise run deploy:test` is the smoke test for it: the image starts, it simulates against a bind mount with readable output, and a containerised gRPC server with a published port is reachable from a native client.

### Tests and debugging

See [CONTRIBUTING.md](CONTRIBUTING.md), which covers the unit and integration suites, the cross suites under qemu and wine, valgrind and GDB.

## Built with

* [Zig](https://ziglang.org/) - `zig cc` is the compiler and `build.zig` the build system, for Linux, Windows and aarch64 alike from a single host.
* [LLVM](https://llvm.org/) - clang and libc++, which is what `zig cc` is; Visual Studio and libstdc++ are **not** supported.
* [mise](https://mise.jdx.dev/) - Task runner for everything around the build.
* [Nix](https://nixos.org/) - Pins the toolchain and the development shell.
* [Boost](https://www.boost.org/) - For command-line options, regular expressions, filesystem library.
* [yaml-cpp](https://github.com/jbeder/yaml-cpp) - To parse the input files.
* [HDF5](https://support.hdfgroup.org/products/hdf5_tools/index.html) - To store the outputs.
* [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page) - For matrix manipulations.
* [SSC](https://github.com/naval-group/scientific_computing) - For websockets, units decoding, interpolations, kinematics, CSV file reading and exception handling.

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for how to submit issues and pull requests, and for how to run the test suite and debug the code. Our code of conduct is the [Contributor Covenant](CODE_OF_CONDUCT.md) (original version available [here](https://www.contributor-covenant.org/version/1/4/code-of-conduct)).

## Versioning

Versions are calendar-based, `YY.M` with no leading zero on the month, so `26.8` is the August 2026 release; a second release within the same month appends a patch number, `26.8.1`. The version is read from the nearest git tag at build time: the Python module reports it as `xdyn.__version__`, and the container image carries it as the `org.opencontainers.image.version` label. 
The executables themselves print the commit they were built from, in `xdyn --help`. For the versions available, see the [tags on this repository](https://github.com/naval-group/lxdyn/tags).

## Authors

The main contributors to the original project are Charles-Edouard Cady, Guillaume Jacquenot, Léa Lincker and Moran Charlou. The full list is in the [contributor graph](https://github.com/naval-group/lxdyn/graphs/contributors).

## License

This project is licensed under the Eclipse Public License (version 2) - see the [LICENSE.md](LICENSE.md) file for details.
