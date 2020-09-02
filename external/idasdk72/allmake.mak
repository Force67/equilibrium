#
#       Common part of make files for IDA under MS Windows.
#

NDEBUG=1
# find directory of allmake.mak:
IDA:=$(dir $(lastword $(MAKEFILE_LIST)))

# define the version number we are building
IDAVER_MAJOR:=7
IDAVER_MINOR:=2
# 710
IDAVERDECIMAL:=$(IDAVER_MAJOR)$(IDAVER_MINOR)0
# 7.1
IDAVERDOTTED:=$(IDAVER_MAJOR).$(IDAVER_MINOR)

# if no targets are defined, default to host OS
ifeq ($(or $(__ANDROID__),$(__ANDROID_X86__),$(__ARMLINUX__),$(__LINUX__),$(__MAC__),$(__NT__)),)
  ifeq ($(OS),Windows_NT)
    __NT__=1
  else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
      __LINUX__=1
    endif
    ifeq ($(UNAME_S),Darwin)
      __MAC__=1
    endif
  endif
endif

# only one build target may be defined
ifneq ($(__ANDROID__)$(__ANDROID_X86__)$(__ARMLINUX__)$(__LINUX__)$(__MAC__)$(__NT__),1)
  $(error Only one build target may be defined (__ANDROID__, __ANDROID_X86__, __ARMLINUX__, __LINUX__, __MAC__, or __NT__))
endif

# detect build configuration
# Note: will set one of M, MM, M32, MO, MMO, MO32, MSO, MMSO, or MSO32
BUILD_CONFIG-1                     := M
BUILD_CONFIG-$(__EA64__)           += M
BUILD_CONFIG-$(USE_STATIC_RUNTIME) += S
BUILD_CONFIG-$(NDEBUG)             += O
BUILD_CONFIG-$(__X86__)            += 32
empty :=
space := $(empty) $(empty)
comma := ,
BUILD_CONFIG := $(subst $(space),,$(BUILD_CONFIG-1))
$(BUILD_CONFIG) := 1

# disable x86 ida64 builds
ifeq ($(or $(MM32),$(MMO32),$(MMSO32)),1)
  $(error x86 ida64 builds have been disabled)
endif

# set processor type for target
ifeq ($(or $(__ANDROID__),$(__ARMLINUX__)),1)
  __ARM__=1
endif

ifdef __ARM__
  PROCDEF = __ARM__
  TARGET_PROCESSOR_NAME=arm
else ifndef __X86__
  ARCH_FLAGS = -m64
  TARGET_PROCESSOR_NAME=x64
else
  ARCH_FLAGS = -m32
  TARGET_PROCESSOR_NAME=x86
endif

# define some variables to simplify build system
ifndef __X86__
  __X64__ = 1
  ifndef __EA64__
    __X32__ = 1
  endif
endif
ifndef __NT__
  __UNIX__ = 1
endif

# define SYSNAME
SYSNAME-$(__ANDROID_X86__) = android
SYSNAME-$(__ANDROID__)     = android
SYSNAME-$(__ARMLINUX__)    = linux
SYSNAME-$(__LINUX__)       = linux
SYSNAME-$(__MAC__)         = mac
SYSNAME-$(__NT__)          = win
SYSNAME = $(SYSNAME-1)

# path functions (depending on host OS)
ifeq ($(OS),Windows_NT)
  # define: convert unix path to dos path by replacing slashes by backslashes
  dospath=$(subst /,\\,$(1))
  # define: return absolute path given a relative path
  qabspath=$(subst /idasrc,:/idasrc,$(subst /cygdrive/,,$(abspath $(1))))/
else
  # define: dospath does not do anything in unix
  dospath=$(1)
  # define: return absolute path given a relative path
  qabspath=$(abspath $(1))/
endif
# define: return 1 if path exists, 0 otherwise
ls=$(if $(wildcard $(1)),1,0)

