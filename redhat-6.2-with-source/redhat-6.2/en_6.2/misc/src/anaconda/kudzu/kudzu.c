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

#include "kudzu.h"
#include "ide.h"
#include "parallel.h"
#include "pci.h"
#include "psaux.h"
#include "serial.h"
#include "sbus.h"
#include "scsi.h"
#include "keyboard.h"
#ifdef _i_wanna_build_this_crap_
#include "isapnp.h"
#endif

#include <fcntl.h>
#include <fnmatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "device.h"

#ifndef __LOADER__

static struct {
    char *prefix;
    char *match;
} fbcon_drivers[] = {
/* The first string is a prefix of fix->id reported by fbcon
   (check linux/drivers/video for that), the latter is
   a shell pattern (see glob(7)) of either the desc or driver
   strings probed by kudzu library (for PCI you can find this
   in pcitable file). */
{ "ATY Mach64", "*:*Mach64*" },
{ "BWtwo", "Sun|Monochrome (bwtwo)" },
{ "CGfourteen", "Sun|SX*" },
{ "CGsix ", "Sun|*GX*" },
{ "CGthree", "Sun|Color3 (cgthree)" },
{ "CLgen", "Cirrus Logic|GD *" },
{ "Creator", "Sun|FFB*" },
{ "DEC 21030 TGA", "DEC|DECchip 21030 [TGA]" },
{ "Elite 3D", "Sun|Elite3D*" },
{ "Leo", "Sun|*ZX*" },
{ "MATROX", "Matrox|MGA*" },
{ "Permedia2", "*3DLabs*" },
{ "TCX", "Sun|TCX*" },
/* { "VESA VGA", ??? }, */
/* { "VGA16 VGA", ??? }, */
{ NULL, NULL },
};

#endif

typedef struct device *(newFunc)(struct device *);
typedef int (initFunc)();
typedef struct device *(probeFunc)(enum deviceClass, int, struct device *);

struct bus {
	char *string;
	struct device *(*newFunc)(struct device *);
	int (*initFunc)(char *filename);
	void (*freeFunc)();
	struct device *(*probeFunc)(enum deviceClass, int, struct device *);
};

char *classStrings[] = {
	"UNSPEC", "OTHER", "NETWORK", "SCSI", "VIDEO", "AUDIO",
	"MOUSE", "MODEM", "CDROM", "TAPE", "FLOPPY", "SCANNER",
	"HD", "RAID", "PRINTER", "CAPTURE", "KEYBOARD", NULL
};

struct bus buses[] = {
	{ "UNSPEC", NULL, NULL, NULL, NULL },
	{ "OTHER", (newFunc *)newDevice, NULL, NULL, NULL },
	{ "PCI", (newFunc *)pciNewDevice, pciReadDrivers, pciFreeDrivers, pciProbe },
	{ "SBUS", (newFunc *)sbusNewDevice, NULL, NULL, sbusProbe },
#ifndef __LOADER__
	{ "PSAUX", (newFunc *)psauxNewDevice, NULL, NULL, psauxProbe },
	{ "SERIAL", (newFunc *)serialNewDevice, NULL, NULL, serialProbe },
	{ "PARALLEL", (newFunc *)parallelNewDevice, parallelReadDrivers, parallelFreeDrivers, parallelProbe },
	{ "SCSI", (newFunc *)scsiNewDevice, NULL, NULL, scsiProbe },
	{ "IDE", (newFunc *)ideNewDevice, NULL, NULL, ideProbe },
	{ "KEYBOARD", (newFunc *)keyboardNewDevice, NULL, NULL, keyboardProbe },
#ifdef _i_wanna_build_this_crap_
        { "ISAPNP", (newFunc *)isapnpNewDevice, NULL, NULL, isapnpProbe },
#endif /* crap */
#endif /* LOADER */
	{ NULL, NULL, NULL, NULL, NULL }
};

struct device *newDevice(struct device *old, struct device *new) {
    if (!old) {
	if (!new) {
	    new = malloc(sizeof(struct device));
	    memset(new,'\0',sizeof(struct device));
	}
	    new->class = CLASS_UNSPEC;
    } else {
	    new->class = old->class;
	    if (old->device) new->device = strdup(old->device);
	    if (old->driver) new->driver = strdup(old->driver);
	    if (old->desc) new->desc = strdup(old->desc);
    }
    new->newDevice = newDevice;
    new->freeDevice = freeDevice;
	new->compareDevice = compareDevice;
    return new;
}

