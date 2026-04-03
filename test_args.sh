#!/bin/bash

SERVER=./ircserv

BASE_PORT=2000
PORT_OFFSET=0
BASE_PORT=2000
PORT_OFFSET=0
TESTS_PASSED=0
TESTS_FAILED=0

get_port() {
    PORT=$((BASE_PORT + PORT_OFFSET))
    PORT_OFFSET=$((PORT_OFFSET + 1))
    echo $PORT
}

run_test() {
    DESC=$1
    CMD=$2

    echo "TEST: $DESC"
    echo "Command: $CMD"

    OUTPUT=$($CMD 2>&1)
    STATUS=$?

    echo "Exit code: $STATUS"
    echo "Output: $OUTPUT"
    echo "--------------------"

    sleep 0.2
}

run_test_expect_fail() {
    DESC=$1
    CMD=$2

    echo "TEST: $DESC"
    echo "Command: $CMD"

    OUTPUT=$(timeout 2 bash -c "$CMD" 2>&1)
    STATUS=$?

    if [ $STATUS -eq 124 ]; then
        echo "❌ FAILED (command timed out - likely didn't exit properly)"
        ((TESTS_FAILED++))
    elif [ $STATUS -eq 0 ]; then
        echo "❌ FAILED (should not succeed)"
        ((TESTS_FAILED++))
    else
        echo "✅ PASSED (correctly failed)"
        ((TESTS_PASSED++))
    fi

    echo "Exit code: $STATUS"
    echo "Output:"
    echo "$OUTPUT"
    echo "-------------------------------------------------------------"

    sleep 0.2
}

run_test() {
    DESC=$1
    CMD=$2

    echo "TEST: $DESC"
    echo "Command: $CMD"

    OUTPUT=$($CMD 2>&1)
    STATUS=$?

    echo "Exit code: $STATUS"
    echo "Output: $OUTPUT"
    ((TESTS_PASSED++))
    echo "--------------------"

    sleep 0.2
}


echo "========== IRC SERVER ARGUMENT TESTS =========="

# =========================
# VALID CASE
# =========================
PORT=$(get_port)
run_test "Valid input" "timeout 2 $SERVER $PORT 123"

# =========================
# MISSING ARGUMENTS
# =========================
run_test_expect_fail "No arguments" "$SERVER"
run_test_expect_fail "Only port" "$SERVER 2000"

# =========================
# INVALID PORTS
# =========================
run_test_expect_fail "Port is string" "$SERVER abc 123"
run_test_expect_fail "Negative port" "$SERVER -1 123"
run_test_expect_fail "Port too large" "$SERVER 70000 123"

# =========================
# PASSWORD EDGE CASES
# =========================
PORT=$(get_port)
run_test_expect_fail "Empty password" "$SERVER $PORT ''"

PORT=$(get_port)
run_test_expect_fail "Spaces-only password" "$SERVER $PORT '   '"

PORT=$(get_port)
run_test "Password with symbols" "timeout 1 $SERVER $PORT 'p@ss!#\$%^&*()'"

# =========================
# PASSWORD WITH SPACES
# =========================
PORT=$(get_port)
run_test "Password with spaces (quoted)" "timeout 1 $SERVER $PORT 'my password'"

PORT=$(get_port)
run_test_expect_fail "Password with spaces (not quoted)" "$SERVER $PORT my password"


# ...existing code...

echo "========== TESTING FINISHED =========="
echo ""
echo "📊 TEST RESULTS:"
echo "✅ Passed: $TESTS_PASSED"
echo "❌ Failed: $TESTS_FAILED"
echo "📈 Total: $((TESTS_PASSED + TESTS_FAILED))"
echo "========== TESTING FINISHED =========="