# xdyn_wrapper_fmi

xdyn as an FMI 2.0 Co-Simulation FMU. `fmi2DoStep(t, h)` calls `XdynForCS::handle` as
`xdyn-for-cs` does for one request: same state, commands and `Dt`, same result.

```sh
mise run fmu:build      # build/<triple>/fmu/xdyn_fmu.so
mise run fmu:test       # packs unit_tests/propelled_ship.yml, runs it through FMPy

python xdyn_wrapper_fmi/make_fmu.py ship.yml -o ship.fmu \
    --library build/x86_64-linux-gnu/fmu/xdyn_fmu.so \
    --assets-root models --asset ship/hull.stl --dt 0.1 --solver rk4
```

`--asset` files land in the FMU's `resources/` under the path the YAML gives them, and
`resources/` is the asset root the model is built against. `--binary win64=<dll>` adds the
Windows library (`zig build fmu -Dtarget=x86_64-windows-gnu`).

## Variables

| Value refs | Name | Causality |
|---|---|---|
| 1-13 | `x y z u v w p q r qr qi qj qk` | output: NED position, FRD velocities, quaternion of the first body |
| 14-16 | `phi theta psi` | output, rad |
| 100 | `xdyn_dt` | parameter: xdyn's step inside a communication step |
| 101 | `solver` | parameter (String): `euler`, `rk4`, `rkck` |
| 200 | `reset_state` | input (Boolean): the next step starts from `reset_*` |
| 201-213 | `reset_x … reset_qk` | input: the state to impose |
| 1000+ | the model's commands, e.g. `propeller(rpm)` | input, SI units |

`reset_state` is there because FMI 2.0 Co-Simulation has no other way to set a state: a master
that teleports a vessel or switches it to another model needs one.

## Loading

Only the `fmi2*` functions and the two `xdyn_fmu_*` helpers of `make_fmu.py` are exported
(`src/exports.map`), so the closure's Boost, yaml-cpp and gRPC do not clash with other copies in
the importing process. On Linux the library keeps itself loaded once opened: a dependency
registers an exit handler at load time, which would crash the process at exit after an
importer unloads it.
