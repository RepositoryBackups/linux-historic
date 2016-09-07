# $Id: Makefile,v 1.4 1999/09/22 16:51:32 notting Exp $

include ../Makeconfig

DEFSNOARCH:=$(DEFS)
DEFS+=-DELF_MACHINE_H='"elf_$(ARCH).h"' -DARCH_$(ARCH)
ifeq ($(ARCH),sparc)
INSMOD_O=insmod.o insmod64.o
else
INSMOD_O=insmod.o
endif

#----------------------------------------------------------------------

all: insmod rmmod lsmod ksyms \
	libmodutils.a(main.o $(INSMOD_O) rmmod.o logger.o)

%.o: %.c
	$(CC) $(CFLAGS) $(DEFS) -c -o $@ $<

ifeq ($(COMBINE_INSMOD_RMMOD),y)

insmod: main.o $(INSMOD_O) rmmod.o logger.o ../obj/libobj.a ../util/libutil.a
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^
rmmod: insmod
	ln -sf $^ $@
insmod.o: insmod.c
	$(CC) $(CFLAGS) $(DEFS) -Dmain=insmod_main -c -o $@ $<
rmmod.o: rmmod.c
	$(CC) $(CFLAGS) $(DEFS) -Dmain=rmmod_main -c -o $@ $<

ifeq ($(ARCH),sparc)
insmod64.o: insmod.c
	$(CC) $(CFLAGS) $(DEFSNOARCH) -DELF_MACHINE_H='"elf_sparc64.h"' \
	-DARCH_sparc64 -Dmain=insmod64_main -c -o $@ $<
endif

else

insmod: insmod.o logger.o ../obj/libobj.a ../util/libutil.a
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^
rmmod: rmmod.o logger.o ../util/libutil.a
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^

endif

lsmod: lsmod.o logger.o ../util/libutil.a
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^

ksyms: ksyms.o logger.o ../util/libutil.a
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^

realclean clean:
	rm -f *.o *.a .depend insmod rmmod lsmod ksyms

install install-bin: all
	$(INSTALL) insmod $(SBINDIR)
	$(INSTALL) -c rmmod $(SBINDIR)
	$(INSTALL) -c lsmod $(SBINDIR)
	$(INSTALL) -c ksyms $(SBINDIR)

dep depend:
	gcc -M $(CFLAGS) $(DEFS) *.c > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
