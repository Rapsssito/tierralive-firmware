#!/usr/bin/env bash
set -euo pipefail

if [ $# -lt 2 ]; then
	echo "Usage: $0 <DEVICE_CONF_NAME> <OUTPUT_PREFIX>" >&2
	exit 1
fi

DEVICE_CONF_NAME="$1"
OUTPUT_PREFIX="$2"

ESPHOME_VERSION=$(cat ./esphome.version)

CONFIG_FILE="./${DEVICE_CONF_NAME}/main.yaml"

# Get the bash script directory
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
CONFIG_FILE_DOCKER="/config/${DEVICE_CONF_NAME}/main.yaml"
DOCKER_COMMAND="docker run --rm -v "${SCRIPT_DIR}/../config:/config" ghcr.io/esphome/esphome:"${ESPHOME_VERSION}""

CONFIG_OUTPUT=$(${DOCKER_COMMAND} config "${CONFIG_FILE_DOCKER}")
DEVICE_NAME=$(awk '/^esphome:/{f=1;next} f && /^[[:space:]]*name:[[:space:]]*/{gsub(/^[[:space:]]*name:[[:space:]]*/, "", $0); print $0; exit}' <<< "${CONFIG_OUTPUT}")
if [ -z "${DEVICE_NAME}" ]; then
	echo "Unable to read device name from esphome config output" >&2
	exit 1
fi

${DOCKER_COMMAND} compile "${CONFIG_FILE_DOCKER}"

OTA_BIN_PATH="./${DEVICE_CONF_NAME}/build/${DEVICE_NAME}/.pioenvs/${DEVICE_NAME}/firmware.ota.bin"
OUTPUT_FIRMWARE_PATH="${OUTPUT_PREFIX}/${DEVICE_NAME}.firmware.ota.bin"
OUTPUT_MD5_PATH="${OUTPUT_PREFIX}/${DEVICE_NAME}.firmware.ota.md5"

mkdir -p "${OUTPUT_PREFIX}"
cp "${OTA_BIN_PATH}" "${OUTPUT_FIRMWARE_PATH}"

# Calculate the md5 checksum
MD5_CHECKSUM=$(md5sum "${OUTPUT_FIRMWARE_PATH}" | awk '{ print $1 }')
echo "${MD5_CHECKSUM}" > "${OUTPUT_MD5_PATH}"

echo "OTA firmware path: ${OUTPUT_FIRMWARE_PATH}"
echo "OTA firmware MD5 path: ${OUTPUT_MD5_PATH}"
