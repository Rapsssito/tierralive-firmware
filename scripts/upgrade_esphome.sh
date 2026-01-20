#!/usr/bin/env bash
set -euo pipefail

TAG=$(cat "${PWD}/esphome.version")

if [ -z "${TAG}" ]; then
  echo "TAG is required (set in esphome.version)" >&2
  exit 1
fi

IMAGE="ghcr.io/esphome/esphome"

echo "Stopping and removing containers using ${IMAGE}..."
CONTAINERS=$(docker ps -aq --filter "ancestor=${IMAGE}")
if [ -n "${CONTAINERS}" ]; then
  docker rm -f ${CONTAINERS}
fi

echo "Removing old images for ${IMAGE}..."
IMAGES=$(docker images -q "${IMAGE}")
if [ -n "${IMAGES}" ]; then
  docker rmi -f ${IMAGES}
fi

echo "Pulling latest image ${IMAGE}:${TAG}..."
docker pull "${IMAGE}:${TAG}"

echo "Done. Latest ${IMAGE}:${TAG} installed."
