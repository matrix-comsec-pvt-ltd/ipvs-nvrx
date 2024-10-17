#!/bin/sh

#==================================================================================================
# Print Debugs
#==================================================================================================
print()
{
    if [ "$1" = "ERR" ]
    then
        logger -t "nvr-x.sh" -p 3 "$2"
    else
        logger -t "nvr-x.sh" -p 6 "$2"
    fi
}

#==================================================================================================
# Define default variables
#==================================================================================================
# Default value of variables
LAN1_DEFAULT_IP_ADDR="192.168.1.123"
LAN2_DEFAULT_IP_ADDR="192.168.2.2"

#==================================================================================================
# Script starts from here
#==================================================================================================
# Set default IP address on LAN1 (eth0)
ifconfig eth0 $LAN1_DEFAULT_IP_ADDR
print DBG "Default IP addr[$LAN1_DEFAULT_IP_ADDR] set on LAN1 (eth0)"

# Set default IP address on LAN2 (eth1)
ifconfig eth1 $LAN2_DEFAULT_IP_ADDR
print DBG "Default IP addr[$LAN2_DEFAULT_IP_ADDR] set on LAN2 (eth1)"

# It is NVRL board.
BOARD_TYPE="RK3588_NVRH"
print DBG "BOARD TYPE[$BOARD_TYPE] detected"

# Apply network related and other run time configuration
print DBG "Apply network configuration"

# Resolve ARP only for specified interface (Do not resolve LAN1 ARP from LAN2 and vice-versa)
echo 1 > /proc/sys/net/ipv4/conf/all/arp_filter

# Increase reserve memory for kernel (64MB)
echo 65536 > /proc/sys/vm/min_free_kbytes

# Remove older unnecessary files from filesystem to make free space
/matrix/scripts/cleanup.sh

# Mount ramfs file system for temp and faster execution
print DBG "Mount ramfs file system"
RAMFS_PATH="/mnt/ramfs"
mkdir -p $RAMFS_PATH
mount -t ramfs /dev/ram0 $RAMFS_PATH

# Create softlink of nvr folder with matrix folder
ln -sf /matrix /nvr

# Check and upgrade firmware if available
print DBG "Check for system firmware upgrade"
/matrix/bin/sysUpgrade.bin

# Enable core dump generation module for application crash
print DBG "Enable coredump generation"
echo "|/matrix/scripts/coredump" > /proc/sys/kernel/core_pattern
echo "1" > /proc/sys/kernel/core_pipe_limit

# Get system run mode [ Hardware Test OR Normal]
TEST_APP=`cat /sys/class/gpios/MxGpioDrv/ipdef`
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/matrix/lib
if [ $TEST_APP -eq 1 ]
then
    # Normal system execution
    print DBG "Start normal system execution for $BOARD_TYPE"
    chmod +x /matrix/bin/nvrAppl.bin
    /matrix/bin/nvrAppl.bin > /dev/null 2>&1 &
    print DBG "Run NVR main application"

    chmod +x /matrix/bin/multiNvrClient
    print DBG "Run GUI application"
    while :
    do
        /matrix/bin/multiNvrClient > /dev/null 2>&1
        if [ $? -eq 0 ]
        then
            print DBG "GUI application exited..!!"
            break
        fi
        print DBG "GUI application exited, starting again..!!"
        sleep 2
    done &
else
    # Hardware test execution
    print DBG "Start hardware test execution for $BOARD_TYPE"
    chmod +x /matrix/bin/nvrTestApp
    /matrix/bin/nvrTestApp > /dev/null 2>&1 &
    print DBG "Run GUI test application"
fi

#==================================================================================================
# End of Script
#==================================================================================================
