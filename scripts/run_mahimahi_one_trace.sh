#!/bin/bash
set -e

WEBRTC_PATH="/opt/home_dir/webrtc-checkout/src/out/Default"
CALL_DURATION=120
CLEANUP_DELAY=10
SETUP_DELAY=3
MAHIMAHI_BASE="10.0.0.1"
OUTPUT_DIR="/opt/home_dir/outputs/"

# cmd args how to run: ./run_mahimahi_one_trace.sh --trace <trace_file> --port <port>
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    --trace)
    trace_file="$2"
    shift # past argument
    shift # past value
    ;;
    --port)
    PORT="$2"
    shift # past argument
    shift # past value
    ;;
    *)    # unknown option
    shift # past argument
    ;;
esac
done

trap EXIT SIGINT SIGTERM

delay=40
up_pkt_loss=0
down_pkt_loss=0

export DISPLAY=:99

echo "Running emulation with trace file: ${trace_file} with port ${PORT}"
bash -c "${WEBRTC_PATH}/peerconnection_server --port ${PORT} > /dev/null 2>&1 &"
sleep "${SETUP_DELAY}"
echo "Setup Server, now starting receiver."
bash -c "${WEBRTC_PATH}/peerconnection_client > ${OUTPUT_DIR}/callee_$(basename "${trace_file}").log 2>&1 &"
sleep "${SETUP_DELAY}"
echo "Setup Receiver, now starting caller."
(mm-delay ${delay} mm-loss uplink ${up_pkt_loss} mm-loss downlink ${down_pkt_loss} \
mm-link ${trace_file} ${trace_file} -- \
bash -c "${WEBRTC_PATH}/peerconnection_client --server "${MAHIMAHI_BASE}" --port ${PORT} --autocall > ${OUTPUT_DIR}/caller_$(basename "${trace_file}").log 2>&1") &
echo "Running call... Sleeping for ${CALL_DURATION} seconds"
sleep "${CALL_DURATION}"  # let the call run for 2 minutes
echo "Finished call, cleaning up for next iteration."
sleep "${CLEANUP_DELAY}"  # wait for everything to close before starting the next one