
#__FUZZ_PROCS__=1

BASE_OBJS += $(F)ana$(O)
BASE_OBJS += $(F)emu$(O)
BASE_OBJS += $(F)ins$(O)
BASE_OBJS += $(F)out$(O)
BASE_OBJS += $(F)reg$(O)

SRC_PATH = $(IDA)module/
BIN_PATH = $(R)procs/

ifdef __NT__
  DLLFLAGS += /BASE:0x130000000
endif

include ../../module.mak
