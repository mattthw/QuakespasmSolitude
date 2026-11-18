#!/bin/sh
# Build the Solitude dedicated-server image.
#
#   ./docker/build.sh                  # native arch (linux/arm64 on Apple Silicon)
#   ARCH=linux/amd64 ./docker/build.sh # x86_64 host (needs buildx or a slow qemu build)
#   TAG=ghcr.io/me/solitude:1 ./docker/build.sh
#
# Runs from the repo root regardless of where it is invoked.
set -e

REPO="$(cd "$(dirname "$0")/.." && pwd)"
TAG="${TAG:-solitude-server:latest}"
ARCH="${ARCH:-}"

cd "$REPO"

if [ -n "$ARCH" ]; then
  exec docker buildx build --platform "$ARCH" -f docker/Dockerfile -t "$TAG" --load .
else
  exec docker build -f docker/Dockerfile -t "$TAG" .
fi