# define: logical negation
not = $(if $(1),,1)

# define: greater or equal
gte = $(if $(filter-out $(1),$(word 2,$(sort $(1) $(2)))),,1)

include $(IDA)defaults.mk

#############################################################################
ifeq ($(or $(__ANDROID__),$(__ANDROID_X86__),$(__ARMLINUX__)),1)
  ifeq ($(__EA64__),$(__X86__))
    $(error Please define only one of __EA64__/__X86__ to compile for ARM Linux/Android)
  endif
  COMPILER_NAME=gcc
  ifeq ($(OS),Windows_NT)
    HOST_PART = win32
  else
    HOST_PART = linux
  endif
  BUILD_ONLY_SERVER=1
  CCPART-$(__ANDROID__)-$(__X64__)     = aarch64-linux-android
  CCPART-$(__ANDROID__)-$(__X86__)     = arm-linux-androideabi
  CCPART-$(__ANDROID_X86__)-$(__X64__) = x86_64-linux-android
  CCPART-$(__ANDROID_X86__)-$(__X86__) = i686-linux-android
  CCPART-$(__ARMLINUX__)-$(__X86__)    = arm-none-linux-gnueabi
  CCPART = $(CCPART-1-1)
  ARCHPART-$(__ANDROID__)-$(__X64__)     = arm64-v8a-21
  ARCHPART-$(__ANDROID__)-$(__X86__)     = armeabi-8
  ARCHPART-$(__ANDROID_X86__)-$(__X64__) = x64-21
  ARCHPART-$(__ANDROID_X86__)-$(__X86__) = x86-19
  ARCHPART-$(__ARMLINUX__)-$(__X86__)    = $(CCPART)
  ARCHPART = $(ARCHPART-1-1)
  ARCH_FLAGS-$(__ANDROID__)-$(__X86__) = -march=armv5te -mtune=xscale -msoft-float -mthumb
  ARCH_FLAGS += $(ARCH_FLAGS-1-1)
  ifeq ($(or $(__ANDROID__),$(__ANDROID_X86__)),1)
    VENDORPART = android-ndk
    ifdef __ANDROID_X86__
      ifndef __X86__
        LIBSUFF=64
      endif
    endif
    SYSROOT=$(TOOLCHAIN)/sysroot
    CRTBEGIN=$(SYSROOT)/usr/lib$(LIBSUFF)/crtbegin_dynamic.o
    CRTEND=$(SYSROOT)/usr/lib$(LIBSUFF)/crtend_android.o
  else
    VENDORPART = codesourcery
  endif
  TOOLCHAIN = $(THIRD_PARTY)$(VENDORPART)/$(HOST_PART)/$(ARCHPART)
  CROSS_PREFIX = $(TOOLCHAIN)/bin/$(CCPART)-

  PTHR_SWITCH = -pthread
#############################################################################
else ifdef __LINUX__
  COMPILER_NAME=gcc
  PTHR_SWITCH=-pthread
  STDLIBS += -lrt -lpthread -lc
#############################################################################
else ifdef __MAC__
  COMPILER_NAME=gcc
  STDLIBS += -lpthread -liconv
  ARCH_FLAGS-$(__X64__) = -arch x86_64
  ARCH_FLAGS-$(__X86__) = -arch i386
  ARCH_FLAGS += $(ARCH_FLAGS-1)
  # The following values are defined in defaults.mk.
  ARCH_FLAGS += -mmacosx-version-min=$(MACOSX_DEPLOYMENT_TARGET)
  ARCH_FLAGS += -isysroot $(MACSDK)
