#!/bin/sh
#set -e
#
#echo "=== launch sig_receiver ==="
#./sig_receiver & SIG_RECEIVER=$!
#
#sleep 0.5
#
#echo "=== launch sig_sender ==="
#./sig_sender
#
#wait "${SIG_RECEIVER}"
#echo "=== both processes have been finished, EXIT: $? ==="
