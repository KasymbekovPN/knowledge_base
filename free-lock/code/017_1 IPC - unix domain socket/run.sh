#!/bin/sh
set -e
UDS_PATH=/tmp/demo_uds.sock

rm -f "${UDS_PATH}"

echo "=== launch server ==="
./uds_server & SERVER_PID=$!

sleep 0.3

echo "=== launch client ==="
./uds_client

wait "${SERVER_PID}"
echo "=== both processes have been finished, EXIT: $? ==="