#############################################################################
else ifdef __NT__
  COMPILER_NAME=vc
  # Not needed for builds, but well for lint and external scripts using the
  # print-MSVCDIR target.
  MSVCDIR=$(VSPATH15)/VC/
  ifeq ($(or $(__LINT__),$(__PVS__)),1)
    MSVC_INCLUDE=$(MSVCDIR)Include
    MSVCBINDIR-$(__X64__) = $(MSVCDIR)bin/amd64
    MSVCBINDIR-$(__X86__) = $(MSVCDIR)bin
  else
    MSVC_INCLUDE=$(VSPATH)/Include
    MSVCBINDIR-$(__X64__) = $(VSPATH)/bin/HostX64/x64
    MSVCBINDIR-$(__X86__) = $(VSPATH)/bin/HostX86/x86
  endif
  MSVCBINDIR = $(MSVCBINDIR-1)
endif

#############################################################################
# toolchain-specific variables

ifeq ($(COMPILER_NAME),gcc)
  # file extensions
  A     = .a
  B     = $(SUFF64)
  O     = .o
  II    = .i
  # toolchain output switches
  OBJSW = -o # with space
  OUTAR =
  OUTII = -o # with space
  OUTSW = -o # with space
  ifdef __MAC__
    OUTMAP = -Wl,-map,
  else
    OUTMAP = -Wl,-Map,
  endif
  # misc switches
  AROPT = rc
  CPPONLY = -E
  FORCEC = -xc
  NORTTI = -fno-rtti
  ifdef __MAC__
    OUTDLL = -dynamiclib
  else
    OUTDLL = --shared
  endif
  # utilities
  CCACHE-$(USE_CCACHE) = ccache
  ifdef __MAC__
    _CC  = cc
    _CXX = c++
  else
    _CC  = gcc
    _CXX = g++
    ifdef USE_GOLD
      GOLD = -fuse-ld=gold
    endif
  endif
  AR  =             $(CROSS_PREFIX)ar$(HOST_EXE) $(AROPT)
  CC  = $(CCACHE-1) $(CROSS_PREFIX)$(_CC)$(HOST_EXE) $(ARCH_FLAGS)
  CCL =             $(CROSS_PREFIX)$(_CXX)$(HOST_EXE) $(ARCH_FLAGS) $(GOLD)
  CXX = $(CCACHE-1) $(CROSS_PREFIX)$(_CXX)$(HOST_EXE) $(ARCH_FLAGS)
else ifeq ($(COMPILER_NAME),vc)
  # file extensions
  A     = .lib
  B     = $(SUFF64).exe
  O     = .obj
  II    = .i
  # toolchain output switches
  OBJSW = /Fo
  OUTAR = /OUT:
  OUTII = /Fi
  OUTSW = /OUT:
  OUTMAP = /map:
  # misc switches
  CPPONLY = /P
  FORCEC = /TC
  NOLOGO = /nologo
  NORTTI = /GR-
  OUTDLL = /DLL
  # utilities
  AR  = $(MSVCBINDIR)/lib.exe $(NOLOGO)
  CC  = $(MSVCBINDIR)/cl.exe $(NOLOGO)
  CCL = $(MSVCBINDIR)/link.exe $(NOLOGO)
  CXX = $(CC)
endif

