
include ../../allmake.mak

LDRDIR = $(R)loaders

LOADERS += archldr_tar.py
LOADERS += bios_image.py
LOADERS += hppacore.idc
LOADERS += wince.py
LOADERS += pdfldr.py
LOADERS += uimage.py

all: $(addprefix $(LDRDIR)/, $(LOADERS))

$(LDRDIR)/%.idc: %.idc
	$(CP) $? $@

$(LDRDIR)/%.py: %.py
	$(CP) $? $@

clean::
	rm -f $(addprefix $(LDRDIR)/, $(LOADERS))
