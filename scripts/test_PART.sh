#!/bin/bash

# PART Command Test Script for IRC Server
# Tests all PART command cases

# ANSI Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color
BOLD='\033[1m'

echo -e "${BOLD}${CYAN}========================================${NC}"
echo -e "${BOLD}${CYAN}       PART Command Test Suite${NC}"
echo -e "${BOLD}${CYAN}========================================${NC}"
echo ""

# Test 1: Normal PART from channel
echo -e "${BOLD}${YELLOW}Test 1:${NC} Normal PART from channel"
echo -e "${BLUE}Expected:${NC} User successfully leaves #test1"
(printf 'PASS pass\r\nNICK user1\r\nUSER user1 0 0 :User One\r\nJOIN #test1\r\nPART #test1\r\n') | nc -q1 localhost 6667 &
sleep 0.5

# Test 2: PART with reason
echo -e "${BOLD}${YELLOW}Test 2:${NC} PART with reason"
echo -e "${BLUE}Expected:${NC} User leaves #test2 with reason 'Goodbye!'"
(printf 'PASS pass\r\nNICK user2\r\nUSER user2 0 0 :User Two\r\nJOIN #test2\r\nPART #test2 :Goodbye!\r\n') | nc -q1 localhost 6667 &
sleep 0.5

# Test 3: PART from non-existent channel
echo -e "${BOLD}${YELLOW}Test 3:${NC} PART from non-existent channel"
echo -e "${BLUE}Expected:${NC} ERR_NOSUCHCHANNEL (403)"
(printf 'PASS pass\r\nNICK user3\r\nUSER user3 0 0 :User Three\r\nPART #nonexistent\r\n') | nc -q1 localhost 6667 &
sleep 0.5

# Test 4: PART when not in channel
echo -e "${BOLD}${YELLOW}Test 4:${NC} PART when not in channel"
echo -e "${BLUE}Expected:${NC} ERR_NOTONCHANNEL (442)"
(printf 'PASS pass\r\nNICK user4\r\nUSER user4 0 0 :User Four\r\nPART #test4\r\n') | nc -q1 localhost 6667 &
sleep 0.5

# Test 5: PART with multiple channels
echo -e "${BOLD}${YELLOW}Test 5:${NC} PART with multiple channels"
echo -e "${BLUE}Expected:${NC} User leaves #multi1 and #multi2"
(printf 'PASS pass\r\nNICK user5\r\nUSER user5 0 0 :User Five\r\nJOIN #multi1\r\nJOIN #multi2\r\nPART #multi1,#multi2\r\n') | nc -q1 localhost 6667 &
sleep 0.5

# Test 6: PART without parameters
echo -e "${BOLD}${YELLOW}Test 6:${NC} PART without parameters"
echo -e "${BLUE}Expected:${NC} ERR_NEEDMOREPARAMS (461)"
(printf 'PASS pass\r\nNICK user6\r\nUSER user6 0 0 :User Six\r\nPART\r\n') | nc -q1 localhost 6667 &
sleep 0.5

# Test 7: Two users, one PARTs while other watches
echo -e "${BOLD}${YELLOW}Test 7:${NC} Two users - PART broadcast"
echo -e "${BLUE}Expected:${NC} user8 receives PART notification from user7"
(printf 'PASS pass\r\nNICK user7\r\nUSER user7 0 0 :User Seven\r\nJOIN #watch\r\nPART #watch :Leaving now\r\n') | nc -q1 localhost 6667 &
sleep 0.3
(printf 'PASS pass\r\nNICK user8\r\nUSER user8 0 0 :User Eight\r\nJOIN #watch\r\n') | nc -q1 localhost 6667 &
sleep 0.5

# Test 8: PART with empty reason (just colon)
echo -e "${BOLD}${YELLOW}Test 8:${NC} PART with empty reason"
echo -e "${BLUE}Expected:${NC} User leaves #test8 with empty reason"
(printf 'PASS pass\r\nNICK user9\r\nUSER user9 0 0 :User Nine\r\nJOIN #test8\r\nPART #test8 :\r\n') | nc -q1 localhost 6667 &
sleep 0.5

# Test 9: PART channel becomes empty (should be deleted)
echo -e "${BOLD}${YELLOW}Test 9:${NC} PART - channel cleanup"
echo -e "${BLUE}Expected:${NC} Channel #cleanup is deleted after last user leaves"
(printf 'PASS pass\r\nNICK user10\r\nUSER user10 0 0 :User Ten\r\nJOIN #cleanup\r\nPART #cleanup\r\n') | nc -q1 localhost 6667 &
sleep 0.5

echo ""
echo -e "${BOLD}${CYAN}========================================${NC}"
echo -e "${GREEN}All tests launched!${NC}"
echo -e "${BLUE}Check server.log for detailed output${NC}"
echo -e "${BOLD}${CYAN}========================================${NC}"
echo ""

exit 0