##############################################################################
# target-specific cflags/ldflags
ifeq ($(COMPILER_NAME),gcc)

  # system cflags
  CC_DEFS += $(PROCDEF)
  ifdef __MAC__
    CC_DEFS += __MAC__
  else
    CC_DEFS += __LINUX__
    ifdef __ARMLINUX__
      CC_DEFS += __ARMLINUX__
    endif
  endif

  # pic-related flags
  ifdef __ARMLINUX__
    # disable -fPIC for armlinux
    PIC =
  else
    # use -fPIC by default
    PIC = -fPIC
  endif

  ifdef __MAC__
    LDPIE = $(PIC) -Wl,-pie
  else
    LDPIE = $(PIC) -pie
  endif

  # common cflags
  CC_DEFS += $(DEF64)
  CC_DEFS += $(DEFX86)

  CC_F += $(PIC)
  CC_F += -fdiagnostics-show-option
  CC_F += -fno-strict-aliasing
  CC_F += -fvisibility-inlines-hidden
  CC_F += -fvisibility=hidden
  CC_F += -fwrapv

  CC_INCP += $(I)

  CC_W += -Wall
  CC_W += -Wextra
  CC_W += -Wformat=2
  CC_W += -Werror=format-security
  CC_W += -Werror=format-nonliteral
  CC_W += -Wshadow
  CC_W += -Wunused

  CC_WNO += -Wno-format-y2k
  CC_WNO += -Wno-missing-field-initializers
  CC_WNO += -Wno-sign-compare

  CC_X += -g
  CC_X += -pipe

  # system-specific cflags
  ifdef __MAC__
    # 'cc -dumpversion' always reports 4.2.1 for mac
    # https://stackoverflow.com/questions/12893731/why-does-clang-dumpversion-report-4-2-1

    # enable c++11 for clang
    CXXSTD = -std=c++11

    # clang is extra picky - need to add some warning supressions
    # must eventually get rid of most of these
    CC_WNO += -Wno-char-subscripts
    CC_WNO += -Wno-dynamic-class-memaccess
    CC_WNO += -Wno-int-to-pointer-cast
    CC_WNO += -Wno-invalid-source-encoding
    CC_WNO += -Wno-logical-not-parentheses
    CC_WNO += -Wno-logical-op-parentheses
    CC_WNO += -Wno-null-conversion
    CC_WNO += -Wno-parentheses-equality
    CC_WNO += -Wno-self-assign
    CC_WNO += -Wno-unused-const-variable
    CC_WNO += -Wno-unused-function
    CC_WNO += -Wno-unused-private-field
    CC_WNO += -Wno-unused-variable
    CC_WNO += -Wno-varargs

    CC_F += -fno-caret-diagnostics
  else # (arm)linux/android
    # our current gcc toolchain versions (cc -dumpversion)
    # android-ndk aarch64     4.9
    # android-ndk arm         4.8
    # android-ndk x64         4.9
    # android-ndk x86         4.8
    # codesourcery armlinux   4.5.1
    # linux centos6.8         4.8.2

    # get gcc version
    ifndef _GCC_VERSION
      _GCC_VERSION:=$(wordlist 1,2,$(subst ., ,$(shell $(CC) -dumpversion)))
      export _GCC_VERSION
    endif
    GCC_VERSION=$(firstword $(_GCC_VERSION)).$(lastword $(_GCC_VERSION))

    # enable c++11 for gcc >= 4.8 (actually it should be >= 4.8.1)
    ifeq ($(call gte,$(GCC_VERSION),4.8),1)
      CXXSTD = -std=c++11
    endif

    CC_WNO-$(call gte,$(GCC_VERSION),4.8) += -Wno-unused-local-typedefs
    CC_F-$(call gte,$(GCC_VERSION),4.8) += -fno-diagnostics-show-caret
    CC_DEFS-$(call gte,$(GCC_VERSION),5.0) += _GLIBCXX_USE_CXX11_ABI=0
    CC_W-$(call gte,$(GCC_VERSION),7.0) += -Wimplicit-fallthrough=0

    # suppress warning about ABI change in GCC 4.4
    CC_WNO-$(__ARMLINUX__) += -Wno-psabi
  endif

  # optimization cflags
  ifdef NDEBUG
    CC_F += -fdata-sections
    CC_F += -ffunction-sections
    CC_F += -fomit-frame-pointer
    # stack protector
    ifeq ($(or $(__ARMLINUX__),$(__TARGET_MAC_HOST_LINUX__)),1)
      # disable stack protector for armlinux (since it fails to link)
      # and for our osxcross toolchain (it is hard to check for version
      # number in clang, so we check against __TARGET_MAC_HOST_LINUX__).
    else ifeq ($(call gte,$(GCC_VERSION),4.9),1)
      CC_F += -fstack-protector-strong
    else
      CC_F += -fstack-protector
    endif
    CC_DEFS += NDEBUG
    ifdef __ARMLINUX__
      # disable _FORTIFY_SOURCE for armlinux to avoid dependency on
      # GLIBC 2.11.
    else
      CC_DEFS += _FORTIFY_SOURCE=2
    endif
  else
    CC_DEFS += _DEBUG
  endif

  # system-specific ldflags
  ifdef __LINUX__
    LDFLAGS += -Wl,--build-id
    LDFLAGS += -Wl,--gc-sections
    LDFLAGS += -Wl,--warn-shared-textrel

    NO_UNDEFS = -Wl,--no-undefined
    DLL_W += $(NO_UNDEFS)
  else ifdef __MAC__
    LDFLAGS += -Wl,-dead_strip

    ifndef __TARGET_MAC_HOST_LINUX__
      DLL_X += -compatibility_version 1.0
      DLL_X += -current_version 1.0
      DLL_X += -single_module
    endif
  endif

  # common linker/compiler flags
  ifdef NDEBUG
    CCOPT += -O2
    ifdef __LINUX__
      LDOPT += -Wl,-O1
    endif
  endif

  # final compiler flags
  CC_F += $(CC_F-1)
  CC_W += $(CC_W-1)
  CC_WNO += $(CC_WNO-1)
  CC_INCP += $(CC_INCP-1)
  CC_DEFS += $(CC_DEFS-1)
  CC_D += $(addprefix -D,$(CC_DEFS))
  CC_I += $(addprefix -I,$(CC_INCP))

  # the -Wno-* flags must come after the -W enabling flags
  WARNS = $(sort $(CC_W)) $(sort $(CC_WNO))

  CFLAGS += $(sort $(CC_X))
  CFLAGS += $(CCOPT)
  CFLAGS += $(sort $(CC_I))
  CFLAGS += $(sort $(CC_D))
  CFLAGS += $(sort $(CC_F))
  CFLAGS += $(WARNS)
  CFLAGS += $(PTHR_SWITCH)

  # for warning suppression, override the WARNS variable with NOWARNS:
  # $(TARGET): WARNS = $(NOWARNS)
  NOWARNS = -w

  # dll linker flags
  DLLFLAGS += $(DLL_W) $(DLL_X)

else ifeq ($(COMPILER_NAME),vc)
  # for warning suppression, override the WARNS variable with NOWARNS:
  # $(TARGET): WARNS = $(NOWARNS)
  NOWARNS = -w -wd4702 -wd4738

  # optimization ldflags
  CCOPT += /DEBUG
  ifdef NDEBUG
    CCOPT += /INCREMENTAL:NO /OPT:ICF /OPT:REF
  endif
  LDOPT += $(CCOPT)

  # set c runtime to use
  ifdef NDEBUG
    ifdef USE_STATIC_RUNTIME
      RUNTIME_LIBSW = /MT
    else
      RUNTIME_LIBSW = /MD
    endif
  else
    ifdef USE_STATIC_RUNTIME
      RUNTIME_LIBSW = /MTd
    else
      RUNTIME_LIBSW = /MDd
    endif
  endif

  # final compiler flags
  CC_DEFS += $(DEF64)
  CC_DEFS += $(DEFX86)
  CC_INCP += $(CC_INCP-1)
  CC_DEFS += $(CC_DEFS-1)
  CC_D += $(addprefix -D,$(CC_DEFS))
  CC_I += $(addprefix -I,$(CC_INCP))

  CFGFILE = @$(IDA)$(SYSDIR).cfg
  CFLAGS += $(CFGFILE)
  CFLAGS += $(RUNTIME_LIBSW)
  CFLAGS += $(sort $(CC_I))
  CFLAGS += $(sort $(CC_D))
  CFLAGS += $(sort $(CC_F))
  CFLAGS += $(WARNS)

  # PDB options
  PDBFLAGS = /PDB:$(PDBDIR)/
  ifdef NDEBUG
    PDBFLAGS += /PDBALTPATH:%_PDB%
  endif

  # final linker flags
  LDFLAGS += $(PDBFLAGS)
  LDFLAGS += /ERRORREPORT:QUEUE
  ifdef __X86__
    LDFLAGS += /LARGEADDRESSAWARE
  endif

  ifdef __XPCOMPAT__
    XPSUBSYS-$(__X64__) = /SUBSYSTEM:CONSOLE,5.02
    XPSUBSYS-$(__X86__) = /SUBSYSTEM:CONSOLE,5.01
    LDFLAGS += $(XPSUBSYS-1)
  endif

