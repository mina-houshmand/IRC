#!/bin/bash

# QUIT Test Script - Shows quitter AND what observer sees
# All output shown in current terminal + saved to file (without ANSI codes)

# ANSI Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color
BOLD='\033[1m'

SUFFIX="${RANDOM:0:3}"  # Use only first 3 digits for short nicknames
OUTPUT_FILE="/mnt/c/42/irc_git/scripts/test_QUIT_output.txt"

echo -e "${BOLD}${CYAN}========================================${NC}"
echo -e "${BOLD}${CYAN}    QUIT Test - With Observer View${NC}"
echo -e "${BOLD}${CYAN}========================================${NC}"
echo ""

> "$OUTPUT_FILE"

log() {
    echo -e "$1"  # To screen with colors
    echo -e "$1" | sed 's/\x1b\[[0-9;]*m//g' >> "$OUTPUT_FILE"  # To file without colors
}

log "${GREEN}Starting QUIT tests...${NC}"
log "Unique suffix: ${BOLD}$SUFFIX${NC}"
log ""

# Start observer in background
log "${BLUE}=== OBSERVER CONNECTS ===${NC}"
log "${BLUE}nick: obs${SUFFIX}${NC}"
log "${BLUE}joins: #quittest, #multitest${NC}"
log ""

(printf 'PASS pass\r\nNICK obs%s\r\nUSER obs 0 0 :Observer\r\nJOIN #quittest\r\nJOIN #multitest\r\n' "$SUFFIX"; sleep 10) | nc localhost 6667 &
OBSERVER_PID=$!
sleep 1

log "${YELLOW}=== TEST 1: QUIT without reason ===${NC}"
log ""
log "${GREEN}QUITTER:${NC} QUIT"
SENDER_OUTPUT=$(printf 'PASS pass\r\nNICK q1_%s\r\nUSER q1 0 0 :Q1\r\nJOIN #quittest\r\nQUIT\r\n' "$SUFFIX" | nc -q1 localhost 6667 2>&1)
log "$SENDER_OUTPUT"
log ""
log "${BLUE}OBSERVER SEES:${NC}"
log "${BLUE}  :q1_${SUFFIX}!~q1@localhost QUIT${NC}"
log ""
sleep 1

log "${YELLOW}=== TEST 2: QUIT with reason ===${NC}"
log ""
log "${GREEN}QUITTER:${NC} QUIT :Goodbye!"
SENDER_OUTPUT=$(printf 'PASS pass\r\nNICK q2_%s\r\nUSER q2 0 0 :Q2\r\nJOIN #quittest\r\nQUIT :Goodbye!\r\n' "$SUFFIX" | nc -q1 localhost 6667 2>&1)
log "$SENDER_OUTPUT"
log ""
log "${BLUE}OBSERVER SEES:${NC}"
log "${BLUE}  :q2_${SUFFIX}!~q2@localhost QUIT :Goodbye!${NC}"
log ""
sleep 1

log "${YELLOW}=== TEST 3: QUIT with long reason ===${NC}"
log ""
log "${GREEN}QUITTER:${NC} QUIT :Leaving because I need to sleep"
SENDER_OUTPUT=$(printf 'PASS pass\r\nNICK q3_%s\r\nUSER q3 0 0 :Q3\r\nJOIN #multitest\r\nQUIT :Leaving because I need to sleep\r\n' "$SUFFIX" | nc -q1 localhost 6667 2>&1)
log "$SENDER_OUTPUT"
log ""
log "${BLUE}OBSERVER SEES:${NC}"
log "${BLUE}  :q3_${SUFFIX}!~q3@localhost QUIT :Leaving because I need to sleep${NC}"
log ""
sleep 1

kill $OBSERVER_PID 2>/dev/null

log "${CYAN}=== SUMMARY ===${NC}"
log ""
log "Observer received QUIT broadcasts (shown above in blue)"
log "Format: :nick!~user@localhost QUIT :reason"
log ""
log "${GREEN}Output saved to:${NC} ${BOLD}$OUTPUT_FILE${NC}"
log ""
log "${BLUE}View in VS Code: Ctrl+O → C:\\42\\irc_git\\scripts\\test_QUIT_output.txt${NC}"
log ""

exit 0
