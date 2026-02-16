#!/usr/bin/env bash
set -euo pipefail

if [ $# -lt 2 ]; then
	echo "Usage: $0 <DEVICE_CONF_PATH> <OUTPUT_PREFIX>" >&2
	exit 1
fi

DEVICE_CONF_PATH="$1"
OUTPUT_PREFIX="$2"

# Ensure output directory exists
if [[ "$(dirname "${OUTPUT_PREFIX}")" != "." ]]; then
	mkdir -p "$(dirname "${OUTPUT_PREFIX}")"
fi

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
ROOT_DIR="${SCRIPT_DIR}/.."

CONFIG_BASE="${ROOT_DIR}/devices/${DEVICE_CONF_PATH}/config"
CONFIG_FILE="${CONFIG_BASE}/main.yaml"
PACKAGE_JSON="${CONFIG_BASE}/../package.json"
FIRMWARE_VERSION=""
if [ -f "${PACKAGE_JSON}" ]; then
	FIRMWARE_VERSION=$(python3 - "${PACKAGE_JSON}" <<'PY'
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

ESPHOME_BIN="esphome"

CONFIG_OUTPUT=$(FIRMWARE_VERSION="${FIRMWARE_VERSION}" "${ESPHOME_BIN}" config "${CONFIG_FILE}") || {
	echo "Failed to generate esphome config for ${CONFIG_FILE}" >&2
	if [ -n "${CONFIG_OUTPUT}" ]; then
		printf '%s\n' "${CONFIG_OUTPUT}" >&2
	fi
	exit 1
}
echo "${CONFIG_OUTPUT}" > "${OUTPUT_PREFIX}.esphome_config.yaml"
DEVICE_NAME=$(awk '/^esphome:/{f=1;next} f && /^[[:space:]]*name:[[:space:]]*/{gsub(/^[[:space:]]*name:[[:space:]]*/, "", $0); print $0; exit}' <<< "${CONFIG_OUTPUT}")
if [ -z "${DEVICE_NAME}" ]; then
	echo "Unable to read device name from esphome config output" >&2
	exit 1
fi

COMPILE_OUTPUT=$(FIRMWARE_VERSION="${FIRMWARE_VERSION}" "${ESPHOME_BIN}" compile "${CONFIG_FILE}") || {
	echo "Failed to compile esphome config for ${CONFIG_FILE}" >&2
	if [ -n "${COMPILE_OUTPUT}" ]; then
		printf '%s\n' "${COMPILE_OUTPUT}" >&2
	fi
	exit 1
}

OTA_BIN_PATH="${CONFIG_BASE}/.esphome/build/${DEVICE_NAME}/.pioenvs/${DEVICE_NAME}/firmware.ota.bin"
FACTORY_BIN_PATH="${CONFIG_BASE}/.esphome/build/${DEVICE_NAME}/.pioenvs/${DEVICE_NAME}/firmware.factory.bin"
OUTPUT_OTA_FIRMWARE_PATH="${OUTPUT_PREFIX}.ota.bin"
OUTPUT_OTA_MD5_PATH="${OUTPUT_PREFIX}.ota.bin.md5"
OUTPUT_FACTORY_FIRMWARE_PATH="${OUTPUT_PREFIX}.factory.bin"
OUTPUT_FACTORY_MD5_PATH="${OUTPUT_PREFIX}.factory.bin.md5"

cp "${OTA_BIN_PATH}" "${OUTPUT_OTA_FIRMWARE_PATH}"
cp "${FACTORY_BIN_PATH}" "${OUTPUT_FACTORY_FIRMWARE_PATH}"

# Calculate the md5 checksum
md5sum "${OUTPUT_OTA_FIRMWARE_PATH}" | awk '{print $1}' > "${OUTPUT_OTA_MD5_PATH}"
md5sum "${OUTPUT_FACTORY_FIRMWARE_PATH}" | awk '{print $1}' > "${OUTPUT_FACTORY_MD5_PATH}"

echo "OTA firmware path: ${OUTPUT_OTA_FIRMWARE_PATH}"
echo "OTA firmware MD5 path: ${OUTPUT_OTA_MD5_PATH}"
echo "Factory firmware path: ${OUTPUT_FACTORY_FIRMWARE_PATH}"
echo "Factory firmware MD5 path: ${OUTPUT_FACTORY_MD5_PATH}"
