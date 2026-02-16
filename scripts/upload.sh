#!/usr/bin/env bash
set -euo pipefail

if [ $# -lt 1 ]; then
	echo "Usage: $0 <DEVICE_CONF_PATH>" >&2
	exit 1
fi

DEVICE_CONF_PATH="$1"

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
# If not firmware version, but in a _debug device (the config path ends with _debug), use a debug version
if [ -z "${FIRMWARE_VERSION}" ] && [[ "${DEVICE_CONF_PATH}" == *_debug ]]; then
	FIRMWARE_VERSION="debug"
fi
if [ -z "${FIRMWARE_VERSION}" ]; then
	echo "FIRMWARE_VERSION not found in ${PACKAGE_JSON}" >&2
	exit 1
fi

export FIRMWARE_VERSION

# esphome clean ${CONFIG_FILE} || {
#     echo "Failed to compile esphome config for debug build" >&2
#     exit 1
# }

esphome compile ${CONFIG_FILE} || {
    echo "Failed to compile esphome config for debug build" >&2
    exit 1
}

esphome upload ${CONFIG_FILE} || {
    echo "Failed to upload esphome firmware for debug build" >&2
    exit 1
}

# Show the logs after upload
echo "Use the following command to view the logs of the device:"
echo "FIRMWARE_VERSION=${FIRMWARE_VERSION} esphome logs ${CONFIG_FILE}"