void freeDevice(struct device *dev) {
    if (!dev) {
	    printf("freeDevice(null)\n");
	    abort(); /* return; */
    }
    if (dev->device) free (dev->device);
    if (dev->driver) free (dev->driver);
    if (dev->desc) free (dev->desc);
    free (dev);
}

void writeDevice(FILE *file, struct device *dev) {
	int bus, i;

	if (!file) {
		printf("writeDevice(null,dev)\n");
		abort();
	}
	if (!dev) {
		printf("writeDevice(file,null)\n");
		abort();
	}
	if (dev->bus == BUS_UNSPEC)
		bus = 0;
	else
		for (bus = 1, i = dev->bus; !(i & 1); bus++, i>>=1);
	fprintf(file,"-\nclass: %s\nbus: %s\ndetached: %d\n",
		classStrings[dev->class],buses[bus].string,dev->detached);
	if (dev->device) 
	  fprintf(file,"device: %s\n",dev->device);
	fprintf(file,"driver: %s\ndesc: \"%s\"\n",dev->driver,dev->desc);
}

int compareDevice(struct device *dev1, struct device *dev2) {
	if (!dev1 || !dev2) return 1;
	if (dev1->class != dev2->class) return 1;
	if (dev1->bus != dev2->bus) return 1;
	if (dev1->device && dev2->device && strcmp(dev1->device,dev2->device))
	  return 1;
	/* Look - a special case!
	 * If it's just the driver that changed, we might
	 * want to act differently on upgrades.
	 */
	if (strcmp(dev1->driver,dev2->driver)) return 2;
	return 0;
}

#ifndef __LOADER__

struct device *readDevice(FILE *file) {
	char *linebuf=malloc(512);
	struct device *retdev=NULL, *tmpdev;
	int i;

