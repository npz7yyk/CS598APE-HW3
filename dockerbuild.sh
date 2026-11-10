#!/bin/sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
IMAGE=${1:-598ape-hw3}

docker build -t "$IMAGE" -f "$SCRIPT_DIR/docker/Dockerfile" "$SCRIPT_DIR/docker"
