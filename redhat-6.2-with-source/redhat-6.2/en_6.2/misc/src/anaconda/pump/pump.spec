Summary: A Bootp and DHCP client for automatic IP configuration.
Name: pump
Version: 0.7.8
Release: 1
Group: System Environment/Daemons
Copyright: MIT
BuildRoot: /var/tmp/pump-root
Source: pump-%{version}.tar.gz
Obsoletes: bootpc netconfig
Requires: initscripts >= 3.92

%description
DHCP (Dynamic Host Configuration Protocol) and BOOTP (Boot Protocol)
are protocols which allow individual devices on an IP network to get
their own network configuration information (IP address, subnetmask,
broadcast address, etc.) from network servers.  The overall purpose of
DHCP and BOOTP is to make it easier to administer a large network.

Pump is a combined BOOTP and DHCP client daemon, which allows your
machine to retrieve configuration information from a server.  You
should install this package if you are on a network which uses BOOTP
or DHCP.

%prep
%setup -q

%build
make

%install
make RPM_BUILD_ROOT=$RPM_BUILD_ROOT install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/usr/sbin/netconfig
/sbin/pump
/usr/man/man8/pump.8*

%changelog
* Thu Feb 24 2000 Erik Troan <ewt@redhat.com>
- set hw type properly (safford@watson.ibm.com)

* Wed Feb 23 2000 Erik Troan <ewt@redhat.com>
- fixed # parsing (aaron@schrab.com)
* Tue Feb 15 2000 Erik Troan <ewt@redhat.com>
- added script argument (Guy Delamarter <delamart@pas.rochester.edu>)
- fixed bug in hostname passing (H.J. Lu)
- fixed time displays to be in wall time, not up time (Chris Johnson)

* Wed Feb  9 2000 Bill Nottingham <notting@redhat.com>
- fix bug in netconfig - hitting 'back' causes bogus config files
  to get written

* Thu Feb 03 2000 Erik Troan <ewt@redhat.com>
- added patch from duanev@io.com which improves debug messages and
  uses /proc/uptime rather time time() -- this should be correct for
  everything but systems that are suspended during their lease time, in
  which case we'll be wrong <sigh>
- added hostname to DISCOVER and RELEASE events; hopefully this gets us
  working for all @HOME systems.
- patch from dunham@cse.msu.edu fixed /etc/resolv.conf parsing

* Wed Feb 02 2000 Cristian Gafton <gafton@redhat.com>
- fix description
- man pages are compressed

* Wed Nov 10 1999 Erik Troan <ewt@redhat.com>
- at some point a separate dhcp.c was created
- include hostname in renewal request
- changed default lease time to 6 hours
- if no hostname is specified on the command line, use gethostname()
  to request one (unless it's "localhost" or "localhost.localdomain")
- properly handle failed renewal attempts
- display (and request) syslog, lpr, ntp, font, and xdm servers

* Tue Sep 14 1999 Michael K. Johnson <johnsonm@redhat.com>
- pump processes cannot accumulate because of strange file
  descriptors (bug only showed up under rp3)

* Tue Sep  7 1999 Bill Nottingham <notting@redhat.com>
- add simple network configurator

* Wed Jun 23 1999 Erik Troan <ewt@redhat.com>
- patch from Sten Drescher for syslog debugging info 
- patch from Sten Drescher to not look past end of dhcp packet for options
- patches form Alan Cox for cleanups, malloc failures, and proper udp checksums
- handle replies with more then 3 dns servers specified
- resend dhcp_discover with proper options field
- shrank dhcp_vendor_length to 312 for rfc compliance (thanks to Ben Reed)
- added support for a config file
- don't replace search pass in /etc/resolv.conf unless we have a better one
- bringing down a device didn't work properly

* Sat May 29 1999 Erik Troan <ewt@redhat.com>
- bootp interfaces weren't being brought down properly
- segv could result if no domain name was given

* Sat May 08 1999 Erik Troan <ewt@redhat.com>
- fixed some file descriptor leakage

* Thu May 06 1999 Erik Troan <ewt@redhat.com>
- set option list so we'll work with NT
- tried to add a -h option, but I have no way of testing it :-(

* Wed Apr 28 1999 Erik Troan <ewt@redhat.com>
- closing fd 1 is important

* Mon Apr 19 1999 Bill Nottingham <notting@redhat.com>
- don't obsolete dhcpcd

* Tue Apr 06 1999 Erik Troan <ewt@redhat.com>
- retry code didn't handle failure terribly gracefully

* Tue Mar 30 1999 Erik Troan <ewt@redhat.com>
- added --lookup-hostname
- generate a DNS search path based on full domain set
- use raw socket for revieving reply; this lets us work properly on 2.2
  kernels when we recieve unicast replies from the bootp server

* Mon Mar 22 1999 Erik Troan <ewt@redhat.com>
- it was always requesting a 20 second lease

* Mon Mar 22 1999 Michael K. Johnson <johnsonm@redhat.com>
- added minimal man page /usr/man/man8/pump.8
