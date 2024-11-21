#************************************************************************
# Filename	: Makefile
# Description	: Makefile to compile NVRX products
# NOTE		: This make file uses moderately advanced feature of gnu make
#		  and should be edited with proper care.
#************************************************************************

#########################################################################
# Different packages name, folder paths
#########################################################################
# Sourcing of common variables
include define.mk

# Get current buid path
NVRX_SOURCE_PATH := $(patsubst %/,%,$(shell pwd))/src
#NVRX_SOURCE_PATH = $(GIT_HOME_DIR)/ipvs-nvr-app/src
#PRODUCT_SOURCE_PATH := $(patsubst %/,%,$(dir $(shell pwd)))

# Buildroot lib path
BUILDROOT_USR_LIB_PATH := $(BUILDROOT_INSTALL_PATH)/host/usr/lib

# NVR main application source path
NVR_APP_BUILD_PATH := $(NVRX_SOURCE_PATH)/Application/Build

# Application build folder tag
ifneq ($(OEM_TYPE), OEM_NONE)
APP_BUILD_TAG := $(OEM_TYPE)
else
APP_BUILD_TAG := $(BOARD_TYPE)
endif

# NVR main application and utilies build path
NVR_APP_BIN_PATH := $(NVR_APP_BUILD_PATH)/$(APP_BUILD_TAG)/bin
NVR_APP_CLIENT_BIN_PATH := $(NVR_APP_BUILD_PATH)/$(APP_BUILD_TAG)/binAppClient

# NVR GUI application and library source path
NVR_GUI_LIB_PATH := $(NVRX_SOURCE_PATH)/GuiApplication/DecoderLib
NVR_GUI_APP_PATH := $(NVRX_SOURCE_PATH)/GuiApplication/nvrgui
NVR_GUI_TEST_APP_PATH := $(NVRX_SOURCE_PATH)/GuiApplication/nvrguitest

# NVR GUI application and test application build path
NVR_GUI_BIN_PATH := $(NVR_GUI_APP_PATH)/Build/$(APP_BUILD_TAG)/bin
NVR_GUI_TEST_BIN_PATH := $(NVR_GUI_TEST_APP_PATH)/Build/$(APP_BUILD_TAG)/bin

# Platform host and target paths
ifeq ($(BOARD_TYPE), $(filter $(BOARD_TYPE), RK3568_NVRL RK3588_NVRH))
#PLATFORM_TARGET_PATH := $(PLATFORM_RELEASE_PATH)/target
PLATFORM_UPGRADE_ZIP := $(NVR_APP_BUILD_PATH)/platform_upgrade.zip
endif

# GPIO driver include path
ifeq ($(BOARD_TYPE), $(filter $(BOARD_TYPE), RK3568_NVRL RK3588_NVRH))
GPIO_DRIVER_INCLUDE_PATH := $(PLATFORM_HOST_PATH)/drv_include
endif

# Field upgrade zip target path
#NVR_FIELD_RELEASE_PATH := $(PACKAGE_RELEASE_PATH)/$(APP_BUILD_TAG)/web
PRODUCT_FIRMWARE_NAME := $(DEVICE_NAME_PREFIX)-$(PRODUCT_VER_REV_STR).zip

# Prepare device client name
DEVICE_CLIENT_EXE_NAME := Device_Client_Setup_$(DEVICE_CLIENT_VERSION).$(DEVICE_CLIENT_REVISION).$(DEVICE_CLIENT_SUB_REVISION).exe

# Temp path for field package building
PACKAGE_BUILD_PATH := $(NVRX_SOURCE_PATH)/package

# rootfs target path for production package (BUILDROOT_TARGET_PATH will passed by platform)
ROOTFS_TARGET_PATH := $(BUILDROOT_TARGET_PATH)

# Common and product specific filesystem content path for package building
ROOTFS_CONFIG_PATH := $(NVRX_SOURCE_PATH)/Config/rootfs

# qrencode path
QRENCODE_LIB_PATH := $(QRENCODE_INSTALL_PATH)/lib
QRENCODE_INCLUDE_PATH := $(QRENCODE_INSTALL_PATH)/include

# File for firmware upgrade confirmation to prevent config corruption
FIRMWARE_UPGRADE_CONFIRMATION_FILE := /tmp/firmware_upgrade_done.txt

# Mainly used by GUI application
export BOARD_TYPE OEM_TYPE APP_BUILD_TAG
export DEVICE_NAME_PREFIX FIRMWARE_UPGRADE_CONFIRMATION_FILE
export SOFTWARE_VERSION SOFTWARE_REVISION PRODUCT_SUB_REVISION
export COMMUNICATION_VERSION COMMUNICATION_REVISION
export GPIO_DRIVER_INCLUDE_PATH
export HW_DECODER_INCLUDE_PATH HW_DECODER_LIB_PATH
export BUILDROOT_USR_LIB_PATH
export QRENCODE_LIB_PATH QRENCODE_INCLUDE_PATH

