Summary: Module utilities and kerneld
Name: modutils
Version: 2.1.55
Release: 1
Copyright: GPL
Group: Utilities/System
Source: ftp://ftp.redhat.com/pub/alphabits/modutils-2.1.55.tar.gz
ExclusiveOS: Linux
BuildRoot: /tmp/modutils

%description
The Linux kernel allows new kernel pieces to be loaded and old ones to
be unloaded while the kernel continues to run. These loadable piecs
are called modules, and can include device drivers and filesystems among
other things. This package includes program to load and unload programs
both automatically and manually.

%prep
%setup

%build
make dep all

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/sbin
mkdir -p $RPM_BUILD_ROOT/usr/man/man1
mkdir -p $RPM_BUILD_ROOT/usr/man/man2
mkdir -p $RPM_BUILD_ROOT/usr/man/man8
make install ROOT=$RPM_BUILD_ROOT
cd kerneld
make install-scripts ROOT=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
/sbin/depmod
/sbin/genksyms
/sbin/insmod
/sbin/kerneld
/sbin/ksyms
/sbin/lsmod
/sbin/modprobe
/sbin/rmmod
/sbin/request-route
/usr/man/man1/depmod.1
/usr/man/man1/insmod.1
/usr/man/man1/ksyms.1
/usr/man/man1/lsmod.1
/usr/man/man1/modprobe.1
/usr/man/man1/rmmod.1
/usr/man/man2/create_module.2
/usr/man/man2/delete_module.2
/usr/man/man2/get_kernel_syms.2
/usr/man/man2/init_module.2
/usr/man/man2/query_module.2
/usr/man/man8/genksyms.8
/usr/man/man8/kerneld.8
