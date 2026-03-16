#!/bin/bash

# KICK Test Script - Tests with proper timing
# Output saved to file without ANSI codes

SUFFIX="${RANDOM:0:3}"
OUTPUT_FILE="/mnt/c/42/irc_git/scripts/test_KICK_output.txt"

echo "========================================"
echo "     KICK Command Tests"
echo "========================================"
echo ""

> "$OUTPUT_FILE"

log() {
    echo "$1" | tee -a "$OUTPUT_FILE"
}

log "Starting KICK tests... Suffix: $SUFFIX"
log ""

# TEST 1: KICK - not operator (keep connection open)
CH1="#k1${SUFFIX}"
log "=== TEST 1: KICK - not operator ==="
log "Setup: Join channel (not op), try to kick"
log "CMD: KICK $CH1 someone"

# Use subshell to keep connection open during KICK
{
    printf 'PASS pass\r\n'
    printf 'NICK u1\r\n'
    printf 'USER u1 0 0 :U1\r\n'
    printf 'JOIN %s\r\n' "$CH1"
    sleep 0.5
    printf 'KICK %s someone\r\n' "$CH1"
    sleep 0.5
} | nc -q1 localhost 6667 2>&1 | tee -a "$OUTPUT_FILE"

log "Expected: 482 ... :You're not channel operator"
log ""

# TEST 2: KICK - no channel
log "=== TEST 2: KICK - no channel ==="
log "CMD: KICK #nochannel user"

{
    printf 'PASS pass\r\n'
    printf 'NICK u2\r\n'
    printf 'USER u2 0 0 :U2\r\n'
    printf 'KICK #nochannel user\r\n'
    sleep 0.3
} | nc -q1 localhost 6667 2>&1 | tee -a "$OUTPUT_FILE"

log "Expected: 403 ... :No such channel"
log ""

# TEST 3: KICK - need more params
log "=== TEST 3: KICK - need more params ==="
log "CMD: KICK #channel"

{
    printf 'PASS pass\r\n'
    printf 'NICK u3\r\n'
    printf 'USER u3 0 0 :U3\r\n'
    printf 'KICK #channel\r\n'
    sleep 0.3
} | nc -q1 localhost 6667 2>&1 | tee -a "$OUTPUT_FILE"

log "Expected: 461 ... :Not enough parameters"
log ""

log "=== DONE ==="
log "Output: $OUTPUT_FILE"
log "VS Code: Ctrl+O -> C:\\42\\irc_git\\scripts\\test_KICK_output.txt"

exit 0
