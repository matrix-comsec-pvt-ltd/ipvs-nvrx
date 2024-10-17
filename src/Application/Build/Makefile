#########################################################################
# Specify general variables
#########################################################################
# Sourcing of common variables
include ../../../define.mk

# Enable/Disable debugging symbols (Enable: Debug = on)
DEBUG ?= on
VERBOSE ?= off

#########################################################################
# Specify Compiler, Linker, Archiver and other options
#########################################################################
CC            := $(CROSS_COMPILE)gcc
CPP           := $(CROSS_COMPILE)g++
AR            := $(CROSS_COMPILE)ar
LINK          := $(CROSS_COMPILE)g++
STRIP         := $(CROSS_COMPILE)strip

PROJ_BUILD_PATH        := $(APP_BUILD_TAG)
PROJ_BIN_DIR           := $(PROJ_BUILD_PATH)/bin
PROJ_BIN_DIR_APPCLIENT := $(PROJ_BUILD_PATH)/binAppClient
PROJ_OBJ_DIR           := $(PROJ_BUILD_PATH)/obj
PROJ_OBJ_DIR_APPCLIENT := $(PROJ_BUILD_PATH)/objAppClient

ifeq ($(DEBUG), on)
DEBUG_FLAG := -g
else
DEBUG_FLAG :=
LDFLAGS += -s
endif

ifeq ($(VERBOSE), on)
_@_ :=
else
_@_ := @
NO_PRINT_DIR := -s --no-print-directory
endif

#########################################################################
# COMPILER SELECTION + PROJECT EXTENSIONS
#########################################################################
DEPEND_EXTENSION = .d
OBJ_EXTENSION = .o
NODEPEND_TARGETS := clean

export _@_
export CC CPP AR LINK
export DEPEND_EXTENSION OBJ_EXTENSION NODEPEND_TARGETS
export PROJ_OBJ_DIR BOARD_TYPE OEM_TYPE

#########################################################################
# SOURCE DIRECTORY PATH
#########################################################################
NVR_APPL_BIN			:= nvrAppl.bin
RTSP_CLIENT_BIN			:= rtspClient.bin

DEV_DETECT_BIN			:= devDetect.bin
SYSTEM_UPGRADE_BIN		:= sysUpgrade.bin
NVR_NOTIFY_BIN			:= nvrNotify.bin
SYS_CMD_EXE_BIN			:= sysCmdExe.bin

DEV_DETECT_OBJ			:= DeviceDetect.o
SYSTEM_UPGRARE_OBJ		:= SysUpgrade.o
NVR_NOTIFY_OBJ			:= NvrNotify.o
SYS_CMD_EXE_OBJ			:= SysCmdExe.o

NVR_APPL_SRC_PATH		:= $(patsubst %/,%,$(dir $(shell pwd)))
NVRX_SRC_PATH			:= $(patsubst %/,%,$(dir $(shell cd ..;pwd)))

