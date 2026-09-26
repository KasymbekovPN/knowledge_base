#!/bin/sh
set -e
FIFO_PATH=/tmp/demo_fifo

rm -f "${FIFO_PATH}"
mkfifo "${FIFO_PATH}"

echo "=== FIFO has been created, launch the reader in the background ==="
./fifo_reader & READER_PID=$!

sleep 0.3

echo "=== launch writer ==="
./fifo_writer

wait "$READER_PID"
echo "=== both processes have been finished, EXIT: $? ==="
