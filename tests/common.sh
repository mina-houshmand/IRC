#!/bin/bash

SERVER_HOST="${SERVER_HOST:-127.0.0.1}"
SERVER_PORT="${SERVER_PORT:-6667}"
SERVER_PASS="${SERVER_PASS:-pass}"
TEST_CHANNEL="${TEST_CHANNEL:-#test}"

irc_step() {
    case "$1" in
        sleep:*)
            sleep "${1#sleep:}"
            ;;
        *)
            printf '%s\r\n' "$1"
            ;;
    esac
}

run_client() {
    local label="$1"
    shift

    {
        while [ "$#" -gt 0 ]; do
            irc_step "$1"
            shift
        done
    } | nc -q1 "$SERVER_HOST" "$SERVER_PORT" | sed -u "s/^/[$label] /"
}

irc_auth() {
    local nick="$1"
    local user="$2"
    local real="$3"

    printf 'PASS %s\r\n' "$SERVER_PASS"
    printf 'NICK %s\r\n' "$nick"
    printf 'USER %s 0 0 :%s\r\n' "$user" "$real"
}
