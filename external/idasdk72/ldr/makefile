include ../allmake.mak

GOALS += loaders
.PHONY: $(GOALS)
all: $(GOALS)

#----------------------------------------------------------------------
ALLDIRS += aif
ALLDIRS += amiga
ALLDIRS += aof
ALLDIRS += aout
ALLDIRS += dos
ALLDIRS += dump
ALLDIRS += geos
ALLDIRS += hex
ALLDIRS += hpsom
ALLDIRS += intelomf
ALLDIRS += javaldr
ALLDIRS += mas
ALLDIRS += nlm
ALLDIRS += os9
ALLDIRS += pef
ALLDIRS += pilot
ALLDIRS += qnx
ALLDIRS += rt11
ALLDIRS += script_ldrs
ALLDIRS += snes
ALLDIRS += w32run
ALLDIRS += $(ALLDIRS-1)
loaders: $(ALLDIRS)

#----------------------------------------------------------------------
.PHONY: $(ALLDIRS)
$(ALLDIRS):
	$(Q)$(MAKE) -C $@

#----------------------------------------------------------------------
clean::
	$(foreach dir,$(ALLDIRS),$(MAKE) -C $(dir) clean;)
