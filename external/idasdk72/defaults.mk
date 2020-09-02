#############################################################################
# versions and paths for various external libraries and utils
THIRD_PARTY?=$(IDA)../third_party/

ifeq ($(origin SYSTEMDRIVE), undefined)
  SYSTEMDRIVE := "C:"
endif

# define: convert dos path to unix path by replacing backslashes by slashes
unixpath=$(subst \,/,$(1))

ifeq ($(origin PROGRAMFILES), undefined)
  PROGRAMFILES := "$(SYSTEMDRIVE)/Program Files (x86)"
else
  PROGRAMFILES := $(call unixpath,$(PROGRAMFILES))
endif

# Visual Studio (see "toolchain paths" below)
VSROOT = "$(PROGRAMFILES)/Microsoft Visual Studio/2017/Professional"
# Note: the default VC tools version is obtained from Microsoft.VCToolsVersion.default.txt
# VCTOOLSVER?=14.11.25503
_VSPATH15 = "$(PROGRAMFILES)/Microsoft Visual Studio 14.0"

# Windows SDK (see "toolchain paths" below)
WSDKVER?=10.0.17134.0

WSDKPATH = "$(PROGRAMFILES)/Windows Kits/10"

# Microsoft SDK
MSSDK71=$(shell cygpath -ma $(THIRD_PARTY)mssdk/7.1A)/
MSSDK=$(call qabspath,$(THIRD_PARTY)mssdk/8.1)

# Python
PYTHON_VERSION_MAJOR?=2
PYTHON_VERSION_MINOR?=7
PYTHON_VERNAME=python$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)

PYTHON_ROOT?=$(SYSTEMDRIVE)
PYTHON-$(__LINUX__) = /usr/bin/$(PYTHON_VERNAME)
PYTHON-$(__MAC__)   = $(PYTHON_VERNAME)
PYTHON-$(__NT__)    = $(PYTHON_ROOT)/python$(PYTHON_VERSION_MAJOR)$(PYTHON_VERSION_MINOR)-x64/python.exe
PYTHON ?= $(PYTHON-1)

# oldest supported version of MacOSX
MACOSX_DEPLOYMENT_TARGET = 10.9

# Qt
QTVER?=5.6.3-x64

QTDIR-$(__LINUX__) = /usr/local/Qt/$(QTVER)/
QTDIR-$(__MAC__)   = /Users/Shared/Qt/$(QTVER)/
QTDIR-$(__NT__)    = $(SYSTEMDRIVE)/Qt/$(QTVER)/
QTDIR ?= $(QTDIR-1)

ifdef __NT__
  ifdef NDEBUG
    QTSUFF=.dll
  else
    QTSUFF=d.dll
  endif
  QTLIBDIR=bin
else ifdef __LINUX__
  QTPREF=lib
  QTSUFF=.so.5
  QTLIBDIR=lib
endif

# SWiG
SWIG_VERSION?=2.0.12
ifdef __NT__
  SWIG_HOME?=$(THIRD_PARTY)swig/swigwin-$(SWIG_VERSION)
  SWIG?=$(SWIG_HOME)/swig.exe
  SWIG_INCLUDES?=-I$(SWIG_HOME)/Lib/python -I$(SWIG_HOME)/Lib
else
  ifdef __MAC__
    SWIG_HOME?=$(THIRD_PARTY)swig/swigmac-$(SWIG_VERSION)/swig-$(SWIG_VERSION)-install
  else
    SWIG_HOME?=$(THIRD_PARTY)swig/swiglinux-$(SWIG_VERSION)-x64/swig-$(SWIG_VERSION)-install
  endif
  ifdef USE_CCACHE
    # we set CCACHE_DIR so as to not interfere with the system's ccache
    # and we set CCACHE_CPP2 to prevent SWiG from printing a bunch of
    # warnings due to re-using of the preprocessed source.
    SWIG?=CCACHE_DIR='$${HOME}/.ccache-swig' CCACHE_CPP2=1 $(SWIG_HOME)/bin/ccache-swig $(SWIG_HOME)/bin/swig
  else
    SWIG?=$(SWIG_HOME)/bin/swig
  endif
  SWIG_INCLUDES?=-I$(SWIG_HOME)/share/swig/$(SWIG_VERSION)/python -I$(SWIG_HOME)/share/swig/$(SWIG_VERSION)
endif

#############################################################################
# toolchain paths

ifdef __NT__                                                  # Visual Studio
  # This function converts a windows path to a 8.3 path with forward slashes as
  # file separator.
  shortdospath=$(subst \,/,$(shell cygpath -d $(1) 2>/dev/null))

  ifndef VCTOOLSVER
    VCTOOLSVER := $(shell cat $(VSROOT)/VC/Auxiliary/Build/Microsoft.VCToolsVersion.default.txt)
    export VCTOOLSVER
  endif
  ifndef VSPATH
    VSPATH := $(call shortdospath,$(VSROOT)/VC/Tools/MSVC/$(VCTOOLSVER))
    export VSPATH
  endif
  ifndef VSPATH15
    VSPATH15 := $(call shortdospath,$(_VSPATH15))
    export VSPATH15
  endif
  ifndef UCRT_INCLUDE
    UCRT_INCLUDE := $(call shortdospath,$(WSDKPATH)/Include/$(WSDKVER)/ucrt)
    ifeq ($(UCRT_INCLUDE),)
      $(error Could not find Windows SDK Include path (see defaults.mk))
    endif
    export UCRT_INCLUDE
  endif
  ifndef UCRT_LIB
    UCRT_LIB := $(call shortdospath,$(WSDKPATH)/Lib/$(WSDKVER)/ucrt)
    ifeq ($(UCRT_LIB),)
      $(error Could not find Windows SDK Lib path (see defaults.mk))
    endif
    export UCRT_LIB
  endif

  # Should Qt build use nmake, or jom?
  JOM_PATH?=$(THIRD_PARTY)jom/jom_1_1_2/jom.exe
  ifneq ($(JOM_PATH),)
    IDAQ_BUILD=$(JOM_PATH) /NOLOGO /F
  else
    IDAQ_BUILD=$(MSVCBINDIR)/nmake -nologo -f
  endif
else ifdef __MAC__
  ifndef MACSDK
    MACSDK := $(shell /usr/bin/xcrun --sdk macosx --show-sdk-path)
    ifeq ($(MACSDK),)
      $(error Could not find MacOSX SDK)
    endif
    export MACSDK
  endif
endif

#############################################################################
# keep all paths in unix format, with forward slashes
ifeq ($(OS),Windows_NT)
  VSPATH       :=$(call unixpath,$(VSPATH))
  VSPATH15     :=$(call unixpath,$(VSPATH15))
  UCRT_INCLUDE :=$(call unixpath,$(UCRT_INCLUDE))
  UCRT_LIB     :=$(call unixpath,$(UCRT_LIB))
  PYTHON_ROOT  :=$(call unixpath,$(PYTHON_ROOT))
  # unixpath-ify PYTHON _only_ if it was defined. Otherwise, this will
  # define it and conditional assignments (i.e., '?=') will never apply
  ifneq ($(origin PYTHON), undefined)
    PYTHON     :=$(call unixpath,$(PYTHON))
  endif
  SWIG         :=$(call unixpath,$(SWIG))
  QTDIR        :=$(call unixpath,$(QTDIR))
  THIRD_PARTY  :=$(call unixpath,$(THIRD_PARTY))
endif

#############################################################################
# http://stackoverflow.com/questions/16467718/how-to-print-out-a-variable-in-makefile
.print-%  : ; @echo $($*)
