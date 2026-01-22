#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
ROOT_DIR="${SCRIPT_DIR}/.."
DIST_DIR="${ROOT_DIR}/dist"

mkdir -p "${DIST_DIR}"

DEVICE_CONFIGS=()
while IFS= read -r -d '' config_file; do
	device_conf_path="${config_file#${ROOT_DIR}/devices/}"
	device_conf_path="${device_conf_path%/config/main.yaml}"
	DEVICE_CONFIGS+=("${device_conf_path}")
done < <(find "${ROOT_DIR}/devices" -type f -name main.yaml -path "*/config/main.yaml" -print0 | sort -z)

if [ ${#DEVICE_CONFIGS[@]} -eq 0 ]; then
	echo "No device configs found under ${ROOT_DIR}/devices" >&2
	exit 1
fi

for device_conf_path in "${DEVICE_CONFIGS[@]}"; do
	device_conf_name_flat="${device_conf_path//\//_}"
	echo "Compiling device config: ${device_conf_name_flat}"
	output_prefix="${DIST_DIR}/${device_conf_name_flat}"
	bash "${SCRIPT_DIR}/compile_device.sh" "${device_conf_path}" "${output_prefix}"
done
