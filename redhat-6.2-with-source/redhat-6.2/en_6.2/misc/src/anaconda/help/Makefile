include ../Makefile.inc

LANGS=C cs de es fr it ja no ro ru sk sl sv tr zh uk da

LANGHTML=$(patsubst %,%-html,$(LANGS))

all:
	@echo "nothing to do"

%-html:
	mkdir -p $*; \
	cd $*;       \
	rm *.html;   \
        if [ "$*" = "ja" ]; then \
            SP_BCTF=ujis jade -t sgml -ihtml -d ../dave-html-ja.dsl ../help-screens-$*.sgml; \
        else         \
            jade -t sgml -ihtml -d ../dave-html.dsl ../help-screens-$*.sgml; \
        fi;          \
	cd -;

html: $(LANGHTML)

install:
	mkdir -p $(DESTDIR)/$(ANACONDADATADIR)/help
	for I in $(LANGS); do \
            mkdir -p $(DESTDIR)/$(ANACONDADATADIR)/help/$$I; \
	    install $$I/*.html $(DESTDIR)/$(ANACONDADATADIR)/help/$$I; \
        done