endif

# to enable obsolete functions, disable the NO_OBSOLETE_FUNCS variable:
# $(TARGET): NO_OBSOLETE_FUNCS =
NO_OBSOLETE_FUNCS = NO_OBSOLETE_FUNCS
CC_DEFS += $(NO_OBSOLETE_FUNCS)
CXXFLAGS += $(CXXSTD) $(CFLAGS)
LDFLAGS += $(LDOPT)

#############################################################################
ifdef __X86__
  DEFX86 = __X86__
endif

ifdef __EA64__
  SUFF64=64
  ADRSIZE=64
  DEF64 = __EA64__
else
  ADRSIZE=32
endif

ifdef USE_STATIC_RUNTIME
  STATSUF=_s
endif

#############################################################################
BINDIR=$(TARGET_PROCESSOR_NAME)_$(SYSNAME)_$(COMPILER_NAME)$(OPTSUF)$(STATSUF)
SYSDIR=$(TARGET_PROCESSOR_NAME)_$(SYSNAME)_$(COMPILER_NAME)_$(ADRSIZE)$(OPTSUF)$(STATSUF)
# libraries directory
LIBDIR=$(IDA)lib/$(SYSDIR)
# object files directory (using ?= to allow overriding)
OBJDIR?=obj/$(SYSDIR)
# PDB files directory
PDBDIR=$(IDA)pdb/$(SYSDIR)
# output directory for target platform
R=$(IDA)bin/
# input directory with existing build utilities
RS=$(IDA)bin/
# _ida.hlp placed in main tool directory
HI=$(RS)
# help source
HS=.hls
# help headers
HH=.hhp
# include,help and other directories are common for all platforms and compilers:
I =$(IDA)include/
C =$(R)cfg/
RI=$(R)idc/
F=$(OBJDIR)/
L=$(LIBDIR)/

DUMB=$(L)dumb$(O)
HELP=$(L)help$(O)
HLIB=$(HI)_ida.hlp

# to be used like this:
# $(L)va$(A): $(call lib, $(VA_OBJS))
lib=$(1); $(strip $(QARf)$(AR) $(OUTAR)$$@ $$^)

# to be used like this: $(call _link_exe, target, objs, libs)
_link_exe=$(strip $(QCCL)$(CCL) $(OUTSW)$(1) $(2) $(3) $(LDFLAGS) $(STDLIBS))

# to be used like this: $(call link_exe, objs, libs)
link_exe=$(call _link_exe,$@,$(1),$(2))

# to be used like this: $(call _link_dll, target, objs, libs)
_link_dll=$(strip $(QCCL)$(CCL) $(OUTDLL) $(DLLFLAGS) $(OUTSW)$(1) $(2) $(3) $(LDFLAGS) $(STDLIBS))

# to be used like this: $(call link_dll, objs, libs)
link_dll=$(call _link_dll,$@,$(1),$(2))

# to be used like this: $(call link_dumb, target, libs, objs)
link_dumb=$(3) $(patsubst %,$(L)%$(A),$(2)); $(strip $(QCCLf)$(CCL) $(OUTSW)$(1) $(LDFLAGS) $(3) $(patsubst %,$(L)%$(A),$(2)) $(STDLIBS))

