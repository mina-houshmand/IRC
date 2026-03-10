#!/bin/bash

# PRIVMSG Command Test Script for IRC Server
# Tests all PRIVMSG command cases

# ANSI Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color
BOLD='\033[1m'

# Generate unique suffix for this test run to avoid nickname collisions
SUFFIX="_$$"  # Use process ID

echo -e "${BOLD}${CYAN}========================================${NC}"
echo -e "${BOLD}${CYAN}     PRIVMSG Command Test Suite${NC}"
echo -e "${BOLD}${CYAN}========================================${NC}"
echo ""

# Test 1: Normal PRIVMSG to channel (sender only, no broadcast visible)
echo -e "${BOLD}${YELLOW}Test 1:${NC} PRIVMSG to channel"
echo -e "${BLUE}Expected:${NC} Message sent to #msgtest channel"
(printf 'PASS pass\r\nNICK t1_sender1'"$SUFFIX"'\r\nUSER t1_sender1 0 0 :Sender One\r\nJOIN #msgtest\r\nPRIVMSG #msgtest :Hello channel!\r\n'; sleep 0.3) | nc -q1 localhost 6667 &
sleep 0.5

# Test 2: PRIVMSG to user - keep receiver open to see message
echo -e "${BOLD}${YELLOW}Test 2:${NC} PRIVMSG to user"
echo -e "${BLUE}Expected:${NC} receiver gets PRIVMSG from sender2"
# Start receiver first (keep open for 3 seconds)
{ printf 'PASS pass\r\nNICK t2_receiver'"$SUFFIX"'\r\nUSER t2_receiver 0 0 :Receiver User\r\n'; sleep 3; } | nc localhost 6667 &
sleep 0.5
# Now sender sends message
(printf 'PASS pass\r\nNICK t2_sender2'"$SUFFIX"'\r\nUSER t2_sender2 0 0 :Sender Two\r\nPRIVMSG t2_receiver'"$SUFFIX"' :Hello receiver!\r\n') | nc -q1 localhost 6667 &
sleep 1

# Test 3: PRIVMSG to non-existent channel
echo -e "${BOLD}${YELLOW}Test 3:${NC} PRIVMSG to non-existent channel"
echo -e "${BLUE}Expected:${NC} ERR_NOSUCHCHANNEL (403)"
(printf 'PASS pass\r\nNICK t3_sender3'"$SUFFIX"'\r\nUSER t3_sender3 0 0 :Sender Three\r\nPRIVMSG #nochannel :Hello?\r\n') | nc -q1 localhost 6667 &
sleep 0.5

# Test 4: PRIVMSG to non-existent user
echo -e "${BOLD}${YELLOW}Test 4:${NC} PRIVMSG to non-existent user"
echo -e "${BLUE}Expected:${NC} ERR_NOSUCHNICK (401)"
(printf 'PASS pass\r\nNICK t4_sender4'"$SUFFIX"'\r\nUSER t4_sender4 0 0 :Sender Four\r\nPRIVMSG nouser :Hello?\r\n') | nc -q1 localhost 6667 &
sleep 0.5

# Test 5: PRIVMSG to channel when not member (channel exists)
echo -e "${BOLD}${YELLOW}Test 5:${NC} PRIVMSG to channel when not member"
echo -e "${BLUE}Expected:${NC} ERR_CANNOTSENDTOCHAN (404)"
# First create the channel with another user
{ printf 'PASS pass\r\nNICK t5_chowner'"$SUFFIX"'\r\nUSER t5_chowner 0 0 :Channel Owner\r\nJOIN #restricted\r\n'; sleep 3; } | nc localhost 6667 &
sleep 0.5
# Now try to send without being member
(printf 'PASS pass\r\nNICK t5_sender5'"$SUFFIX"'\r\nUSER t5_sender5 0 0 :Sender Five\r\nPRIVMSG #restricted :Hello?\r\n') | nc -q1 localhost 6667 &
sleep 1

# Test 6: PRIVMSG without parameters
echo -e "${BOLD}${YELLOW}Test 6:${NC} PRIVMSG without parameters"
echo -e "${BLUE}Expected:${NC} ERR_NEEDMOREPARAMS (461)"
(printf 'PASS pass\r\nNICK t6_sender6'"$SUFFIX"'\r\nUSER t6_sender6 0 0 :Sender Six\r\nPRIVMSG\r\n') | nc -q1 localhost 6667 &
sleep 0.5

# Test 7: PRIVMSG to multiple channels
echo -e "${BOLD}${YELLOW}Test 7:${NC} PRIVMSG to multiple channels"
echo -e "${BLUE}Expected:${NC} Message sent to #multi1 and #multi2"
(printf 'PASS pass\r\nNICK t7_sender7'"$SUFFIX"'\r\nUSER t7_sender7 0 0 :Sender Seven\r\nJOIN #multi1\r\nJOIN #multi2\r\nPRIVMSG #multi1,#multi2 :Hello all!\r\n'; sleep 0.3) | nc -q1 localhost 6667 &
sleep 0.5

# Test 8: PRIVMSG with empty message
echo -e "${BOLD}${YELLOW}Test 8:${NC} PRIVMSG with empty message"
echo -e "${BLUE}Expected:${NC} Message with empty content sent"
(printf 'PASS pass\r\nNICK t8_sender8'"$SUFFIX"'\r\nUSER t8_sender8 0 0 :Sender Eight\r\nJOIN #empty\r\nPRIVMSG #empty :\r\n'; sleep 0.3) | nc -q1 localhost 6667 &
sleep 0.5

# Test 9: Two users in channel, one sends message (proper timing)
echo -e "${BOLD}${YELLOW}Test 9:${NC} PRIVMSG broadcast in channel"
echo -e "${BLUE}Expected:${NC} user10 receives message from user9"
# user9 connects and joins (waits 4 seconds before sending PRIVMSG)
{ printf 'PASS pass\r\nNICK t9_user9'"$SUFFIX"'\r\nUSER t9_user9 0 0 :User Nine\r\nJOIN #broadcast\r\n'; sleep 4; printf 'PRIVMSG #broadcast :Hello everyone!\r\n'; sleep 1; } | nc localhost 6667 &
sleep 0.5
# user10 joins after user9 (should be in channel when PRIVMSG is sent)
{ printf 'PASS pass\r\nNICK t9_user10'"$SUFFIX"'\r\nUSER t9_user10 0 0 :User Ten\r\nJOIN #broadcast\r\n'; sleep 5; } | nc localhost 6667 &
sleep 1

# Test 10: PRIVMSG to self (should be ignored)
echo -e "${BOLD}${YELLOW}Test 10:${NC} PRIVMSG to self"
echo -e "${BLUE}Expected:${NC} No message sent (ignored)"
(printf 'PASS pass\r\nNICK t10_selfsend'"$SUFFIX"'\r\nUSER t10_selfsend 0 0 :Self Send\r\nPRIVMSG t10_selfsend'"$SUFFIX"' :Hello myself!\r\n') | nc -q1 localhost 6667 &
sleep 0.5

echo ""
echo -e "${BOLD}${CYAN}========================================${NC}"
echo -e "${GREEN}All tests launched!${NC}"
echo -e "${BLUE}Check server.log for detailed output${NC}"
echo -e "${BOLD}${CYAN}========================================${NC}"
echo ""

exit 0
