# lxdyn

lxdyn is the LOTUSim fork of xdyn, a lightweight time-domain ship simulator. It models the dynamic behaviour of a ship at sea with its actuators, including some non-linear effects, by solving Newton's second law for a solid body under hydrodynamic forces.

A simulation is a YAML file: the bodies, their meshes and hydrodynamic databases, the force models acting on them, the wave, wind and current models, and the outputs. The [user documentation](https://sirehna_naval_group.gitlab.io/sirehna/xdyn/) describes the format; it is in French and incomplete in places.

## What it ships

The project, the PyPI package and the image are called lxdyn; everything they hold keeps the name xdyn.

| | What | Where |
|---|---|---|
| `xdyn` | the simulator: a YAML file in, CSV, TSV, JSON or HDF5 out | the container image, `ghcr.io/naval-group/lxdyn` |
| `xdyn-for-cs`, `xdyn-for-me` | xdyn as a co-simulation or model-exchange server, over gRPC or websocket | the container image |
| `import xdyn` | the Python bindings, CPython 3.11 to 3.15 on Linux x86_64 | the [`lxdyn`](https://pypi.org/project/lxdyn/) wheel on PyPI |
| `xdyn_fmu` | xdyn as an FMI 2.0 FMU, Linux and Windows, and the script that packs a YAML into a `.fmu` | the zip attached to each [release](https://github.com/naval-group/lxdyn/releases) |
| `xdyngrpc` | the Python package for writing force models, wave models and controllers that xdyn calls over gRPC | [`xdyn_grpc/`](xdyn_grpc/) |

## Running a simulation

With [podman](https://podman.io/):

```bash
alias lxdyn='podman run --rm --userns=keep-id -v "$PWD:/data" ghcr.io/naval-group/lxdyn'
lxdyn --help
lxdyn my_simulation.yml --dt 0.1 --tend 10
```

The mount is where the results get written, and `--userns=keep-id` makes you the owner of them. Use `:26.8` instead of `:latest` to pin a release. With docker, replace `--userns=keep-id` with `-u $(id -u):$(id -g)`. The [command-line options](https://sirehna_naval_group.gitlab.io/sirehna/xdyn/#ligne-de-commande) are documented upstream.

The tutorial inputs, with the meshes and HDB files they reference, are in [`test_data/tutorials/`](test_data/tutorials/):

```bash
git clone --depth 1 https://github.com/naval-group/lxdyn.git
cd lxdyn/test_data/tutorials
lxdyn tutorial_01_falling_ball.yml --dt 0.1 --tend 1 -o tsv
```

## What lxdyn changes from xdyn

- One toolchain: [Zig](https://ziglang.org/)'s `zig cc`, clang with libc++, compiles xdyn and every C++ dependency, and `build.zig` replaces CMake.
- Four targets from one Linux machine: `x86_64-linux-gnu` (glibc 2.28 and newer), `x86_64-linux-musl`, `aarch64-linux-musl` and `x86_64-windows-gnu`.
- The dependencies are built here: Boost, gRPC, HDF5, yaml-cpp and GoogleTest are rebuilt against libc++ from recipes in [`tools/deps/`](tools/deps/), and published prebuilt as release assets.
- SSC and the gRPC interfaces are part of the project: they were separate repositories upstream, and live in [`ssc/`](ssc/), [`proto/`](proto/) and [`xdyn_grpc/`](xdyn_grpc/).

## The repository

| Folder | What |
|---|---|
| [`xdyn/`](xdyn/) | the simulator, one folder per module; [`xdyn/README.md`](xdyn/README.md) says what each holds |
| [`ssc/`](ssc/) | units, interpolation, kinematics, solvers, websockets and exception handling |
| [`proto/`](proto/) | xdyn's gRPC API |
| [`xdyn_grpc/`](xdyn_grpc/) | the Python package for external models over gRPC |
| [`xdyn_wrapper_python/`](xdyn_wrapper_python/) | the Python bindings and the wheel |
| [`xdyn_wrapper_fmi/`](xdyn_wrapper_fmi/) | the FMU |
| [`external/`](external/) | vendored third-party sources: stb, websocketpp and others |
| [`test_data/`](test_data/) | every input the tests read, and the tutorials |
| [`integration_tests/`](integration_tests/) | the CLI, gRPC, JSON and deploy scenarios |
| [`validation/`](validation/) | a test ship and model-test data to compare xdyn against: turning circles, roll and pitch decay |
| [`postprocessing/`](postprocessing/) | MATLAB and Python scripts for xdyn's outputs |
| [`tools/`](tools/) | the dependency recipes, the lint and format scripts |

## Contributing

[CONTRIBUTING.md](CONTRIBUTING.md) explains how to propose a change and how to build and test lxdyn. See also [GOVERNANCE.md](GOVERNANCE.md), the [code of conduct](CODE_OF_CONDUCT.md) and the [contributors](https://github.com/naval-group/lxdyn/graphs/contributors).

lxdyn follows [CalVer](https://calver.org/), `YY.MM.MICRO`: `26.8` is the August 2026 release. See the [tags](https://github.com/naval-group/lxdyn/tags).

## History and license

xdyn was developed through self-funded and collaborative projects, including the IRT Jules Verne's ["Bassin Numérique" project](https://www.irt-jules-verne.fr/wp-content/uploads/bassin-numerique.pdf).

(c) 2014-2015, [IRT Jules Verne](https://www.irt-jules-verne.fr/), [SIREHNA](https://www.sirehna.com/), [Naval Group](https://www.naval-group.com/en/), [Bureau Veritas](https://www.bureauveritas.fr/), [Hydrocean](https://marine-offshore.bureauveritas.com/bvsolutions), [STX France](https://chantiers-atlantique.com/en/), [LHEEA](https://lheea.ec-nantes.fr/) for the initial version.

(c) 2015-2022 [SIREHNA](https://www.sirehna.com/) & [Naval Group](https://www.naval-group.com/en/) for all subsequent versions.

Eclipse Public License 2.0, see [LICENSE.md](LICENSE.md).
