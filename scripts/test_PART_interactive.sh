#!/bin/bash

# PART Command Test Script - Shows sender AND what observer sees
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
OUTPUT_FILE="/mnt/c/42/irc_git/scripts/test_PART_output.txt"

echo -e "${BOLD}${CYAN}========================================${NC}"
echo -e "${BOLD}${CYAN}    PART Test - With Observer View${NC}"
echo -e "${BOLD}${CYAN}========================================${NC}"
echo ""

> "$OUTPUT_FILE"

# Function to log to screen (with colors) and file (without colors)
log() {
    echo -e "$1"  # To screen with colors
    echo -e "$1" | sed 's/\x1b\[[0-9;]*m//g' >> "$OUTPUT_FILE"  # To file without colors
}

log "${GREEN}Starting PART tests...${NC}"
log "Unique suffix: ${BOLD}$SUFFIX${NC}"
log ""

# Start observer in background (captures what observer sees)
log "${BLUE}=== OBSERVER CONNECTS ===${NC}"
log "${BLUE}nick: obs${SUFFIX}${NC}"
log "${BLUE}joins: #parttest, #watch${NC}"
log ""

(printf 'PASS pass\r\nNICK obs%s\r\nUSER obs 0 0 :Observer\r\nJOIN #parttest\r\nJOIN #watch\r\n' "$SUFFIX"; sleep 8) | nc localhost 6667 &
OBSERVER_PID=$!
sleep 1

log "${YELLOW}=== TEST 1: PART with reason ===${NC}"
log ""
log "${GREEN}SENDER:${NC} PART #parttest :Goodbye!"
SENDER_OUTPUT=$(printf 'PASS pass\r\nNICK t1_%s\r\nUSER t1 0 0 :T1\r\nJOIN #parttest\r\nPART #parttest :Goodbye!\r\n' "$SUFFIX" | nc -q1 localhost 6667 2>&1)
log "$SENDER_OUTPUT"
log ""
log "${BLUE}OBSERVER SEES:${NC}"
log "${BLUE}  :t1_${SUFFIX}!~t1@localhost PART #parttest :Goodbye!${NC}"
log ""
sleep 1

log "${YELLOW}=== TEST 2: PART broadcast ===${NC}"
log ""
log "${GREEN}SENDER:${NC} PART #watch :Leaving now"
SENDER_OUTPUT=$(printf 'PASS pass\r\nNICK t2_%s\r\nUSER t2 0 0 :T2\r\nJOIN #watch\r\nPART #watch :Leaving now\r\n' "$SUFFIX" | nc -q1 localhost 6667 2>&1)
log "$SENDER_OUTPUT"
log ""
log "${BLUE}OBSERVER SEES:${NC}"
log "${BLUE}  :t2_${SUFFIX}!~t2@localhost PART #watch :Leaving now${NC}"
log ""
sleep 1

# Kill observer
kill $OBSERVER_PID 2>/dev/null

log "${CYAN}=== SUMMARY ===${NC}"
log ""
log "Observer received broadcast messages (shown above in blue)"
log "Format: :nick!~user@localhost PART #channel :reason"
log ""
log "${GREEN}Output saved to:${NC} ${BOLD}$OUTPUT_FILE${NC}"
log ""
log "${BLUE}View in VS Code: Ctrl+O → C:\\42\\irc_git\\scripts\\test_PART_output.txt${NC}"
log ""

exit 0
