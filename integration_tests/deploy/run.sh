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
#   3. the image's own gRPC server, on a published port, answers a host client with the
#      values the simulation determines
#
# It is NOT a protocol test — integration_tests/grpc_tests covers the protocol, natively, in
# 25 s with no daemon. Do not grow this file into a second copy of that suite; the whole
# point of A4b was that a test needing a container daemon is a test nobody runs.
set -eu

SRC=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO=$(CDPATH= cd -- "$SRC/../.." && pwd)
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
NAME=lxdyn-deploy-smoke-$$

# `kill` first: xdyn ignores SIGTERM, so `rm -f` alone waits out the 10 s stop timeout.
_cleanup() {
    status=$?
    "$CONTAINER" kill "$NAME" > /dev/null 2>&1 || true
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

# --- 3. the image serves gRPC on a published port -----------------------------------------
echo "--- 3/3  image serves gRPC on a published port"
"$CONTAINER" run -d --name "$NAME" $USERNS -p 127.0.0.1:9002:9002 -v "$WORK:/data" \
    --entrypoint /usr/bin/xdyn-for-me "$IMAGE" tutorial_01_falling_ball.yml --grpc -p 9002 > /dev/null

# A gRPC channel, not a TCP connect: podman's forwarder accepts before the container binds.
if ! "$REPO/build/venv/grpc/bin/python" - <<'PY'
import sys, time
import grpc
deadline = time.monotonic() + 60
channel = grpc.insecure_channel("localhost:9002")
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

# A proxy that intercepts localhost breaks exactly this hop.
no_proxy=${no_proxy:-localhost,127.0.0.1}; export no_proxy

# The falling ball at rest: every derivative is 0 except dw/dt, which is g.
PYTHONPATH="$REPO/xdyn_grpc" "$REPO/build/venv/grpc/bin/python" - <<'PY' || { echo "model-exchange call failed. Server log:"; "$CONTAINER" logs "$NAME" 2>&1 | tail -20; exit 1; }
import math, sys
from xdyngrpc.modelexchange import ModelExchangeEuler

state = dict(t=0, x=0, y=0, z=0, u=0, v=0, w=0, p=0, q=0, r=0, phi=0, theta=0, psi=0)
d_dt = ModelExchangeEuler("localhost:9002").dx_dt(state, [])
expected = dict.fromkeys(("x", "y", "z", "u", "v", "p", "q", "r", "qr", "qi", "qj", "qk"), 0.0)
expected["w"] = 9.81
wrong = {k: d_dt.get(k) for k, v in expected.items() if not math.isclose(d_dt.get(k, math.nan), v, abs_tol=1e-9)}
if wrong:
    print("unexpected derivatives: %s" % wrong)
    sys.exit(1)
print("    dx/dt round trip returned dw/dt = %g, every other derivative 0" % d_dt["w"])
PY

echo "=== deploy smoke test PASSED ==="
