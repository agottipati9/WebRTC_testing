#!/bin/bash
set -e

WEBRTC_PATH="/opt/home_dir/webrtc-checkout/src/out/Default"
CALL_DURATION=120
CLEANUP_DELAY=10
SETUP_DELAY=3
OUTPUT_DIR="/opt/home_dir/outputs/"

# cmd args how to run: ./run_mahimahi_one_trace.sh --trace <trace_file> --port <port>
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    --id)
    display_id="$2"
    shift # past argument
    shift # past value
    ;;
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

cleanup() {
    pkill -f "peerconnection_.*--port ${PORT}" || true
    pkill -f "Xvfb :${xvfb_display_id}" || true
}
trap cleanup EXIT SIGINT SIGTERM

delay=40
up_pkt_loss=0
down_pkt_loss=0

# create a new display for this process
xvfb_display_id=$((99 + 1 + ${display_id}))
export DISPLAY=:${xvfb_display_id}
Xvfb :${xvfb_display_id} -screen 0 1920x1080x24 &
sleep 5


${WEBRTC_PATH}/peerconnection_server --port ${PORT} > /dev/null 2>&1 & echo $! > /tmp/server_pid_${PORT}
SERVER_PID=$(cat /tmp/server_pid_${PORT})
sleep "${SETUP_DELAY}"
${WEBRTC_PATH}/peerconnection_client --port ${PORT} > ${OUTPUT_DIR}/callee_$(basename "${trace_file}").log 2>&1 & echo $! > /tmp/cl1_pid_${PORT}
CLIENT_ONE_PID=$(cat /tmp/cl1_pid_${PORT})
sleep "${SETUP_DELAY}"
(mm-delay ${delay} mm-loss uplink ${up_pkt_loss} mm-loss downlink ${down_pkt_loss} \
mm-link ${trace_file} ${trace_file} -- \
bash -c "${WEBRTC_PATH}/peerconnection_client --server \$MAHIMAHI_BASE --port ${PORT} --autocall > ${OUTPUT_DIR}/caller_$(basename "${trace_file}").log 2>&1") & echo $! > /tmp/cl2_pid_${PORT}
CLIENT_TWO_PID=$(cat /tmp/cl2_pid_${PORT})
echo "Running call... Sleeping for ${CALL_DURATION} seconds"
sleep "${CALL_DURATION}"  # let the call run for 2 minutes
# remove processes
cleanup
sleep "${CLEANUP_DELAY}"  # wait for everything to close before starting the next one


