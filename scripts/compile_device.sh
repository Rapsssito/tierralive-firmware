#!/usr/bin/env bash
set -euo pipefail

if [ $# -lt 2 ]; then
	echo "Usage: $0 <DEVICE_CONF_PATH> <OUTPUT_PREFIX>" >&2
	exit 1
fi

DEVICE_CONF_PATH="$1"
OUTPUT_PREFIX="$2"

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)

ESPHOME_VERSION=$(cat "${SCRIPT_DIR}/../esphome.version" | tr -d ' \n')
CONFIG_BASE="${SCRIPT_DIR}/../devices/${DEVICE_CONF_PATH}/config/"
CONFIG_FILE_DOCKER="/config/devices/${DEVICE_CONF_PATH}/config/main.yaml"
PACKAGE_JSON="${CONFIG_BASE}/../package.json"
FIRMWARE_VERSION=""
if [ -f "${PACKAGE_JSON}" ]; then
	FIRMWARE_VERSION=$(python - "${PACKAGE_JSON}" <<'PY'
import json
import sys
from pathlib import Path

path = Path(sys.argv[1])
data = json.loads(path.read_text(encoding="utf-8"))
print(data.get("version", ""))
PY
)
fi
if [ -z "${FIRMWARE_VERSION}" ]; then
	echo "FIRMWARE_VERSION not found in ${PACKAGE_JSON}" >&2
	exit 1
fi

if [[ "$(dirname "${OUTPUT_PREFIX}")" != "." ]]; then
	mkdir -p "$(dirname "${OUTPUT_PREFIX}")"
fi

DOCKER_COMMAND="docker run --rm -e FIRMWARE_VERSION=${FIRMWARE_VERSION} -v "${SCRIPT_DIR}/../:/config" ghcr.io/esphome/esphome:"${ESPHOME_VERSION}""

CONFIG_OUTPUT=$(${DOCKER_COMMAND} config "${CONFIG_FILE_DOCKER}")
echo "${CONFIG_OUTPUT}" > "${OUTPUT_PREFIX}.esphome_config.yaml"
DEVICE_NAME=$(awk '/^esphome:/{f=1;next} f && /^[[:space:]]*name:[[:space:]]*/{gsub(/^[[:space:]]*name:[[:space:]]*/, "", $0); print $0; exit}' <<< "${CONFIG_OUTPUT}")
if [ -z "${DEVICE_NAME}" ]; then
	echo "Unable to read device name from esphome config output" >&2
	exit 1
fi

${DOCKER_COMMAND} compile "${CONFIG_FILE_DOCKER}"

OTA_BIN_PATH="${CONFIG_BASE}/.esphome/build/${DEVICE_NAME}/.pioenvs/${DEVICE_NAME}/firmware.ota.bin"
OUTPUT_FIRMWARE_PATH="${OUTPUT_PREFIX}.bin"
OUTPUT_MD5_PATH="${OUTPUT_PREFIX}.bin.md5"

cp "${OTA_BIN_PATH}" "${OUTPUT_FIRMWARE_PATH}"

# Calculate the md5 checksum
MD5_CHECKSUM=$(md5sum "${OUTPUT_FIRMWARE_PATH}" | awk '{ print $1 }')
echo "${MD5_CHECKSUM}" > "${OUTPUT_MD5_PATH}"

echo "OTA firmware path: ${OUTPUT_FIRMWARE_PATH}"
echo "OTA firmware MD5 path: ${OUTPUT_MD5_PATH}"
