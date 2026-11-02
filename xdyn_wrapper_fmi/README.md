# xdyn_wrapper_fmi

xdyn as an FMI 2.0 FMU, for Co-Simulation and, where the model allows it, Model Exchange; one
library and one `.fmu` for both.

- Co-Simulation: each instance owns a `Sim`, and `fmi2DoStep(t, h)` integrates it from `t` to
  `t + h` with xdyn's solver and step, as the `xdyn` executable does over a whole run: the
  history force models read (radiation damping) goes on across steps.
- Model Exchange: the importer integrates the 13 states of the body, and `fmi2GetDerivatives` is
  `Sim::dx_dt`. The importer evaluates the derivatives at trial states and times, which no
  history can follow, so `modelDescription.xml` only declares Model Exchange for a model with
  no force model reading past states (`get_Tmax() > 0`) and no blocked DOFs (xdyn imposes them
  after each step of its own solver). The quaternion is normalized before xdyn reads it, the
  importer's states left as they are.

An FMU simulates one vessel: a YAML with several bodies is refused, the master instantiates the
FMU once per vessel. So is a YAML with controllers: the commands are FMU inputs, the master
computes them.

```sh
mise run fmu:build      # build/<triple>/fmu/xdyn_fmu.so
mise run fmu:test       # packs unit_tests/propelled_ship.yml, runs it through FMPy

python xdyn_wrapper_fmi/make_fmu.py ship.yml -o ship.fmu \
    --library build/x86_64-linux-gnu/fmu/xdyn_fmu.so \
    --assets-root models --asset ship/hull.stl --dt 0.1 --solver rk4
```

`--asset` files land in the FMU's `resources/` under the path the YAML gives them, and
`resources/` is the asset root the model is built against. `--binary win64=<dll>` adds the
Windows library (`zig build fmu -Dtarget=x86_64-windows-gnu`). xdyn resolves meshes against the
asset root but reads HDB files from the working directory: give those an absolute path.

## Variables

Names are structured (`variableNamingConvention="structured"`) and start with the body's
name, `TestShip` below; each variable's description gives xdyn's own name for it.

| Value refs | Name | Causality |
|---|---|---|
| 1-13 | `TestShip.x` … `TestShip.qk` | output: NED position, FRD velocities, quaternion |
| 14-16 | `TestShip.phi`, `.theta`, `.psi` | output: Euler angles in the YAML's rotations convention |
| 100 | `xdyn.dt` | parameter: xdyn's step inside a communication step |
| 101 | `xdyn.solver` | parameter (Enumeration `xdyn.Solver`): `euler`, `rk4`, `rkck` |
| 200 | `TestShip.reset_state` | input (Boolean): going true, the next step starts from `reset_*` |
| 201-213 | `TestShip.reset_x` … `TestShip.reset_qk` | input: the state to impose |
| 1000+ | the model's commands, e.g. `TestShip.propeller.rpm` | input |
| 300-312 | `der(TestShip.x)` … `der(TestShip.qk)` | local: the state derivatives, with Model Exchange |

A command is named body, force model and command, each part with what is not `[A-Za-z0-9_]`
turned into `_` (and `_` put before a leading digit): `propeller(P/D)` is
`TestShip.propeller.P_D`. `make_fmu.py` fails if two variables end up with the same name.

Everything is in SI units, with display units where they help (`deg`, `deg/s`, `rpm`):
`TestShip.propeller.rpm` is in rad/s. Command units come from the command's name (`rpm`,
`beta`, `P/D`, `psi_co`, `x_co`, `y_co`), not from the YAML; other commands have none.

`reset_state` is there because FMI 2.0 Co-Simulation has no other way to set a state: a master
that teleports a vessel or switches it to another model needs one. It acts when it goes from
false to true, as a master sets every input before each step.
In Model Exchange the importer sets the states itself, and `xdyn.dt`, `xdyn.solver` and
`reset_*` play no part.

## Loading

Only the `fmi2*` functions and the two `xdyn_fmu_*` helpers of `make_fmu.py` are exported
(`src/exports.map`), so the closure's Boost, yaml-cpp and gRPC do not clash with other copies in
the importing process. On Linux the library keeps itself loaded once opened: a dependency
registers an exit handler at load time, which would crash the process at exit after an
importer unloads it.
