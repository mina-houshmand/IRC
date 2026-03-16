#!/bin/bash

# MODE Test Script - Tests with proper timeouts
# Output saved to file without ANSI codes

SUFFIX="${RANDOM:0:3}"
OUTPUT_FILE="/mnt/c/42/irc_git/scripts/test_MODE_output.txt"

echo "========================================"
echo "     MODE Command Tests"
echo "========================================"
echo ""

> "$OUTPUT_FILE"

log() {
    echo "$1" | tee -a "$OUTPUT_FILE"
}

log "Starting MODE tests... Suffix: $SUFFIX"
log ""

# TEST 1: MODE +i
CH1="#m1${SUFFIX}"
log "=== TEST 1: MODE +i (invite only) ==="
log "CMD: MODE $CH1 +i"

timeout 3 bash -c "printf 'PASS pass\r\nNICK u1\r\nUSER u1 0 0 :U1\r\nJOIN %s\r\nMODE %s +i\r\n' '$CH1' '$CH1' | nc -q1 localhost 6667" 2>&1 | tee -a "$OUTPUT_FILE"

log "Expected: :localhost MODE $CH1 +i"
log ""

# TEST 2: MODE +t
CH2="#m2${SUFFIX}"
log "=== TEST 2: MODE +t (topic restrict) ==="
log "CMD: MODE $CH2 +t"

timeout 3 bash -c "printf 'PASS pass\r\nNICK u2\r\nUSER u2 0 0 :U2\r\nJOIN %s\r\nMODE %s +t\r\n' '$CH2' '$CH2' | nc -q1 localhost 6667" 2>&1 | tee -a "$OUTPUT_FILE"

log "Expected: :localhost MODE $CH2 +t"
log ""

# TEST 3: MODE +k
CH3="#m3${SUFFIX}"
log "=== TEST 3: MODE +k (set key) ==="
log "CMD: MODE $CH3 +k secret"

timeout 3 bash -c "printf 'PASS pass\r\nNICK u3\r\nUSER u3 0 0 :U3\r\nJOIN %s\r\nMODE %s +k secret\r\n' '$CH3' '$CH3' | nc -q1 localhost 6667" 2>&1 | tee -a "$OUTPUT_FILE"

log "Expected: :localhost MODE $CH3 +k secret"
log ""

# TEST 4: MODE +l
CH4="#m4${SUFFIX}"
log "=== TEST 4: MODE +l (set limit) ==="
log "CMD: MODE $CH4 +l 10"

timeout 3 bash -c "printf 'PASS pass\r\nNICK u4\r\nUSER u4 0 0 :U4\r\nJOIN %s\r\nMODE %s +l 10\r\n' '$CH4' '$CH4' | nc -q1 localhost 6667" 2>&1 | tee -a "$OUTPUT_FILE"

log "Expected: :localhost MODE $CH4 +l 10"
log ""

# TEST 5: MODE query
CH5="#m5${SUFFIX}"
log "=== TEST 5: MODE query (get modes) ==="
log "CMD: MODE $CH5"

timeout 3 bash -c "printf 'PASS pass\r\nNICK u5\r\nUSER u5 0 0 :U5\r\nJOIN %s\r\nMODE %s\r\n' '$CH5' '$CH5' | nc -q1 localhost 6667" 2>&1 | tee -a "$OUTPUT_FILE"

log "Expected: 324 ... $CH5 +modes"
log ""

# TEST 6: MODE - not operator
CH6="#m6${SUFFIX}"
log "=== TEST 6: MODE - not operator ==="
log "Setup: join channel without op"
log "CMD: MODE $CH6 +i"

timeout 3 bash -c "printf 'PASS pass\r\nNICK u6\r\nUSER u6 0 0 :U6\r\nJOIN %s\r\nMODE %s +i\r\n' '$CH6' '$CH6' | nc -q1 localhost 6667" 2>&1 | tee -a "$OUTPUT_FILE"

log "Expected: 482 ... :You're not channel operator"
log ""

log "=== DONE ==="
log "Output: $OUTPUT_FILE"
log "VS Code: Ctrl+O -> C:\\42\\irc_git\\scripts\\test_MODE_output.txt"

exit 0
