#!/bin/bash
# tools/mac.sh — quick path to build / launch / host the Vita-fork QuakeSpasm on macOS.
#
# Lives in the repo so the recipes are versioned with the source. Everything it
# touches outside git (the app bundle, the Vita SDK, the 0.97 frameworks it
# borrows) sits in the workspace root, derived below — override with
# SOLITUDE_ROOT=/path/to/projects-vita if the tree moves again.
#
# The fork (quakespasm) ships Makefile.vita and a stock Makefile.darwin, but
# building the darwin target for THIS fork needs an extra untracked layer
# created 2026-11-11:
#   - vgl_shim_host.h       (Vita API no-op stubs, injected via -include)
#   - vita_cvars_host.c    (5 Vita cvars whose defs are #ifdef VITA)
#   - gl_hud.o              (Makefile.darwin doesn't list it; Makefile.vita does)
#   - tracked guarded edits: main_sdl.c (argc/argv seed), sbar.c (HUD_Draw off)
# Codec dylibs and SDL2.framework are borrowed from the official 0.97 app.
#
# Usage (from anywhere):
#   ./mac.sh launch [map]          # start Solitude.app (main menu, -game solitude)
#   ./mac.sh server [map] [port]   # SolitudeServer, dedicated, in this terminal
#   ./mac.sh build                 # rebuild + relink + deploy + sign (auto-cleans
#                                  #   first — stale ARM/mismatched-arch .o silently
#                                  #   break links; make won't notice flag changes)
#   ./mac.sh vita                  # PS Vita build (vitasdk): quakehalo.vpk +
#                                  #   eboot.bin, staged + sha256 in dist/staging/
#   ./mac.sh all [map]             # ONE-SHOT FLOW: mac build+deploy -> vita vpk -> launch
#   ./mac.sh stock [map]           # launch official 0.97 instead (known: / cycles,
#                                  #   spam of Unknown cl_ww/cl_scope/cl_ch_red)
set -e

