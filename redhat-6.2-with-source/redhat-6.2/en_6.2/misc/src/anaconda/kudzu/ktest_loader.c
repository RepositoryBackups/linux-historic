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
    
	initializeDeviceList(BUS_UNSPEC);
	baz=probeDevices(CLASS_UNSPEC,BUS_UNSPEC,PROBE_ALL);
	for (x=0;baz[x];x++)
	{
		baz[x]->writeDevice(stdout,baz[x]);
		baz[x]->freeDevice(baz[x]);
	}
	free(baz);
	exit(0);
}
