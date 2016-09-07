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
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "scsi.h"

static void scsiFreeDevice( struct scsiDevice *dev ) {
    freeDevice( (struct device *)dev);
}

static void scsiWriteDevice( FILE *file, struct scsiDevice *dev) {
	writeDevice(file, (struct device *)dev);
	fprintf(file,"host: %d\nid: %d\nchannel: %d\nlun: %d\n",
		dev->host, dev->id, dev->channel, dev->lun);
}

static int scsiCompareDevice (struct scsiDevice *dev1, struct scsiDevice *dev2)
{
        int c,h,i,l,x;
	x = compareDevice( (struct device *)dev1, (struct device *)dev2);
	if (x && x!=2) return x;
	c=dev1->channel-dev2->channel;
	h=dev1->host-dev2->host;
	i=dev1->id-dev2->id;
	l=dev1->lun-dev2->lun;
	if ( c || h || i || l) return 1;
	return x;
}

struct scsiDevice *scsiNewDevice( struct scsiDevice *old) {
    struct scsiDevice *ret;
    
    ret = malloc( sizeof(struct scsiDevice) );
    memset(ret, '\0', sizeof (struct scsiDevice));
    ret=(struct scsiDevice *)newDevice((struct device *)old,(struct device *)ret);
    ret->bus = BUS_SCSI;
    ret->newDevice = scsiNewDevice;
    ret->freeDevice = scsiFreeDevice;
	ret->writeDevice = scsiWriteDevice;
	ret->compareDevice = scsiCompareDevice;
    if (old && old->bus ==BUS_SCSI) {
	ret->host = old->host;
	ret->id = old->id;
	ret->channel = old->channel;
	ret->lun = old->lun;
    }
    return ret;
}

#define SCSISCSI_TOP    0
#define SCSISCSI_HOST   1
#define SCSISCSI_VENDOR 2
#define SCSISCSI_TYPE   3

struct device *scsiProbe( enum deviceClass probeClass, int probeFlags,
			 struct device *devlist) {
    int fd;
    char buf[16384];
    char linebuf[80];
    char typebuf[10];
    int i, state = SCSISCSI_TOP;
    char * start, * chptr, * next, *end;
    int host=-1, channel=-1, id=-1, lun=-1;
    char driveName = 'a';
    char cdromNum = '0';
    char tapeNum = '0';
    char genericName = 'a';
    struct scsiDevice *newdev;
    
    if (
	(probeClass == CLASS_UNSPEC) ||
	(probeClass == CLASS_OTHER) ||
	(probeClass == CLASS_CDROM) ||
	(probeClass == CLASS_TAPE) ||
	(probeClass == CLASS_FLOPPY) ||
	(probeClass == CLASS_SCANNER) ||
	(probeClass == CLASS_HD) ||
	(probeClass == CLASS_PRINTER)
	) {

	if (access("/proc/scsi/scsi", R_OK)) return devlist;

	fd = open("/proc/scsi/scsi", O_RDONLY);
	if (fd < 0) return devlist;
    
	i = read(fd, buf, sizeof(buf) - 1);
	if (i < 1) {
	    close(fd);
	    return devlist;
	}
	close(fd);
	buf[i] = '\0';

	start = buf;
	while (*start) {
	    chptr = start;
	    while (*chptr != '\n') chptr++;
	    *chptr = '\0';
	    next = chptr + 1;
	    
	    switch (state) {
	     case SCSISCSI_TOP:
		if (strcmp("Attached devices: ", start)) {
		    return devlist;
 		}
		state = SCSISCSI_HOST;
		break;

	     case SCSISCSI_HOST:
		if (strncmp("Host: ", start, 6)) {
		    return devlist;
		}
		
		start = strstr(start, "scsi");
		if (!start) {
		    return devlist;
		}
		start += 4;
		host = strtol(start, NULL, 10);

		start = strstr(start, "Channel: ");
		if (!start) {
		    return devlist;
		}
		start += 9;
		channel = strtol(start, NULL, 10);

		start = strstr(start, "Id: ");
		if (!start) {
		    return devlist;
		}
		start += 4;
		id = strtol(start, NULL, 10);
		
		start = strstr(start, "Lun: ");
		if (!start) {
		    return devlist;
		}
		start += 5;
		lun = strtol(start, NULL, 10);
		
		state = SCSISCSI_VENDOR;
		break;
		
	     case SCSISCSI_VENDOR:
		if (strncmp("  Vendor: ", start, 10)) {
		    return devlist;
		}
		
		start += 10;
		end = chptr = strstr(start, "Model:");
		if (!chptr) {
		    return devlist;
		}
		
		chptr--;
		while (*chptr == ' ' && *chptr != ':' ) chptr--;
	        if (*chptr == ':') {
			chptr++;
			*(chptr + 1) = '\0';
			strcpy(linebuf,"Unknown");
		} else {
 			*(chptr + 1) = '\0';
			strcpy(linebuf, start);
		}
		*linebuf = toupper(*linebuf);
		chptr = linebuf + 1;
		while (*chptr) {
		    *chptr = tolower(*chptr);
		    chptr++;
		}
		
		start = end;  /* beginning of "Model:" */
		start += 7;
		
		chptr = strstr(start, "Rev:");
		if (!chptr) {
		    return devlist;
		}
		
		chptr--;
		while (*chptr == ' ') chptr--;
		*(chptr + 1) = '\0';
		
		strcat(linebuf, " ");
		strcat(linebuf, start);
		
		state = SCSISCSI_TYPE;
		
		break;
		
	     case SCSISCSI_TYPE:
		if (strncmp("  Type:", start, 7)) {
		    return devlist;
		}
		*typebuf = '\0';
		
		newdev = scsiNewDevice(NULL);
		if (strstr(start, "Direct-Access")) {
		    sprintf(typebuf, "sd%c", driveName++);
		    newdev->class = CLASS_HD;
		    genericName++;
		} else if (strstr(start, "Sequential-Access")) {
		    sprintf(typebuf, "st%c", tapeNum++);
		    newdev->class = CLASS_TAPE;
		    genericName++;
		} else if (strstr(start, "CD-ROM")) {
		    sprintf(typebuf, "scd%c", cdromNum++);
		    newdev->class = CLASS_CDROM;
		    genericName++;
		} else if (strstr(start, "Scanner")) {
		    sprintf(typebuf, "sg%c", genericName++);
		    newdev->class = CLASS_SCANNER;
		} else if (strstr(start, "Printer")) { /* WTF? */
		    sprintf(typebuf, "sg%c", genericName++);
		    newdev->class = CLASS_PRINTER;
		} else {
		    sprintf(typebuf, "sg%c", genericName++);
		    newdev->class = CLASS_OTHER;
		}
		newdev->device = strdup(typebuf);
		newdev->host = host;
		newdev->channel = channel;
		newdev->lun = lun;
		newdev->id = id;
		newdev->driver = strdup("ignore");
		newdev->desc = strdup(linebuf);
		if (newdev->class == probeClass || probeClass == CLASS_UNSPEC) {
		    if (devlist)
		      newdev->next = devlist;
		    devlist = (struct device *) newdev;
		} else {
		    newdev->freeDevice(newdev);
		}
		state = SCSISCSI_HOST;
	    }
	    start = next;
	}
    }
    return devlist;
}