	if (!file) {
		printf("readDevice(null)\n");
		abort();
	}
	memset(linebuf,'\0',512);
	while (strcmp(linebuf,"-")) {
		memset(linebuf,'\0',512);
		linebuf=fgets(linebuf,512,file);
		if (!linebuf) break;
		/* kill trailing \n */
		(*rindex(linebuf,'\n'))='\0';
		if (!strcmp(linebuf,"-")) {
			break;
		} else {
			if (!retdev) retdev = newDevice(NULL,NULL);
		}
		if (!strncmp(linebuf,"class:",6)) {
			for (i=0; 
			     classStrings[i] && strcmp(classStrings[i],linebuf+7);
			     i++);
			if (classStrings[i])
			  retdev->class = i;
			else
			  retdev->class = CLASS_OTHER;
		} else if (!strncmp(linebuf,"bus:",4)) {
			for (i=0; 
			     buses[i].string && strcmp(buses[i].string,linebuf+5);
			     i++);
			if (buses[i].string) {
				tmpdev = (struct device *)buses[i].newFunc(retdev);
				retdev->freeDevice(retdev);
				retdev = tmpdev;
			} else
			  retdev->bus = BUS_OTHER;
		} else if (!strncmp(linebuf,"driver:",7)) {
			retdev->driver = strdup(linebuf+8);
		} else if (!strncmp(linebuf,"detached:",9)) {
			retdev->detached = atoi(linebuf+10);
		} else if (!strncmp(linebuf,"device:",7)) {
			retdev->device = strdup(linebuf+8);
		} else if (!strncmp(linebuf,"desc:",5)) {
			if (rindex(linebuf,'"')!=index(linebuf,'"')) {
				(*rindex(linebuf,'"')) = '\0';
				retdev->desc = strdup(index(linebuf,'"')+1);
			} else {
				retdev->desc = strdup(linebuf+6);
			}
		}
		switch (retdev->bus) {
		 case BUS_PCI:
			if (!strncmp(linebuf,"vendorId:",9))
			  ((struct pciDevice *)retdev)->vendorId = strtol(linebuf+10, (char **) NULL, 16);
			else if (!strncmp(linebuf,"deviceId:",9))
			  ((struct pciDevice *)retdev)->deviceId = strtol(linebuf+10, (char **) NULL, 16);
			else if (!strncmp(linebuf,"pciType:",8))
			  ((struct pciDevice *)retdev)->pciType = strtol(linebuf+6, (char **) NULL, 9);
			break;
		 case BUS_PARALLEL:
			if (!strncmp(linebuf,"pnpmodel:",9))
			  ((struct parallelDevice *)retdev)->pnpmodel = strdup(linebuf+10);
			if (!strncmp(linebuf,"pnpmfr:",7))
			  ((struct parallelDevice *)retdev)->pnpmfr = strdup(linebuf+8);
			if (!strncmp(linebuf,"pnpmodes:",9))
			  ((struct parallelDevice *)retdev)->pnpmodes = strdup(linebuf+10);
			if (!strncmp(linebuf,"pnpdesc:",8))
			  ((struct parallelDevice *)retdev)->pnpdesc = strdup(linebuf+9);
			if (!strncmp(linebuf,"pinfo.xres:",11))
			  ((struct parallelDevice *)retdev)->pinfo->xres = atoi(linebuf+12);
			if (!strncmp(linebuf,"pinfo.yres:",11))
			  ((struct parallelDevice *)retdev)->pinfo->yres = atoi(linebuf+12);
			if (!strncmp(linebuf,"pinfo.color:",12))
			  ((struct parallelDevice *)retdev)->pinfo->color = atoi(linebuf+13);
			if (!strncmp(linebuf,"pinfo.ascii:",12))
			  ((struct parallelDevice *)retdev)->pinfo->ascii = atoi(linebuf+13);
			if (!strncmp(linebuf,"pinfo.uniprint:",15))
			  ((struct parallelDevice *)retdev)->pinfo->uniprint = strdup(linebuf+16);
			break;
		 case BUS_SERIAL:
			if (!strncmp(linebuf,"pnpmodel:",9))
			  ((struct serialDevice *)retdev)->pnpmodel = strdup(linebuf+10);
			if (!strncmp(linebuf,"pnpmfr:",7))
			  ((struct serialDevice *)retdev)->pnpmfr = strdup(linebuf+8);
			if (!strncmp(linebuf,"pnpcompat:",10))
			  ((struct serialDevice *)retdev)->pnpcompat = strdup(linebuf+11);
			if (!strncmp(linebuf,"pnpdesc:",8))
			  ((struct serialDevice *)retdev)->pnpdesc = strdup(linebuf+9);
			break;
		 case BUS_SBUS:
			if (!strncmp(linebuf,"width:",6))
			  ((struct sbusDevice *)retdev)->width = atoi(linebuf+7);
			if (!strncmp(linebuf,"height:",7))
			  ((struct sbusDevice *)retdev)->height = atoi(linebuf+8);
			if (!strncmp(linebuf,"freq:",5))
			  ((struct sbusDevice *)retdev)->freq = atoi(linebuf+6);
			if (!strncmp(linebuf,"monitor:",8))
			  ((struct sbusDevice *)retdev)->monitor = atoi(linebuf+9);
			break;
		 case BUS_SCSI:
			if (!strncmp(linebuf,"host:",5))
			  ((struct scsiDevice *)retdev)->host = atoi(linebuf+6);
			if (!strncmp(linebuf,"channel:",8))
			  ((struct scsiDevice *)retdev)->channel = atoi(linebuf+9);
			if (!strncmp(linebuf,"id:",3))
			  ((struct scsiDevice *)retdev)->id = atoi(linebuf+4);
			if (!strncmp(linebuf,"lun:",3))
			  ((struct scsiDevice *)retdev)->lun = atoi(linebuf+4);
			break;
		 case BUS_IDE:
			if (!strncmp(linebuf,"physical:",9))
			  ((struct ideDevice *)retdev)->physical = strdup(linebuf+10);
			if (!strncmp(linebuf,"logical:",8))
			  ((struct ideDevice *)retdev)->logical = strdup(linebuf+9);
		        break;
#ifdef _i_wanna_build_this_crap_
		 case BUS_ISAPNP:
			if (!strncmp(linebuf,"pdeviceId:",10))
			  ((struct isapnpDevice *)retdev)->pdeviceId = strdup(linebuf+11);
			if (!strncmp(linebuf,"ppnpdesc:",9))
			  ((struct isapnpDevice *)retdev)->ppnpdesc = strdup(linebuf+10);
			if (!strncmp(linebuf,"deviceId:",9))
			  ((struct isapnpDevice *)retdev)->deviceId = strdup(linebuf+10);
			if (!strncmp(linebuf,"pnpdesc:",8))
			  ((struct isapnpDevice *)retdev)->pnpdesc = strdup(linebuf+9);
			if (!strncmp(linebuf,"compat:",7))
			  ((struct isapnpDevice *)retdev)->compat = strdup(linebuf+8);
			if (!strncmp(linebuf,"io:",3))
			  ((struct isapnpDevice *)retdev)->io = isapnpReadResources(linebuf+4,16);
			if (!strncmp(linebuf,"irq:",4))
			  ((struct isapnpDevice *)retdev)->irq = isapnpReadResources(linebuf+5,10);
			if (!strncmp(linebuf,"dma:",4))
			  ((struct isapnpDevice *)retdev)->dma = isapnpReadResources(linebuf+5,10);
			if (!strncmp(linebuf,"mem:",4))
			  ((struct isapnpDevice *)retdev)->mem = isapnpReadResources(linebuf+5,16);
			break;
#endif
		 default:
			break;
		}
	}
	return retdev;
}

