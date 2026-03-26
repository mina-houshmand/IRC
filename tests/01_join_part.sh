#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
# shellcheck disable=SC1091
source "$SCRIPT_DIR/common.sh"

echo "[TEST 01] JOIN / PART / QUIT on $TEST_CHANNEL"
echo "[TEST 01] client1 + client2 + client3 join, client3 observes, client2 parts"

run_client c1 \
    "PASS $SERVER_PASS" \
    "NICK alpha" \
    "USER alpha 0 0 :alpha" \
    "JOIN $TEST_CHANNEL" \
    "sleep:1" \
    "PRIVMSG $TEST_CHANNEL :hello from alpha" \
    "sleep:1" \
    "PART $TEST_CHANNEL" \
    "sleep:1" \
    "QUIT :alpha out" &
PID1=$!

run_client c2 \
    "PASS $SERVER_PASS" \
    "NICK beta" \
    "USER beta 0 0 :beta" \
    "JOIN $TEST_CHANNEL" \
    "sleep:2" \
    "PART $TEST_CHANNEL" \
    "sleep:1" \
    "QUIT :beta out" &
PID2=$!

run_client c3 \
    "PASS $SERVER_PASS" \
    "NICK gamma" \
    "USER gamma 0 0 :gamma" \
    "JOIN $TEST_CHANNEL" \
    "sleep:4" \
    "QUIT :gamma out" &
PID3=$!

wait "$PID1" "$PID2" "$PID3"

echo "[TEST 01] done"
