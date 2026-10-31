#!/bin/sh
# Runs the game with a hard wall-clock limit (SIGTERM after $LIMIT s, SIGKILL 2 s later) so a hang never leaves a window behind.
# usage: tools/safe_run.sh [LIMIT_SECONDS] -- ./build/megamix args...      (default 25 s)
LIMIT=${1:-25}; shift; [ "$1" = "--" ] && shift
# Scripted runs must not open a window over the user's screen or take their keyboard. SHOW_WINDOW=1 tools/safe_run.sh ... shows it.
[ -n "$SHOW_WINDOW" ] || export MEGAMIX_HIDDEN=1
exec timeout -k 2 "$LIMIT" "$@"
