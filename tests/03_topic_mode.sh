#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
# shellcheck disable=SC1091
source "$SCRIPT_DIR/common.sh"

echo "[TEST 03] TOPIC / MODE on $TEST_CHANNEL"
echo "[TEST 03] client1 sets +t and topic, client2 and client3 see it"

run_client c1 \
    "PASS $SERVER_PASS" \
    "NICK alpha" \
    "USER alpha 0 0 :alpha" \
    "JOIN $TEST_CHANNEL" \
    "sleep:1" \
    "MODE $TEST_CHANNEL +t" \
    "sleep:1" \
    "TOPIC $TEST_CHANNEL :weekly test topic" \
    "sleep:1" \
    "QUIT :alpha out" &
PID1=$!

run_client c2 \
    "PASS $SERVER_PASS" \
    "NICK beta" \
    "USER beta 0 0 :beta" \
    "sleep:3" \
    "JOIN $TEST_CHANNEL" \
    "sleep:3" \
    "QUIT :beta out" &
PID2=$!

run_client c3 \
    "PASS $SERVER_PASS" \
    "NICK gamma" \
    "USER gamma 0 0 :gamma" \
    "sleep:4" \
    "JOIN $TEST_CHANNEL" \
    "sleep:3" \
    "QUIT :gamma out" &
PID3=$!

wait "$PID1" "$PID2" "$PID3"

echo "[TEST 03] done"