ifeq ($(BOARD_TYPE), RK3568_NVRL)
export LD_LIBRARY_PATH=/usr/local/lib:$(BUILDROOT_INSTALL_PATH)/host/toolchain/lib
else ifeq ($(BOARD_TYPE), RK3588_NVRH)
export LD_LIBRARY_PATH=/usr/local/lib:$(BUILDROOT_INSTALL_PATH)/host/lib
endif

# Define the folder name for enterprise specific images
ifeq ($(OEM_TYPE), OEM_NONE)
export BRAND_TAG = Matrix
else ifeq ($(OEM_TYPE), OEM_JCI)
export BRAND_TAG = JCI
endif

CSV_FILE := $(BRAND_TAG)_English.csv

# SVN path of CSV files of SAD and SWD
#SATATYA_DEVICES_PATH := $(SVN_REPO_URL)/Products/SATATYA_DEVICES

#########################################################################
# TARGETS
#########################################################################
default: release

all: error_msg svnup toolchain release
release: error_msg appFinal guiFinal guiTestFinal field

error_msg:
	@$(call chk_board_type)
	@$(call chk_sudo_password)

svnup:
	# Get update of whole source of NVR
	#@svn $(SVN_CREDENTIALS) co --force $(patsubst $(SVN_HOME_DIR)%,$(SVN_REPO_URL)%,$(PRODUCT_SOURCE_PATH)) $(PRODUCT_SOURCE_PATH)

	# Get update of source packages and pre-installed packages
	#@svn $(SVN_CREDENTIALS) co --force $(patsubst $(SVN_HOME_DIR)%,$(SVN_REPO_URL)%,$(PREBUILT_PACKAGES_PATH)) $(PREBUILT_PACKAGES_PATH)

	# Get update of device client
	#@svn $(SVN_CREDENTIALS) co --force $(patsubst $(SVN_HOME_DIR)%,$(SVN_REPO_URL)%,$(DEVICE_CLIENT_PATH)) $(DEVICE_CLIENT_PATH)

	# Get update of platform release path
	#@svn $(SVN_CREDENTIALS) co --force $(patsubst $(SVN_HOME_DIR)%,$(SVN_REPO_URL)%,$(PLATFORM_RELEASE_PATH)) $(PLATFORM_RELEASE_PATH)
	#@svn $(SVN_CREDENTIALS) co --force $(patsubst $(SVN_HOME_DIR)%,$(SVN_REPO_URL)%,$(PLATFORM_HOST_PATH)) $(PLATFORM_HOST_PATH)

	# Get update of source dependent packages
	#@make -C $(NVR_APP_BUILD_PATH) svnup BOARD_TYPE=$(BOARD_TYPE) OEM_TYPE=$(OEM_TYPE)

toolchain: error_msg
	# Check host PC architecture. It must be 64bit
	@if [ `uname -i` != "x86_64" ]; then \
		echo -e "$(RED_BOLD)64bit host OS architecture is must for toolchain installation...$(COLOR)"; \
		exit 1; \
	fi

	# Get local user and group for ownership
	$(eval LOCAL_USER=$(shell whoami))
	$(eval LOCAL_GROUP=$(shell id -ng))

	# Extract toolchain in /opt folder and install it if not present
ifeq ($(BOARD_TYPE), $(filter $(BOARD_TYPE), RK3568_NVRL RK3588_NVRH))
	@if [ ! -d "$(HOST_SYSROOT_PATH)" ]; then \
		echo "Extracting prebuilt toolchain..."; \
		sudo -S mkdir -p $(BUILDROOT_INSTALL_PATH); \
		sudo -S chown -R $(LOCAL_USER):$(LOCAL_GROUP) $(BUILDROOT_INSTALL_PATH); \
		tar -xvf $(NVR_APP_BUILD_PATH)/host.tar.xz -C $(BUILDROOT_INSTALL_PATH); \
	fi
endif

appFinal: appClean appl

appl:
	cd $(NVR_APP_BUILD_PATH) && make appl BOARD_TYPE=$(BOARD_TYPE) OEM_TYPE=$(OEM_TYPE)

appClean:
	cd $(NVR_APP_BUILD_PATH) && make appClean BOARD_TYPE=$(BOARD_TYPE) OEM_TYPE=$(OEM_TYPE)

guiFinal: guiLibClean guiLib guiAppClean guiApp

guiLibClean:
	cd $(NVR_GUI_LIB_PATH) && make clean CROSS_COMPILE=$(CROSS_COMPILE) BOARD_TYPE=$(BOARD_TYPE) OEM_TYPE=$(OEM_TYPE)

