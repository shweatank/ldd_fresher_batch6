#!/usr/bin/env bash
set -euo pipefail

BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PID_FILE="${BASE_DIR}/.monitor_pids"
SOCK_PATH="/tmp/sensor_dashboard.sock"

cd "${BASE_DIR}"

if [[ ! -x "./daemon" || ! -x "./logger" || ! -x "./alert_manager" || ! -x "./dashboard" ]]; then
    echo "Binaries missing. Run: make"
    exit 1
fi

if [[ ! -e "/dev/sensor_char" ]]; then
    echo "/dev/sensor_char not found. Load kernel driver first:"
    echo "  cd ../kernel_driver && sudo insmod sensor_driver.ko"
    exit 1
fi

if [[ -f "${PID_FILE}" ]]; then
    echo "Existing PID file found (${PID_FILE}). Run ./stop_all.sh first."
    exit 1
fi

rm -f sensor_events.log critical_events.log

./logger > logger.out 2>&1 &
LOGGER_PID=$!
./daemon > daemon.out 2>&1 &
DAEMON_PID=$!
./alert_manager > alert_manager.out 2>&1 &
ALERT_PID=$!

for _ in {1..30}; do
    if ! kill -0 "${DAEMON_PID}" 2>/dev/null; then
        echo "daemon exited early. Check daemon.out"
        kill "${LOGGER_PID}" "${ALERT_PID}" 2>/dev/null || true
        exit 1
    fi
    [[ -S "${SOCK_PATH}" ]] && break
    sleep 0.2
done

if [[ ! -S "${SOCK_PATH}" ]]; then
    echo "dashboard socket not ready (${SOCK_PATH}). Check daemon.out"
    kill "${LOGGER_PID}" "${DAEMON_PID}" "${ALERT_PID}" 2>/dev/null || true
    exit 1
fi

{
    echo "${LOGGER_PID} logger"
    echo "${DAEMON_PID} daemon"
    echo "${ALERT_PID} alert_manager"
} > "${PID_FILE}"

echo "Started logger (${LOGGER_PID}), daemon (${DAEMON_PID}), alert_manager (${ALERT_PID})"
echo "Launching dashboard in foreground..."
echo "Use Ctrl+C to exit dashboard, then run ./stop_all.sh to stop background services."

exec ./dashboard
