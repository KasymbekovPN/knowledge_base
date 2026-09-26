#!/bin/sh
set -e

echo "=== launch mq_sender ==="
./mq_sender & MQ_SENDER_PID=$!

#sleep 0.3

echo "=== launch mq_receiver ==="
./mq_receiver

wait "$MQ_SENDER_PID"
echo "=== both processes have been finished, EXIT: $? ==="
