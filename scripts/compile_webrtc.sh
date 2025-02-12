#!/bin/bash

cd /opt/home_dir/webrtc-checkout/src

# Compile WebRTC
gn gen out/Default
ninja -C out/Default