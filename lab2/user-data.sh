#!/bin/bash
set -e
apt-get update
apt-get install -y build-essential cmake
echo "Instance var07 ready: $(date -u)" > /etc/motd
