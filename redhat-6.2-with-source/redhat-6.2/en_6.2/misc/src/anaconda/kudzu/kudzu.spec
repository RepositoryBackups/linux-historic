Name: kudzu
Version: 0.36
Release: 2
Copyright: GPL
Summary: The Red Hat Linux hardware probing tool.
Group: Applications/System
Source: kudzu-%{PACKAGE_VERSION}.tar.gz
Obsoletes: rhs-hwdiag setconsole
Prereq: chkconfig
BuildRoot: /tmp/kudzu

%description
Kudzu is a hardware probing tool run at system boot time to determine
what hardware has been added or removed from the system.

%package devel
Summary: The development library for hardware probing.
Group: Development/Libraries

%description devel
The kudzu-devel package contains the libkudzu and libmodules
libraries, which are libraries used for hardware probing and
configuration.

%prep

%setup

%build
make RPM_OPT_FLAGS="$RPM_OPT_FLAGS" all kudzu ktest

%install
rm -rf $RPM_BUILD_ROOT
make prefix=$RPM_BUILD_ROOT/usr sysconfdir=$RPM_BUILD_ROOT/etc install \
 install-program
strip $RPM_BUILD_ROOT/usr/sbin/kudzu

%clean
rm -rf $RPM_BUILD_ROOT

%post
chkconfig --add kudzu

%preun
if [ $1 = 0 ]; then
	chkconfig --del kudzu
fi

%files
%defattr(-,root,root)
%doc README
/usr/sbin/kudzu
/usr/share/kudzu
/usr/man/man1/*
%config /etc/rc.d/init.d/kudzu

%files devel
%defattr(-,root,root)
/usr/lib/libkudzu.a
/usr/lib/libmodules.a
/usr/include/kudzu/*

%changelog
* Sat Mar  4 2000 Matt Wilson <msw@redhat.com>
- added 810 SVGA mapping

* Thu Mar  2 2000 Bill Nottingham <notting@redhat.com>
- fixes in pci device list merging

* Thu Feb 24 2000 Bill Nottingham <notting@redhat.com>
- fix aliasing and configuration of network devices
- only configure modules that are available

* Mon Feb 21 2000 Bill Nottingham <notting@redhat.com>
- fix handling of token ring devices

* Thu Feb 17 2000 Bill Nottingham <notting@redhat.com>
- yet more serial fixes

* Wed Feb 16 2000 Bill Nottingham <notting@redhat.com>
- more serial fixes; bring back DTR and RTS correctly

* Fri Feb  4 2000 Bill Nottingham <notting@redhat.com>
- don't run serial probe on serial console, fixed right

* Tue Feb  1 2000 Bill Nottingham <notting@redhat.com>
- fix previous fixes.

* Wed Jan 26 2000 Bill Nottingham <notting@redhat.com>
- fix add/remove logic somewhat

* Wed Jan 19 2000 Bill Nottingham <notting@redhat.com>
- don't run serial probe on serial console

* Fri Jan  7 2000 Bill Nottingham <notting@redhat.com>
- fix stupid bug in configuring scsi/net cards

* Mon Oct 25 1999 Bill Nottingham <notting@redhat.com>
- oops, don't try to configure 'unknown's.

* Mon Oct 11 1999 Bill Nottingham <notting@redhat.com>
- fix creation of /etc/sysconfig/soundcard...

* Wed Oct  6 1999 Bill Nottingham <notting@redhat.com>
- add inittab munging for sparc serial consoles...

* Thu Sep 30 1999 Bill Nottingham <notting@redhat.com>
- add sun keyboard probing (from jakub)
- add some bttv support

* Wed Sep 22 1999 Bill Nottingham <notting@redhat.com>
- run 'telinit 5' if needed in the initscript

* Mon Sep 20 1999 Bill Nottingham <notting@redhat.com>
- new & improved UI
- module aliasing fixes

* Thu Sep  9 1999 Bill Nottingham <notting@redhat.com>
- sanitize, homogenize, sterilize...

* Wed Sep  8 1999 Bill Nottingham <notting@redhat.com>
- get geometry for ide drives
- enumerate buses (jj@ultra.linux.cz)