# Resolve symlinks first: the convenience link at the workspace root points
# here, and ROOT has to be derived from the real location, not the link's.
SELF="$0"
while [ -L "$SELF" ]; do
  LINK="$(readlink "$SELF")"
  case "$LINK" in
    /*) SELF="$LINK" ;;
    *)  SELF="$(dirname "$SELF")/$LINK" ;;
  esac
done
REPO="$(cd "$(dirname "$SELF")/.." && pwd)"          # <repo>/
ROOT="${SOLITUDE_ROOT:-$(cd "$REPO/../.." && pwd)}"  # <workspace>/ (outside git)

ENG="$REPO/quakespasm/Quake"
STOCK="$ROOT/toolchain/quakespasm-0.97.0/QuakeSpasm-SDL2.app"
BUNDLE="$ROOT/toolchain/fork-engine-mac/Solitude.app"
CLIENT_BIN="$BUNDLE/Contents/MacOS/Solitude"
SERVER_DIR="$ROOT/toolchain/fork-engine-mac/server"
SERVER_BIN="$SERVER_DIR/SolitudeServer"   # deliberately OUTSIDE the .app: a
                                          # bundled executable is a GUI app
                                          # (dock tile + "Solitude" window)
FW="$STOCK/Contents/Frameworks"
CODECLIB="$REPO/quakespasm/MacOSX/codecs/lib"
OPUSINC="$REPO/quakespasm/MacOSX/codecs/include/opus"

CF="-arch x86_64 -Wall -Wno-error=return-mismatch -Wno-return-mismatch -Wno-unused-function -include vgl_shim_host.h -I$OPUSINC -DMAD_F_64"

launch() {
  pkill -x Solitude 2>/dev/null || true
  rm -f "$REPO/solitude/mapshot.cfg"   # r94: normal launches must not inherit photo mode (flag was left by mapshot())
  # r94: start at the MAIN MENU (no auto +map) and force hud_mapshot 0 so a
  # leftover photo-mode cvar from a prior mapshot run can never hide the HUD.
  open "$BUNDLE" --args -nolauncher -basedir "$REPO" -game solitude \
    -window -condebug +exec config-mac.cfg +hud_mapshot 0 "$@"
  echo "Solitude launched: main menu (log: $BUNDLE/../qconsole.log)"
}

mapshot() {   # photo mode armed at the main menu: no bots, HUD hidden once a map loads
    pkill -x Solitude 2>/dev/null || true
    sleep 2
    # r92: no longer auto-loads a map (user ask) — start any map from the menu;
    # mapshot.cfg still arms sv_cheats + hud_mapshot, so the spawn is a botless
    # real deathmatch (weapon in hand) and noclip/fly pass the r89 gates.
    # r88: +exec of NEW loose files gets token-mangled at startup (mapshot.cfg ->
    # 'mapshot.c'); piggyback on config-mac.cfg which ends with a plain
    # 'exec mapshot.cfg' (silently fails when absent; launch() deletes it).
    touch "$REPO/solitude/mapshot.cfg"
    open "$BUNDLE" --args -nolauncher -basedir "$REPO" -game solitude \
        -window -condebug +exec config-mac.cfg "$@"
    echo "Solitude launched (mapshot): pick a map, fly (noclip 1 if you want it), ',' saves a screenshot to solitude/spasmNNNN.png, hud back on: hud_mapshot 0 (log: $BUNDLE/../qconsole.log)"
}

server() {   # dedicated host in THIS terminal: ./mac.sh server [map] [port]
  local map="${1:-highground}"
  local port="${2:-26000}"
  pkill -f "MacOS/SolitudeServer -dedicated" 2>/dev/null || true
  cd "$REPO"
  echo "SolitudeServer: map=$map  udp/$port  (Ctrl-C stops; log: $ROOT/toolchain/fork-engine-mac/qconsole.log)"
  # server.cfg is auto-exec'd by Host_Init in dedicated mode, so no +exec here.
  # A tty is required or the console reader spins on EOF and floods the log with
  # "Console input too long!"; when the caller has no tty (nohup, agent shells)
  # script(1) supplies one.
  # NOTE: no `-game solitude` -- the engine already adds the Solitude game dir
  # unconditionally (common.c COM_AddGameDirectory("Solitude")), and passing it
  # again made the advertised game-dir list "Solitude;solitude", which tripped
  # the r123 gamedir warning on every client connect.
  # SDL_VIDEODRIVER=dummy: a dedicated host has no video, and without this SDL
  # still attaches to the WindowServer -- macOS then registers the process as a
  # GUI app inside Solitude.app and puts a "Solitude" window/dock tile on screen.
  if [ -t 0 ]; then
    SDL_VIDEODRIVER=dummy exec "$SERVER_BIN" -dedicated -basedir "$REPO" -port "$port" -nolauncher -condebug +map "$map"
  else
    SDL_VIDEODRIVER=dummy exec script -q /dev/null "$SERVER_BIN" -dedicated -basedir "$REPO" -port "$port" -nolauncher -condebug +map "$map"
  fi
}

build() {
  cd "$ENG"
  make -f Makefile.darwin clean >/dev/null 2>&1 || true
  rm -f *.o quakespasm        # mandatory: ARM or stale-arch objects link as garbage
  CFLAGS="$CF" make -f Makefile.darwin USE_SDL2=1 USE_ZLIB=0 USE_CODEC_OPUS=0 \
    SDL_FRAMEWORK_PATH="$FW" -j8 || true   # gl_hud/vita_cvars absent from makefile;
                                           # vendored libopus.dylib is DECODE-ONLY,
                                           # so VOIP encode must stay compiled out
  cc -c gl_hud.c -o gl_hud.o $CF -DUSE_SDL2 -DSDL_FRAMEWORK \
    -mmacosx-version-min=10.14 -F"$FW"
  cc -c vita_cvars_host.c -o vita_cvars_host.o $CF -DUSE_SDL2 -DSDL_FRAMEWORK \
    -mmacosx-version-min=10.14 -F"$FW"
  cc -c gl_font.c -o gl_font.o $CF -DUSE_SDL2 -DSDL_FRAMEWORK \
    -mmacosx-version-min=10.14 -F"$FW"
  rm -rf obj && mkdir obj && cp *.o obj/ && rm -f obj/quakespasm.o
  cc -o quakespasm obj/*.o -arch x86_64 \
    -mmacosx-version-min=10.14 -fobjc-arc \
    -Wl,-rpath,@executable_path/../Frameworks -Wl,-rpath,@executable_path \
    -L"$CODECLIB" -lFLAC -lvorbisfile -lvorbis -logg -lmad -lmikmod -lopusfile -lopus \
    -F"$FW" -Wl,-framework,SDL2 -Wl,-framework,Cocoa -Wl,-framework,OpenGL \
    -Wl,-framework,IOKit -Wl,-framework,AudioToolbox -Wl,-framework,ForceFeedback -lm
  test -x quakespasm || { echo "link produced no binary"; exit 1; }
  cp quakespasm "$CLIENT_BIN"
  rm -f "$BUNDLE/Contents/MacOS/SolitudeServer"   # never bundle the server

  # Dedicated host: a real CLI program. Two things make that work:
  #  - main_sdl.c is recompiled with -DSOLITUDE_CLI so `main` stays `main`
  #    instead of being renamed to SDL_main behind the Cocoa launcher;
  #  - the Cocoa launcher objects (SDLMain/AppController/SDLApplication/
  #    QuakeArgument(s)/ScreenInfo) are left out of the link.
  # Result: no NSApplicationMain, no dock tile, no "Solitude" window -- just a
  # process that prints to the terminal. It lives outside Solitude.app because a
  # bundled executable is adopted by the bundle's GUI identity.
  cc -c main_sdl.c -o main_sdl_cli.o $CF -DSOLITUDE_CLI -DUSE_SDL2 -DSDL_FRAMEWORK \
    -mmacosx-version-min=10.14 -F"$FW"
  mkdir -p "$SERVER_DIR" "$ROOT/toolchain/fork-engine-mac/Frameworks"
  for f in "$BUNDLE"/Contents/MacOS/lib*.dylib; do
    ln -sfn "../Solitude.app/Contents/MacOS/$(basename "$f")" "$SERVER_DIR/$(basename "$f")"
  done
  ln -sfn ../Solitude.app/Contents/Frameworks/SDL2.framework \
    "$ROOT/toolchain/fork-engine-mac/Frameworks/SDL2.framework"
  SRV_OBJS=$(ls obj/*.o | grep -vE '/(main_sdl|SDLMain|AppController|SDLApplication|QuakeArgument|QuakeArguments|ScreenInfo)\.o$')
  cc -o "$SERVER_BIN" $SRV_OBJS main_sdl_cli.o -arch x86_64 \
    -mmacosx-version-min=10.14 -fobjc-arc \
    -Wl,-rpath,@executable_path/../Frameworks -Wl,-rpath,@executable_path \
    -L"$CODECLIB" -lFLAC -lvorbisfile -lvorbis -logg -lmad -lmikmod -lopusfile -lopus \
    -F"$FW" -Wl,-framework,SDL2 -Wl,-framework,Cocoa -Wl,-framework,OpenGL \
    -Wl,-framework,IOKit -Wl,-framework,AudioToolbox -Wl,-framework,ForceFeedback -lm
  test -x "$SERVER_BIN" || { echo "server link produced no binary"; exit 1; }
  chmod +x "$CLIENT_BIN" "$SERVER_BIN"

  codesign --force -s - --identifier solitude.server "$SERVER_BIN" 2>/dev/null || true
  codesign --force -s - --identifier solitude.game "$CLIENT_BIN" 2>/dev/null || true
  codesign --force -s - --identifier solitude.game "$BUNDLE" 2>/dev/null || true
  xattr -cr "$BUNDLE"
  echo "deployed: $(md5 -q "$CLIENT_BIN") $(stat -f%z "$CLIENT_BIN") bytes -> Solitude.app (client)"
  echo "deployed: $(md5 -q "$SERVER_BIN") $(stat -f%z "$SERVER_BIN") bytes -> server/SolitudeServer (dedicated, CLI)"
}

vita() {
  cd "$ENG"
  make -f Makefile.vita clean >/dev/null 2>&1 || true
  rm -f *.o quakehalo.elf quakehalo.vpk   # x86_64 leftovers poison the ARM link
  # B8 PARKED 2026-11-14: back to the 2022 stack (old SDK + 2022 vitaGL
  # archive 960affbe). Modern vitaGL cannot boot this device (issue #49).
  # The 2026.08 migration lives in git history (d7df6b27) + toolchain backups.
  VITASDK=/usr/local/vitasdk PATH=/usr/local/vitasdk/bin:$PATH make -f Makefile.vita -j8 >/dev/null
  test -f quakehalo.vpk || { echo "vita build produced no vpk"; exit 1; }
  STAGE="$REPO/dist/staging"
  mkdir -p "$STAGE"
  cp quakehalo.vpk build/eboot.bin "$STAGE/"
  (cd "$STAGE" && shasum -a 256 quakehalo.vpk eboot.bin > SHA256SUMS-staging.txt)
  # B8: record exact toolchain provenance — vitaGL ships no version macro,
  # so pin git commit + installed archive md5s + build flags per output vpk.
  {
    echo "vitasdk: $(cat /usr/local/vitasdk/version_info.txt 2>/dev/null | head -1) (/usr/local/vitasdk, 2022 stack)"
    echo "libvitaGL.a $(md5 -q /usr/local/vitasdk/arm-vita-eabi/lib/libvitaGL.a) (0a4b031 era; new-stack archives in /usr/local/vitasdk-2026.08 + toolchain backups)"
    echo "libvitashark.a $(md5 -q /usr/local/vitasdk/arm-vita-eabi/lib/libvitashark.a)"
    echo "quakehalo.vpk $(md5 -q quakehalo.vpk)"
  } > "$STAGE/TOOLCHAIN.lock"
  echo "      provenance -> $STAGE/TOOLCHAIN.lock"
  echo "vita: quakehalo.vpk $(md5 -q quakehalo.vpk) $(stat -f%z quakehalo.vpk) bytes"
  echo "      staged -> $STAGE/ (+ SHA256SUMS-staging.txt)"
}

case "${1:-launch}" in
  launch)    shift; if [ $# -gt 0 ]; then m="$1"; shift; else m="highground"; fi; launch "$m" "$@" ;;
  mapshot)   shift; mapshot "$@" ;;
  server)    shift; server "${1:-highground}" "${2:-26000}" ;;
  build)     build ;;
  cleanbuild) build ;;
  vita)      vita ;;
  all)       shift; build; vita; launch "${1:-highground}" ;;
  stock)     shift; pkill -x QuakeSpasm 2>/dev/null || true
             open "$STOCK" --args -basedir "$REPO" -game solitude +map "${1:-highground}" -window +exec config-mac.cfg ;;
  *) echo "usage: $0 [launch|build|vita|all|stock|server] [map]"; exit 1 ;;
esac
