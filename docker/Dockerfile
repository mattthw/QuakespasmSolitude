# Solitude dedicated server — Linux image.
#
# Build from the repo root so the context has the engine sources:
#   docker build -f docker/Dockerfile -t solitude-server .
#   ./docker/build.sh            (same thing, plus a tag with the git describe)
#
# The game data is NOT baked in: mount a directory at /data containing
#   id1/        (id1/pak0.pak, pak1.pak)
#   Solitude/   (progs.dat, maps/, gfx/, models/ ... the runtime game dir)
# "Solitude" must be capitalised: the engine registers that game dir by name
# and Linux filesystems are case-sensitive. See docker/entrypoint.sh.

# ---------- build ----------
FROM debian:bookworm-slim AS build
RUN apt-get update -qq && apt-get install -y -qq --no-install-recommends \
      build-essential libsdl2-dev ca-certificates \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY quakespasm /src/quakespasm
WORKDIR /src/quakespasm/Quake
# Codecs and zlib are client concerns (music, pk3); a dedicated host needs
# neither, which keeps the runtime image free of libmad/libvorbis/libz.
RUN make -f Makefile USE_SDL2=1 USE_ZLIB=0 \
      USE_CODEC_WAVE=0 USE_CODEC_MP3=0 USE_CODEC_VORBIS=0 USE_CODEC_OPUS=0 \
      -j"$(nproc)" \
    && test -x quakespasm

# ---------- runtime ----------
FROM debian:bookworm-slim
RUN apt-get update -qq && apt-get install -y -qq --no-install-recommends \
      libsdl2-2.0-0 libgl1 ca-certificates \
    && rm -rf /var/lib/apt/lists/*
COPY --from=build /src/quakespasm/Quake/quakespasm /opt/solitude/quakespasm
COPY docker/entrypoint.sh /usr/local/bin/solitude-entrypoint

# A dedicated host has no display or sound card; without these SDL still tries
# to attach to X11/ALSA and the process can die or stall at startup.
ENV SDL_VIDEODRIVER=dummy \
    SDL_AUDIODRIVER=dummy \
    MAP=highground \
    PORT=26000 \
    GAME=

VOLUME ["/data"]
EXPOSE 26000/udp
WORKDIR /opt/solitude
ENTRYPOINT ["/usr/local/bin/solitude-entrypoint"]
