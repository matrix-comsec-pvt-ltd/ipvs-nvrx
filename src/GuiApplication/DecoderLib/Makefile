#########################################################################
# Check options passed to make file
#########################################################################
# Sourcing of common variables
include ../../../define.mk

CC := $(CROSS_COMPILE)gcc
AR := $(CROSS_COMPILE)ar
LINK := $(CROSS_COMPILE)gcc

ifeq ($(DEBUG), on)
DEBUG_FLAG := -g
else
DEBUG_FLAG :=
endif

ifeq ($(VERBOSE), on)
_@_ :=
else
_@_ := @
endif

#########################################################################
# COMPILER SELECTION + PROJECT EXTENSIONS
#########################################################################
DEPEND_EXTENSION = .d
OBJ_EXTENSION = .o
NODEPEND_TARGETS := clean

#########################################################################
# SOURCE DIRECTORY PATH
#########################################################################
ifeq ($(BOARD_TYPE), $(filter $(BOARD_TYPE), HI3536_NVRL HI3536_NVRH))
MODULE_SOURCE_DIR := ./hisilicon
else ifeq ($(BOARD_TYPE), $(filter $(BOARD_TYPE), RK3568_NVRL RK3588_NVRH))
MODULE_SOURCE_DIR := ./rockchip
endif

DECODER_LIB_BUILD_DIR := ./Build/$(APP_BUILD_TAG)
MODULE_LIB_DIR := $(DECODER_LIB_BUILD_DIR)/lib
MODULE_OBJ_DIR := $(DECODER_LIB_BUILD_DIR)/obj
MODULE_LIB_NAME := libDecDisplay.so

#########################################################################
# INCLUDE DIRECTORY PATH (SOURCE + LIBRARY)
#########################################################################
INCLUDE_DIRS += ./include
INCLUDE_DIRS += $(HW_DECODER_INCLUDE_PATH)
INCLUDE_DIRS += ../../Application/DebugLog
INCLUDE_DIRS += $(TOOLCHAIN_SYSROOT_PATH)/usr/include/drm

#########################################################################
# COMPILER & LINKER RELATED VARIABLES
#########################################################################
COMPILE_ONLY_FLAG := -c
INCLUDE_FLAG := -I
LIB_DIR_FLAG := -L
CC_OBJ_OUTPUT = -o $@
CFLAGS := $(SOC_CFLAGS) -fPIC -Wall -pthread -Werror $(DEBUG_FLAG) -DGUI_SYSTEM -D$(BOARD_TYPE) -D$(OEM_TYPE)
CFLAGS += $(INCLUDE_DIRS:%=$(INCLUDE_FLAG)%)

LDFLAGS += $(SOC_LDFLAGS) -shared -fPIC -o $(MODULE_LIB_DIR)/$(MODULE_LIB_NAME) -pthread

#########################################################################
# Source / Object lists
#########################################################################
# Create object list for both CPP and C files
MODULE_C_OBJ_LIST := $(foreach src,$(wildcard $(MODULE_SOURCE_DIR)/*.c),$(MODULE_OBJ_DIR)/$(notdir $(src:.c=$(OBJ_EXTENSION))))
MODULE_OBJ_LIST := $(MODULE_CPP_OBJ_LIST) $(MODULE_C_OBJ_LIST)

#########################################################################
# GENERATING DEPENDENCIES
#########################################################################
MAKEDEPEND = set -e; $(CC) -P -M -c $(CFLAGS) $< | \
		sed '1s=^=$@ $(PROJ_OBJ_DIR)/$(*D)/=' > $@; \
		[ -s $@ ] || rm -f $@

#########################################################################
# CREATING DEPENDENCIES FOR CPP / C FILES
#########################################################################
$(MODULE_OBJ_DIR)/%$(DEPEND_EXTENSION): $(MODULE_SOURCE_DIR)/%.c
	$(_@_) [ -d $(@D) ] || mkdir -p $(@D)
	$(_@_) $(MAKEDEPEND)

#########################################################################
# COMPILING CPP / C FILES
#########################################################################
$(MODULE_OBJ_DIR)/%$(OBJ_EXTENSION): $(MODULE_SOURCE_DIR)/%.c $(MODULE_OBJ_DIR)/%$(DEPEND_EXTENSION)
	@echo -e "\033[94m[compiling]\033[92m $(notdir $(CC))\033[0m" $(notdir $<) $(ECHO_STDOUT)
	$(_@_) $(CC) $(COMPILE_ONLY_FLAG) $(CFLAGS) $< $(CC_OBJ_OUTPUT)

#########################################################################
# TARGETS
#########################################################################
# if no goal (target) was specified then assume "all"
.PHONY: default
default: all

.PHONY: clean
clean:
	$(_@_) echo "Cleaning all object and binary files"
	$(_@_) rm -fR $(MODULE_OBJ_DIR)
	$(_@_) rm -fR $(MODULE_LIB_DIR)/$(MODULE_LIB_NAME)

all :$(MODULE_OBJ_LIST)
	$(_@_) mkdir -m777 -p $(MODULE_LIB_DIR)
	$(_@_) $(CC) $(LDFLAGS) $(MODULE_OBJ_LIST) $(LIBRARY_DIRS:%=$(LIB_DIR_FLAG)%) $(APPL_LIBS_NEEDED)
	$(_@_) chmod 644 $(MODULE_LIB_DIR)/$(MODULE_LIB_NAME)
	$(_@_) echo -e "\033[1;35mLibrary \"$(MODULE_LIB_DIR)/$(MODULE_LIB_NAME)\" created successfully!\033[0m"
	$(_@_) echo -e "\033[1;32m`ls -go $(MODULE_LIB_DIR)/$(MODULE_LIB_NAME)`\033[0m"

#########################################################################
# END OF MAKEFILE
#########################################################################
