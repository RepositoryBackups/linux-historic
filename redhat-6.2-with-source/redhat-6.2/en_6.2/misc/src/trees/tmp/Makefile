# You can override this from command line
# Should be path to the distribution root
# (root of CD tree)
DISTROOT=../../..
DDISKS=gdth paride

all: update-root do-update

do-update: update-boots update-modules

do-make: make-boots make-hdimage make-ddisks

update-root:
	( cd ../anaconda; ./upd-instroot $(DISTROOT) )

update-boots:
	./updboots

update-modules:
	./updmodules $(DISTROOT)/RedHat/RPMS BOOT-`rpm -qp --qf %{VERSION}-%{RELEASE} $(DISTROOT)/RedHat/RPMS/kernel-BOOT-*.i386.rpm`

make-boots:
	./mkboot -f

make-hdimage:
	./mkstage2

make-ddisks:
	rm -rf ../../../images/drivers
	mkdir -p ../../../images/drivers
	for n in $(DDISKS); do \
	    ./mkdd $$n; \
	done

clean:
	rm -rf hdimage *.nogz
