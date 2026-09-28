#!/bin/sh
set -e

echo "=== launch shm_writer ==="
./shm_writer & SHM_WRITER_PID=$!

sleep 0.01

echo "=== launch shm_reader ==="
./shm_reader

wait "${SHM_WRITER_PID}"
echo "=== both processes have been finished, EXIT: $? ==="
