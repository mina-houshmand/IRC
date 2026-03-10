#!/bin/bash

# TOPIC Command Test Script for IRC Server
# Tests all TOPIC command cases

# ANSI Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color
BOLD='\033[1m'

# Generate unique suffix for this test run (simple numbers only)
SUFFIX="${RANDOM}"

echo -e "${BOLD}${CYAN}========================================${NC}"
echo -e "${BOLD}${CYAN}       TOPIC Command Test Suite${NC}"
echo -e "${BOLD}${CYAN}========================================${NC}"
echo ""

# Test 1: TOPIC query on channel with no topic
echo -e "${BOLD}${YELLOW}Test 1:${NC} TOPIC query (no topic set)"
echo -e "${BLUE}Expected:${NC} RPL_NOTOPIC (331)"
(printf 'PASS pass\r\nNICK t1u1%s\r\nUSER t1u1 0 0 :User One\r\nJOIN #topic1\r\nTOPIC #topic1\r\n' "$SUFFIX") | nc -q1 localhost 6667 &
sleep 0.5

# Test 2: TOPIC set on channel
echo -e "${BOLD}${YELLOW}Test 2:${NC} TOPIC set"
echo -e "${BLUE}Expected:${NC} Topic set and broadcast"
(printf 'PASS pass\r\nNICK t2u2%s\r\nUSER t2u2 0 0 :User Two\r\nJOIN #topic2\r\nTOPIC #topic2 :Welcome to the channel!\r\n' "$SUFFIX") | nc -q1 localhost 6667 &
sleep 0.5

# Test 3: TOPIC query after setting
echo -e "${BOLD}${YELLOW}Test 3:${NC} TOPIC query (topic set)"
echo -e "${BLUE}Expected:${NC} RPL_TOPICIS (332) with topic"
(printf 'PASS pass\r\nNICK t3u3%s\r\nUSER t3u3 0 0 :User Three\r\nJOIN #topic3\r\nTOPIC #topic3 :My Topic\r\nTOPIC #topic3\r\n' "$SUFFIX") | nc -q1 localhost 6667 &
sleep 0.5

# Test 4: TOPIC on non-existent channel
echo -e "${BOLD}${YELLOW}Test 4:${NC} TOPIC on non-existent channel"
echo -e "${BLUE}Expected:${NC} ERR_NOSUCHCHANNEL (403)"
(printf 'PASS pass\r\nNICK t4u4%s\r\nUSER t4u4 0 0 :User Four\r\nTOPIC #nochannel\r\n' "$SUFFIX") | nc -q1 localhost 6667 &
sleep 0.5

# Test 5: TOPIC when not in channel
echo -e "${BOLD}${YELLOW}Test 5:${NC} TOPIC when not in channel"
echo -e "${BLUE}Expected:${NC} ERR_NOTONCHANNEL (442)"
# First create channel with another user
{ printf 'PASS pass\r\nNICK t5owner%s\r\nUSER t5owner 0 0 :Owner\r\nJOIN #topic5\r\nTOPIC #topic5 :Private\r\n'; sleep 2; } | nc localhost 6667 &
sleep 0.3
(printf 'PASS pass\r\nNICK t5u5%s\r\nUSER t5u5 0 0 :User Five\r\nTOPIC #topic5\r\n' "$SUFFIX") | nc -q1 localhost 6667 &
sleep 1

# Test 6: TOPIC with topic_restriction (+t) - operator
echo -e "${BOLD}${YELLOW}Test 6:${NC} TOPIC with +t (operator)"
echo -e "${BLUE}Expected:${NC} Operator can change topic"
(printf 'PASS pass\r\nNICK t6op%s\r\nUSER t6op 0 0 :Operator\r\nJOIN #topic6\r\nMODE #topic6 +t\r\nTOPIC #topic6 :OP set this\r\n' "$SUFFIX") | nc -q1 localhost 6667 &
sleep 0.5

# Test 7: TOPIC with topic_restriction (+t) - non-operator
echo -e "${BOLD}${YELLOW}Test 7:${NC} TOPIC with +t (non-operator)"
echo -e "${BLUE}Expected:${NC} ERR_CHANOPRIVSNEEDED (482)"
# Create channel with owner who sets +t
{ printf 'PASS pass\r\nNICK t7owner%s\r\nUSER t7owner 0 0 :Owner\r\nJOIN #topic7\r\nMODE #topic7 +t\r\nTOPIC #topic7 :Owner topic\r\n'; sleep 3; } | nc localhost 6667 &
sleep 0.5
# Non-op tries to change topic
(printf 'PASS pass\r\nNICK t7u7%s\r\nUSER t7u7 0 0 :User Seven\r\nJOIN #topic7\r\nTOPIC #topic7 :Hacked!\r\n' "$SUFFIX") | nc -q1 localhost 6667 &
sleep 1

# Test 8: TOPIC with empty topic
echo -e "${BOLD}${YELLOW}Test 8:${NC} TOPIC with empty topic"
echo -e "${BLUE}Expected:${NC} Topic cleared"
(printf 'PASS pass\r\nNICK t8u8%s\r\nUSER t8u8 0 0 :User Eight\r\nJOIN #topic8\r\nTOPIC #topic8 :\r\n' "$SUFFIX") | nc -q1 localhost 6667 &
sleep 0.5

# Test 9: TOPIC broadcast to channel members
echo -e "${BOLD}${YELLOW}Test 9:${NC} TOPIC broadcast"
echo -e "${BLUE}Expected:${NC} user2 sees TOPIC change from user1"
# user1 joins and changes topic
{ printf 'PASS pass\r\nNICK t9u1%s\r\nUSER t9u1 0 0 :User One\r\nJOIN #topic9\r\n'; sleep 1; printf 'TOPIC #topic9 :New Topic!\r\n'; sleep 1; } | nc localhost 6667 &
sleep 0.3
# user2 joins and should see topic
{ printf 'PASS pass\r\nNICK t9u2%s\r\nUSER t9u2 0 0 :User Two\r\nJOIN #topic9\r\n'; sleep 2; } | nc localhost 6667 &
sleep 1

# Test 10: TOPIC with long topic
echo -e "${BOLD}${YELLOW}Test 10:${NC} TOPIC with long topic"
echo -e "${BLUE}Expected:${NC} Full topic preserved"
(printf 'PASS pass\r\nNICK t10u%s\r\nUSER t10u 0 0 :User Ten\r\nJOIN #topic10\r\nTOPIC #topic10 :This is a very long topic that contains many words and should be preserved exactly as it is\r\n' "$SUFFIX") | nc -q1 localhost 6667 &
sleep 0.5

echo ""
echo -e "${BOLD}${CYAN}========================================${NC}"
echo -e "${GREEN}All tests launched!${NC}"
echo -e "${BLUE}Check server.log for detailed output${NC}"
echo -e "${BOLD}${CYAN}========================================${NC}"
echo ""

exit 0
