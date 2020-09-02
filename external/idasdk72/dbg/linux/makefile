include ../../allmake.mak

GOALS-$(BUILD_IDA) += modules # target in $(IDA)module.mak
GOALS-$(BUILD_DBGSRV) += server # target in $(IDA)dbg/server.mak
.PHONY: $(GOALS-1)
all: $(GOALS-1)

#----------------------------------------------------------------------
ifdef __ANDROID__
  SERVER = android_server$(SUFF64)
else ifdef __ANDROID_X86__
  ifdef __X86__
    SERVER = android_x86_server
  else
    SERVER = android_x64_server
  endif
else ifdef __ARMLINUX__
  SERVER = armlinux_server
else ifdef __LINUX__
  SERVER = linux_server$(SUFF64)
endif
ifdef SERVER
  SERVERS += $(call server_exe,$(SERVER))
endif

#----------------------------------------------------------------------
ARMLINUX_STUB = $(call module_dll,armlinux_stub)
STUB = $(call module_dll,linux_stub)
ifdef BUILD_IDA
  MODULES += $(ARMLINUX_STUB)
  MODULES += $(STUB)
endif

#----------------------------------------------------------------------
USER = $(call module_dll,linux_user)
ifeq ($(and $(BUILD_IDA),$(__LINUX__)),1)
  MODULES += $(USER)
endif

#----------------------------------------------------------------------
# we explicitly added our module targets
NO_DEFAULT_MODULE = 1

# NOTE: all MODULES must be defined before including plugin.mak.
include ../plugin.mak
# NOTE: target-specific rules and dependencies that use variable
#       expansion to name the target (such as "$(MODULE): [...]") must
#       come after including plugin.mak

#----------------------------------------------------------------------
# select OBJS common to user plugin and debugger server
ifeq ($(or $(__LINUX__),$(__ANDROID__),$(__ANDROID_X86__),$(__ARMLINUX__)),1)
  BUILD_LINUX:=1
endif

BASE_OBJS-$(BUILD_LINUX) += $(F)linuxbase_debmod$(O)
BASE_OBJS-$(BUILD_LINUX) += $(F)linux_debmod$(O)
BASE_OBJS-$(BUILD_LINUX) += $(F)linux_wait$(O)

BASE_OBJS += $(BASE_OBJS-1) $(F)symelf$(O)

#----------------------------------------------------------------------
ANDROID_LDFLAGS += -nostdlib
ANDROID_LDFLAGS += -Bdynamic
ANDROID_LDFLAGS += -Wl,-z,nocopyreloc
ANDROID_LDFLAGS += -Wl,-z,noexecstack
ANDROID_LDFLAGS += -Wl,-z,relro
ANDROID_LDFLAGS += -Wl,-z,now
ANDROID_LDFLAGS += -Wl,--export-dynamic

ANDROID_LIBS += -lstdc++
ANDROID_LIBS += -lc
ANDROID_LIBS += -lgcc
ANDROID_LIBS += -ldl

ifdef __ANDROID__
  SERVER_POSTACTION=$(CROSS_PREFIX)strip $@
  SERVER_LDFLAGS += $(ANDROID_LDFLAGS)
  ifndef __X86__
    SERVER_LDFLAGS += -Wl,-dynamic-linker,/system/bin/linker64
  else
    SERVER_LDFLAGS += -Wl,-dynamic-linker,/system/bin/linker
  endif
  SERVER_LDFLAGS += $(LDPIE)
  SERVER_OBJS += $(F)libthread_db$(O) $(CRTBEGIN) $(CRTEND)
  SERVER_STDLIBS += $(ANDROID_LIBS)
else ifdef __ANDROID_X86__
  SERVER_POSTACTION=$(CROSS_PREFIX)strip $@
  SERVER_LDFLAGS += $(ANDROID_LDFLAGS)
  SERVER_LDFLAGS += $(LDPIE)
  SERVER_OBJS += $(F)libthread_db$(O) $(CRTBEGIN) $(CRTEND)
  SERVER_STDLIBS += $(ANDROID_LIBS)
else ifdef __ARMLINUX__
  SERVER_POSTACTION=$(CROSS_PREFIX)strip $@
  SERVER_STDLIBS += -lthread_db -lrt -lc -lpthread
else ifdef __LINUX__
  SERVER_LDFLAGS += -Wl,--version-script=linux_debmod.script
  SERVER_LDFLAGS += $(OUTMAP)$(F)$(@F).map
  SERVER_STDLIBS += -lthread_db -lrt -lc -lpthread -ldl
