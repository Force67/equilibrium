
#__FUZZ_LOADERS__=1

BASE_OBJS += $(F)$(PROC)$(O)

SRC_PATH = $(IDA)ldr/
BIN_PATH = $(R)loaders/

ifdef __NT__
  DLLFLAGS += /BASE:0x140000000
endif

include ../../module.mak
