#!/usr/bin/env bash
# Push the Solitude dedicated server to a remote docker host and (re)start it.
#
#   ./docker/deploy.sh user@host /srv/solitude/data            # build on host, copy data, run
#   ./docker/deploy.sh user@host /srv/solitude/data pit 26000
#   ./docker/deploy.sh user@host /srv/solitude/data --no-data  # image only (data already there)
#   ./docker/deploy.sh user@host /srv/solitude/data --local-build
#
# What it does, in order:
#   1. rsyncs the build context to <src-dir> on the host and runs `docker build`
#      there (native arch, no emulation), or builds locally with --local-build
#   2. rsyncs id1/ + Solitude/ to the remote data dir (skip with --no-data)
#   3. stops the old container, starts a new one on <port>/udp
#
# <src-dir> defaults to <data-dir>/../src. The host and paths are arguments:
# nothing about a particular server is baked in.
#
# The game data is mounted read-only; the engine only wants to write
# qconsole.log, and it prints one harmless line and keeps running when it
# cannot. Logs come from `docker logs` instead.

set -eu

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
IMAGE="solitude-server:linux"
NAME="solitude"

HOST=""
DATADIR=""
SRCDIR=""
MAP="highground"
PORT="26000"
PUSH_DATA=1
LOCAL_BUILD=0
RUN_CONTAINER=1

for arg in "$@"; do
	case "$arg" in
		--no-data) PUSH_DATA=0 ;;
		--local-build) LOCAL_BUILD=1 ;;
		--no-run) RUN_CONTAINER=0 ;;
		-*) echo "unknown option: $arg" >&2; exit 1 ;;
		*)
			if [ -z "$HOST" ]; then HOST="$arg"
			elif [ -z "$DATADIR" ]; then DATADIR="$arg"
			elif [ "$MAP" = "highground" ]; then MAP="$arg"
			else PORT="$arg"
			fi ;;
	esac
done

if [ -z "$HOST" ] || [ -z "$DATADIR" ]; then
	echo "usage: $0 user@host <remote-data-dir> [map] [port] [--no-data] [--local-build]" >&2
	exit 1
fi

[ -n "$SRCDIR" ] || SRCDIR="$(dirname "$DATADIR")/src"

say() { printf '\n== %s\n' "$*"; }

say "remote: $HOST"
ARCH="$(ssh "$HOST" 'uname -m')"
case "$ARCH" in
	x86_64|amd64) PLATFORM=linux/amd64 ;;
	aarch64|arm64) PLATFORM=linux/arm64 ;;
	*) echo "unsupported remote arch: $ARCH" >&2; exit 1 ;;
esac
echo "arch: $ARCH -> $PLATFORM"

if [ "$LOCAL_BUILD" = 1 ]; then
	say "building image locally for $PLATFORM"
	if [ "$PLATFORM" = "linux/amd64" ] && [ "$(uname -m)" = "x86_64" ]; then
		docker build -t "$IMAGE" -f "$REPO/docker/Dockerfile" "$REPO"
	else
		docker buildx build --platform "$PLATFORM" -t "$IMAGE" --load \
			-f "$REPO/docker/Dockerfile" "$REPO"
	fi
	ARCH_ACTUAL="$(docker image inspect "$IMAGE" --format '{{.Architecture}}')"
	case "$PLATFORM:$ARCH_ACTUAL" in
		linux/amd64:amd64|linux/arm64:arm64) ;;
		*) echo "built $ARCH_ACTUAL, host needs $PLATFORM" >&2; exit 1 ;;
	esac

	say "sending image"
	docker save "$IMAGE" | gzip -1 | ssh "$HOST" 'gunzip | docker load'
else
	# Build ON the host. Cross-building from an arm64 Mac needs emulation that
	# the default buildx/docker driver does not actually provide: it silently
	# produces a native-arch image, which then dies on the host with
	# "exec format error". Sending ~40 MB of source and compiling there is
	# faster, and the image lands exactly where it is used.
	say "sending build context to $SRCDIR"
	ssh "$HOST" "mkdir -p '$SRCDIR/quakespasm' '$SRCDIR/docker'"
	rsync -a --delete --info=progress2 \
		--exclude '*.o' --exclude '*.a' --exclude '*.dylib' \
		--exclude 'Quake/quakespasm' --exclude '*.vpk' \
		"$REPO/quakespasm/" "$HOST:$SRCDIR/quakespasm/"
	rsync -a --delete "$REPO/docker/" "$HOST:$SRCDIR/docker/"
	rsync -a "$REPO/.dockerignore" "$HOST:$SRCDIR/.dockerignore"

	say "building image on $HOST"
	ssh "$HOST" "docker build -t $IMAGE -f '$SRCDIR/docker/Dockerfile' '$SRCDIR'" \
		| tail -20
	ARCH_ACTUAL="$(ssh "$HOST" "docker image inspect $IMAGE --format '{{.Architecture}}'")"
	case "$PLATFORM:$ARCH_ACTUAL" in
		linux/amd64:amd64|linux/arm64:arm64) echo "image arch: $ARCH_ACTUAL" ;;
		*) echo "built $ARCH_ACTUAL, host needs $PLATFORM" >&2; exit 1 ;;
	esac
fi

if [ "$PUSH_DATA" = 1 ]; then
	say "sending game data to $DATADIR (about 600 MB, first run only)"
	ssh "$HOST" "mkdir -p '$DATADIR'"
	rsync -a --info=progress2 --delete "$REPO/id1/" "$HOST:$DATADIR/id1/"
	# The two config files are personal (local keybinds, Mac-only settings)
	# and are never part of a server: keep them off the host.
	rsync -a --info=progress2 --delete \
		--exclude config.cfg --exclude config-mac.cfg \
		"$REPO/solitude/" "$HOST:$DATADIR/Solitude/"
fi

if [ "$RUN_CONTAINER" = 0 ]; then
	say "skipping container start (--no-run)"
	exit 0
fi

say "starting container"
# --no-data runs still need the directory to exist
ssh "$HOST" "mkdir -p '$DATADIR' && docker rm -f $NAME >/dev/null 2>&1 || true"
# Hardened on purpose: the engine parses untrusted UDP from the internet, so a
# memory bug there must not become root-with-capabilities on the host.
ssh "$HOST" "docker run -d --name $NAME --restart unless-stopped \
	-p $PORT:26000/udp \
	--user 65534:65534 \
	--read-only \
	--tmpfs /tmp:rw,noexec,nosuid,size=16m \
	--tmpfs /opt/solitude:rw,noexec,nosuid,size=16m,mode=1777 \
	--cap-drop ALL \
	--security-opt no-new-privileges:true \
	--pids-limit 128 \
	--memory 512m --memory-swap 512m --cpus 2 \
	--log-opt max-size=10m --log-opt max-file=3 \
	-e HOME=/tmp \
	-v '$DATADIR:/data:ro' \
	-e MAP='$MAP' -e PORT=26000 \
	$IMAGE"

sleep 6
say "logs"
ssh "$HOST" "docker logs --tail 25 $NAME" || true
say "done"
echo "join: connect <this-host-ip>:${PORT}   (or Join -> LAN search)"
echo "stop: ssh $HOST docker rm -f $NAME"
