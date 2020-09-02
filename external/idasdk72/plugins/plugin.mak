
#__FUZZ_PLUGINS__=1

ifndef NO_DEFAULT_MODULE
  BASE_OBJS += $(F)$(PROC)$(O)
endif

SRC_PATH = $(IDA)plugins/
BIN_PATH = $(R)plugins/

NO_IDENT = 1

include ../../module.mak

ifdef __NT__
  ifndef NDEBUG
    $(MODULES): PDBFLAGS = /PDB:$(@:$(DLLEXT)=.pdb)
  endif
endif