APP_SOCKET_IP_CLIENT_PATH	:= $(NVR_APPL_SRC_PATH)/AppSocketIPCClient
BACKUP_MANAGER_PATH		:= $(NVR_APPL_SRC_PATH)/BackupManager
CAMERA_INITIATION_PATH		:= $(NVR_APPL_SRC_PATH)/CameraInitiation
CAMERA_INTERFACE_PATH		:= $(NVR_APPL_SRC_PATH)/CameraInterface
CONFIG_PATH			:= $(NVR_APPL_SRC_PATH)/Config
DATE_TIME_PATH			:= $(NVR_APPL_SRC_PATH)/DateTime
DDNS_CLIENT_PATH		:= $(NVR_APPL_SRC_PATH)/DdnsClient
DEBUG_LOG_PATH			:= $(NVR_APPL_SRC_PATH)/DebugLog
DEVICE_INITIATION_PATH		:= $(NVR_APPL_SRC_PATH)/DeviceInitiation
DHCP_SERVER_PATH		:= $(NVR_APPL_SRC_PATH)/DhcpServer
DISK_MANAGER_PATH		:= $(NVR_APPL_SRC_PATH)/DiskManager
EVENT_HANDLER_PATH		:= $(NVR_APPL_SRC_PATH)/EventHandler
EVENT_LOGGER_PATH		:= $(NVR_APPL_SRC_PATH)/EventLogger
FTP_CLIENT_PATH			:= $(NVR_APPL_SRC_PATH)/FtpClient
HTTP_CLIENT_PATH		:= $(NVR_APPL_SRC_PATH)/HttpClient
IMAGE_UPLOAD_PATH		:= $(NVR_APPL_SRC_PATH)/ImageUpload
COMMON_INCLUDE_PATH		:= $(NVR_APPL_SRC_PATH)/Include
INPUT_OUTPUT_PATH		:= $(NVR_APPL_SRC_PATH)/InputOutput
MATRIX_MAC_CLIENT_PATH		:= $(NVR_APPL_SRC_PATH)/MatrixMacClient
MEDIA_STREAMER_PATH		:= $(NVR_APPL_SRC_PATH)/MediaStreamer
NETWORK_CONTROLLER_PATH		:= $(NVR_APPL_SRC_PATH)/NetworkController
NETWORK_MANAGER_PATH		:= $(NVR_APPL_SRC_PATH)/NetworkManager
EMAIL_NOTIFY_PATH		:= $(NVR_APPL_SRC_PATH)/Notification/EmailNotify
SMS_NOTIFY_PATH			:= $(NVR_APPL_SRC_PATH)/Notification/SmsNotify
TCP_NOTIFY_PATH			:= $(NVR_APPL_SRC_PATH)/Notification/TcpNotify
PUSH_NOTIFICATION_PATH		:= $(NVR_APPL_SRC_PATH)/Notification/PushNotification
ONVIF_CLIENT_PATH		:= $(NVR_APPL_SRC_PATH)/OnvifClient
P2P_PATH			:= $(NVR_APPL_SRC_PATH)/P2P
PTZ_TOUR_PATH			:= $(NVR_APPL_SRC_PATH)/PtzTour
RECORD_MANGER_PATH		:= $(NVR_APPL_SRC_PATH)/RecordManager
RTSP_CLIENT_INTERFACE_PATH	:= $(NVR_APPL_SRC_PATH)/RtspClientInterface
RTSP_CLIENT_PATH		:= $(NVR_APPL_SRC_PATH)/RtspClient
SNAP_SHOT_SCHD_PATH		:= $(NVR_APPL_SRC_PATH)/SnapShotSchd
SYS_TIMER_PATH			:= $(NVR_APPL_SRC_PATH)/SysTimer
SYSTEM_UPGRADE_PATH		:= $(NVR_APPL_SRC_PATH)/SystemUpgrade
UTILS_PATH			:= $(NVR_APPL_SRC_PATH)/Utils
MATRIX_BRAND_PATH		:= $(CAMERA_INTERFACE_PATH)/Matrix

SOAP_INCLUDE_PATH		:= $(SOAP_INSTALL_PATH)/include
SOAP_LIB_PATH			:= $(SOAP_INSTALL_PATH)/lib

