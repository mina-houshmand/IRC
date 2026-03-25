#!/bin/bash

# JOIN 0 Test Script
# Tests if JOIN 0 effectively forces the user to part from all channels

OUTPUT_FILE="/mnt/c/42/irc_git/scripts/test_JOIN_0_output.txt"

echo "========================================"
echo "       JOIN 0 Command Test Suite"
echo "========================================"
echo ""

> "$OUTPUT_FILE"

log() {
    echo "$1" | tee -a "$OUTPUT_FILE"
}

log "TEST: JOIN 0 parts all channels"
log ""

{
    printf 'PASS pass\r\n'
    printf 'NICK j0test\r\n'
    printf 'USER j0test 0 0 :j0test\r\n'
    # Join a few channels
    printf 'JOIN #channelA\r\n'
    sleep 0.2
    printf 'JOIN #channelB\r\n'
    sleep 0.2
    printf 'JOIN #channelC\r\n'
    sleep 0.2
    
    # Issue the JOIN 0 command
    printf 'JOIN 0\r\n'
    sleep 0.5
    
    # Try to send a message to one of the channels to prove we aren't in it
    printf 'PRIVMSG #channelA :Hello?\r\n'
    sleep 0.2
    
    printf 'QUIT\r\n'
} | nc -q1 localhost 6667 2>&1 | tee -a "$OUTPUT_FILE"

log ""
log "If JOIN 0 worked, you should see PART broadcasts for channelA, channelB, and channelC."
log "Also, PRIVMSG #channelA should return ERR_CANNOTSENDTOCHAN (404) or ERR_NOSUCHCHANNEL (403)"
log ""
log "Finished testing JOIN 0!"