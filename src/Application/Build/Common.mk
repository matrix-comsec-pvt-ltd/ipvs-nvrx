#########################################################################
# MODULE PARAMS
#########################################################################
CC_OBJ_OUTPUT = -o $@

# Create object list for both CPP and C files
MODULE_CPP_OBJ_LIST := $(foreach src,$(wildcard $(MODULE_SOURCE_DIR)/*.cpp),$(PROJ_OBJ_DIR)/$(notdir $(src:.cpp=$(OBJ_EXTENSION))))
MODULE_C_OBJ_LIST := $(foreach src,$(wildcard $(MODULE_SOURCE_DIR)/*.c),$(PROJ_OBJ_DIR)/$(notdir $(src:.c=$(OBJ_EXTENSION))))
MODULE_OBJ_LIST := $(MODULE_CPP_OBJ_LIST) $(MODULE_C_OBJ_LIST)
MODULE_DEPEND_LIST := $(MODULE_OBJ_LIST:$(OBJ_EXTENSION)=$(DEPEND_EXTENSION))

#########################################################################
# GENERATING DEPENDENCIES
#########################################################################
CPP_MAKEDEPEND = set -e; $(CC) -P -M -c $(CPPFLAGS) $< | \
		sed '1s=^=$@ $(PROJ_OBJ_DIR)/$(*D)/=' > $@; \
		[ -s $@ ] || rm -f $@

C_MAKEDEPEND = set -e; $(CC) -P -M -c $(CFLAGS) $< | \
		sed '1s=^=$@ $(PROJ_OBJ_DIR)/$(*D)/=' > $@; \
		[ -s $@ ] || rm -f $@

#########################################################################
# CREATING DEPENDENCIES FOR CPP / C FILES
#########################################################################
$(PROJ_OBJ_DIR)/%$(DEPEND_EXTENSION): $(MODULE_SOURCE_DIR)/%.cpp
	$(_@_) [ -d $(@D) ] || mkdir -p $(@D)
	$(_@_) $(CPP_MAKEDEPEND)

$(PROJ_OBJ_DIR)/%$(DEPEND_EXTENSION): $(MODULE_SOURCE_DIR)/%.c
	$(_@_) [ -d $(@D) ] || mkdir -p $(@D)
	$(_@_) $(C_MAKEDEPEND)

#########################################################################
# COMPILING CPP / C FILES
#########################################################################
$(PROJ_OBJ_DIR)/%$(OBJ_EXTENSION): $(MODULE_SOURCE_DIR)/%.cpp $(PROJ_OBJ_DIR)/%$(DEPEND_EXTENSION)
	@echo -e "\033[94m[compiling]\033[92m $(notdir $(CPP))\033[0m" $(notdir $<) $(ECHO_STDOUT)
	$(_@_) $(CPP) $(COMPILE_ONLY_FLAG) $(CPPFLAGS) $< $(CC_OBJ_OUTPUT)

$(PROJ_OBJ_DIR)/%$(OBJ_EXTENSION): $(MODULE_SOURCE_DIR)/%.c $(PROJ_OBJ_DIR)/%$(DEPEND_EXTENSION)
	@echo -e "\033[94m[compiling]\033[92m $(notdir $(CC))\033[0m" $(notdir $<) $(ECHO_STDOUT)
	$(_@_) $(CC) $(COMPILE_ONLY_FLAG) $(CFLAGS) $< $(CC_OBJ_OUTPUT)

default:all

all:$(MODULE_OBJ_LIST)

# Include module dependencies list
-include $(MODULE_DEPEND_LIST)

#########################################################################
# END OF MAKEFILE
#########################################################################
