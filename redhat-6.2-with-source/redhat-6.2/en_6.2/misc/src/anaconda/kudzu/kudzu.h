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

#ifndef _KUDZU_H_
#define _KUDZU_H_

/* kudzu: it grows on you */

#include "device.h"
#include "scsi.h"
#include "ide.h"
#include "sbus.h"
#include "pci.h"
#include "serial.h"
#include "parallel.h"
#include "psaux.h"

/* Do any preinitialization of device data */
/* Only required if you desire to change the default device lists */
int initializeDeviceList();

/* Free any driver lists initialized with initializeDeviceList() */
void freeDeviceList();

/* Probe for devices of the specified class, on the specified bus,
 * with the specified class. Returns a NULL-terminated array of
 * device (or subclass) pointers */
struct device ** probeDevices ( enum deviceClass probeClass, 
			      enum deviceBus probeBus,
			      int probeFlags
			       );

/* Read a config file for a device list */
struct device ** readDevices ( char *fn );

/* Write the NULL-terminated device pointer array to a file, in a
 * format suitable for readDevices() */
int writeDevices ( char *fn, struct device **devlist);

/* Compares the two lists. 
 * retlist1 and retlist2 contain items that are unique to list1/list2. */
int listCompare( struct device **list1, struct device **list2,
		struct device ***retlist1, struct device ***retlist2);
#endif
