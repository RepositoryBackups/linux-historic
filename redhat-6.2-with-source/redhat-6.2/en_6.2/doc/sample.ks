### Language Specification
#lang ru_RU.KOI8-R 
lang en_US

### Network Configuration
network --bootproto dhcp
#network --bootproto static --ip 192.168.0.1 --netmask 255.255.254.0 --gateway 192.168.0.1 --nameserver 192.168.0.254 

### Source File Location
#cdrom
nfs --server porkchop.redhat.com --dir /mnt/dist/6.2/i386	

### Ethernet Device Configuration
#device ethernet wd --opts "io=0x280, irq=3" 

### Keyboard Configuration
### Will get set to 'us' by default 
### if nothing specified in /etc/sysconfig/keyboard
keyboard us

### Partitioning Information
### Whether to clear out the Master Boot Record (yes/no)
### Which partitions to format (--linux/--all)
###      --linux - only format existing linux partitions
###      --all   - format all existing partitions
### Which partitions to set up on new system as well 
### as size of those partitions
zerombr yes
clearpart --linux
part /boot --size 16
part swap --size 127
part / --size 1000 --grow --maxsize 1400
#part raid.0 --size 80
#part raid.1 --size 80
#raid swap raid.0 raid.1 --level 1 --device md0
install

### Mouse Configuration
### Will only setup 3 types of mice
###     generic        - 2-button serial
###     genericps/2    - 2-button ps/2
###     msintellips/2  - MS Intellimouse
### All three can be setup with or without 3-button
### emulation
### Run 'mouseconfig --help' in order to see other
### supported mouse type and make appropriate change
### NOTE: You will need to run 'mouseconfig' manually
### after installation if you have a non-ps/2 mouse
### and are installing X, as a ps/2 mouse is setup
### by default
#mouse generic --device ttyS0
mouse generic3ps/2

### Time Zone Configuration
### Will get set to 'US/Eastern' if ZONE is missing
### from /etc/sysconfig/clock or if file is missing entirely
timezone --utc US/Eastern

### X Configuration
### Will set up system for minimal resolution and color depth;
### may wish to run Xconfigurator manually after system installation
#xconfig --server "SVGA" --monitor "viewsonic g773"

### Root Password Designation
### '--iscrypted' does not work properly with release 6.1 as shipped;
### will need to get updates from http://support.redhat.com/errata
### in order for this to work correctly; can also just specify
### root password in plain text and change it after system installation;
### e.g. 'rootpw ThisIsThePassword' will get root's password to
### "ThisIsThePassword"
rootpw paSSword 

### Authorization Configuration
#auth --useshadow --enablemd5
auth --enablemd5

### Lilo Configuration
### Does not support pulling kernel 'append' arguments, but those
### can be added into config file using '--append' argument;
### e.g. 'lilo --append "mem=128M" --location mbr' will put the
### memory argument in the /etc/lilo.conf file at install
lilo --location mbr
#lilo --location none

### Package Designation
### The package names, as well as the groups they are a part of can be
### found in the /RedHat/base/comps file; individual packages can be
### specified by entering their names one per line;
### groups (e.g. 'X Window System') can be specified
### by appending a "@" in front of the group name;
### e.g. '@ X Window System'
%packages
python
@ X Window System

### Commands To Be Run Post-Installation
%post
echo "This is in the chroot" > /tmp/message
