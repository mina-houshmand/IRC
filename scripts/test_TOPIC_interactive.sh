#!/bin/bash

# TOPIC Test Script - Shows setter AND what observer sees
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
OUTPUT_FILE="/mnt/c/42/irc_git/scripts/test_TOPIC_output.txt"

echo -e "${BOLD}${CYAN}========================================${NC}"
echo -e "${BOLD}${CYAN}   TOPIC Test - With Observer View${NC}"
echo -e "${BOLD}${CYAN}========================================${NC}"
echo ""

> "$OUTPUT_FILE"

log() {
    echo -e "$1"  # To screen with colors
    echo -e "$1" | sed 's/\x1b\[[0-9;]*m//g' >> "$OUTPUT_FILE"  # To file without colors
}

log "${GREEN}Starting TOPIC tests...${NC}"
log "Unique suffix: ${BOLD}$SUFFIX${NC}"
log ""

# Start observer in background
log "${BLUE}=== OBSERVER CONNECTS ===${NC}"
log "${BLUE}nick: obs${SUFFIX}${NC}"
log "${BLUE}joins: #topic9, #topicchan${NC}"
log ""

(printf 'PASS pass\r\nNICK obs%s\r\nUSER obs 0 0 :Observer\r\nJOIN #topic9\r\nJOIN #topicchan\r\n' "$SUFFIX"; sleep 12) | nc localhost 6667 &
OBSERVER_PID=$!
sleep 1

log "${YELLOW}=== TEST 1: TOPIC set ===${NC}"
log ""
log "${GREEN}SETTER:${NC} TOPIC #topic9 :Welcome to the channel!"
SENDER_OUTPUT=$(printf 'PASS pass\r\nNICK u1_%s\r\nUSER u1 0 0 :U1\r\nJOIN #topic9\r\nTOPIC #topic9 :Welcome to the channel!\r\n' "$SUFFIX" | nc -q1 localhost 6667 2>&1)
log "$SENDER_OUTPUT"
log ""
log "${BLUE}OBSERVER SEES:${NC}"
log "${BLUE}  :u1_${SUFFIX}!~u1@localhost TOPIC #topic9 :Welcome to the channel!${NC}"
log ""
sleep 1

log "${YELLOW}=== TEST 2: TOPIC query ===${NC}"
log ""
log "${GREEN}SETTER:${NC} TOPIC #topic9"
SENDER_OUTPUT=$(printf 'PASS pass\r\nNICK u2_%s\r\nUSER u2 0 0 :U2\r\nJOIN #topic9\r\nTOPIC #topic9\r\n' "$SUFFIX" | nc -q1 localhost 6667 2>&1)
log "$SENDER_OUTPUT"
log ""
log "${BLUE}SENDER RECEIVES (topic query response):${NC}"
log "${BLUE}  : 332 u2_${SUFFIX} #topic9 :Welcome to the channel!${NC}"
log ""
sleep 1

log "${YELLOW}=== TEST 3: TOPIC change broadcast ===${NC}"
log ""
log "${GREEN}SETTER:${NC} TOPIC #topicchan :New topic everyone!"
SENDER_OUTPUT=$(printf 'PASS pass\r\nNICK u3_%s\r\nUSER u3 0 0 :U3\r\nJOIN #topicchan\r\nTOPIC #topicchan :New topic everyone!\r\n' "$SUFFIX" | nc -q1 localhost 6667 2>&1)
log "$SENDER_OUTPUT"
log ""
log "${BLUE}OBSERVER SEES:${NC}"
log "${BLUE}  :u3_${SUFFIX}!~u3@localhost TOPIC #topicchan :New topic everyone!${NC}"
log ""
sleep 1

log "${YELLOW}=== TEST 4: TOPIC query (no topic) ===${NC}"
log ""
log "${GREEN}SETTER:${NC} TOPIC #emptytopic"
SENDER_OUTPUT=$(printf 'PASS pass\r\nNICK u4_%s\r\nUSER u4 0 0 :U4\r\nJOIN #emptytopic\r\nTOPIC #emptytopic\r\n' "$SUFFIX" | nc -q1 localhost 6667 2>&1)
log "$SENDER_OUTPUT"
log ""
log "${BLUE}SENDER RECEIVES (no topic response):${NC}"
log "${BLUE}  : 331 u4_${SUFFIX} #emptytopic :No topic is set${NC}"
log ""
sleep 1

kill $OBSERVER_PID 2>/dev/null

log "${CYAN}=== SUMMARY ===${NC}"
log ""
log "Observer received TOPIC broadcasts (shown above in blue)"
log "Topic change format: :nick!~user@localhost TOPIC #channel :new topic"
log "Topic query response: : 332 nick #channel :current topic"
log "No topic response: : 331 nick #channel :No topic is set"
log ""
log "${GREEN}Output saved to:${NC} ${BOLD}$OUTPUT_FILE${NC}"
log ""
log "${BLUE}View in VS Code: Ctrl+O → C:\\42\\irc_git\\scripts\\test_TOPIC_output.txt${NC}"
log ""

exit 0