guiLib:
	cd $(NVR_GUI_LIB_PATH) && make all CROSS_COMPILE=$(CROSS_COMPILE) BOARD_TYPE=$(BOARD_TYPE) OEM_TYPE=$(OEM_TYPE)

guiApp:
	cd $(NVR_GUI_APP_PATH) && $(HOST_SYSROOT_PATH)/usr/bin/qmake -o Build/$(APP_BUILD_TAG)/Makefile "DEFINES+=$(BOARD_TYPE) $(OEM_TYPE)" multiNvrClient.pro
	$(MAKE) -C $(NVR_GUI_APP_PATH)/Build/$(APP_BUILD_TAG) -j16

guiAppClean:
	mkdir -p $(NVR_GUI_APP_PATH)/Build/$(APP_BUILD_TAG)
	cd $(NVR_GUI_APP_PATH) && $(HOST_SYSROOT_PATH)/usr/bin/qmake -o Build/$(APP_BUILD_TAG)/Makefile "DEFINES+=$(BOARD_TYPE) $(OEM_TYPE)" multiNvrClient.pro
	$(MAKE) -C $(NVR_GUI_APP_PATH)/Build/$(APP_BUILD_TAG) distclean

guiTestFinal: guiAppTestClean guiAppTest

guiAppTest:
	cd $(NVR_GUI_TEST_APP_PATH) && $(HOST_SYSROOT_PATH)/usr/bin/qmake -o Build/$(APP_BUILD_TAG)/Makefile "DEFINES+=$(BOARD_TYPE) $(OEM_TYPE)" nvrTestApp.pro
	$(MAKE) -C $(NVR_GUI_TEST_APP_PATH)/Build/$(APP_BUILD_TAG) -j16

guiAppTestClean:
	mkdir -p $(NVR_GUI_TEST_APP_PATH)/Build/$(APP_BUILD_TAG)
	cd $(NVR_GUI_TEST_APP_PATH) && $(HOST_SYSROOT_PATH)/usr/bin/qmake -o Build/$(APP_BUILD_TAG)/Makefile "DEFINES+=$(BOARD_TYPE) $(OEM_TYPE)" nvrTestApp.pro
	$(MAKE) -C $(NVR_GUI_TEST_APP_PATH)/Build/$(APP_BUILD_TAG) distclean

