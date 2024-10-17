#!/bin/sh

# Validate number of args
[ $# -lt 3 ] && echo "Error: should be called from dnsmasq" && exit 1

# NVR notify binary
NVR_NOTIFY_BIN="/matrix/bin/nvrNotify.bin"

# Identification of caller
CALLER_TYPE="dhcpserver"

# Send data to notify binary
$NVR_NOTIFY_BIN $CALLER_TYPE "$@"
