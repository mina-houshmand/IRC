#!/bin/bash

# QUIT Command Test Script for IRC Server
# Tests all QUIT command cases

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
echo -e "${BOLD}${CYAN}        QUIT Command Test Suite${NC}"
echo -e "${BOLD}${CYAN}========================================${NC}"
echo ""

# Test 1: QUIT without reason
echo -e "${BOLD}${YELLOW}Test 1:${NC} QUIT without reason"
echo -e "${BLUE}Expected:${NC} User disconnects cleanly"
(printf 'PASS pass\r\nNICK q1u1%s\r\nUSER q1u1 0 0 :User One\r\nQUIT\r\n' "$SUFFIX") | nc -q1 localhost 6667 &
sleep 0.5

# Test 2: QUIT with reason
echo -e "${BOLD}${YELLOW}Test 2:${NC} QUIT with reason"
echo -e "${BLUE}Expected:${NC} User disconnects with reason 'Goodbye!'"
(printf 'PASS pass\r\nNICK q2u2%s\r\nUSER q2u2 0 0 :User Two\r\nQUIT :Goodbye!\r\n' "$SUFFIX") | nc -q1 localhost 6667 &
sleep 0.5

# Test 3: QUIT with long reason
echo -e "${BOLD}${YELLOW}Test 3:${NC} QUIT with long reason"
echo -e "${BLUE}Expected:${NC} User disconnects with full reason message"
(printf 'PASS pass\r\nNICK q3u3%s\r\nUSER q3u3 0 0 :User Three\r\nQUIT :Leaving because I need to sleep\r\n' "$SUFFIX") | nc -q1 localhost 6667 &
sleep 0.5

# Test 4: QUIT broadcast to channel members
echo -e "${BOLD}${YELLOW}Test 4:${NC} QUIT broadcast to channel"
echo -e "${BLUE}Expected:${NC} user2 receives QUIT notification from user1"
# user1 joins and will QUIT
{ printf 'PASS pass\r\nNICK q4u1%s\r\nUSER q4u1 0 0 :User One\r\nJOIN #quittest\r\n' "$SUFFIX"; sleep 1; printf 'QUIT :Bye everyone!\r\n'; sleep 0.5; } | nc localhost 6667 &
sleep 0.3
# user2 joins and should see QUIT message
{ printf 'PASS pass\r\nNICK q4u2%s\r\nUSER q4u2 0 0 :User Two\r\nJOIN #quittest\r\n' "$SUFFIX"; sleep 2; } | nc localhost 6667 &
sleep 1

# Test 5: QUIT removes user from all channels
echo -e "${BOLD}${YELLOW}Test 5:${NC} QUIT removes from multiple channels"
echo -e "${BLUE}Expected:${NC} User removed from #ch1 and #ch2"
(printf 'PASS pass\r\nNICK q5u5%s\r\nUSER q5u5 0 0 :User Five\r\nJOIN #ch1\r\nJOIN #ch2\r\nQUIT\r\n' "$SUFFIX") | nc -q1 localhost 6667 &
sleep 0.5

# Test 6: QUIT deletes empty channel
echo -e "${BOLD}${YELLOW}Test 6:${NC} QUIT - empty channel cleanup"
echo -e "${BLUE}Expected:${NC} Channel #tempchan deleted after last user QUITs"
(printf 'PASS pass\r\nNICK q6u6%s\r\nUSER q6u6 0 0 :User Six\r\nJOIN #tempchan\r\nQUIT\r\n' "$SUFFIX") | nc -q1 localhost 6667 &
sleep 0.5

# Test 7: Multiple users, one QUITs
echo -e "${BOLD}${YELLOW}Test 7:${NC} Multiple users - one QUITs"
echo -e "${BLUE}Expected:${NC} user8 sees user7 QUIT"
# user1 joins, sends QUIT
{ printf 'PASS pass\r\nNICK q7u7%s\r\nUSER q7u7 0 0 :User Seven\r\nJOIN #multitest\r\n' "$SUFFIX"; sleep 2; printf 'QUIT :Leaving now\r\n'; sleep 0.5; } | nc localhost 6667 &
sleep 0.3
# user2 joins after user1, should see QUIT
{ printf 'PASS pass\r\nNICK q7u8%s\r\nUSER q7u8 0 0 :User Eight\r\nJOIN #multitest\r\n' "$SUFFIX"; sleep 3; } | nc localhost 6667 &
sleep 1

# Test 8: QUIT with colon but empty reason
echo -e "${BOLD}${YELLOW}Test 8:${NC} QUIT with empty reason"
echo -e "${BLUE}Expected:${NC} User disconnects with empty reason"
(printf 'PASS pass\r\nNICK q8u8%s\r\nUSER q8u8 0 0 :User Eight\r\nQUIT :\r\n' "$SUFFIX") | nc -q1 localhost 6667 &
sleep 0.5

# Test 9: QUIT with reason containing spaces
echo -e "${BOLD}${YELLOW}Test 9:${NC} QUIT with multi-word reason"
echo -e "${BLUE}Expected:${NC} Full reason preserved"
(printf 'PASS pass\r\nNICK q9u9%s\r\nUSER q9u9 0 0 :User Nine\r\nQUIT :This is a longer reason with spaces\r\n' "$SUFFIX") | nc -q1 localhost 6667 &
sleep 0.5

# Test 10: QUIT then reconnect with same nick
echo -e "${BOLD}${YELLOW}Test 10:${NC} QUIT and reconnect"
echo -e "${BLUE}Expected:${NC} Can reconnect with same nickname"
(printf 'PASS pass\r\nNICK q10u%s\r\nUSER q10u 0 0 :User Ten\r\nJOIN #reconnect\r\nQUIT\r\n' "$SUFFIX") | nc -q1 localhost 6667 &
sleep 0.5
(printf 'PASS pass\r\nNICK q10u%s\r\nUSER q10u 0 0 :User Ten Again\r\nJOIN #reconnect\r\n' "$SUFFIX") | nc -q1 localhost 6667 &
sleep 0.5

echo ""
echo -e "${BOLD}${CYAN}========================================${NC}"
echo -e "${GREEN}All tests launched!${NC}"
echo -e "${BLUE}Check server.log for detailed output${NC}"
echo -e "${BOLD}${CYAN}========================================${NC}"
echo ""

exit 0
