#!/usr/bin/env bash
# start_nc_wsl.sh
# Usage: run this inside WSL (bash) or via `wsl` from PowerShell.
# This sends the handshake, joins #test, and keeps the socket open
# so you can continue typing IRC commands interactively.

NC_HOST="localhost"
NC_PORT=6667

{
	printf 'PASS pass\r\n'
	printf 'NICK dddd\r\n'
	printf 'USER d1111 0 0 d2222\r\n'
	printf 'JOIN #test\r\n'
	cat
} | nc "$NC_HOST" "$NC_PORT"
