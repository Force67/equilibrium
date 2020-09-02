
# definitions for idapython (& other plugins dynamically linked to Python)
ifdef __NT__
  PYTHON_ROOT?=c:
  PYTHON_DIR=$(PYTHON_ROOT)/python$(PYTHON_VERSION_MAJOR)$(PYTHON_VERSION_MINOR)-x64
  PYTHON_CFLAGS=-I$(PYTHON_DIR)/include /EHsc
  PYTHON_LDFLAGS=$(PYTHON_DIR)/libs/python$(PYTHON_VERSION_MAJOR)$(PYTHON_VERSION_MINOR).lib
else ifneq ($(LINUX_PYTHON_HOME_X64),)
  PYTHON_CFLAGS=-I$(LINUX_PYTHON_HOME_X64)/include/$(PYTHON_VERNAME)
  PYTHON_LDFLAGS=-L$(LINUX_PYTHON_HOME_X64)/lib -lpython$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR) -ldl
else
  PYTHON_CFLAGS:=$(shell $(PYTHON)-config --includes)
  PYTHON_LDFLAGS:=$(shell $(PYTHON)-config --ldflags)
endif