#endif

int initializeDeviceList() {
	int bus;
	
	for (bus=0;buses[bus].string;bus++)
	  if (buses[bus].initFunc)
	    buses[bus].initFunc(NULL);
	return 0;
}

void freeDeviceList() {
	int bus;
	
	for (bus=0;buses[bus].string;bus++)
	  if (buses[bus].freeFunc)
	    buses[bus].freeFunc();
}

/* used to sort device lists by a) type, b) device, c) description */
static int devCmp( const void *a, const void *b )
{
        const struct device *one,*two;
	int x,y,z,zz;
	
	one=((const struct device **)a)[0];
	two=((const struct device **)b)[0];
	x=one->class - two->class;
	if (one->device && two->device)
	  y=strcmp(one->device,two->device);
	else {
		y = one->device - two->device;
	}
	z=two->index - one->index;
	zz=strcmp(one->desc,two->desc);
	if (x)
	  return x;
	else if (y)
	  return y;
	else if (z)
	  return z;
	else
	  return zz;
}
	
#ifndef __LOADER__

struct device ** readDevices ( char *fn ) {
	FILE *confFile;
	char *linebuf;
	struct device *dev, **retdevs=NULL;
	int num=0;
	int index=0,x;
	enum deviceClass cl=CLASS_UNSPEC;
	
	linebuf=calloc(512,sizeof(char));
	confFile = fopen(fn,"r");
	if (!confFile) return NULL;
	
	while (strcmp(linebuf,"-\n")) {
		linebuf=fgets(linebuf,512,confFile);
		if (!linebuf) return NULL;
	}
	while (1) {
		dev = readDevice(confFile);
		if (!dev) break;
		retdevs = realloc (retdevs,(num+2) * sizeof (struct device *));
		retdevs[num] = dev;
		retdevs[num+1] = NULL;
		num++;
	}
	fclose(confFile);
	qsort(retdevs, num, sizeof(struct device *), devCmp);
	for (x=0;retdevs[x];x++) {
		if (retdevs[x]->class!=cl) {
			index = 0;
		}
		retdevs[x]->index = index;
		cl = retdevs[x]->class;
		index++;
	}
	return retdevs;	
}

int writeDevices ( char *fn, struct device **devlist ) {
	int x;
	FILE *confFile;
	
	if (!devlist || !devlist[0]) return 1;
	confFile = fopen(fn,"w");
	if (!confFile) return 1;
	for (x=0;devlist[x];x++) {
		devlist[x]->writeDevice(confFile,devlist[x]);
	}
	fclose(confFile);
	return 0;
}

