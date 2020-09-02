
# This file is included by:
#   - ldr/loader.mak
#   - module/module.mak
#   - plugins/plugin.mak

ifdef __LINT__
  # Info 785 Too few initializers for aggregate
  CFLAGS += -e785
endif

#----------------------------------------------------------------------
# include allmake.mak and prepare default goal if needed
ifndef NO_DEFAULT_MODULE
  include ../../allmake.mak
  # prepare targets
  GOALS += modules
  GOALS += $(addprefix $(RI),$(IDCS))
  GOALS += configs
  all: $(GOALS)

  # create default module and add it to targets list
  DEFAULT_MODULE = $(call module_dll,$(PROC))
  DEFAULT_OBJS += $(BASE_OBJS)
  DEFAULT_OBJS += $(call objs,$(foreach n,1 2 3 4 5 6 7 8 9,$(O$(n))))
  $(DEFAULT_MODULE): MODULE_OBJS += $(DEFAULT_OBJS)
  # object file dependencies must be explicitly added to each module
  $(DEFAULT_MODULE): $(DEFAULT_OBJS)
  MODULES += $(DEFAULT_MODULE)
endif

#----------------------------------------------------------------------
# prepare ldflags for all modules
MODULE_LDFLAGS += $(OUTMAP)$(F)$(@F).map
ifdef __LINUX__
  DEFFILE ?= $(SRC_PATH)exports.def
  MODULE_LDFLAGS += -Wl,--version-script=$(DEFFILE)
else ifdef __MAC__
  INSTALL_NAME ?= $(@F)
  MODULE_LDFLAGS += -Wl,-install_name,$(INSTALL_NAME)
endif

#----------------------------------------------------------------------
# main rule for modules
.PHONY: modules
modules: $(MODULES)
$(MODULES): LDFLAGS += $(MODULE_LDFLAGS)
$(MODULES): $(LIBS) $(IDALIB) $(MAKEFILE_DEP) $(DEFFILE)
	$(call link_dll, $(MODULE_OBJS), $(LIBS) $(LINKIDA))
ifdef __NT__
  ifndef DONT_ERASE_LIB
	$(Q)$(RM) $(@:$(DLLEXT)=.exp) $(@:$(DLLEXT)=.lib)
  endif
endif
	$(CHECKSYMS_CMD)
	$(POSTACTION)

#----------------------------------------------------------------------
# auxiliary rules
configs: $(addprefix $(C),$(CONFIGS))

$(RI)%.idc: %.idc
	$(CP) $? $@

#----------------------------------------------------------------------
include $(IDA)objdir.mak
