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

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	struct device **baz,**baz2=NULL,**inbaz,**inbaz2;
	int x=0,ret;
    
	initializeDeviceList();
	pciReadDrivers("./pcitable2");
	baz=probeDevices(CLASS_UNSPEC,BUS_UNSPEC,0);
	if (!baz || !(*baz)) exit(-1);
	for (;baz[x];x++);
	if (argc < 2 || strcmp(argv[1],"read")) {
		
		if (argc >= 2 )
		writeDevices(argv[1],baz);
		for (x=0;baz[x];x++)
		{
			baz[x]->freeDevice(baz[x]);
		}
		free(baz);
	} else {
		baz2 = readDevices(argv[2]);

		ret = listCompare(baz,baz2,&inbaz,&inbaz2);
		printf("listcomp returned %d\n",ret);
		x=0;
		printf("list one:\n");
		for (;inbaz[x];x++)
		  inbaz[x]->writeDevice(stdout,inbaz[x]);
		x=0;
		printf("list two:\n");
		for (;inbaz2[x];x++)
		  inbaz2[x]->writeDevice(stdout,inbaz2[x]);
	}
	freeDeviceList();
	exit(0);
}