endif
SERVER_OBJS += $(BASE_OBJS)

# suppress warnings for libthread_db.c
$(F)libthread_db$(O): WARNS = $(NOWARNS)

include ../server.mak

#----------------------------------------------------------------------
ARMLINUX_STUB_OBJS += $(F)armlinux_stub$(O)
$(ARMLINUX_STUB): MODULE_OBJS += $(ARMLINUX_STUB_OBJS)
$(ARMLINUX_STUB): $(ARMLINUX_STUB_OBJS)

#----------------------------------------------------------------------
STUB_OBJS += $(F)linux_stub$(O)
$(STUB): MODULE_OBJS += $(STUB_OBJS)
$(STUB): $(STUB_OBJS)

#----------------------------------------------------------------------
USER_OBJS += $(F)linux_user$(O)
USER_OBJS += $(BASE_OBJS)
$(USER): MODULE_OBJS += $(USER_OBJS)
$(USER): $(USER_OBJS)
$(USER): DEFFILE = linux_debmod.script
$(USER): STDLIBS += -ldl
$(USER): STDLIBS += -lthread_db

ifeq ($(COMPILER_NAME),gcc)
  $(USER): LDFLAGS += -Wl,--export-dynamic
endif

#----------------------------------------------------------------------
include $(IDA)objdir.mak

# MAKEDEP dependency list ------------------
$(F)armlinux_stub$(O): $(I)bitrange.hpp $(I)bytes.hpp $(I)config.hpp        \
                  $(I)dbg.hpp $(I)err.h $(I)expr.hpp $(I)fpro.h             \
                  $(I)funcs.hpp $(I)ida.hpp $(I)idd.hpp $(I)idp.hpp         \
                  $(I)kernwin.hpp $(I)lines.hpp $(I)llong.hpp               \
                  $(I)loader.hpp $(I)nalt.hpp $(I)name.hpp $(I)netnode.hpp  \
                  $(I)network.hpp $(I)pro.h $(I)range.hpp $(I)segment.hpp   \
                  $(I)segregs.hpp $(I)typeinf.hpp $(I)ua.hpp $(I)xref.hpp   \
                  ../../module/arm/notify_codes.hpp ../arm_local_impl.cpp   \
                  ../arm_regs.cpp ../common_local_impl.cpp                  \
                  ../common_stub_impl.cpp ../dbg_rpc_client.h               \
                  ../dbg_rpc_engine.h ../deb_arm.hpp ../debmod.h            \
                  ../rpc_debmod.h armlinux_stub.cpp linux_local_impl.cpp
$(F)linux_debmod$(O): $(I)bitrange.hpp $(I)bytes.hpp $(I)config.hpp         \
                  $(I)diskio.hpp $(I)err.h $(I)fpro.h $(I)funcs.hpp         \
                  $(I)ida.hpp $(I)idd.hpp $(I)idp.hpp $(I)kernwin.hpp       \
                  $(I)lines.hpp $(I)llong.hpp $(I)loader.hpp $(I)nalt.hpp   \
                  $(I)name.hpp $(I)netnode.hpp $(I)network.hpp $(I)pro.h    \
                  $(I)prodir.h $(I)range.hpp $(I)segment.hpp $(I)ua.hpp     \
                  $(I)xref.hpp ../../plugins/dwarf/look_for_debug_file.cpp  \
                  ../arm_debmod.h ../dbg_rpc_engine.h ../dbg_rpc_handler.h  \
                  ../deb_arm.hpp ../deb_pc.hpp ../debmod.h ../pc_debmod.h   \
                  android.cpp android.hpp linux_debmod.cpp linux_debmod.h   \
                  linux_threads.cpp linuxbase_debmod.h symelf.hpp
