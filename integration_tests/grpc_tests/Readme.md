These are integration tests for xdyn's gRPC interfaces: they show how users can define
their own wave, force and controller models and have xdyn call them over gRPC.

Each scenario starts at least two processes — one running xdyn, one running the
user-implemented model — and a test client that checks what came back. Run them with:

    mise run integration:grpc          # all of them
    make -C integration_tests/grpc_tests waves        # just one
    sh integration_tests/grpc_tests/waves/run.sh      # same thing, directly

| Scenario           | What it exercises                                              |
|--------------------|----------------------------------------------------------------|
| `waves`            | a Python wave model called by xdyn (tutorial 9)                 |
| `waves_server`     | xdyn's own C++ Airy server, called by the Python client         |
| `force`            | Python force models: simulation, cosimulation, HDB, PRECAL-R, filtered states (tutorials 10, 13, 14) |
| `wave+force`       | a force model that requests wave data, so it round-trips through xdyn |
| `cosim`            | xdyn as a gRPC cosimulation server                              |
| `model_exchange`   | xdyn as a gRPC model-exchange server                            |
| `controller`       | a Python PID controller steering a ship (tutorial 11)           |

## These used to be docker-compose stacks

Every scenario was a compose file with one container per process. Compose supplied four
things: hostname discovery, volume mounts, uid mapping, and startup ordering. With static
binaries and an importable `xdyngrpc`, only the last one still applies — and that is a
port poll, not an orchestrator. So each scenario is a shell script that starts the same
processes on localhost. See `../tools/scenario.sh`.

Two consequences worth knowing:

- **Ports are hardcoded in `xdyngrpc`** (50051 for waves, 9002 for forces and
  controllers). Where a Python model and xdyn's own server both wanted 9002, it is xdyn
  that moves to 9003 — see `wave+force/run.sh` and `force/run.sh`.
- **The demo inputs come from `test_data/tutorials/`**: every tutorial YAML, mesh, HDB
  and PRECAL-R file the scenarios need. The YAMLs name their peers by compose service
  name, so the harness rewrites those to localhost.

Testing the *deployment* — the wave/force server as a real container, reached over a real
network hop — is a separate concern, and is not covered here.
