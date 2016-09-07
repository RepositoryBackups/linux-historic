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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "psaux.h"

static void psauxFreeDevice(struct psauxDevice *dev)
{
	freeDevice((struct device *) dev);
}

static void psauxWriteDevice(FILE *file, struct psauxDevice *dev)
{
	writeDevice(file, (struct device *)dev);
}

static int psauxCompareDevice(struct psauxDevice *dev1, struct psauxDevice *dev2)
{
	return compareDevice( (struct device *)dev1, (struct device *)dev2);
}

struct psauxDevice *psauxNewDevice(struct psauxDevice *old)
{
	struct psauxDevice *ret;

	ret = malloc(sizeof(struct psauxDevice));
	memset(ret, '\0', sizeof(struct psauxDevice));
	ret = (struct psauxDevice *) newDevice((struct device *) old, (struct device *) ret);
	ret->bus = BUS_PSAUX;
	ret->newDevice = psauxNewDevice;
	ret->freeDevice = psauxFreeDevice;
	ret->writeDevice = psauxWriteDevice;
	ret->compareDevice = psauxCompareDevice;
	return ret;
}

struct device *psauxProbe(enum deviceClass probeClass, int probeFlags,
			struct device *devlist)
{
	int portfd;
	char buf[256];
	struct psauxDevice *ps2dev;
	
	if (
	    (probeClass == CLASS_UNSPEC) ||
	    (probeClass == CLASS_MOUSE) 
	    ) {
		portfd=open("/dev/psaux", O_RDONLY|O_NONBLOCK);
		if (portfd>0) {
			if (!((read(portfd,buf,256)==1) && buf[0] == -2)) {
				ps2dev=psauxNewDevice(NULL);
				ps2dev->device=strdup("psaux");
				ps2dev->driver=strdup("genericps/2");
				ps2dev->class=CLASS_MOUSE;
				ps2dev->desc=strdup("Generic PS/2 Mouse");
				if (devlist)
				  ps2dev->next = devlist;
				devlist = (struct device *) ps2dev;
			}
			close(portfd);
		}
	}
	return devlist;
}
