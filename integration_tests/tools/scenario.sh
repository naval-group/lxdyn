# Shared harness for the native gRPC & JSON integration scenarios.
#
# These used to be docker-compose stacks: one container per process. Compose supplied
# exactly four things -- hostname discovery, volume mounts, uid mapping, and startup
# ordering. With static binaries and an importable xdyngrpc, only the last one still
# matters, and that is a port poll, not an orchestrator. So each scenario is now a shell
# script that starts the same processes on localhost.
#
# Usage, from <scenario>/run.sh:
#
#     #!/bin/sh
#     set -eu
#     . "$(dirname "$0")/../../tools/scenario.sh"
#     scenario waves               # sets $WORK, cds into it, arms the cleanup trap
#     demos                        # generate the tutorial inputs into $WORK
#     serve waves-server "$PYTHON" -m xdyngrpc.waves.server.airy
#     wait_port 50051
#     serve xdyn "$BIN/xdyn-for-cs" tutorial_09_gRPC_wave_model.yml -g --dt 1 -p 9002
#     wait_port 9002
#     pytest tests.py
#
# Every background process is logged to $WORK/<label>.log and killed on exit. If the
# scenario fails, those logs are dumped -- that is the one thing `docker-compose up`
# gave for free that a bare `&` does not.

SRC=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

# Walk up to the repo root rather than counting "..", because the scenarios are not all
# at the same depth (model_exchange/test-1 is one deeper than waves).
_find_repo() {
    d=$1
    while [ "$d" != "/" ]; do
        if [ -f "$d/build.zig" ]; then
            echo "$d"
            return 0
        fi
        d=$(dirname "$d")
    done
    echo "scenario.sh: no build.zig above $1 -- where is the repo root?" >&2
    return 1
}

REPO=${REPO:-$(_find_repo "$SRC")}
BIN=${BIN:-$(sh "$REPO/tools/build-dir.sh")/bin}
PYTHON=${PYTHON:-$REPO/build/venv/grpc/bin/python}

# xdyngrpc is not installed, it is imported from xdyn_grpc/ -- exactly what the
# xdyngrpc-python image does with `ENV PYTHONPATH=/opt`. $SRC is on the path too, so a
# scenario's own force/wave model modules import like they did with the volume mount.
PYTHONPATH="$REPO/xdyn_grpc:$SRC${PYTHONPATH:+:$PYTHONPATH}"
export PYTHONPATH
# We run scripts straight out of the source tree; don't litter it with __pycache__.
PYTHONDONTWRITEBYTECODE=1
export PYTHONDONTWRITEBYTECODE

_pids=''
_name=''

scenario() {
    _name=$1
    WORK=${WORK:-$REPO/build/scratch/integration-tests/$_name}
    rm -rf "$WORK"
    mkdir -p "$WORK"
    trap _cleanup EXIT HUP INT TERM
    cd "$WORK"
    echo "=== $_name (work dir: $WORK)"
}

# Terminate one pid and do not return until it is actually gone -- otherwise the next
# server to claim the same port can silently fail to bind and the tests talk to a corpse.
_kill() {
    kill "$1" 2>/dev/null || true
    for _ in 1 2 3 4 5 6 7 8 9 10; do
        kill -0 "$1" 2>/dev/null || break
        sleep 0.2
    done
    kill -9 "$1" 2>/dev/null || true
    wait "$1" 2>/dev/null || true
}

_cleanup() {
    status=$?
    for pid in $_pids; do
        kill "$pid" 2>/dev/null || true
    done
    for pid in $_pids; do
        _kill "$pid"
    done
    if [ "$status" -ne 0 ]; then
        for log in "$WORK"/*.log; do
            [ -f "$log" ] || continue
            echo "--- $log"
            tail -40 "$log"
        done
        echo "=== $_name FAILED (status $status)"
    else
        echo "=== $_name passed"
    fi
    return $status
}

# serve <label> <command...> -- start a background process, log it, remember its pid.
#
# The label becomes both a log filename and a shell variable name, so it is restricted to
# [A-Za-z0-9_] and checked. That is cheaper than sanitising it: BSD `tr -c` with a one-character
# replacement is underspecified, and a silently-mangled label would lose a pid.
serve() {
    label=$1
    shift
    case $label in
        *[!A-Za-z0-9_]*) echo "scenario.sh: bad label '$label' (use [A-Za-z0-9_])" >&2; return 1 ;;
    esac
    "$@" > "$WORK/$label.log" 2>&1 &
    _pids="$_pids $!"
    eval "_pid_$label=$!"
    echo "--- started $label (pid $!)"
}

# stop <label> -- shut one server down early, for scenarios that reuse a port.
stop() {
    eval "pid=\${_pid_$1:-}"
    [ -n "$pid" ] || return 0
    _kill "$pid"
    echo "--- stopped $1"
}

# wait_port <port> [timeout_seconds] -- what wait-for-it did, without the bash.
wait_port() {
    "$PYTHON" - "$1" "${2:-20}" <<'PY'
import socket, sys, time
port, timeout = int(sys.argv[1]), float(sys.argv[2])
deadline = time.monotonic() + timeout
while time.monotonic() < deadline:
    with socket.socket() as s:
        s.settimeout(1)
        if s.connect_ex(("127.0.0.1", port)) == 0:
            sys.exit(0)
    time.sleep(0.05)
sys.exit("timed out after %gs waiting for 127.0.0.1:%d" % (timeout, port))
PY
}

# The YAMLs name their gRPC peers by compose service name. That is the service discovery
# compose was providing; on localhost it is a sed.
#
# Deliberately not `sed -i`: GNU takes no argument, BSD/macOS takes the backup suffix, so
# `sed -i -e …` silently means "back up to file-e" there. Temp file + mv works on both.
_localize() {
    for f in "$@"; do
        sed -e 's/waves-server:50051/localhost:50051/g' \
            -e 's/force-model:9002/localhost:9002/g' \
            -e 's/python-controller:9002/localhost:9002/g' \
            -e 's/pid:9002/localhost:9002/g' \
            "$f" > "$f.localized"
        mv "$f.localized" "$f"
    done
}

# demos -- generate every tutorial/mesh/hdb input into $WORK.
#
# These used to be copied out of /usr/demos inside the xdyn image; the same binaries that
# ship in that image generate them, so the image was never the source of truth.
demos() {
    "$BIN/generate_yaml_example" . > /dev/null
    "$BIN/generate_stl_examples" . > /dev/null
    _localize ./*.yml
}

# fixtures <file...> -- copy tracked scenario inputs into $WORK and localize them.
fixtures() {
    for f in "$@"; do
        cp "$SRC/$f" "$WORK/$f"
    done
    _localize "$@"
}

# pytest <file...> -- run a scenario's tests out of the source tree, with $WORK as cwd.
#
# The originals were driven by nose2 (grpc) and nosetests (json). nose is dead on modern
# Python and nose2 is unmaintained; pytest runs both the unittest.TestCase classes and
# the bare test_* functions these files use, with no edits to either.
pytest() {
    for f in "$@"; do
        # no:cacheprovider: the tests live in the source tree, .pytest_cache must not.
        "$PYTHON" -m pytest -v -p no:cacheprovider "$SRC/$f"
    done
}

# check <script> -- run a post-processing assertion script (they read xdyn's CSV output
# from the current directory, which is $WORK). Not tests in the pytest sense: they are
# plain scripts whose assertions fire at import time.
check() {
    "$PYTHON" "$SRC/$1"
}