LIVE555_INCLUDE_PATH		:= $(shell printf "%s " $(LIVE555_INSTALL_PATH)/usr/local/include/*)
LIVE555_LIB_PATH		:= $(LIVE555_INSTALL_PATH)/usr/local/lib

JANSSON_INCLUDE_PATH		:= $(JANSSON_INSTALL_PATH)/include
JANSSON_LIB_PATH		:= $(JANSSON_INSTALL_PATH)/lib

COTURN_INCLUDE_PATH		:= $(COTURN_INSTALL_PATH)/include/turn/client
COTURN_LIB_PATH			:= $(COTURN_INSTALL_PATH)/lib

NETWORK_MANAGER_INCLUDE_PATH	:= $(NETWORK_MANAGER_INSTALL_PATH)/include
NETWORK_MANAGER_LIB_PATH	:= $(NETWORK_MANAGER_INSTALL_PATH)/lib

# Add applications sub directory names here
# NOTE: Add directories in APP_SRC_SUB_DIR whose source files needs to be compiled.
APP_SRC_SUB_DIR := $(NVR_APPL_SRC_PATH)
APP_SRC_SUB_DIR += $(COMMON_INCLUDE_PATH)
APP_SRC_SUB_DIR += $(BACKUP_MANAGER_PATH)
APP_SRC_SUB_DIR += $(CAMERA_INITIATION_PATH)
APP_SRC_SUB_DIR += $(CAMERA_INTERFACE_PATH)
APP_SRC_SUB_DIR += $(CONFIG_PATH)
APP_SRC_SUB_DIR += $(DATE_TIME_PATH)
APP_SRC_SUB_DIR += $(DDNS_CLIENT_PATH)
APP_SRC_SUB_DIR += $(DEBUG_LOG_PATH)
APP_SRC_SUB_DIR += $(DEVICE_INITIATION_PATH)
APP_SRC_SUB_DIR += $(DHCP_SERVER_PATH)
APP_SRC_SUB_DIR += $(DISK_MANAGER_PATH)
APP_SRC_SUB_DIR += $(EVENT_HANDLER_PATH)
APP_SRC_SUB_DIR += $(EVENT_LOGGER_PATH)
APP_SRC_SUB_DIR += $(FTP_CLIENT_PATH)
APP_SRC_SUB_DIR += $(HTTP_CLIENT_PATH)
APP_SRC_SUB_DIR += $(IMAGE_UPLOAD_PATH)
APP_SRC_SUB_DIR += $(MATRIX_MAC_CLIENT_PATH)
APP_SRC_SUB_DIR += $(MEDIA_STREAMER_PATH)
APP_SRC_SUB_DIR += $(NETWORK_CONTROLLER_PATH)
APP_SRC_SUB_DIR += $(NETWORK_MANAGER_PATH)
APP_SRC_SUB_DIR += $(EMAIL_NOTIFY_PATH)
APP_SRC_SUB_DIR += $(SMS_NOTIFY_PATH)
APP_SRC_SUB_DIR += $(TCP_NOTIFY_PATH)
APP_SRC_SUB_DIR += $(PUSH_NOTIFICATION_PATH)
APP_SRC_SUB_DIR += $(ONVIF_CLIENT_PATH)
APP_SRC_SUB_DIR += $(P2P_PATH)
APP_SRC_SUB_DIR += $(PTZ_TOUR_PATH)
APP_SRC_SUB_DIR += $(RECORD_MANGER_PATH)
APP_SRC_SUB_DIR += $(SNAP_SHOT_SCHD_PATH)
APP_SRC_SUB_DIR += $(SYS_TIMER_PATH)
APP_SRC_SUB_DIR += $(SYSTEM_UPGRADE_PATH)
APP_SRC_SUB_DIR += $(UTILS_PATH)
APP_SRC_SUB_DIR += $(RTSP_CLIENT_INTERFACE_PATH)
APP_SRC_SUB_DIR += $(MATRIX_BRAND_PATH)
APP_SRC_SUB_DIR += $(INPUT_OUTPUT_PATH)

COMMON_INCLUDE_PATHS := $(APP_SRC_SUB_DIR) $(GPIO_DRIVER_INCLUDE_PATH) $(SOAP_INCLUDE_PATH) $(LIVE555_INCLUDE_PATH) $(HW_DECODER_INCLUDE_PATH) \
			$(JANSSON_INCLUDE_PATH) $(COTURN_INCLUDE_PATH) $(NETWORK_MANAGER_INCLUDE_PATH) $(TOOLCHAIN_SYSROOT_PATH)/usr/include

# Add required libraries
CRYPTO_LIBS := -lssl -lcrypto
APP_LIBS_NEEDED += $(CRYPTO_LIBS) -lcurl -lrt -lpcap -ludev
APP_LIBS_NEEDED += -L $(SOAP_LIB_PATH) -lsoap
APP_LIBS_NEEDED += $(JANSSON_LIB_PATH)/libjansson.a
APP_LIBS_NEEDED += $(COTURN_LIB_PATH)/libturnclient.a
APP_LIBS_NEEDED += $(NETWORK_MANAGER_LIB_PATH)/libnetworkmanager.a

LIVE555_LIB += $(LIVE555_LIB_PATH)/libliveMedia.a
LIVE555_LIB += $(LIVE555_LIB_PATH)/libgroupsock.a
LIVE555_LIB += $(LIVE555_LIB_PATH)/libBasicUsageEnvironment.a
LIVE555_LIB += $(LIVE555_LIB_PATH)/libUsageEnvironment.a

#########################################################################
# COMPILER & LINKER RELATED VARIABLES
#########################################################################
COMPILE_ONLY_FLAG := -c
INCLUDE_FLAG := -I
LIB_DIR_FLAG := -L
OPT_FLAG := -O3
CC_OBJ_OUTPUT = -o $@

# Add compilation warning flags
WARNING_FLAGS := -Wall -Wextra -Wno-format-truncation -Wno-unused-parameter -Wno-ignored-qualifiers -Werror
ifeq ($(BOARD_TYPE), RK3588_NVRH)
WARNING_FLAGS += -Wno-restrict
endif

# Add standard compilation flags
STD_COMPILATION_FLAGS := -pthread -D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 $(DEBUG_FLAG) $(OPT_FLAG)

# Add application's custom flags
APP_CUSTOM_FLAGS := -D$(BOARD_TYPE) -D$(OEM_TYPE)
APP_CUSTOM_FLAGS += -DSOFTWARE_VERSION=$(SOFTWARE_VERSION) -DSOFTWARE_REVISION=$(SOFTWARE_REVISION) -DPRODUCT_SUB_REVISION=$(PRODUCT_SUB_REVISION)
APP_CUSTOM_FLAGS += -DCOMMUNICATION_VERSION=$(COMMUNICATION_VERSION) -DCOMMUNICATION_REVISION=$(COMMUNICATION_REVISION)
APP_CUSTOM_FLAGS += -DDEVICE_NAME_PREFIX=\"$(DEVICE_NAME_PREFIX)\" -DFIRMWARE_UPGRADE_CONFIRMATION_FILE=\"$(FIRMWARE_UPGRADE_CONFIRMATION_FILE)\"

# Add all headers include paths
INCLUDE_PATHS_FLAGS := $(COMMON_INCLUDE_PATHS:%=$(INCLUDE_FLAG)%)

# Prepare C compilation flags
CFLAGS := $(SOC_CFLAGS) $(WARNING_FLAGS) $(STD_COMPILATION_FLAGS) $(APP_CUSTOM_FLAGS) $(INCLUDE_PATHS_FLAGS)

# Suppress warnings for unused variables and functions for OEM_JCI
ifeq ($(OEM_TYPE), OEM_JCI)
CFLAGS += -Wno-unused-variable -Wno-unused-function
endif

# Prepare CPP compilation flags
ifeq ($(BOARD_TYPE), $(filter $(BOARD_TYPE), HI3536_NVRL HI3536_NVRH))
CPPFLAGS := $(SOC_CFLAGS) -Wall -Wextra -Wno-unused-parameter -Wno-ignored-qualifiers -Werror $(STD_COMPILATION_FLAGS) $(APP_CUSTOM_FLAGS) $(INCLUDE_PATHS_FLAGS) -std=c++11
else
CPPFLAGS := $(CFLAGS)
endif

# Prepare linker flags
LDFLAGS := $(SOC_LDFLAGS) -pthread --sysroot=$(TOOLCHAIN_SYSROOT_PATH)

# Export compilation flags (e.g. c and cpp flags)
export CFLAGS CPPFLAGS COMPILE_ONLY_FLAG INCLUDE_FLAG LIB_DIR_FLAG OPT_FLAG PROJ_INSTALL_DIR

#########################################################################
# Source / Object lists
#########################################################################
SRC_CPP_FILE_LIST = $(wildcard $(addsuffix /*.cpp, $(APP_SRC_SUB_DIR)))
SRC_C_FILE_LIST = $(wildcard $(addsuffix /*.c, $(APP_SRC_SUB_DIR)))

PROJ_CPP_OBJ_LIST := $(foreach var, $(SRC_CPP_FILE_LIST), $(PROJ_OBJ_DIR)/$(notdir $(var:.cpp=$(OBJ_EXTENSION))))
PROJ_C_OBJ_LIST := $(foreach var, $(SRC_C_FILE_LIST), $(PROJ_OBJ_DIR)/$(notdir $(var:.c=$(OBJ_EXTENSION))))
PROJ_OBJ_LIST := $(PROJ_CPP_OBJ_LIST) $(PROJ_C_OBJ_LIST)
PROJ_DEPEND_LIST := $(PROJ_OBJ_LIST:$(OBJ_EXTENSION)=$(DEPEND_EXTENSION))

#########################################################################
# TARGETS
#########################################################################
# if no goal (target) was specified then assume "all"
.PHONY: default
default: all

appl: makedir start_msg utils nvrapp end_msg
svnup:
	# Get latest copy of source from SVN
	$(_@_) svn $(SVN_CREDENTIALS) co --force $(patsubst $(SVN_HOME_DIR)%,$(SVN_REPO_URL)%,$(NVRX_SRC_PATH))			$(NVRX_SRC_PATH)
	$(_@_) svn $(SVN_CREDENTIALS) co --force $(patsubst $(SVN_HOME_DIR)%,$(SVN_REPO_URL)%,$(GPIO_DRIVER_INCLUDE_PATH))	$(GPIO_DRIVER_INCLUDE_PATH)

makedir:
	$(_@_) mkdir -m775 -p $(PROJ_BIN_DIR)
	$(_@_) mkdir -m775 -p $(PROJ_OBJ_DIR)
	$(_@_) mkdir -m775 -p $(PROJ_BIN_DIR_APPCLIENT)
	$(_@_) mkdir -m775 -p $(PROJ_OBJ_DIR_APPCLIENT)

start_msg:
	$(_@_) echo -e "\033[1;91m===========================================================================\033[0m"
	$(_@_) echo -e "\033[1;32m Compiling NVR application and utilities for $(APP_BUILD_TAG)\033[0m"
	$(_@_) echo -e "\033[1;91m===========================================================================\033[0m"

end_msg:
	$(_@_) echo -e ""
	$(_@_) echo -e "\033[1;31mApplication Created Successfully for $(APP_BUILD_TAG)\033[0m"
	$(_@_) echo -e "\033[32m`ls -go $(PROJ_BIN_DIR)/$(NVR_APPL_BIN)`\033[0m"
	$(_@_) echo -e ""

utils:
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(APP_SOCKET_IP_CLIENT_PATH) PROJ_OBJ_DIR=$(PROJ_OBJ_DIR_APPCLIENT)
	$(_@_) $(CC) $(COMPILE_ONLY_FLAG) $(DEBUG_LOG_PATH)/DebugLog.c $(CFLAGS) -DSYS_CMD_EXE_APP -o $(PROJ_OBJ_DIR_APPCLIENT)/DebugLog.o
	$(_@_) $(CC) $(COMPILE_ONLY_FLAG) $(UTILS_PATH)/CommonApi.c $(CFLAGS) -o $(PROJ_OBJ_DIR_APPCLIENT)/CommonApi.o
	$(_@_) $(CC) $(CFLAGS) -o $(PROJ_BIN_DIR_APPCLIENT)/$(DEV_DETECT_BIN) $(PROJ_OBJ_DIR_APPCLIENT)/$(DEV_DETECT_OBJ)
	$(_@_) $(CC) $(CFLAGS) -o $(PROJ_BIN_DIR_APPCLIENT)/$(SYSTEM_UPGRADE_BIN) $(PROJ_OBJ_DIR_APPCLIENT)/$(SYSTEM_UPGRARE_OBJ) -lcrypto
	$(_@_) $(CC) $(CFLAGS) -o $(PROJ_BIN_DIR_APPCLIENT)/$(NVR_NOTIFY_BIN) $(PROJ_OBJ_DIR_APPCLIENT)/$(NVR_NOTIFY_OBJ)
	$(_@_) $(CC) $(CFLAGS) -o $(PROJ_BIN_DIR_APPCLIENT)/$(SYS_CMD_EXE_BIN) $(PROJ_OBJ_DIR_APPCLIENT)/$(SYS_CMD_EXE_OBJ) $(PROJ_OBJ_DIR_APPCLIENT)/DebugLog.o $(PROJ_OBJ_DIR_APPCLIENT)/CommonApi.o
	$(_@_) $(STRIP) $(PROJ_BIN_DIR_APPCLIENT)/*

nvrapp: $(NVR_APPL_BIN) rtspclient
	$(_@_) $(LINK) $(LDFLAGS) -o $(PROJ_BIN_DIR)/$(NVR_APPL_BIN) $(PROJ_OBJ_LIST) $(LIBRARY_DIRS:%=$(LIB_DIR_FLAG)%) $(APP_LIBS_NEEDED)
	$(_@_) $(STRIP) $(PROJ_BIN_DIR)/*

rtspclient:
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(RTSP_CLIENT_PATH) PROJ_OBJ_DIR=$(PROJ_OBJ_DIR_APPCLIENT)
	$(_@_) $(CC) $(COMPILE_ONLY_FLAG) $(DEBUG_LOG_PATH)/DebugLog.c $(CFLAGS) -DRTSP_CLIENT_APP -o $(PROJ_OBJ_DIR_APPCLIENT)/DebugLog.o

	$(_@_) $(LINK) $(LDFLAGS) -o $(PROJ_BIN_DIR)/$(RTSP_CLIENT_BIN) $(PROJ_OBJ_DIR_APPCLIENT)/Live555Interface.o $(PROJ_OBJ_DIR_APPCLIENT)/RtspClient.o \
	$(PROJ_OBJ_DIR_APPCLIENT)/DebugLog.o $(PROJ_OBJ_DIR)/VideoParser.o $(PROJ_OBJ_DIR)/UtilCommon.o $(PROJ_OBJ_DIR)/AudioParser.o $(PROJ_OBJ_DIR)/CommonApi.o \
	$(LIBRARY_DIRS:%=$(LIB_DIR_FLAG)%) -lpthread -lrt $(LIVE555_LIB) $(CRYPTO_LIBS)

$(NVR_APPL_BIN):
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(BACKUP_MANAGER_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(CAMERA_INITIATION_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(CAMERA_INTERFACE_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(CONFIG_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(DATE_TIME_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(DDNS_CLIENT_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(DEBUG_LOG_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(DEVICE_INITIATION_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(DHCP_SERVER_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(DISK_MANAGER_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(EVENT_HANDLER_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(EVENT_LOGGER_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(FTP_CLIENT_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(HTTP_CLIENT_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(IMAGE_UPLOAD_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(MATRIX_MAC_CLIENT_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(MEDIA_STREAMER_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(NETWORK_CONTROLLER_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(MEDIA_STREAMER_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(NETWORK_MANAGER_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(DEV_INFO_CLIENT_DIR)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(EMAIL_NOTIFY_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(SMS_NOTIFY_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(TCP_NOTIFY_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(PUSH_NOTIFICATION_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(ONVIF_CLIENT_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(P2P_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(PTZ_TOUR_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(RECORD_MANGER_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(SNAP_SHOT_SCHD_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(SYS_TIMER_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(SYSTEM_UPGRADE_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(UTILS_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(MATRIX_BRAND_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(INPUT_OUTPUT_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(RTSP_CLIENT_INTERFACE_PATH)
	$(_@_) make $(NO_PRINT_DIR) -f Common.mk MODULE_SOURCE_DIR=$(NVR_APPL_SRC_PATH)

appClean:
	rm -rf $(PROJ_BUILD_PATH)

.PHONY: depend
depend:
	$(_@_) echo Only dependencies generated.
	$(_@_) echo $(PROJ_OBJ_LIST)

.PHONY: clean
clean: appClean
	$(_@_) echo Cleaning all object and binary files

# Include module dependencies list
-include $(PROJ_DEPEND_LIST)

#########################################################################
# END OF MAKEFILE
#########################################################################
