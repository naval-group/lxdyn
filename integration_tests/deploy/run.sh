#!/bin/sh
# Deployment smoke test — the coverage A4b owes back.
#
# Deleting docker-compose removed the only test in the repo that crossed a real network
# boundary, and every compose file carried `no_proxy` — scar tissue from a proxy that once
# broke this. The 9 native gRPC scenarios replaced it with loopback in one process tree,
# which is faster and reproducible but proves nothing about *packaging*.
#
# So this proves exactly three things, and deliberately no more:
#   1. the deploy image starts
#   2. it runs a real simulation on a bind-mounted workdir, and the output is readable by the
#      invoking user -- that second half is the uid-mapping assertion, see USERNS below
#   3. a containerised gRPC server with a published port is reachable from outside it, with
#      the server's own logs used to confirm the traffic rather than trusting the client
#
# It is NOT a protocol test — integration_tests/grpc_tests covers the protocol, natively, in
# 25 s with no daemon. Do not grow this file into a second copy of that suite; the whole
# point of A4b was that a test needing a container daemon is a test nobody runs.
set -eu

SRC=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO=$(CDPATH= cd -- "$SRC/../.." && pwd)
BIN=${BIN:-$(sh "$REPO/tools/build-dir.sh")/bin}
WORK=$REPO/build/scratch/deploy-test

# podman first: it reads ./Containerfile without -f, and it is what this was developed
# against. docker works too; both are invoked with the same arguments below.
CONTAINER=${CONTAINER:-}
if [ -z "$CONTAINER" ]; then
    for c in podman docker; do
        if command -v "$c" > /dev/null 2>&1; then CONTAINER=$c; break; fi
    done
fi
[ -n "$CONTAINER" ] || { echo "deploy: no podman or docker on PATH" >&2; exit 1; }

# Rootless podman already maps the invoking UID to root inside the container, so the usual
# `-u $(id -u):$(id -g)` lands on a *subuid* and bind-mount writes come out owned by a stray
# high UID. --userns=keep-id is the correct spelling, and it is podman-only.
USERNS=
[ "${CONTAINER##*/}" = podman ] && USERNS=--userns=keep-id

IMAGE=${IMAGE:-localhost/lxdyn}
SERVER_IMAGE=${SERVER_IMAGE:-localhost/xdyngrpc-python}
NAME=lxdyn-deploy-smoke-$$

xdyn_pid=
# SIGTERM then SIGKILL, exactly like tools/scenario.sh's _kill, and for a reason learned the
# hard way here: xdyn's gRPC servers do not act on SIGTERM. A plain `kill` leaves the process
# alive, holding 9002, and the *next* run's client then talks to the survivor instead of the
# server it just started — which surfaced as an intermittent UNIMPLEMENTED (a stale
# xdyn-for-cs answering a model-exchange call) and cost an hour of blaming the wave model.
_cleanup() {
    status=$?
    if [ -n "$xdyn_pid" ]; then
        kill "$xdyn_pid" 2> /dev/null || true
        n=0
        while kill -0 "$xdyn_pid" 2> /dev/null && [ "$n" -lt 10 ]; do n=$((n + 1)); sleep 0.2; done
        kill -9 "$xdyn_pid" 2> /dev/null || true
        wait "$xdyn_pid" 2> /dev/null || true
    fi
    "$CONTAINER" rm -f "$NAME" > /dev/null 2>&1 || true
    [ $status -eq 0 ] || echo "--- deploy smoke test FAILED (status $status)"
    return $status
}
trap _cleanup EXIT INT TERM

rm -rf "$WORK"
mkdir -p "$WORK"
cd "$WORK"

echo "=== deploy smoke test ($CONTAINER) ==="

# Refuse to start if 9002 is already taken. xdyn binds with SO_REUSEPORT, so a leftover
# server does not make the new one fail — both listen, and the kernel hands each connection
# to one of them. The test then passes or fails depending on which one answers.
if "$REPO/build/venv/grpc/bin/python" -c \
    'import socket,sys; s=socket.socket(); s.settimeout(0.5); sys.exit(0 if s.connect_ex(("127.0.0.1",9002))==0 else 1)' 2>/dev/null
