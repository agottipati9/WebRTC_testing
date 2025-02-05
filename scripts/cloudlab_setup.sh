#!/bin/bash

sudo chown -R ${USER}:bonsai /opt
sudo chown -R ${USER}:bonsai /opt/*

# update .bashrc file
echo "export PATH=/opt/home_dir/depot_tools:$PATH" >> /users/${USER}/.bashrc
echo "export DISPLAY=:99" >> /users/${USER}/.bashrc

# set our environment variables
export PATH=/opt/home_dir/depot_tools:$PATH
export DISPLAY=:99

# start Xvfb
Xvfb :99 -screen 0 1024x768x24 &

# setup webrtc code
cd /opt/home_dir
mkdir webrtc-checkout
cd webrtc-checkout
git clone https://github.com/agottipati9/WebRTC_testing.git
cp ./WebRTC_testing/.gclient .
mv ./WebRTC_testing src
gclient sync

# compile webrtc code
cd src && gn gen out/Default && ninja -C out/Default
