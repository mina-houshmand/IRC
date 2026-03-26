#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
# shellcheck disable=SC1091
source "$SCRIPT_DIR/common.sh"

echo "[TEST 04] INVITE / KICK / MODE +i on $TEST_CHANNEL"
echo "[TEST 04] client1 invites client2, then kicks client2, client3 observes"

run_client c1 \
    "PASS $SERVER_PASS" \
    "NICK alpha" \
    "USER alpha 0 0 :alpha" \
    "JOIN $TEST_CHANNEL" \
    "sleep:1" \
    "MODE $TEST_CHANNEL +i" \
    "sleep:1" \
    "INVITE beta $TEST_CHANNEL" \
    "sleep:1" \
    "KICK $TEST_CHANNEL beta :go away" \
    "sleep:1" \
    "QUIT :alpha out" &
PID1=$!

run_client c2 \
    "PASS $SERVER_PASS" \
    "NICK beta" \
    "USER beta 0 0 :beta" \
    "sleep:3" \
    "JOIN $TEST_CHANNEL" \
    "sleep:2" \
    "QUIT :beta out" &
PID2=$!

run_client c3 \
    "PASS $SERVER_PASS" \
    "NICK gamma" \
    "USER gamma 0 0 :gamma" \
    "sleep:1" \
    "JOIN $TEST_CHANNEL" \
    "sleep:4" \
    "QUIT :gamma out" &
PID3=$!

wait "$PID1" "$PID2" "$PID3"

echo "[TEST 04] done"
