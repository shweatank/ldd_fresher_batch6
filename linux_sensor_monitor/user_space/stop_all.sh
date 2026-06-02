#!/usr/bin/env bash
set -euo pipefail

BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PID_FILE="${BASE_DIR}/.monitor_pids"

cd "${BASE_DIR}"

if [[ ! -f "${PID_FILE}" ]]; then
    echo "No PID file found. Nothing to stop."
    exit 0
fi

while read -r pid name; do
    if [[ -n "${pid}" ]] && kill -0 "${pid}" 2>/dev/null; then
        kill "${pid}" || true
        echo "Stopped ${name} (${pid})"
    else
        echo "${name} (${pid}) is not running"
    fi
done < "${PID_FILE}"

rm -f "${PID_FILE}" daemon.out logger.out alert_manager.out
echo "Cleanup complete."
