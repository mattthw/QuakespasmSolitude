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
#       RCON_PASSWORD  set rcon_password  (default: rcon disabled)
#       SV_PUBLIC      1 = dpmaster heartbeat (default 0, LAN only)
#       EXTRA_ARGS     any extra args, e.g. "+set timelimit 20"
#
# No -condebug on purpose: it writes qconsole.log into the basedir, which is
# mounted read-only, and the container prints one alarming "Error: Unable to
# create log file" for it. `docker logs` already has everything.
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

# MAP=random (or MAP=cycle) starts on a random map from the rotation chain
# instead of always the same one. The chain is the `set <map> <nextmap>` block
# in server.cfg; keys that look like cvars (sv_*, lobby_*) are not maps, and a
# candidate is only accepted if its .bsp is actually there.
case "$MAP" in
  random|cycle)
    # r165: awk's srand() seeds from the clock in whole seconds, so every
    # container created by one `compose up` (same second) rolled the SAME
    # map. Seed from /dev/urandom so each instance is independent.
    # r174 (user: "all servers are running babybloodgulch, is that luck"):
    # not luck -- mawk's rand() right after srand() with a large seed is
    # badly clustered (5 of 7 random seeds picked the same map on the NAS).
    # With a urandom seed, `seed % n` is uniform; skip the PRNG entirely.
    MAP_SEED=$(od -An -N4 -tu4 /dev/urandom 2>/dev/null | tr -d ' \n')
    [ -n "$MAP_SEED" ] || MAP_SEED="$$"
    MAP=$(awk -v seed="$MAP_SEED" '
      /^[[:space:]]*set[[:space:]]+/ {
        k = $2
        if (k ~ /^(sv_|lobby_)/) next
        if (!(k in seen)) { seen[k] = 1; maps[n++] = k }
      }
      END { if (n > 0) print maps[seed % n] }
    ' "/data/$GAME_DIR/server.cfg" 2>/dev/null)
    if [ -z "$MAP" ] || [ ! -f "/data/$GAME_DIR/maps/$MAP.bsp" ]; then
      MAP=highground
    fi
    ;;
esac

set -- /opt/solitude/quakespasm \
  -dedicated \
  -basedir /data \
  -port "$PORT" \
  -nolauncher

if [ -n "$GAME" ]; then
  set -- "$@" -game "$GAME"
fi

# rcon: the protocol sends the password in the clear with no rate limiting, so
# a public server needs a long random one. Anything passed here comes from the
# orchestrator, never from the image: RCON_PASSWORD="" leaves rcon disabled
# (the server answers "rcon is not enabled on this server").
if [ -n "${RCON_PASSWORD:-}" ]; then
  set -- "$@" +set rcon_password "$RCON_PASSWORD"
fi

# sv_public 1 publishes the server to the dpmaster list every 5 minutes.
if [ -n "${SV_PUBLIC:-}" ]; then
  set -- "$@" +set sv_public "$SV_PUBLIC"
fi

# Escape hatch for any other cvar, e.g. EXTRA_ARGS="+set timelimit 20".
# Word splitting is intended here, globbing is not.
if [ -n "${EXTRA_ARGS:-}" ]; then
  set -f
  set -- "$@" $EXTRA_ARGS
  set +f
fi

if [ -n "$MAP" ]; then
  set -- "$@" +map "$MAP"
fi

echo "solitude: starting '${MAP:-startmap}' on udp/$PORT (server.cfg is exec'd from the game dir)"

# Line-buffer stdout: Sys_Printf is printf(), which is block-buffered when it
# is not a tty, so `docker logs` would otherwise stay silent for minutes.
exec stdbuf -oL -eL "$@"
