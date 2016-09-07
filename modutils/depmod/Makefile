# $Id: Makefile,v 1.2 1999/09/11 15:43:57 msw Exp $

include ../Makeconfig

DEFSNOARCH := $(DEFS)
DEFS += -DELF_MACHINE_H='"elf_$(ARCH).h"' -DARCH_$(ARCH)

######################################################################

OBJS = misc.o conf_file.o

all: modprobe depmod

modprobe: modprobe.o $(OBJS) ../util/libutil.a
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^

ifeq ($(ARCH),sparc)
depmod: main.o depmod.o depmod64.o $(OBJS) ../obj/libobj.a ../util/libutil.a
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^

depmod.o: depmod.c
	$(CC) $(CFLAGS) $(DEFS) -Dmain=depmod_main -c -o $@ $<

depmod64.o: depmod.c
	$(CC) $(CFLAGS) $(DEFSNOARCH) -DELF_MACHINE_H='"elf_sparc64.h"' \
	-DARCH_sparc64 -Dmain=depmod64_main -c -o $@ $<
else
depmod: depmod.o $(OBJS) ../obj/libobj.a ../util/libutil.a
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^
endif

clean:
	rm -f *~ core *.bak *.o *.obt *.old
	rm -f depmod modprobe

realclean distclean: clean
	rm -f .depend

install install-bin: all
	$(INSTALL) -c modprobe $(SBINDIR)
	$(INSTALL) -c depmod $(SBINDIR)

dep depend:
	gcc -M $(CFLAGS) $(DEFS) *.c > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
