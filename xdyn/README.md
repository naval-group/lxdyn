# xdyn

One folder per module, each named after what it holds. A module's tests are in its `unit_tests/`.

| Module | What it holds |
|---|---|
| `core/` | the engine: `Sim`, `Body`, `SimulatorBuilder`, `EnvironmentAndFrames`, the model interfaces, `History`. `mise run lint` holds it to the model interfaces |
| `simulator_api/` | YAML and meshes to a `Sim`: the one module that depends on every model, and `model_tables.cpp`, which adds the gRPC models to the folder tables |
| `force_models/` | one class per force model, and `force_model_table.cpp`, the force models a YAML file can name but `grpc` |
| `wind_models/` | `WindModel`, its implementations, and `wind_model_table.cpp` |
| `current_models/` | `UWCurrentModel`, its implementations, `Seabed`, and `current_model_table.cpp` |
| `wave_models/` | the free surfaces (`waves`, `no waves`), wave models, spectra, spreadings, discretization, and `wave_model_table.cpp` |
| `observers/` | the CSV, TSV, JSON, map, HDF5 and websocket outputs, and `ListOfObservers` |
| `servers/` | `xdyn-for-cs` and `xdyn-for-me`: the simulation servers and their gRPC and websocket services |
| `cli/` | command-line parsing and error reporting for the executables |
| `executables/` | the `main` of each executable and of `run_all_tests` |
| `input/` | the parsed input types, `YamlSimulatorInput` and its parts |
| `yaml_parser/` | YAML to `input/`; the one module whose headers name yaml-cpp |
| `hydro_db/` | HDB and Precal files: grammar, parsers, radiation damping, RAO interpolation |
| `mesh/` | meshes, `GeometricTypes3d`, and the STL reader and writers |
| `controllers/` | the PID, CSV and gRPC controllers, and the listeners for `commands:` and `setpoints:` |
| `grpc/` | the client side of gRPC force, wave and controller models, and the protobuf conversions |
| `gz_curves/` | GZ stability curves |
| `hdf5/` | the HDF5 C helpers |
| `exceptions/` | the exception types |
| `version/` | the version and commit stamped into the binaries |
