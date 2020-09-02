
OUTDIR?=../../www/www.hex-rays.com/public_html/hex-rays/products/ida/7.0/docs
SD=/tmp/sdk_guides_staging

EXT=.shtml

TARGETS=$(OUTDIR)/i18n$(EXT) $(OUTDIR)/strlits$(EXT) $(OUTDIR)/idapython_backward_compat_695$(EXT)

all: $(TARGETS)

stem=$(basename $(notdir $(1)))

$(SD):
	mkdir -p $(SD)

$(OUTDIR)/i18n$(EXT): www_skeleton$(EXT) i18n.md Makefile $(SD)
	p4 edit $@
	rm -f $@
	markdown i18n.md > $(SD)/$(call stem,$@).html
	python insert_contents.py -t $< -c $(SD)/$(call stem,$@).html -T BODY --title "Internationalization (i18n)" -o $@
	recode UTF-8..CP1252 $@
	p4 revert -a $@

$(OUTDIR)/strlits$(EXT): www_skeleton$(EXT) strlits.md strlits_utf8.md strlits_nonutf8.md Makefile $(SD)
	p4 edit $@
	rm -f $@
	markdown strlits_utf8.md > $(SD)/$(call stem,$@).html.utf8
	markdown strlits_nonutf8.md > $(SD)/$(call stem,$@).html.nonutf8
	markdown strlits.md > $(SD)/$(call stem,$@).html
	python insert_contents.py -t $(SD)/$(call stem,$@).html  -c $(SD)/$(call stem,$@).html.utf8    -T BODY_UTF8    -o $(SD)/$(call stem,$@).html2
	python insert_contents.py -t $(SD)/$(call stem,$@).html2 -c $(SD)/$(call stem,$@).html.nonutf8 -T BODY_NONUTF8 -o $(SD)/$(call stem,$@).html3
	python insert_contents.py -t $< -c $(SD)/$(call stem,$@).html3 -T BODY --title "Automatic discovery of string literals during auto-analysis" -o $@
	recode UTF-8..CP1252 $@
	p4 revert -a $@

$(OUTDIR)/idapython_backward_compat_695$(EXT): www_skeleton$(EXT) idapython_bc695.md Makefile $(SD)
	p4 edit $@
	rm -f $@
	markdown idapython_bc695.md > $(SD)/$(call stem,$@).html
	python insert_contents.py -t $< -c $(SD)/$(call stem,$@).html -T BODY --title "IDAPython backward-compatibility with 6.95 APIs" -o $@
	recode UTF-8..CP1252 $@
	p4 revert -a $@

clean:
	p4 edit $(TARGETS)
	rm -rf $(SD) $(TARGETS)
