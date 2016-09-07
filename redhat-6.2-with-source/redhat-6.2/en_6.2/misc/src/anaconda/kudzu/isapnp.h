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

#ifndef _KUDZU_ISAPNP_H_
#define _KUDZU_ISAPNP_H_

#include "device.h"

struct isapnpDevice {
	/* common fields */
	struct device *next;	/* next device in list */
	int index;
	enum deviceClass class;	/* type */
	enum deviceBus bus;		/* bus it's attached to */
	char * device;		/* device file associated with it */
	char * driver;		/* driver to load, if any */
	char * desc;		/* a description */
	int detached;
	/* isapnp-specific fields */
	struct isapnpDevice  *(*newDevice) (struct isapnpDevice *dev);
	void (*freeDevice) (struct isapnpDevice *dev);
	void (*writeDevice) (FILE *file, struct isapnpDevice *dev);
	int (*compareDevice) (struct isapnpDevice *dev1, struct isapnpDevice *dev2);
	char * pdeviceId;
	char * ppnpdesc;
	char * deviceId;
	char * pnpdesc;
	char * compat;
	int * io;
	int * irq;
	int * dma;
	int * mem;
};

struct isapnpDevice *isapnpNewDevice(struct isapnpDevice *dev);
struct device *isapnpProbe(enum deviceClass probeClass, int probeFlags,
			struct device *devlist);
int *isapnpReadResources(char *line, int base);

#endif