static void fbProbe( struct device *devices ) {
    FILE *procfb;
    int i, j;
    char name[4], buffer[50], *id;
    struct device *d;

    procfb = fopen("/proc/fb","r");
    if (!procfb) return;
    while (fgets(buffer, 50, procfb) != NULL) {
    	i = atoi (buffer);
    	id = strchr (buffer, ' ') + 1;
	for (j = 0; fbcon_drivers[j].prefix; j++)
	    if (!strncmp (id, fbcon_drivers[j].prefix,
			  strlen (fbcon_drivers[j].prefix)))
		break;
	    if (!fbcon_drivers[j].prefix)
		continue;
	    for (d = devices; d; d = d->next)
		if (!d->device && d->class == CLASS_VIDEO &&
		    (!fnmatch (fbcon_drivers[j].match,
			       d->desc, FNM_NOESCAPE) ||
		     !fnmatch (fbcon_drivers[j].match,
			       d->driver, FNM_NOESCAPE))) {
		    sprintf(name, "fb%d", i);
		    d->device = strdup (name);
		}
    }
    fclose(procfb);
}

#endif

struct device ** probeDevices ( enum deviceClass probeClass,
			      enum deviceBus probeBus,
			      int probeFlags
			      ) {
	struct device *devices=NULL,**devlist=NULL;
	int numDevs=0, bus, x, index=0;
	enum deviceClass cl=CLASS_UNSPEC;
    
	for (bus=1;buses[bus].string;bus++) {
		if ((probeBus & (1 << (bus - 1))) || probeBus == BUS_UNSPEC)
		  if (buses[bus].probeFunc)
		    devices = buses[bus].probeFunc(probeClass, probeFlags, devices);
		if ((probeFlags & PROBE_ONE) && (devices))
		  break;
	}
	if (devices == NULL) return NULL;
#ifndef __LOADER__		
	if (probeClass==CLASS_VIDEO || probeClass == CLASS_UNSPEC)
	    fbProbe(devices);
#endif
	while (devices) {
		devlist=realloc(devlist, (numDevs+2) * sizeof(struct device *));
		devlist[numDevs]=devices;
		devlist[numDevs+1]=NULL;
		numDevs++;
		devices=devices->next;
	}
	qsort(devlist, numDevs, sizeof(struct device *), devCmp);
	for (x=0;devlist[x];x++) {
		if (devlist[x]->class!=cl) {
			index = 0;
		}
		devlist[x]->index = index;
		cl = devlist[x]->class;
		index++;
	}
	return devlist;
}

#ifndef __LOADER__

int listCompare( struct device **list1, struct device **list2, 
		struct device ***retlist1,
		struct device ***retlist2) {
	struct device *curr1, *prev1, *curr2, *prev2, *head1, *head2;
	struct device **ret1=NULL, **ret2=NULL;
	int x, notfound=1;
	
	/* Turn arrays into lists. */
	for (x=0;list1[x];x++) {
		list1[x]->next = list1[x+1];
	}
	for (x=0;list2[x];x++) {
		list2[x]->next = list2[x+1];
	}
	curr1 = head1 = list1[0];
	head2 = list2[0];
	prev1 = NULL;
	while (curr1) {
		curr2 = head2;
		prev2 = NULL;
		while (curr2) {
			if (!(notfound=curr1->compareDevice(curr1,curr2))) {
				if (!prev1) 
				  head1 = curr1->next;
				else
				  prev1->next = curr1->next;
				if (!prev2) 
				  head2 = curr2->next;
				else 
				  prev2->next = curr2->next;
				break;
			} else {
				prev2 = curr2;
				curr2 = curr2->next;
			}
		}
		if (notfound)
		  prev1 = curr1;
		curr1 = curr1 ->next;
	}
	/* Generate return lists */
	if (retlist1) {
		curr1 = head1;
		ret1=malloc(sizeof(struct device *));
		ret1[0]=NULL;
		for(x=0;curr1;x++) {
			ret1=realloc(ret1,(x+2)*sizeof(struct device *));
			ret1[x]=curr1;
			curr1=curr1->next;
		}
		ret1[x]=NULL;
		(*retlist1)=ret1;
	}
	if (retlist2) {
		curr2 = head2;
		ret2=malloc(sizeof(struct device *));
		ret2[0]=NULL;
		for(x=0;curr2;x++) {
			ret2=realloc(ret2,(x+2)*sizeof(struct device *));
			ret2[x]=curr2;
			curr2=curr2->next;
		}
		ret2[x]=NULL;
		(*retlist2)=ret2;
	}
	if (head1 || head2 )
	  return 1;
	else
	  return 0;
}

#endif
