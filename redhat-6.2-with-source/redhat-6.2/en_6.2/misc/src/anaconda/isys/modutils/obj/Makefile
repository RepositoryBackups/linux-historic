# $Id: Makefile,v 1.2 1999/09/11 15:43:57 msw Exp $

include ../Makeconfig

#----------------------------------------------------------------------

all: libobj.a

LIBOBJ_OBJS = obj_common.o obj_load.o obj_reloc.o obj_$(ARCH).o

ifeq ($(ARCH),sparc)
LIBOBJ_OBJS += obj_common_64.o obj_load_64.o obj_reloc_64.o obj_sparc64_64.o
endif

libobj.a: $(LIBOBJ_OBJS)
	ar rv $@ $?
	$(RANLIB) $@

realclean clean:
	rm -f *.o *.a .depend

install-bin: all

dep depend:
	gcc -M $(CFLAGS) $(DEFS) $(LIBOBJ_OBJS:.o=.c) > .depend

%.o: %.c
	$(CC) $(CFLAGS) $(DEFS) -DELF_MACHINE_H='"elf_$(ARCH).h"' -DARCH_$(ARCH) -c -o $@ $<

ifeq ($(ARCH),sparc)
%_64.o: %.c
	$(CC) $(CFLAGS) $(DEFS) -DELF_MACHINE_H='"elf_sparc64.h"' -DARCH_sparc64 -c -o $@ $<
endif

ifeq (.depend,$(wildcard .depend))
include .depend
endif
