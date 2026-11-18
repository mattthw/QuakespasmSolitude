# Solitude dedicated server (Linux container)

Runs the Solitude fork as a headless dedicated Quake server. No X server, no
window: the binary still initialises SDL, so the image sets
`SDL_VIDEODRIVER=dummy` / `SDL_AUDIODRIVER=dummy`.

## Build

```sh
./docker/build.sh                     # native arch of the build machine
ARCH=linux/amd64 ./docker/build.sh    # cross-build (arm64 Mac -> amd64 host)
ARCH=linux/arm64 ./docker/build.sh    # e.g. for an arm SBC
```

The build stage installs `build-essential` + `libsdl2-dev` and compiles the
engine with the generic unix Makefile, all codecs off:

```sh
make -f Makefile USE_SDL2=1 USE_ZLIB=0 \
     USE_CODEC_WAVE=0 USE_CODEC_MP3=0 USE_CODEC_VORBIS=0 USE_CODEC_OPUS=0
```

## Game data

The container does not ship game data. Mount a directory containing:

```
/data/id1/pak0.pak            (Quake registered data; pak0+pak1 also work)
/data/Solitude/progs.dat      (Solitude QuakeC)
/data/Solitude/...            (config.cfg, maps/, gfx/, sound/, *.way, ...)
```

`Solitude` **must keep its capital S**. The engine adds that game dir by name
at startup and Linux paths are case sensitive; passing `-game solitude`
instead produces a duplicated game-dir list (`Solitude;solitude`) that makes
clients log a "Server is using a different gamedir" warning.

Mount it read-only: the server only writes `qconsole.log` when
`-condebug` is on, and losing that is fine.

## Run

```sh
docker run -d --name solitude \
  -p 26000:26000/udp \
  -v /srv/solitude-data:/data:ro \
  -e MAP=highground -e PORT=26000 \
  --restart unless-stopped \
  solitude-server:linux
```

or edit the volume path in `compose.yaml` and `docker compose up -d`.

Environment:

| var | default | meaning |
| --- | --- | --- |
| `MAP` | `highground` | map to load (`+map`) |
| `PORT` | `26000` | UDP port to listen on |
| `GAME` | *(empty)* | extra `-game` value; leave empty (see above) |

Logs: `docker logs -f solitude`. The engine is block-buffered when stdout is
not a tty, so the entrypoint pipes it through `stdbuf -oL`.

Server settings live in `Solitude/server.cfg`, which the engine executes
automatically at startup (hostname, maxplayers, deathmatch/teamplay,
timelimit, `lobby_bots`, `sv_public`). `sv_public 0` = LAN discovery only
(NetQuake broadcast still works); `sv_public 1` adds a dpmaster heartbeat
every 300 s. Remember `EXPOSE 26000/udp` means nothing without a matching
`-p` publish and a firewall/NAT rule.

## Checking it came up

```sh
docker logs solitude | grep -E "Server spawned|map|way"
```

A healthy boot shows `Playing registered version.`, `UDP4 Initialized`,
`Server using protocol 999+ (FTE-RMQ)`, the map name, `execing
data/maps/<map>.way` when a waypoint file exists, a handful of
`Unknown command "Unknown"` lines (harmless, from the base game config) and
`Server spawned.`

## Known limitation: Colima on macOS

On a Linux host, `-p 26000:26000/udp` behaves normally. On macOS with Colima
as the Docker backend, published **UDP** ports do not reach the host: the
container answers probes from inside its own network namespace, but
`127.0.0.1:26000` on the Mac stays silent (nothing bound) and `--network
host` does not expose the VM address either. For local testing use the native
server (`./mac.sh server highground 26000`) and keep the container for a real
Linux box.
