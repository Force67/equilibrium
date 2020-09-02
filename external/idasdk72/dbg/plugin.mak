
ifneq ($(wildcard ../../parse),)
  CC_DEFS += ENABLE_LOWCNDS
endif
CC_INCP += ..

include ../../plugins/plugin.mak

$(MODULES): LIBS += $(L)dbg_plugin$(A)
$(MODULES): LIBS += $(L)dbg_rpc$(A)
$(MODULES): LIBS += $(L)dbg_proc$(A)
$(MODULES): LIBS += $(L)network$(A)

ifeq ($(or $(__LINUX__),$(__MAC__)),1)
  $(MODULES): STDLIBS += -ldl
endif