then
    echo "port 9002 is already in use -- a previous run probably leaked a server:" >&2
    pgrep -af "xdyn-for" >&2 || true
    exit 1
fi

# --- 1. the image runs -----------------------------------------------------------------
echo "--- 1/3  image starts and reports its version"
"$CONTAINER" run --rm "$IMAGE" --help > help.txt 2>&1 \
    || { echo "xdyn --help failed inside the image:"; cat help.txt; exit 1; }
grep -q "USAGE" help.txt || { echo "unexpected --help output:"; cat help.txt; exit 1; }

# --- 2. the image simulates, writing to a bind mount ------------------------------------
# The generators are in the image too, so the inputs come from the same artifact under test
# rather than from the host build. This is also what exercises the uid mapping: if --userns
# is wrong, out.csv appears owned by a subuid and the read below fails.
echo "--- 2/3  image runs a tutorial against a bind-mounted workdir"
for gen in generate_yaml_example generate_stl_examples; do
    "$CONTAINER" run --rm $USERNS -v "$WORK:/data" --entrypoint "/usr/bin/$gen" \
        "$IMAGE" . > /dev/null
done
"$CONTAINER" run --rm $USERNS -v "$WORK:/data" "$IMAGE" \
    tutorial_01_falling_ball.yml --dt 0.1 --tend 1 -o out.csv > /dev/null
[ -s out.csv ] || { echo "no out.csv produced"; ls -la; exit 1; }
[ -r out.csv ] || { echo "out.csv is not readable by $(id -un) -- uid mapping is wrong"; ls -la; exit 1; }
lines=$(( $(wc -l < out.csv) ))
[ "$lines" -gt 1 ] || { echo "out.csv has $lines line(s)"; exit 1; }
echo "    out.csv: $lines lines, owned by $(ls -l out.csv | awk '{print $3}')"

# --- 3. the published port is reachable from outside the container ----------------------
# This is the bit the native scenarios cannot cover: the server is in its real deployment
# image, its port is published, and the client is a *host* process reaching it through the
# container runtime's forwarding. no_proxy is passed through for the same reason every
# compose file did it -- a proxy that intercepts localhost breaks exactly this hop.
echo "--- 3/3  containerised gRPC server, native xdyn client"
"$CONTAINER" run -d --name "$NAME" -p 127.0.0.1:50051:50051 \
    "$SERVER_IMAGE" -m xdyngrpc.waves.server.airy > /dev/null

# Readiness is a *gRPC channel*, not a TCP connect — and that difference is the whole reason
# this test exists. `wait_port` in tools/scenario.sh polls connect() and is correct for a
# native process, but against a rootless published port it is a false positive: podman's
# forwarder (pasta/slirp4netns) accepts the connection on the host side immediately, before
# anything inside the container has bound. The probe returned "reachable after 0 ms" and
# xdyn then failed with UNAVAILABLE against a server that needed ~6 s to come up.
if ! "$REPO/build/venv/grpc/bin/python" - <<'PY'
import sys, time
import grpc
deadline = time.monotonic() + 60
channel = grpc.insecure_channel("localhost:50051")
while time.monotonic() < deadline:
    try:
        grpc.channel_ready_future(channel).result(timeout=2)
        print("    gRPC channel ready after %.1f s" % (60 - (deadline - time.monotonic())))
        sys.exit(0)
    except grpc.FutureTimeoutError:
        pass
sys.exit(1)
PY
then
    echo "gRPC never became ready on the published port. Server log:"
    "$CONTAINER" logs "$NAME" 2>&1 | tail -20
    exit 1
fi

# The wave model in tutorial_09 points at waves-server:50051 in the tracked fixture; the
# native scenarios rewrite it to localhost the same way.
sed -e 's/waves-server:50051/localhost:50051/g' tutorial_09_gRPC_wave_model.yml > localized.yml
mv localized.yml tutorial_09_gRPC_wave_model.yml

# no_proxy for the same reason every deleted compose file set it: a proxy that intercepts
# localhost breaks exactly this hop, and that is scar tissue from a real incident.
no_proxy=${no_proxy:-localhost,127.0.0.1}; export no_proxy