# to be used like this:
# target: $(call dumb_target, libs, objs) extra_ldflags
dumb_target=$(call link_dumb,$$@,$(1),$(2) $(DUMB))

# to be used like this:
# $(R)%$(B): $(F)%$(O) $(call dumb_pattern, libs, objs) extra_ldflags
dumb_pattern=$(call link_dumb,$$@ $$<,$(1),$(2) $(DUMB))

# to be used like this:
# OBJS += $(call objs,obj1 obj2 obj3 ...)
objs=$(addprefix $(F),$(addsuffix $(O),$(1)))

# output name for module dll
module_dll=$(BIN_PATH)$(1)$(SUFF64)$(DLLEXT)

# output name for server executable
server_exe=$(R)dbgsrv/$(1)

ifeq ($(or $(M),$(MM),$(MO),$(MMO)),1)
  BUILD_IDA = 1
endif
ifeq ($(or $(M32),$(MM),$(MO32),$(MMO)),1)
  BUILD_DBGSRV = 1
endif

# target-os specific variables
ifdef __NT__
  DLLEXT=.dll
else ifdef __MAC__
  DLLEXT=.dylib
else
  DLLEXT=.so
endif

# build system commands
ifeq ($(OS),Windows_NT)
  CP=cp -f --preserve=all
  MKDIR=-@mkdir
  AWK=gawk
else
  CP=cp -f
  MKDIR=-@mkdir 2>/dev/null
  AWK=awk
endif
RM=rm -f
MV=mv

# used to silence some makefile commands
# run 'make Q=' to prevent commands from being silenced
Q=@

# some makefiles rebuild targets when the makefile itself changes.
# this makes debugging makefiles a pain.
# run 'make MAKEFILE_DEP=' to disable this behaviour.
MAKEFILE_DEP=makefile

# libida-related
ifdef __NT__
  # Note: on Windows, ida.lib does not have a "64" suffix for ea64
  IDALIB  = $(L)ida$(A)
  LINKIDA = $(IDALIB)
else
  IDALIB  = $(L)libida$(SUFF64)$(DLLEXT)
  LINKIDA = -L$(L)
  LINKIDA += -lida$(SUFF64)
endif

# simplify command echo
ifdef IDAMAKE_SIMPLIFY
  ifeq ($(Q),@)
    DO_IDAMAKE_SIMPLIFY=1
  endif
endif

ifdef DO_IDAMAKE_SIMPLIFY
  ifdef IDAMAKE_SIMPLIFY_NO_COLOR
    qcolor=$(1)
  else
    ifeq ($(OS),Windows_NT)
      qcolor=-e #
    endif
    qcolor+="\033[1;34m$(1)\033[0m"
  endif
  QCXX  = @echo $(call qcolor,compile) $< && #
  QCC   = @echo $(call qcolor,compile) $< && #
  QASM  = @echo $(call qcolor,asm) $< && #
  QARf  = @echo $(call qcolor,lib) $$@ && #
  QCCL  = @echo $(call qcolor,link) $@ && #
  QCCLf = @echo $(call qcolor,link) $$@ && #
endif

# simple build rules
CONLY?=-c

$(F)%$(O): %.cpp
	$(strip $(QCXX)$(CXX) $(CXXFLAGS) $(NORTTI) $(CONLY) $(OBJSW)$@ $<)

$(F)%$(O): %.c
	$(strip $(QCC)$(CC) $(CFLAGS) $(CONLY) $(OBJSW)$@ $(FORCEC) $<)

$(C)%.cfg: %.cfg
	$(CP) $? $@

# http://www.cmcrossroads.com/article/printing-value-makefile-variable
print-%:
	@echo $* = '$($*)'
	@echo $*\'s origin is $(origin $*)

#############################################################################
.PHONY: all test cfg includes

# Force make to delete the target if the rule to build it fails
.DELETE_ON_ERROR:
