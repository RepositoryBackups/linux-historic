# Install method for disk image installs (CD & NFS)

from comps import ComponentSet, HeaderList
import os
import rpm
import time
import urllib
import string
import struct
import socket

# we import these explicitly because urllib loads them dynamically, which stinks
import ftplib
import httplib
import StringIO

import todo

FILENAME = 1000000

class InstallMethod:

    def readComps(self, hdlist):
	return ComponentSet(self.baseUrl + '/RedHat/base/comps',
                            hdlist)

    def getFilename(self, h):
	root = "/mnt/sysimage"
	pathlist = [ "/var/tmp", "/tmp",
		     "/." ]
	for p in pathlist:
	    if (os.access(root + p, os.X_OK)):
		tmppath = root + p
		break

	file = tmppath + h[FILENAME]

        connected = 0
        while not connected:
            try:
                urllib.urlretrieve(self.baseUrl + "/RedHat/RPMS/" + h[FILENAME],
                                   file)
            except IOError, (errnum, msg):
#                print "IOError occurred, trying again"
                time.sleep(5)
            else:
                connected = 1
                
	return file

    def unlinkFilename(self, fullName):
	os.remove(fullName)

    def readHeaders(self):

        connected = 0
        while not connected:
            try:
                url = urllib.urlopen(self.baseUrl + "/RedHat/base/hdlist")
            except IOError, (errnum, msg):
#                print "IOError occurred, trying again"
                time.sleep(5)
            else:
                connected = 1
                
	raw = url.read(16)
	hl = []
	while (raw):
	    info = struct.unpack("iiii", raw)
	    magic1 = socket.ntohl(info[0]) & 0xffffffff
	    if (magic1 != 0x8eade801 or info[1]):
		raise TypeError, "bad magic in header"

	    il = socket.ntohl(info[2])
	    dl = socket.ntohl(info[3])
	    totalSize = il * 16 + dl;
	    hdrString = raw[8:] + url.read(totalSize)
	    hdr = rpm.headerLoad(hdrString)
	    hl.append(hdr)

	    raw = url.read(16)

	return HeaderList(hl)

    def targetFstab(self, fstab):
	pass
	    
    def filesDone(self):
	pass

    def __init__(self, url):
	i = string.index(url, '://') + 2
	self.baseUrl = url[0:i]
	rem = url[i:]
	new = string.replace(rem, "//", "/")
	while (new != rem):
	    rem = new
	    new = string.replace(rem, "//", "/")
	rem = new
	self.baseUrl = self.baseUrl + rem
