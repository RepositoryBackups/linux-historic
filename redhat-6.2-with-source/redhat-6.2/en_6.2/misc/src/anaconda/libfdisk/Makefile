DEBUGFLAGS = -g
CFLAGS     = -Wall $(DEBUGFLAGS) $(INCLUDEPATH) -D_GNU_SOURCE=1 -fPIC -I../isys -DENABLE_NLS
LDFLAGS    = $(LIBPATH)
PYTHONLIBDIR = $(DESTDIR)/usr/lib/python1.5/site-packages
CC = egcs

AR      = ar
RANLIB  = ranlib

LIBFDISK = libfdisk.a

OBJECTS = alloc.o constraints.o extended.o rawio.o primary.o \
	  errlist.o partition.o partspec.o
SOURCES = alloc.c extended.c rawio.c primary.c partition.c \
	  errlist.c partspec.c constraints.c
HEADERS = libfdisk.h alloc.h extended.h rawio.h primary.h \
	  partition.h partspec.h constraints.h

all: $(LIBFDISK) testfdisk newtpyfsedit.so gnomepyfsedit.so

install: all
	install newtpyfsedit.so gnomepyfsedit.so $(PYTHONLIBDIR)

testfdisk: testfdisk.o $(LIBFDISK)

$(LIBFDISK): $(LIBFDISK)($(OBJECTS))

testfdisk.o: testfdisk.c

gnomefsedit.o: gnomefsedit.c gnomefsedit.h fsedit.h
	$(CC) $(CFLAGS) -I/usr/include/python1.5 `glib-config --cflags gthread` `gnome-config --cflags gnomeui` -c gnomefsedit.c

newtpyfsedit.so:	newtpyfsedit.o fsedit.o fstab.o newtfsedit.o libfdisk.a
	gcc -shared -g -o $@ $^ -L. -lfdisk

gnomepyfsedit.o:	pyfsedit.c
	gcc -DINITFN=initgnomepyfsedit -DNAME=\"gnomepyfsedit\" -DGNOME -c $(CFLAGS) -I/usr/include/python1.5 `gtk-config --cflags` -o $@ $^ 

gnomepyfsedit.so:	gnomepyfsedit.o fsedit.o fstab.o gnomefsedit.o libfdisk.a
	gcc -shared -g `gnome-config --cflags gnomeui --libs gnomeui` -o $@ $^ -L. -lfdisk

newtpyfsedit.o:	pyfsedit.c
	gcc -DINITFN=initnewtpyfsedit -DNAME=\"newtpyfsedit\" -c $(CFLAGS) -I/usr/include/python1.5 -o $@ $^ 

clean:
	rm -f *.o *.so *.a *~ core testfdisk testfdisk.o printerrors

%.o: %.c $(HEADERS)