# xdyn-for-**me** (model exchange: dx/dt at one state), not xdyn-for-cs and not plain `xdyn`.
# The reason is worth writing down, because the obvious choices are both wrong here:
# tutorial_09 is a wave-forced floating cube that is violently unstable. Plain `xdyn` NaNs out
# in state U by t=0.2 (rk4 at dt=0.05 no better than Euler at dt=1), and a *single*
# cosimulation Euler step returns z ~ -3e27 from the demo's own initial position. Whether
# that step comes back with a huge number or throws on NaN varies run to run -- a cosim-based
# check passed three times and then failed. Model exchange evaluates derivatives and never
# integrates, so it is finite and deterministic, while still making xdyn's C++ gRPC client
# call out to the containerised wave server. That call is the only thing this step asserts.
#
# The instability is real and unexamined; the native `waves` scenario misses it because its
# tests.py asserts array *lengths* only. A model question, not a packaging one.
PYTHONPATH="$REPO/xdyn_grpc"; export PYTHONPATH
"$BIN/xdyn-for-me" tutorial_09_gRPC_wave_model.yml --grpc -p 9002 > xdyn.log 2>&1 &
xdyn_pid=$!

# connect() IS a valid probe here — xdyn-for-me is a native process that binds 9002 itself.
# That is precisely the case the containerised port above is not.
i=0
until "$REPO/build/venv/grpc/bin/python" -c \
    'import socket,sys; s=socket.socket(); s.settimeout(0.5); sys.exit(0 if s.connect_ex(("127.0.0.1",9002))==0 else 1)' 2>/dev/null
do
    i=$((i + 1))
    [ "$i" -lt 40 ] || { echo "xdyn-for-me never bound 9002:"; cat xdyn.log; exit 1; }
    sleep 0.25
done

# Reaching this line is already the assertion: xdyn only binds 9002 *after* it has sent
# set_parameters to the wave model, so a gRPC round trip has crossed the container boundary
# with a real protobuf payload. When the server is not reachable, xdyn exits here instead —
# "an error occurred when using the distant wave model defined via gRPC (method
# 'set_parameters')" — which is exactly the failure this test is here to catch.
echo "    xdyn-for-me started: set_parameters reached the containerised server"

# Confirmed from the other side, so the step cannot pass on a cached or defaulted wave field:
# the server logs the spectrum it was handed.
"$CONTAINER" logs "$NAME" 2>&1 | grep -q "Hs: 1.5" \
    || { echo "the containerised wave server never received the spectrum";
         "$CONTAINER" logs "$NAME" 2>&1 | tail -20; exit 1; }
echo "    server received the spectrum (Hs 1.5, Tp 10)"

# A second round trip, this time driven by a client. Shape only, never values: the Airy
# server randomises wave phases per run, so the derivatives legitimately differ every time --
# dw/dt was measured at +1744 and -910 m/s^2 on consecutive runs, and sometimes comes back
# non-finite. Asserting on those numbers is what made an earlier version of this test flaky
# (2 passes in 5). The physics is covered natively in integration_tests/grpc_tests; what is
# being asserted here is that a request crosses the boundary and a well-formed reply returns.
"$REPO/build/venv/grpc/bin/python" - <<'PY' || { echo "model-exchange call failed:"; cat xdyn.log; "$CONTAINER" logs "$NAME" 2>&1 | tail -20; exit 1; }
import sys
from xdyngrpc.modelexchange import ModelExchangeEuler

state = dict(t=0, x=0, y=0, z=0.25, u=0, v=0, w=0, p=0, q=0, r=0, phi=0, theta=0, psi=0)
d_dt = ModelExchangeEuler("localhost:9002").dx_dt(state, [])
missing = [k for k in ("x", "y", "z", "u", "v", "w", "p", "q", "r") if k not in d_dt]
if missing:
    print("reply is missing derivatives: %s" % missing)
    sys.exit(1)
print("    dx/dt round trip returned a complete state vector")
PY

echo "=== deploy smoke test PASSED ==="