$(F)linux_stub$(O): $(I)bitrange.hpp $(I)bytes.hpp $(I)config.hpp           \
                  $(I)dbg.hpp $(I)err.h $(I)expr.hpp $(I)fpro.h             \
                  $(I)funcs.hpp $(I)ida.hpp $(I)idd.hpp $(I)idp.hpp         \
                  $(I)kernwin.hpp $(I)lines.hpp $(I)llong.hpp               \
                  $(I)loader.hpp $(I)nalt.hpp $(I)name.hpp $(I)netnode.hpp  \
                  $(I)network.hpp $(I)pro.h $(I)range.hpp $(I)segment.hpp   \
                  $(I)segregs.hpp $(I)typeinf.hpp $(I)ua.hpp $(I)xref.hpp   \
                  ../common_local_impl.cpp ../common_stub_impl.cpp          \
                  ../dbg_rpc_client.h ../dbg_rpc_engine.h ../deb_pc.hpp     \
                  ../debmod.h ../pc_local_impl.cpp ../rpc_debmod.h          \
                  linux_local_impl.cpp linux_stub.cpp
$(F)linux_user$(O): $(I)bitrange.hpp $(I)bytes.hpp $(I)config.hpp           \
                  $(I)dbg.hpp $(I)err.h $(I)expr.hpp $(I)fpro.h             \
                  $(I)funcs.hpp $(I)ida.hpp $(I)idd.hpp $(I)idp.hpp         \
                  $(I)kernwin.hpp $(I)lines.hpp $(I)llong.hpp               \
                  $(I)loader.hpp $(I)nalt.hpp $(I)name.hpp $(I)netnode.hpp  \
                  $(I)network.hpp $(I)pro.h $(I)range.hpp $(I)segment.hpp   \
                  $(I)segregs.hpp $(I)typeinf.hpp $(I)ua.hpp $(I)xref.hpp   \
                  ../arm_debmod.h ../common_local_impl.cpp                  \
                  ../common_stub_impl.cpp ../deb_arm.hpp ../deb_pc.hpp      \
                  ../debmod.h ../pc_debmod.h ../pc_local_impl.cpp           \
                  linux_debmod.h linux_local_impl.cpp linux_user.cpp        \
                  linuxbase_debmod.h
$(F)linux_wait$(O): $(I)bytes.hpp $(I)ida.hpp $(I)idd.hpp $(I)kernwin.hpp   \
                  $(I)lines.hpp $(I)llong.hpp $(I)nalt.hpp $(I)netnode.hpp  \
                  $(I)network.hpp $(I)pro.h $(I)range.hpp $(I)ua.hpp        \
                  $(I)xref.hpp ../arm_debmod.h ../deb_arm.hpp               \
                  ../deb_pc.hpp ../debmod.h ../pc_debmod.h linux_debmod.h   \
                  linux_wait.cpp linuxbase_debmod.h
$(F)linuxbase_debmod$(O): $(I)bytes.hpp $(I)diskio.hpp $(I)fpro.h           \
                  $(I)ida.hpp $(I)idd.hpp $(I)kernwin.hpp $(I)lines.hpp     \
                  $(I)llong.hpp $(I)nalt.hpp $(I)netnode.hpp                \
                  $(I)network.hpp $(I)pro.h $(I)prodir.h $(I)range.hpp      \
                  $(I)ua.hpp $(I)xref.hpp ../arm_debmod.h ../deb_arm.hpp    \
                  ../deb_pc.hpp ../debmod.h ../pc_debmod.h                  \
                  linuxbase_debmod.cpp linuxbase_debmod.h
$(F)symelf$(O)  : $(I)auto.hpp $(I)bitrange.hpp $(I)bytes.hpp               \
                  $(I)config.hpp $(I)diskio.hpp $(I)entry.hpp               \
                  $(I)fixup.hpp $(I)fpro.h $(I)funcs.hpp $(I)ida.hpp        \
                  $(I)idd.hpp $(I)idp.hpp $(I)kernwin.hpp $(I)lines.hpp     \
                  $(I)llong.hpp $(I)loader.hpp $(I)nalt.hpp $(I)name.hpp    \
                  $(I)netnode.hpp $(I)network.hpp $(I)offset.hpp $(I)pro.h  \
                  $(I)range.hpp $(I)segment.hpp $(I)segregs.hpp $(I)ua.hpp  \
                  $(I)xref.hpp ../../ldr/elf/../idaldr.h                    \
                  ../../ldr/elf/common.cpp ../../ldr/elf/elf.h              \
                  ../../ldr/elf/elfbase.h ../../ldr/elf/elfr_arm.h          \
                  ../../ldr/elf/elfr_ia64.h ../../ldr/elf/elfr_mips.h       \
                  ../../ldr/elf/elfr_ppc.h ../../ldr/elf/reader.cpp         \
                  ../../module/arm/notify_codes.hpp ../debmod.h symelf.cpp  \
                  symelf.hpp
