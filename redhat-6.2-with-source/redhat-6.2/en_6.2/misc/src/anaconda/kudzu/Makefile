ARCH := $(patsubst i%86,i386,$(shell uname -m))
ARCH := $(patsubst sparc%,sparc,$(ARCH))

CFLAGS= $(RPM_OPT_FLAGS) -Wall -D_GNU_SOURCE
LDFLAGS = 

prefix=$(DESTDIR)/usr
sysconfdir=/etc

VERSION=$(shell awk '/Version:/ { print $$2 }' kudzu.spec)
CVSTAG = r$(subst .,-,$(VERSION))

CFLAGS += -DVERSION=\"$(VERSION)\" -I/usr/include/python1.5

ARCH := $(patsubst sparc64,sparc,$(patsubst i%86,i386,$(shell uname -m)))

all: libkudzu.a libkudzu_loader.a kudzumodule.so

AR = ar
RANLIB = ranlib

OBJS = kudzu.o pci.o serial.o ide.o scsi.o parallel.o psaux.o sbus.o keyboard.o
LOADEROBJS = kudzu_loader.o pci.o sbus.o
KUDOBJS = hwconf.o 

%.o: %.c %.h
	$(CC) -c $(CFLAGS) -o $@ $<

kudzu_loader.o: kudzu.c
	$(CC) -c $(CFLAGS) -D__LOADER__ -o $@ $<

libkudzu_loader.a: $(LOADEROBJS)
	$(AR) cr libkudzu_loader.a $(LOADEROBJS)
	$(RANLIB) libkudzu_loader.a

libkudzu.a: $(OBJS)
	$(AR) cr libkudzu.a $(OBJS)
	$(RANLIB) libkudzu.a

libmodules.a: modules.o
	$(AR) cr libmodules.a modules.o
	$(RANLIB) libmodules.a

ktest: libkudzu.a ktest.c
	$(CC) $(CFLAGS) $(LDFLAGS) ktest.c -o ktest -L. -lkudzu -lpci

ktest_loader: libkudzu_loader.a ktest_loader.c
	$(CC) $(CFLAGS) $(LDFLAGS) ktest_loader.c -o ktest_loader -L. -lkudzu_loader -lpci

kudzu: libkudzu.a libmodules.a $(KUDOBJS) po
	$(CC) $(CFLAGS) $(LDFLAGS) $(KUDOBJS) -o kudzu -L. -lkudzu -lmodules -lpci -lnewt -lslang -lpopt

kudzumodule.so: kudzumodule.o libkudzu.a
	$(CC) -o $@ $< -shared -Wl,-soname,kudzumodule.so -L. -lkudzu -lpci

po:	dummy
	make -C po

installdata:
	[ -d $(prefix)/share/kudzu ] || install -m 755 -d $(prefix)/share/kudzu
	if [ "$(ARCH)" = "sparc" ] ; then \
	   cat pcitable | sed "s|ncr53c8xx|sym53c8xx|g" > $(prefix)/share/kudzu/pcitable ; \
	else \
	   install -m 644 pcitable $(prefix)/share/kudzu/pcitable ;\
	fi

install-program: kudzu
	install -m 644 printertable $(prefix)/share/kudzu/printertable

	install -m 755 -d $(prefix)/sbin
	install -m 755 -d $(prefix)/man/man1
	install -m 755 kudzu $(prefix)/sbin/kudzu
	install -m 755 -d $(sysconfdir)/rc.d/init.d
	install -m 644 kudzu.1 $(prefix)/man/man1/kudzu.1
	install -m 755 kudzu.init $(sysconfdir)/rc.d/init.d/kudzu
	install -m 755 -d $(prefix)/lib
	install -m 644 libkudzu.a $(prefix)/lib/libkudzu.a
	install -m 644 libmodules.a $(prefix)/lib/libmodules.a
	install -m 755 -d $(prefix)/include/kudzu
	for header in device.h ide.h keyboard.h kudzu.h modules.h parallel.h pci.h \
	    	psaux.h sbus.h scsi.h serial.h ; do \
		install -m 644 $$header $(prefix)/include/kudzu/$$header ; \
	done
	make -C po instroot=$(prefix) install


install: installdata 
	mkdir -p $(prefix)/lib/python1.5/site-packages
	install -m 755 kudzumodule.so $(prefix)/lib/python1.5/site-packages

tag-archive:
	@cvs -Q tag -F $(CVSTAG)

create-archive:
	@rm -rf /tmp/kudzu
	@cd /tmp ; cvs -Q -d $(CVSROOT) export -r$(CVSTAG) kudzu || echo "Um... export aborted."
	@mv /tmp/kudzu /tmp/kudzu-$(VERSION)
	@cd /tmp ; tar -czSpf kudzu-$(VERSION).tar.gz kudzu-$(VERSION)
	@rm -rf /tmp/kudzu-$(VERSION)
	@cp /tmp/kudzu-$(VERSION).tar.gz .
	@rm -f /tmp/kudzu-$(VERSION).tar.gz
	@echo ""
	@echo "The final archive is in kudzu-$(VERSION).tar.gz"

archive: clean tag-archive create-archive

clean:
	rm -f *.o kudzu ktest ktest_loader modules *.a core *~ \
	kudzumodule.so *.tar.gz

dummy:
