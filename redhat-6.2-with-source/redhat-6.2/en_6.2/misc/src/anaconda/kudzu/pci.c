
/* Copyright 1999 Red Hat, Inc.
 *
 * This software may be freely redistributed under the terms of the GNU
 * public license.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <ctype.h>
#include <fcntl.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>

#include <pci/pci.h>

#include "pci.h"

struct pciDevice * pciDeviceList = NULL;
static int numPciDevices = 0;
static struct pci_access *pacc=NULL;
static jmp_buf pcibuf;

static int devCmp(const void * a, const void * b) {
    const struct pciDevice * one = a;
    const struct pciDevice * two = b;
    int x=0,y=0,z=0;
    
    x = (one->vendorId - two->vendorId);
    y = (one->deviceId - two->deviceId);
    if (one->pciType && two->pciType)
	  z = (one->pciType - two->pciType);
    if (x)
      return x;
    else if (y)
      return y;
    else
      return z;
}

static void pciFreeDevice(struct pciDevice *dev) {
    freeDevice((struct device *)dev);
}

static void pciWriteDevice(FILE *file, struct pciDevice *dev) {
	writeDevice(file, (struct device *)dev);
	fprintf(file,"vendorId: %04x\ndeviceId: %04x\npciType: %d\n",dev->vendorId,dev->deviceId,dev->pciType);
}

static int pciCompareDevice(struct pciDevice *dev1, struct pciDevice *dev2)
{
	int x,y;
	
	x = compareDevice((struct device *)dev1,(struct device *)dev2);
	if (x && x!=2) 
	  return x;
	if ((y=devCmp( (void *)dev1, (void *)dev2 )))
	  return y;
	return x;
}
			    
struct pciDevice * pciNewDevice(struct pciDevice *dev) {
    struct pciDevice *ret;
    
    ret = malloc(sizeof(struct pciDevice));
    memset(ret,'\0',sizeof(struct pciDevice));
    ret=(struct pciDevice *)newDevice((struct device *)dev,(struct device *)ret);
    ret->bus = BUS_PCI;
    if (dev && dev->bus == BUS_PCI) {
	ret->vendorId = dev->vendorId;
	ret->deviceId = dev->deviceId;
	ret->pciType = dev->pciType;
    } else {
	ret->pciType = PCI_UNKNOWN;
    }
    ret->newDevice = pciNewDevice;
    ret->freeDevice = pciFreeDevice;
    ret->writeDevice = pciWriteDevice;
    ret->compareDevice = pciCompareDevice;
    return ret;
}

static int vendCmp(const void * a, const void * b) {
    const struct pciDevice * one = a;
    const struct pciDevice * two = b;
    
    return (one->vendorId - two->vendorId);
}

static unsigned int kudzuToPci(enum deviceClass class) {
    switch (class) {
     case CLASS_UNSPEC:
	return 0;
     case CLASS_OTHER:
	return 0;
     case CLASS_NETWORK:
	return PCI_BASE_CLASS_NETWORK;
     case CLASS_VIDEO:
	return PCI_BASE_CLASS_DISPLAY;
     case CLASS_AUDIO:
	return PCI_CLASS_MULTIMEDIA_AUDIO;
     case CLASS_SCSI:
	return PCI_CLASS_STORAGE_SCSI;
     case CLASS_FLOPPY:
	return PCI_CLASS_STORAGE_FLOPPY;
     case CLASS_RAID:
	return PCI_CLASS_STORAGE_RAID;
     case CLASS_CAPTURE:
	return PCI_CLASS_MULTIMEDIA_VIDEO;
     case CLASS_MODEM:
	return PCI_CLASS_COMMUNICATION_SERIAL;
     case CLASS_MOUSE: /* !?!? */
	return PCI_CLASS_INPUT_MOUSE;
     default:
	return 0;
    }
}

static enum deviceClass pciToKudzu(unsigned int class) {
    
    if (!class) return CLASS_UNSPEC;
    switch (class >> 8) {
     case PCI_BASE_CLASS_NETWORK:
	return CLASS_NETWORK;
     case PCI_BASE_CLASS_DISPLAY:
	return CLASS_VIDEO;
     default:
	break;
    }
    switch (class) {
     case PCI_CLASS_STORAGE_SCSI:
	return CLASS_SCSI;
     case PCI_CLASS_STORAGE_FLOPPY:
	return CLASS_FLOPPY;
     case PCI_CLASS_STORAGE_RAID:
	return CLASS_RAID;
     case PCI_CLASS_MULTIMEDIA_AUDIO:
	return CLASS_AUDIO;
     case PCI_CLASS_INPUT_MOUSE:
	return CLASS_MOUSE;
     case PCI_CLASS_MULTIMEDIA_VIDEO:
	return CLASS_CAPTURE;
     case PCI_CLASS_COMMUNICATION_SERIAL:
	return CLASS_MODEM;
     case PCI_CLASS_NOT_DEFINED_VGA:
	return CLASS_VIDEO;
     /* Fix for one of the megaraid variants.
      * It claims to be an I2O controller. */
     case 0x0e00:
	return CLASS_SCSI;
     default:
	return CLASS_OTHER;
    }
}

char *getVendor(unsigned int vendor) {
    struct pciDevice *searchDev, key;
    char *tmpstr;
    
    key.vendorId = vendor;
    
    searchDev = bsearch(&key,pciDeviceList,numPciDevices,
			sizeof(struct pciDevice), vendCmp);
    if (searchDev) {
	int x;
	
	x=strchr(searchDev->desc,'|')-searchDev->desc;
	tmpstr=calloc(x,sizeof(char));
	tmpstr=strncpy(tmpstr,searchDev->desc,x);
	return tmpstr;
    } else {
	return NULL;
    }
}

int pciReadDrivers(char *filename) {
    int fd;
    struct stat sb;
    char * buf;
    int numDrivers;
    int vendid, devid;
    int merge = 0;
    char * start;
    struct pciDevice * nextDevice, *tmpdev, key;
    char module[5000];
    char descrip[5000];
     

    if (filename) {
	    fd = open(filename, O_RDONLY);
	    if (fd < 0)
	      return -1;
    } else {
	    fd = open("/usr/share/kudzu/pcitable", O_RDONLY);
	    if (fd < 0) {
		    fd = open("/etc/pcitable", O_RDONLY);
		    if (fd < 0) {
			    fd = open("/modules/pcitable", O_RDONLY);
			    if (fd < 0) {
				    fd = open("./pcitable", O_RDONLY);
				    if (fd < 0)
				      return -1;
			    }
		    }
	    }
    }
    fstat(fd, &sb);
    buf = alloca(sb.st_size + 1);
    read(fd, buf, sb.st_size);
    buf[sb.st_size] = '\0';
    close(fd);

    /* upper bound */
    numDrivers = 1;
    start = buf;
    while ((start = strchr(start, '\n'))) {
	numDrivers++;
	start++;
    }
    
    if (pciDeviceList)
	  merge = 1;

    pciDeviceList = realloc(pciDeviceList, sizeof(*pciDeviceList) *
				(numPciDevices + numDrivers));
    nextDevice = pciDeviceList + numPciDevices;

    start = buf;
    while (start && *start) {
	while (isspace(*start)) start++;
	if (*start != '#' && *start != '\n') {
	    if (sscanf(start, "%x %x \"%[^\"]\" \"%[^\"]", &vendid,
		       &devid, module, descrip ) == 4) {
		if (merge) {
			    tmpdev = nextDevice;
			    key.vendorId = vendid;
			    key.deviceId = devid;
			    if (strncmp (module, "CardBus:", 8) == 0)
			      key.pciType = PCI_CARDBUS;
			    else
			      key.pciType = PCI_NORMAL;
			          
			    nextDevice = bsearch(&key,pciDeviceList,numPciDevices,
						 sizeof(struct pciDevice), devCmp);
			    if (!nextDevice) {
				    nextDevice = tmpdev;
				    tmpdev = NULL;
				    numPciDevices++;
			    } else {
				    if (nextDevice->device) free(nextDevice->device);
				    if (nextDevice->desc) free(nextDevice->desc);
				    if (nextDevice->driver) free(nextDevice->driver);
			    }
				    
		} else
		      numPciDevices++;
		nextDevice->vendorId = vendid;
		nextDevice->deviceId = devid;
		if (strncmp (module, "CardBus:", 8) == 0) {
		    nextDevice->pciType = PCI_CARDBUS;
		    nextDevice->driver = strdup(&module [8]);
		}
		else {
		    nextDevice->pciType = PCI_NORMAL;
		    nextDevice->driver = strdup(module);
		}
		nextDevice->desc = strdup(descrip);
		nextDevice->next = NULL;
		nextDevice->device = NULL;
		nextDevice->class = 0;
		nextDevice->bus = BUS_PCI;
		if (merge && tmpdev)
		      nextDevice = tmpdev;
		else {
		      nextDevice++;
		      if (merge)
		      	qsort(pciDeviceList, numPciDevices, sizeof(*pciDeviceList), devCmp);
		}
	    }
	}

	start = strchr(start, '\n');
	if (start) start++;
    }

    qsort(pciDeviceList, numPciDevices, sizeof(*pciDeviceList), devCmp);

    return 0;
}

void pciFreeDrivers() {
	int x;
	
	if (pciDeviceList) {
		for (x=0;x<numPciDevices;x++) {
			if (pciDeviceList[x].device) free (pciDeviceList[x].device);
			if (pciDeviceList[x].driver) free (pciDeviceList[x].driver);
			if (pciDeviceList[x].desc) free (pciDeviceList[x].desc);
		}
		free(pciDeviceList);
		pciDeviceList=NULL;
		numPciDevices=0;
	}
}

struct pciDevice * pciGetDeviceInfo(unsigned int vend, unsigned int dev, int bus) {
    struct pciDevice *searchDev, key;
    
    key.vendorId = vend;
    key.deviceId = dev;
    key.pciType = bus;
    
    searchDev = bsearch(&key,pciDeviceList,numPciDevices,
			sizeof(struct pciDevice), devCmp);
    if (!searchDev && key.pciType != PCI_NORMAL) {
	key.pciType = PCI_NORMAL;
	searchDev = bsearch(&key,pciDeviceList,numPciDevices,
			    sizeof(struct pciDevice), devCmp);
    }
    if (!searchDev) {
	char *namebuf;

	searchDev = pciNewDevice(NULL);
	searchDev->vendorId = vend;
	searchDev->deviceId = dev;
	searchDev->pciType = bus;
	searchDev->driver = strdup("unknown");
	searchDev->desc = calloc(128, sizeof(char));
	namebuf = getVendor(vend);
	if (!namebuf) {
	    snprintf(searchDev->desc,128,
		     "Unknown vendor|unknown device %04x:%04x",
		     searchDev->vendorId, searchDev->deviceId);
	} else {
		snprintf(searchDev->desc,128,
		     "%s|unknown device %04x:%04x",
		     namebuf, searchDev->vendorId, searchDev->deviceId);
		free(namebuf);
	}
    } else {
        searchDev = pciNewDevice(searchDev);
	searchDev->pciType = bus;
    }
    return searchDev;
}

static void pcinull(char * foo, ...)
{
}

static void pcibail(char * foo, ...)
{
    longjmp(pcibuf,1);
}

struct device * pciProbe(enum deviceClass probeClass, int probeFlags, struct device *devlist) {
    struct pci_dev *p;
    /* This should be plenty. */
    int cardbus_bridges[32];
    int bridgenum = 0;
    int init_list = 0;
    unsigned int type = kudzuToPci(probeClass),devtype;
    
    if ((probeClass==CLASS_UNSPEC) ||
	(probeClass==CLASS_OTHER) ||
	(probeClass==CLASS_NETWORK) ||
	(probeClass==CLASS_SCSI) ||
	(probeClass==CLASS_VIDEO) ||
	(probeClass==CLASS_AUDIO) ||
	(probeClass==CLASS_MODEM) ||
	(probeClass==CLASS_RAID)) {
	pacc = pci_alloc();
	if (!pacc) return devlist;
	if (!pciDeviceList) {
		pciReadDrivers(NULL);
		init_list = 1;
	}
	pacc->debug=pacc->warning=pcinull;
	pacc->error=pcibail;
	if (!setjmp(pcibuf)) {
	    int order=0;
		
	    pci_init(pacc);
	    pci_scan_bus(pacc);
		
	    memset(cardbus_bridges,'\0',32);
	    
	    for (p = pacc->devices; p; p=p->next) {
		byte config[256];
		int x=64,bustype;
		struct pciDevice *dev,*a_dev;
		
		memset(config,'\0',256);
		pci_read_block(p, 0, config, x);
		if (x<128 &&  (config[PCI_HEADER_TYPE] & 0x7f) == PCI_HEADER_TYPE_CARDBUS) {
		    /* Cardbus bridge */
		    pci_read_block(p, 0, config+64, 64);
		    x=128;
		    for (bridgenum=0; cardbus_bridges[bridgenum];bridgenum++);
		    cardbus_bridges[bridgenum] = config[PCI_CB_CARD_BUS];
		}
		bustype = PCI_NORMAL;
	        for (bridgenum=0; cardbus_bridges[bridgenum]; bridgenum++) {
			if (p->bus == cardbus_bridges[bridgenum])
			  bustype = PCI_CARDBUS;
		}
		dev = pciGetDeviceInfo(p->vendor_id,p->device_id, bustype);
		devtype = config[PCI_CLASS_DEVICE+1] << 8 | config[PCI_CLASS_DEVICE];
		if ( (probeFlags & PROBE_ALL) || (strcmp(dev->driver,"unknown") && strcmp(dev->driver,"ignore"))) {
		    if (!type || (type<0xff && (type==devtype>>8))
			|| (type== kudzuToPci (pciToKudzu (devtype)))) {
			a_dev = pciNewDevice(dev);
			a_dev->class = pciToKudzu(devtype);
			if (a_dev->class == CLASS_NETWORK) {
				if (devtype == PCI_CLASS_NETWORK_TOKEN_RING)
				  a_dev->device = strdup("tr");
				else
				  a_dev->device = strdup("eth");
			}
			a_dev->index = order;
			order++;
			if (devlist) {
			    a_dev->next = devlist;
			}
			devlist = (struct device *)a_dev;
		    } 
		}
		pciFreeDevice(dev);
	    }
	    pci_cleanup(pacc);
	}
    }
    if (pciDeviceList && init_list)
	  pciFreeDrivers();
    return devlist;
}

