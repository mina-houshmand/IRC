#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

TESTS=(
    "01_join_part.sh"
    "02_privmsg.sh"
    "03_topic_mode.sh"
    "04_invite_kick.sh"
    "05_help_quit.sh"
)

echo "[TEST RUNNER] using channel #test"

after_each_pause=2

for test in "${TESTS[@]}"; do
    echo ""
    echo "[TEST RUNNER] running: $test"
    bash "$SCRIPT_DIR/$test"
    echo "[TEST RUNNER] finished: $test"
    echo "[TEST RUNNER] pausing $after_each_pause seconds"
    sleep "$after_each_pause"
done

echo "[TEST RUNNER] all tests done"
