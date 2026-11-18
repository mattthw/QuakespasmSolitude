#!/bin/sh
# Solitude dedicated server entrypoint.
#
# Data layout expected under /data (mount it read-only):
#   /data/id1/pak0.pak        the Quake base data
#   /data/Solitude/           the runtime game dir (progs.dat, maps/, gfx/ ...)
#
# "Solitude" is capitalised on purpose: the engine registers that game dir by
# name at startup (common.c COM_AddGameDirectory("Solitude")), and on a
# case-sensitive filesystem a lowercase directory is simply not found. Adding
# `-game solitude` instead would advertise the list "Solitude;solitude", which
# older clients report as a gamedir mismatch, so the directory name is the one
# thing that has to match.
#
# Env:  MAP   map to load on boot        (default highground, empty = startmap)
#       PORT  udp port                   (default 26000)
#       GAME  extra game dir to add      (default none)
set -e

MAP="${MAP-highground}"
PORT="${PORT:-26000}"
GAME_DIR=Solitude

if [ ! -e /data/id1/pak0.pak ]; then
  echo "solitude: /data/id1/pak0.pak not found." >&2
  echo "solitude: mount the game base dir at /data (needs id1/ and $GAME_DIR/)." >&2
  exit 1
fi
if [ ! -e "/data/$GAME_DIR/progs.dat" ]; then
  echo "solitude: /data/$GAME_DIR/progs.dat not found." >&2
  echo "solitude: the game dir must be mounted as /data/$GAME_DIR (capital S)." >&2
  exit 1
fi

set -- /opt/solitude/quakespasm \
  -dedicated \
  -basedir /data \
  -port "$PORT" \
  -nolauncher \
  -condebug

if [ -n "$GAME" ]; then
  set -- "$@" -game "$GAME"
fi

if [ -n "$MAP" ]; then
  set -- "$@" +map "$MAP"
fi

echo "solitude: starting '${MAP:-startmap}' on udp/$PORT (server.cfg is exec'd from the game dir)"

# Line-buffer stdout: Sys_Printf is printf(), which is block-buffered when it
# is not a tty, so `docker logs` would otherwise stay silent for minutes.
exec stdbuf -oL -eL "$@"
