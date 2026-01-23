#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
ROOT_DIR="${SCRIPT_DIR}/.."
TAG=$(cat "${ROOT_DIR}/esphome.version" | tr -d ' \n')

if [ -z "${TAG}" ]; then
  echo "TAG is required (set in esphome.version)" >&2
  exit 1
fi

VENV_DIR="${ROOT_DIR}/.venv"

if [ ! -d "${VENV_DIR}" ]; then
  echo "Creating virtual environment at ${VENV_DIR}..."
  python3 -m venv "${VENV_DIR}"
fi

echo "Installing ESPHome ${TAG} into ${VENV_DIR}..."
"${VENV_DIR}/bin/pip" install --upgrade "esphome==${TAG}"

echo "Done. ESPHome ${TAG} installed in ${VENV_DIR}."
