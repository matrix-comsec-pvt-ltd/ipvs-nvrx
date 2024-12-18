#########################################################################
# Set default shell
#########################################################################
SHELL := /bin/bash

#########################################################################
# Default variables
#########################################################################
RED         =\033[91m
GREEN       =\033[92m
YELLOW      =\033[93m
RED_BOLD    =\033[1;91m
GREEN_BOLD  =\033[1;92m
YELLOW_BOLD =\033[1;93m
COLOR       =\033[0m

# Define default oem type
OEM_TYPE ?= OEM_NONE

# Validate board type
ifneq ($(BOARD_TYPE), $(filter $(BOARD_TYPE), HI3536_NVRL HI3536_NVRH RK3568_NVRL RK3588_NVRH))
    $(error Please provide a valid board type)
endif

# Validate oem type
ifneq ($(OEM_TYPE), $(filter $(OEM_TYPE), OEM_NONE OEM_JCI))
    $(error Please provide a valid oem type)
endif

# Validate oem type combination with board type
ifeq ($(OEM_TYPE), OEM_JCI)
ifneq ($(BOARD_TYPE), RK3588_NVRH)
    $(error Invalid board type for oem: BOARD_TYPE=$(BOARD_TYPE) OEM_TYPE=$(OEM_TYPE))
endif
endif

#########################################################################
# Set variables which are common for all
#########################################################################
# Set software version, revision & sub-revision
SOFTWARE_VERSION=2
SOFTWARE_REVISION=2
PRODUCT_SUB_REVISION=0

# Set device client version
DEVICE_CLIENT_VERSION=8
DEVICE_CLIENT_REVISION=7
DEVICE_CLIENT_SUB_REVISION=52

# Set communication protocol version & revision
COMMUNICATION_VERSION=4
COMMUNICATION_REVISION=21

# Generate product version-revision string
PRODUCT_VER_REV_STR=$(shell printf "%d.%d.%d" $(SOFTWARE_VERSION) $(SOFTWARE_REVISION) $(PRODUCT_SUB_REVISION))

# Prepare device name prefix for application and package
ifeq ($(OEM_TYPE), OEM_JCI)
DEVICE_NAME_PREFIX := HRIN_12_28_48_64
else
ifeq ($(BOARD_TYPE), RK3568_NVRL)
DEVICE_NAME_PREFIX := NVR_8X_16X_P2
else ifeq ($(BOARD_TYPE), RK3588_NVRH)
DEVICE_NAME_PREFIX := NVR_RK3588H
DEVICE_NAME_PREFIX_FOR_PACKAGE := nvrx_rk3588h
endif
endif

#########################################################################
# Set different paths which are common for all
#########################################################################
# SVN repository URL
##SVN_REPO_URL=svn://192.168.100.5
REPO_IPVS_NVR_RK3588_PATH:=$(shell pwd)
BUILD_DIR_PATH="$(REPO_IPVS_NVR_RK3588_PATH)/src/Application/Build"
DEPS_PREBUILT_INTERNAL_DIR="$(REPO_IPVS_NVR_RK3588_PATH)/deps/prebuilt/internal"
DEPS_PREBUILT_THIRD_PARTY_DIR="$(REPO_IPVS_NVR_RK3588_PATH)/deps/prebuilt/third-party"
DEPS_SUBMODULES_DIR="$(REPO_IPVS_NVR_RK3588_PATH)/deps/submodules"
DEPS_SUBMODULES_PLATFORM_DIR="$(DEPS_SUBMODULES_DIR)/ipvs-nvr-rk3588-plat"

# NVR SDT folder path
##NVR_SDT_PATH=$(SVN_HOME_DIR)/Products/SATATYA_DEVICES/SDT

# NVR product briefcase path
##PRODUCT_BRIEFCASE_PATH=$(NVR_SDT_PATH)/Briefcase

# Software release path
##SOFTWARE_RELEASE_PATH=$(NVR_SDT_PATH)/Software_Releases

# Common SWD briefcase path
##COMMON_SWD_BRIEFCASE_PATH=$(SVN_HOME_DIR)/Briefcase/SWD_Briefcase

# Embedded Software modules path
##EMBEDDED_SWD_MODULE_PATH=$(COMMON_SWD_BRIEFCASE_PATH)/EmbeddedSoftwareModules

# Generic Software modules path
##GENERIC_SWD_MODULE_PATH=$(COMMON_SWD_BRIEFCASE_PATH)/GenericSoftwareModules

#########################################################################
# Set Product dependent variables
#########################################################################
ifeq ($(BOARD_TYPE), $(filter $(BOARD_TYPE), RK3568_NVRL RK3588_NVRH))
ifeq ($(BOARD_TYPE), RK3568_NVRL)
BUILDROOT_INSTALL_PATH=/opt/rk3568_nvr
PREBUILT_PACKAGES_PATH=$(PRODUCT_BRIEFCASE_PATH)/Prebuilt_Packages/aarch64-linux-9.3.0
else
BUILDROOT_INSTALL_PATH=/opt/rk3588_nvrx
##PREBUILT_PACKAGES_PATH=$(PRODUCT_BRIEFCASE_PATH)/Prebuilt_Packages/aarch64-linux-10.3.1
PREBUILT_INTERNAL_PACKAGES_PATH=$(DEPS_PREBUILT_INTERNAL_DIR)/aarch64-linux-10.3.1
PREBUILT_THIRD_PARTY_PACKAGES_PATH=$(DEPS_PREBUILT_THIRD_PARTY_DIR)/aarch64-linux-10.3.1
endif
CROSS_COMPILE=$(BUILDROOT_INSTALL_PATH)/host/bin/aarch64-linux-
TOOLCHAIN_SYSROOT_PATH=$(BUILDROOT_INSTALL_PATH)/host/aarch64-buildroot-linux-gnu/sysroot
HOST_SYSROOT_PATH=$(BUILDROOT_INSTALL_PATH)/host
SOC_CFLAGS= -march=armv8-a+crc -fstack-protector-strong -Wformat -Wformat-security -Werror=format-security --sysroot=$(TOOLCHAIN_SYSROOT_PATH)
SOC_LDFLAGS=
TOOLCHAIN_TAR_NAME=
TOOLCHAIN_TAR_PATH=
SOC_MISC_DATA_PATH=
HW_DECODER_LIB_PATH := $(TOOLCHAIN_SYSROOT_PATH)/usr/lib
HW_DECODER_INCLUDE_PATH := $(TOOLCHAIN_SYSROOT_PATH)/usr/include/rockit
endif

#########################################################################
# Set prebuilt packages path
#########################################################################
SOAP_INSTALL_PATH=$(PREBUILT_THIRD_PARTY_PACKAGES_PATH)/gsoap-2.8.97/install
INADYN_INSTALL_PATH=$(PREBUILT_THIRD_PARTY_PACKAGES_PATH)/inadyn-2.12.0/install
LIVE555_INSTALL_PATH=$(PREBUILT_THIRD_PARTY_PACKAGES_PATH)/live555.2024.05.30/install
JANSSON_INSTALL_PATH=$(PREBUILT_THIRD_PARTY_PACKAGES_PATH)/jansson-2.13.1/install
QRENCODE_INSTALL_PATH=$(PREBUILT_THIRD_PARTY_PACKAGES_PATH)/qrencode-4.1.1/install
DNSMASQ_INSTALL_PATH=$(PREBUILT_THIRD_PARTY_PACKAGES_PATH)/dnsmasq-2.85/install
COTURN_INSTALL_PATH=$(PREBUILT_THIRD_PARTY_PACKAGES_PATH)/coturn-docker-4.6.1-r0/install
ODHCP6C_INSTALL_PATH=$(PREBUILT_THIRD_PARTY_PACKAGES_PATH)/odhcp6c-master-git-bcd2836/install
MONGOOSE_INSTALL_PATH=$(PREBUILT_THIRD_PARTY_PACKAGES_PATH)/mongoose-7.14/install
NETWORK_MANAGER_INSTALL_PATH=$(PREBUILT_INTERNAL_PACKAGES_PATH)/nwmanager-1.0.0/install

#########################################################################
# Set NVR package release path
#########################################################################
#ifeq ($(RELEASE_TYPE),QA)
#RELEASE_TYPE_FOLDER_NAME=QA
#else
#RELEASE_TYPE_FOLDER_NAME=Production
#endif

#ifeq ($(PRODUCT_SUB_REVISION),0)
#RELEASE_FOLDER_NAME=$(shell printf "V%02dR%02d" $(SOFTWARE_VERSION) $(SOFTWARE_REVISION))
#else
#RELEASE_FOLDER_NAME=$(shell printf "V%02dR%02d.%02d" $(SOFTWARE_VERSION) $(SOFTWARE_REVISION) $(PRODUCT_SUB_REVISION))
#endif
#PACKAGE_RELEASE_PATH=$(SOFTWARE_RELEASE_PATH)/VideoRecorder/$(RELEASE_TYPE_FOLDER_NAME)/$(RELEASE_FOLDER_NAME)

#########################################################################
# Set Device client release path
#########################################################################
#ifeq ($(RELEASE_TYPE),QA)
#DEVICE_CLIENT_PRODUCT_PATH=$(SOFTWARE_RELEASE_PATH)/DeviceClient/Windows/Sprint_QA/SATATYA_NVRX
#else
#DEVICE_CLIENT_PRODUCT_PATH=$(SOFTWARE_RELEASE_PATH)/DeviceClient/Windows/Production/SATATYA_NVRX
#endif

#ifeq ($(DEVICE_CLIENT_SUB_REVISION),0)
#DEVICE_CLIENT_VER_REV_NAME=$(shell printf "V%02dR%02d" $(DEVICE_CLIENT_VERSION) $(DEVICE_CLIENT_REVISION))
#else
#DEVICE_CLIENT_VER_REV_NAME=$(shell printf "V%02dR%02d.%d" $(DEVICE_CLIENT_VERSION) $(DEVICE_CLIENT_REVISION) $(DEVICE_CLIENT_SUB_REVISION))
#endif

#########################################################################
# Set platform release path
#########################################################################
ifeq ($(BOARD_TYPE), RK3568_NVRL)
PLATFORM_RELEASE_PATH=$(GENERIC_SWD_MODULE_PATH)/Platform/Software_Releases/NVR_RK3568/8.7.0
PLATFORM_HOST_PATH=$(PLATFORM_RELEASE_PATH)/host
else ifeq ($(BOARD_TYPE), RK3588_NVRH)
#PLATFORM_RELEASE_PATH=$(GENERIC_SWD_MODULE_PATH)/Platform/Software_Releases/NVRX_RK3588/8.7.0
PLATFORM_RELEASE_PATH=$(DEPS_SUBMODULES_PLATFORM_DIR)/src
#PLATFORM_HOST_PATH=$(PLATFORM_RELEASE_PATH)/host
PLATFORM_HOST_PATH=$(DEPS_SUBMODULES_PLATFORM_DIR)/src/host
endif

#########################################################################
# END OF MAKEFILE
#########################################################################
