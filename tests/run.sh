#!/bin/sh
# tests/run.sh — end-to-end checks: build, scan a fixture, exercise the API.
#
#   make test          (or: sh tests/run.sh)
#
# The fixture is copied to a temporary folder first, because the save test
# really writes a file. Nothing in the repository is modified.
set -e

ROOT=$(cd "$(dirname "$0")/.." && pwd)
PORT=${PORT:-8791}
BIN="$ROOT/perspecta_see"

if [ ! -x "$BIN" ]; then
    ( cd "$ROOT" && make -s )
fi

WORK=$(mktemp -d)
cp -R "$ROOT/tests/fixture/." "$WORK/"

"$BIN" --port "$PORT" --host 127.0.0.1 --web "$ROOT/web" >"$WORK/server.log" 2>&1 &
SRV=$!
trap 'kill "$SRV" 2>/dev/null || true; rm -rf "$WORK"' EXIT INT TERM

# Wait for the socket (max ~3s).
i=0
while [ $i -lt 30 ]; do
    if (exec 3<>"/dev/tcp/127.0.0.1/$PORT") 2>/dev/null; then break; fi
    i=$((i + 1))
    sleep 0.1
done

python3 "$ROOT/tests/check.py" "$PORT" "$WORK"
