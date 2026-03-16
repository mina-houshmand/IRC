#!/bin/bash

# Master Test Script - Runs all IRC command tests
# Shows both sender and observer/receiver views in your current terminal
# Output saved to files WITHOUT ANSI codes for easy viewing in VS Code

# ANSI Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color
BOLD='\033[1m'

echo -e "${BOLD}${CYAN}========================================${NC}"
echo -e "${BOLD}${CYAN}   IRC Command Test Suite - All Tests${NC}"
echo -e "${BOLD}${CYAN}========================================${NC}"
echo ""
echo -e "${YELLOW}Each test shows:${NC}"
echo "  - ${GREEN}SENDER${NC}: What the command sender sees"
echo "  - ${BLUE}OBSERVER${NC}: What other connected users see (broadcasts)"
echo ""
echo -e "${BLUE}Starting all tests...${NC}"
echo ""

# Run all test scripts
echo -e "${CYAN}========== PART TESTS ==========${NC}"
echo ""
/mnt/c/42/irc_git/scripts/test_PART_interactive.sh
echo ""
echo -e "${CYAN}========== KICK TESTS ==========${NC}"
echo ""
/mnt/c/42/irc_git/scripts/test_KICK_interactive.sh
echo ""
echo -e "${CYAN}========== MODE TESTS ==========${NC}"
echo ""
/mnt/c/42/irc_git/scripts/test_MODE_interactive.sh
echo ""
echo -e "${CYAN}========== PRIVMSG TESTS ==========${NC}"
echo ""
/mnt/c/42/irc_git/scripts/test_PRIVMSG_interactive.sh
echo ""
echo -e "${CYAN}========== QUIT TESTS ==========${NC}"
echo ""
/mnt/c/42/irc_git/scripts/test_QUIT_interactive.sh
echo ""
echo -e "${CYAN}========== TOPIC TESTS ==========${NC}"
echo ""
/mnt/c/42/irc_git/scripts/test_TOPIC_interactive.sh

echo ""
echo -e "${BOLD}${CYAN}========================================${NC}"
echo -e "${BOLD}${GREEN}All tests complete!${NC}"
echo -e "${BOLD}${CYAN}========================================${NC}"
echo ""
echo -e "${BLUE}Output files (clean, no ANSI codes):${NC}"
echo ""
echo -e "  ${BOLD}C:\\42\\irc_git\\scripts\\test_PART_output.txt${NC}"
echo -e "  ${BOLD}C:\\42\\irc_git\\scripts\\test_KICK_output.txt${NC}"
echo -e "  ${BOLD}C:\\42\\irc_git\\scripts\\test_MODE_output.txt${NC}"
echo -e "  ${BOLD}C:\\42\\irc_git\\scripts\\test_PRIVMSG_output.txt${NC}"
echo -e "  ${BOLD}C:\\42\\irc_git\\scripts\\test_QUIT_output.txt${NC}"
echo -e "  ${BOLD}C:\\42\\irc_git\\scripts\\test_TOPIC_output.txt${NC}"
echo ""
echo -e "${BLUE}Open in VS Code: Ctrl+O → navigate to C:\\42\\irc_git\\scripts\\${NC}"
echo ""

exit 0
