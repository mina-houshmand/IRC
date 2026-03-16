#!/usr/bin/env bash
# start_nc_wsl.sh
# Usage: run this inside WSL (bash) or via `wsl` from PowerShell.
# This sends a small IRC handshake and joins #test on localhost:6667.

NC_HOST="localhost"
NC_PORT=6667

nc "$NC_HOST" "$NC_PORT" <<'IRC_CMDS'
PASS pass
NICK dddd
USER d1111 0 0 d2222
JOIN #test
IRC_CMDS
