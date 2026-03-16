#!/bin/bash

# Script to open two terminal windows and connect to nc localhost:6667
# Each terminal sends initial PASS/NICK/USER/JOIN commands then keeps the connection interactive.

TERMS=("gnome-terminal" "konsole" "xfce4-terminal" "xterm" "mate-terminal" "alacritty" "tilix")

CMD1="(printf 'pass pass\r\nnick aaaa\r\nuser AAAAA 0 0 :Aaaaaa\r\njoin #test\r\n'; cat) | nc localhost 6667"
CMD2="(printf 'pass pass\r\nnick bbbb\r\nuser BBBBB 0 0 :Bbbbb\r\njoin #test\r\n'; cat) | nc localhost 6667"

TERM_BIN=""
for t in "${TERMS[@]}"; do
  if command -v "$t" >/dev/null 2>&1; then
    TERM_BIN="$t"
    break
  fi
done

if [ -z "$TERM_BIN" ]; then
  echo "No supported terminal emulator found. Please install xterm or gnome-terminal, or run the commands manually."
  exit 1
fi

case "$TERM_BIN" in
  gnome-terminal|tilix)
    "$TERM_BIN" -- bash -lc "$CMD1" &
    sleep 0.2
    "$TERM_BIN" -- bash -lc "$CMD2" &
    ;;
  konsole)
    "$TERM_BIN" -e bash -lc "$CMD1" &
    sleep 0.2
    "$TERM_BIN" -e bash -lc "$CMD2" &
    ;;
  xfce4-terminal|mate-terminal|alacritty|xterm)
    "$TERM_BIN" -e bash -lc "$CMD1" &
    sleep 0.2
    "$TERM_BIN" -e bash -lc "$CMD2" &
    ;;
  *)
    xterm -e bash -lc "$CMD1" &
    sleep 0.2
    xterm -e bash -lc "$CMD2" &
    ;;
esac

exit 0
