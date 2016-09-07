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

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "ide.h"

static void ideFreeDevice(struct ideDevice *dev)
{
	if (dev->physical) free(dev->physical);
	if (dev->logical) free(dev->logical);
	freeDevice((struct device *) dev);
}

static void ideWriteDevice(FILE *file, struct ideDevice *dev)
{
	writeDevice(file, (struct device *)dev);
	if (dev->physical)
	  fprintf(file,"physical: %s\n",
		dev->physical);
	if (dev->logical)
	  fprintf(file,"logical: %s\n",
		dev->logical);
	  
}

static int ideCompareDevice(struct ideDevice *dev1, struct ideDevice *dev2)
{
	return compareDevice((struct device *)dev1, (struct device *)dev2);
}

struct ideDevice *ideNewDevice(struct ideDevice *old)
{
	struct ideDevice *ret;

	ret = malloc(sizeof(struct ideDevice));
	memset(ret, '\0', sizeof(struct ideDevice));
	ret = (struct ideDevice *) newDevice((struct device *) old, (struct device *) ret);
	ret->bus = BUS_IDE;
	ret->newDevice = ideNewDevice;
	ret->freeDevice = ideFreeDevice;
	ret->writeDevice = ideWriteDevice;
	ret->compareDevice = ideCompareDevice;
	return ret;
}

struct device *ideProbe(enum deviceClass probeClass, int probeFlags,
			struct device *devlist)
{
	DIR *dir;
	struct dirent *ent;
	char path[80];
	char readbuf[256];
	char *buf, *ptr;
	int fd, i;
	struct ideDevice *newdev;

	if (
	    (probeClass == CLASS_UNSPEC) ||
	    (probeClass == CLASS_OTHER) ||
	    (probeClass == CLASS_CDROM) ||
	    (probeClass == CLASS_FLOPPY) ||
	    (probeClass == CLASS_TAPE) ||
	    (probeClass == CLASS_HD) 
	    ) {
		if (access("/proc/ide", R_OK))
			return devlist;

		if (!(dir = opendir("/proc/ide"))) {
			return devlist;
		}
		while ((ent = readdir(dir))) {
			sprintf(path, "/proc/ide/%s/media", ent->d_name);
			if ((fd = open(path, O_RDONLY)) >= 0) {
				i = read(fd, path, 50);
				close(fd);
				path[i - 1] = '\0';	/* chop off trailing \n */

				newdev = ideNewDevice(NULL);
				if (!strcmp(path, "cdrom"))
					newdev->class = CLASS_CDROM;
				else if (!strcmp(path, "disk"))
					newdev->class = CLASS_HD;
				else if (!strcmp(path, "tape"))
					newdev->class = CLASS_TAPE;
			    	else if (!strcmp(path, "floppy"))
			      		newdev->class = CLASS_FLOPPY;
				else
					newdev->class = CLASS_OTHER;
				newdev->device = strdup(ent->d_name);
				newdev->driver = strdup("ignore");

				sprintf(path, "/proc/ide/%s/model", ent->d_name);
				if ((fd = open(path, O_RDONLY)) >= 0) {
					i = read(fd, path, 50);
					close(fd);
					path[i - 1] = '\0';	/* chop off trailing \n */
					newdev->desc = strdup(path);
				} else {
					newdev->desc = strdup("Generic IDE device");
				}
				sprintf(path,"/proc/ide/%s/geometry", ent->d_name);
				if ((fd = open(path, O_RDONLY)) >= 0) {
					
					i  = read(fd, readbuf, 255);
					close(fd);
					buf=alloca(255);
					strncpy(buf,readbuf,254);
					ptr=buf;
					while(*buf) {
						while (*buf && (*buf != '\n')) buf++;
						if (*buf == '\n') {
							*buf = '\0';
							buf++;
						}
						if (!strncmp(ptr,"physical",8))
						  newdev->physical=strdup(ptr+13);
						if (!strncmp(ptr,"logical",7))
						  newdev->logical=strdup(ptr+13);
						ptr=buf;
					}
				}
				if (newdev->class == probeClass || probeClass == CLASS_UNSPEC) {
					if (devlist)
						newdev->next = devlist;
					devlist = (struct device *) newdev;
				} else {
					newdev->freeDevice(newdev);
				}
			}
			errno = 0;
		}
		closedir(dir);
	}
	return devlist;
}
