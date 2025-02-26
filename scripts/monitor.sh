#!/bin/bash
while true; do
  echo "======== $(date) ========"
  echo "CPU USAGE BY CORE:"
  mpstat -P ALL 1 1
  echo "MEMORY USAGE:"
  free -h
  echo "TOP PROCESSES BY CPU:"
  ps -eo pid,ppid,%cpu,%mem,cmd --sort=-%cpu | head -10
  echo "TOP PROCESSES BY MEMORY:"
  ps -eo pid,ppid,%cpu,%mem,cmd --sort=-%mem | head -10
  sleep 5
done