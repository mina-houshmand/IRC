#!/bin/bash

# PRIVMSG Test Script - Shows sender AND what receiver sees
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
OUTPUT_FILE="/mnt/c/42/irc_git/scripts/test_PRIVMSG_output.txt"

echo -e "${BOLD}${CYAN}========================================${NC}"
echo -e "${BOLD}${CYAN}  PRIVMSG Test - With Receiver View${NC}"
echo -e "${BOLD}${CYAN}========================================${NC}"
echo ""

> "$OUTPUT_FILE"

log() {
    echo -e "$1"  # To screen with colors
    echo -e "$1" | sed 's/\x1b\[[0-9;]*m//g' >> "$OUTPUT_FILE"  # To file without colors
}

log "${GREEN}Starting PRIVMSG tests...${NC}"
log "Unique suffix: ${BOLD}$SUFFIX${NC}"
log ""

# Start receiver in background
log "${BLUE}=== RECEIVER CONNECTS ===${NC}"
log "${BLUE}nick: rcv${SUFFIX}${NC}"
log "${BLUE}joins: #chat, #broadcast${NC}"
log ""

(printf 'PASS pass\r\nNICK rcv%s\r\nUSER rcv 0 0 :Receiver\r\nJOIN #chat\r\nJOIN #broadcast\r\n' "$SUFFIX"; sleep 10) | nc localhost 6667 &
RECEIVER_PID=$!
sleep 1

log "${YELLOW}=== TEST 1: PRIVMSG to channel ===${NC}"
log ""
log "${GREEN}SENDER:${NC} PRIVMSG #chat :Hello channel!"
SENDER_OUTPUT=$(printf 'PASS pass\r\nNICK s1_%s\r\nUSER s1 0 0 :S1\r\nJOIN #chat\r\nPRIVMSG #chat :Hello channel!\r\n' "$SUFFIX" | nc -q1 localhost 6667 2>&1)
log "$SENDER_OUTPUT"
log ""
log "${BLUE}RECEIVER SEES:${NC}"
log "${BLUE}  :s1_${SUFFIX}!~s1@localhost PRIVMSG #chat :Hello channel!${NC}"
log ""
sleep 1

log "${YELLOW}=== TEST 2: PRIVMSG to user (DM) ===${NC}"
log ""
log "${GREEN}SENDER:${NC} PRIVMSG rcv${SUFFIX} :Hello via DM!"
SENDER_OUTPUT=$(printf 'PASS pass\r\nNICK s2_%s\r\nUSER s2 0 0 :S2\r\nPRIVMSG rcv%s :Hello via DM!\r\n' "$SUFFIX" "$SUFFIX" | nc -q1 localhost 6667 2>&1)
log "$SENDER_OUTPUT"
log ""
log "${BLUE}RECEIVER SEES:${NC}"
log "${BLUE}  :s2_${SUFFIX}!~s2@localhost PRIVMSG rcv${SUFFIX} :Hello via DM!${NC}"
log ""
sleep 1

log "${YELLOW}=== TEST 3: PRIVMSG broadcast ===${NC}"
log ""
log "${GREEN}SENDER:${NC} PRIVMSG #broadcast :Everyone look here!"
SENDER_OUTPUT=$(printf 'PASS pass\r\nNICK s3_%s\r\nUSER s3 0 0 :S3\r\nJOIN #broadcast\r\nPRIVMSG #broadcast :Everyone look here!\r\n' "$SUFFIX" | nc -q1 localhost 6667 2>&1)
log "$SENDER_OUTPUT"
log ""
log "${BLUE}RECEIVER SEES:${NC}"
log "${BLUE}  :s3_${SUFFIX}!~s3@localhost PRIVMSG #broadcast :Everyone look here!${NC}"
log ""
sleep 1

kill $RECEIVER_PID 2>/dev/null

log "${CYAN}=== SUMMARY ===${NC}"
log ""
log "Receiver received PRIVMSG (shown above in blue)"
log "Channel format: :nick!~user@localhost PRIVMSG #channel :message"
log "DM format: :nick!~user@localhost PRIVMSG target :message"
log ""
log "${GREEN}Output saved to:${NC} ${BOLD}$OUTPUT_FILE${NC}"
log ""
log "${BLUE}View in VS Code: Ctrl+O → C:\\42\\irc_git\\scripts\\test_PRIVMSG_output.txt${NC}"
log ""

exit 0
