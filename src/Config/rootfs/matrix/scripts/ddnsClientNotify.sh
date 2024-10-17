#!/bin/sh

# NVR notify binary
NVR_NOTIFY_BIN="/matrix/bin/nvrNotify.bin"

# Identification of caller
CALLER_TYPE="ddnsclient"

# Send data to notify binary
$NVR_NOTIFY_BIN $CALLER_TYPE "$@"
