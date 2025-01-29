#!/bin/bash
set -e

TRACE_DIR="/users/$USER/network_traces/mahimahi"
# TRACE_DIR="/users/${USER}/toy_trace"
WEBRTC_PATH="/users/${USER}/webrtc-checkout/src/out/Default"
CALL_DURATION=120
CLEANUP_DELAY=10
SETUP_DELAY=3
MAHIMAHI_BASE="10.0.0.1"
OUTPUT_DIR="/users/${USER}/outputs/"

# Cleanup function
cleanup() {
    pkill -f peerconnection_server || true
    pkill -f peerconnection_client || true
}
trap cleanup EXIT SIGINT SIGTERM

delay=40
up_pkt_loss=0
down_pkt_loss=0

export DISPLAY=:99

for trace_file in "${TRACE_DIR}"/* ; do
  echo "Running emulation with trace file: ${trace_file}"
  bash -c "${WEBRTC_PATH}/peerconnection_server > /dev/null 2>&1 &"
  sleep "${SETUP_DELAY}"
  echo "Setup Server, now starting receiver."
  bash -c "${WEBRTC_PATH}/peerconnection_client > ${OUTPUT_DIR}/callee_$(basename "${trace_file}").log 2>&1 &"
  sleep "${SETUP_DELAY}"
  echo "Setup Receiver, now starting caller."
  (mm-delay ${delay} mm-loss uplink ${up_pkt_loss} mm-loss downlink ${down_pkt_loss} \
  mm-link ${trace_file} ${trace_file} -- \
  bash -c "${WEBRTC_PATH}/peerconnection_client --server "${MAHIMAHI_BASE}" --autocall > ${OUTPUT_DIR}/caller_$(basename "${trace_file}").log 2>&1") &
  echo "Running call... Sleeping for ${CALL_DURATION} seconds"
  sleep "${CALL_DURATION}"  # let the call run for 2 minutes
  echo "Finished call, cleaning up for next iteration."
  cleanup
  sleep "${CLEANUP_DELAY}"  # wait for everything to close before starting the next one
done