include ../allmake.mak

GOALS += plugins
GOALS += samples
GOALS += examples
.PHONY: $(GOALS)
all: $(GOALS)

#----------------------------------------------------------------------
PLUGINS += callee
PLUGINS += callgraph
PLUGINS += choose
PLUGINS += findcrypt2
PLUGINS += ht_output
PLUGINS += ht_view
PLUGINS += pdb
PLUGINS += script_plg
PLUGINS += tracing_api
PLUGINS += uiswitch
PLUGINS-$(__NT__) += uunp
PLUGINS += $(PLUGINS-1)
plugins: $(PLUGINS)

#----------------------------------------------------------------------
SAMPLES += custdata
SAMPLES += custview
SAMPLES += formchooser
SAMPLES += formsample
SAMPLES += funclist
SAMPLES += getlines
SAMPLES += hexview
SAMPLES += mtsample
SAMPLES += openform
SAMPLES += procext
SAMPLES += ugraph
SAMPLES += ugraph3
SAMPLES += vcsample
samples: $(SAMPLES)

#----------------------------------------------------------------------
HAS_QT := $(call ls,$(QTDIR))
EXAMPLES += ex_debidc
EXAMPLES += ex_events1
EXAMPLES += $(sort $(wildcard vds* vd_sf_lvars))
EXAMPLES-$(HAS_QT) += qproject
EXAMPLES-$(HAS_QT) += qwindow
# do not include ugraph2 in the samples because it is buggy:
# in some cases it combines blocks in such a way that switching to
# graph view becomes impossible
EXAMPLES += ugraph2
EXAMPLES += $(EXAMPLES-1)
examples: $(EXAMPLES)

#----------------------------------------------------------------------
.PHONY: $(PLUGINS) $(SAMPLES) $(EXAMPLES)
$(PLUGINS) $(SAMPLES) $(EXAMPLES):
	$(Q)$(MAKE) -C $@

#----------------------------------------------------------------------
clean::
	$(foreach dir,$(PLUGINS) $(SAMPLES) $(EXAMPLES),$(MAKE) -C $(dir) clean;)