field: error_msg
	@echo "****************************************************************************************"
	@echo "$(APP_BUILD_TAG): Building application field package"
	@echo "****************************************************************************************"
	# Remove older package build path and create new one
	#rm -rf $(NVR_FIELD_RELEASE_PATH)/*
	@echo $(PASSWORD) | sudo -S rm -rf $(PACKAGE_BUILD_PATH)
	mkdir -p $(PACKAGE_BUILD_PATH)

	# Create file system related paths
	mkdir -p $(PACKAGE_BUILD_PATH)/tmp
	mkdir -p $(PACKAGE_BUILD_PATH)/etc
	mkdir -p $(PACKAGE_BUILD_PATH)/usr/sbin
	mkdir -p $(PACKAGE_BUILD_PATH)/matrix/bin
	mkdir -p $(PACKAGE_BUILD_PATH)/matrix/lib
	mkdir -p $(PACKAGE_BUILD_PATH)/matrix/web/html_pages/cgi-bin

	# Extract platform upgrade zip in package
	unzip $(PLATFORM_UPGRADE_ZIP) -d $(PACKAGE_BUILD_PATH)

	# Copy rootfs related fix contents in package
	cp -r $(ROOTFS_CONFIG_PATH)/* $(PACKAGE_BUILD_PATH)/

ifeq ($(BOARD_TYPE), RK3568_NVRL)
	mv -f $(PACKAGE_BUILD_PATH)/matrix/scripts/nvr-x_rk3568.sh $(PACKAGE_BUILD_PATH)/matrix/scripts/nvr-x.sh
else ifeq ($(BOARD_TYPE), RK3588_NVRH)
	mv -f $(PACKAGE_BUILD_PATH)/matrix/scripts/nvr-x_rk3588.sh $(PACKAGE_BUILD_PATH)/matrix/scripts/nvr-x.sh
endif
	cd $(PACKAGE_BUILD_PATH)/matrix/scripts && rm nvr-x_*.sh

	mv -f $(PACKAGE_BUILD_PATH)/matrix/languages/$(CSV_FILE) $(PACKAGE_BUILD_PATH)/matrix/languages/English.csv
	rm $(PACKAGE_BUILD_PATH)/matrix/languages/*_English.csv

	# Copy third party package binary in package
	cp -r $(NETWORK_MANAGER_INSTALL_PATH)/etc/* $(PACKAGE_BUILD_PATH)/etc
	cp -r $(NETWORK_MANAGER_INSTALL_PATH)/bin/* $(PACKAGE_BUILD_PATH)/matrix/bin
	cp $(ODHCP6C_INSTALL_PATH)/sbin/odhcp6c $(PACKAGE_BUILD_PATH)/usr/sbin
	cp $(INADYN_INSTALL_PATH)/sbin/inadyn $(PACKAGE_BUILD_PATH)/matrix/bin
	cp $(MONGOOSE_INSTALL_PATH)/bin/webserver $(PACKAGE_BUILD_PATH)/usr/sbin

	# Copy compiled binaries in matrix bin folder
	cp $(NVR_APP_BIN_PATH)/*.bin $(PACKAGE_BUILD_PATH)/matrix/bin
	cp $(NVR_APP_CLIENT_BIN_PATH)/*.bin $(PACKAGE_BUILD_PATH)/matrix/bin

	# Copy complied shared libraries in matrix folder
	cp $(SOAP_INSTALL_PATH)/lib/libsoap.so $(PACKAGE_BUILD_PATH)/matrix/lib
	cp $(NVR_GUI_LIB_PATH)/Build/$(APP_BUILD_TAG)/lib/libDecDisplay.so $(PACKAGE_BUILD_PATH)/matrix/lib

	# Copy device client related contents
	cp $(NVR_APP_BUILD_PATH)/$(DEVICE_CLIENT_EXE_NAME) $(PACKAGE_BUILD_PATH)/matrix/web/html_pages/Device_Client_Setup.exe

	# Copy GUI application in matrix bin folder
	cp $(NVR_GUI_BIN_PATH)/multiNvrClient $(PACKAGE_BUILD_PATH)/matrix/bin

	# Copy NVR test application in matrix folder
	cp $(NVR_GUI_TEST_BIN_PATH)/nvrTestApp $(PACKAGE_BUILD_PATH)/matrix/bin

	# Add firmware upgrade confirmation file to prevent config corruption
	touch $(PACKAGE_BUILD_PATH)/$(FIRMWARE_UPGRADE_CONFIRMATION_FILE)

	# Remove svn related config files from package
	find $(PACKAGE_BUILD_PATH) \( -name '.svn' -o -name '.directory' \) -exec rm -rf {} +

	# Strip all binaries and library
	$(CROSS_COMPILE)strip -R .comment -R .note -S --strip-unneeded $(PACKAGE_BUILD_PATH)/matrix/bin/*
	$(CROSS_COMPILE)strip -R .comment -R .note -S --strip-unneeded $(PACKAGE_BUILD_PATH)/matrix/lib/*
	$(CROSS_COMPILE)strip -R .comment -R .note -S --strip-unneeded $(PACKAGE_BUILD_PATH)/usr/sbin/*
	# Provide required permissions and ownership
	chmod -R 755 $(PACKAGE_BUILD_PATH)/*
	@echo $(PASSWORD) | sudo -S chown -R root:root $(PACKAGE_BUILD_PATH)/*

	cd $(PACKAGE_BUILD_PATH) && zip -ry9 $(NVR_APP_BUILD_PATH)/$(PRODUCT_FIRMWARE_NAME) *
	echo -n -e `md5sum $(NVR_APP_BUILD_PATH)/$(PRODUCT_FIRMWARE_NAME) | awk '{print $$1}' | sed 's/../\\\x&/g'` >> $(NVR_APP_BUILD_PATH)/$(PRODUCT_FIRMWARE_NAME)
	echo -n -e "\x44\x33\x22\x11" >> $(NVR_APP_BUILD_PATH)/$(PRODUCT_FIRMWARE_NAME)

	##mkdir -p $(NVR_FIELD_RELEASE_PATH)
	##mv $(NVRX_SOURCE_PATH)/$(PRODUCT_FIRMWARE_NAME) $(NVR_FIELD_RELEASE_PATH)
	@sudo -S rm -rf $(PACKAGE_BUILD_PATH)

#########################################################################
# DEFINED FUNCTIONS
#########################################################################
chk_board_type = @if [ $(BOARD_TYPE) != RK3568_NVRL ] && [ $(BOARD_TYPE) != RK3588_NVRH ]; then \
		echo -e "$(RED_BOLD)"; \
		echo -e "******** You must specify correct board type ********"; \
		echo -e "$(BOLD)"; \
		make -s help; \
		exit 1; \
		fi;

chk_sudo_password = @if [ -z $(PASSWORD) ]; then \
		echo -e "$(RED_BOLD)"; \
		echo -e "******** You must specify sudo password ********"; \
		echo -e "$(BOLD)"; \
		make -s help; \
		exit 1; \
		fi;

#########################################################################
# END OF MAKEFILE
#########################################################################
