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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "isapnp.h"

/* I am in hell. ISAPnP is the fifth circle. */

/* Default to new-style access. */
static int access_type = 1;
/* Needed for old access */
static int readport = 0;

static void isapnpFreeDevice(struct isapnpDevice *dev) {
	if (dev->pdeviceId) free(dev->pdeviceId);
	if (dev->ppnpdesc) free(dev->ppnpdesc);
	if (dev->deviceId) free(dev->deviceId);
	if (dev->pnpdesc) free(dev->pnpdesc);
	if (dev->compat) free(dev->compat);
	if (dev->io) free(dev->io);
	if (dev->irq) free(dev->irq);
	if (dev->dma) free(dev->dma);
	if (dev->mem) free(dev->mem);
	freeDevice((struct device *)dev);
}

static void isapnpWriteDevice(FILE *file, struct isapnpDevice *dev) {
	int x;
	
	writeDevice(file, (struct device *)dev);
	if (dev->pdeviceId)
	  fprintf(file,"pdeviceId: %s\n", dev->pdeviceId);
	if (dev->ppnpdesc)
	  fprintf(file,"ppnpdesc: %s\n", dev->ppnpdesc);
	if (dev->deviceId)
	  fprintf(file,"deviceId: %s\n",dev->deviceId);
	if (dev->pnpdesc)
	  fprintf(file,"pnpdesc: %s\n",dev->pnpdesc);
	if (dev->compat)
	  fprintf(file,"compat: %s\n",dev->compat);
	if (dev->io && dev->io[0]!=-1) {
		fprintf(file,"io: 0x%x", dev->io[0]);
		for (x=1;dev->io[x]!=-1;x++)
		  fprintf(file,",0x%x",dev->io[x]);
		fprintf(file,"\n");
	}
	if (dev->irq && dev->irq[0]!=-1) {
		fprintf(file,"irq: %d", dev->irq[0]);
		for (x=1;dev->irq[x]!=-1;x++)
		  fprintf(file,",%d",dev->irq[x]);
		fprintf(file,"\n");
	}
	if (dev->dma && dev->dma[0]!=-1) {
		fprintf(file,"dma: %d", dev->dma[0]);
		for (x=1;dev->dma[x]!=-1;x++)
		  fprintf(file,",%d",dev->dma[x]);
		fprintf(file,"\n");
	}
	if (dev->mem && dev->mem[0]!=-1) {
		fprintf(file,"mem: 0x%x", dev->mem[0]);
		for (x=1;dev->mem[x]!=-1;x++)
		  fprintf(file,",0x%x",dev->mem[x]);
		fprintf(file,"\n");
	}
}

static int devCmp(const void * a, const void * b) {
	/* All devices are equal... */
	return 0;
}

static int isapnpCompareDevice(struct isapnpDevice *dev1, struct isapnpDevice *dev2)
{
	int x=compareDevice((struct device *)dev1,(struct device *)dev2);
	if (x) return x;
	return devCmp( (void *)dev1, (void *)dev2 );
	/* needs finished */
}

struct isapnpDevice * isapnpNewDevice(struct isapnpDevice *dev) {
	struct isapnpDevice *ret;
    
	ret = malloc(sizeof(struct isapnpDevice));
	memset(ret,'\0',sizeof(struct isapnpDevice));
	ret=(struct isapnpDevice *)newDevice((struct device *)dev,(struct device *)ret);
	ret->bus = BUS_ISAPNP;
	if (dev && dev->bus == BUS_ISAPNP) {
		int x=0;
		
		if (dev->pdeviceId) ret->pdeviceId=strdup(dev->pdeviceId);
		if (dev->ppnpdesc) ret->ppnpdesc=strdup(dev->ppnpdesc);
		if (dev->deviceId) ret->deviceId=strdup(dev->deviceId);
		if (dev->pnpdesc) ret->pnpdesc=strdup(dev->pnpdesc);
		if (dev->compat) ret->compat=strdup(dev->compat);
		if (dev->io) {
			for (x=0;dev->io[x]!=-1;x++) {
				ret->io=realloc(ret->io,(x+1)*sizeof(int));
				ret->io[x]=dev->io[x];
			}
			ret->io[x]=-1;
		}
		if (dev->irq) {
			for (x=0;dev->irq[x]!=-1;x++) {
				ret->irq=realloc(ret->irq,(x+1)*sizeof(int));
				ret->irq[x]=dev->irq[x];
			}
			ret->irq[x]=-1;
		}
		if (dev->dma) {
			for (x=0;dev->dma[x]!=-1;x++) {
				ret->dma=realloc(ret->dma,(x+1)*sizeof(int));
				ret->dma[x]=dev->dma[x];
			}
			ret->dma[x]=-1;
		}
		if (dev->mem) {
			for (x=0;dev->mem[x]!=-1;x++) {
				ret->mem=realloc(ret->mem,(x+1)*sizeof(int));
				ret->mem[x]=dev->mem[x];
			}
			ret->mem[x]=-1;
		}
	}
	ret->newDevice = isapnpNewDevice;
	ret->freeDevice = isapnpFreeDevice;
	ret->writeDevice = isapnpWriteDevice;
	ret->compareDevice = isapnpCompareDevice;
	return ret;
}

int *isapnpReadResources(char *line, int base) {
	int *ret=NULL, x=0;
	char *ptr;
	
	do {
		ptr=strstr(line,",");
		if (ptr) *ptr='\0';
		x++;
		ret = realloc(ret,(x+2)*sizeof(int));
		ret[x-1] = strtoul(line,NULL,base);
		ret[x] = -1;
		if (ptr) line = ptr+1;
	} while (ptr);
	return ret;
}

struct device * isapnpProbeKernel(enum deviceClass probeClass, int probeFlags, struct device *devlist, int fd) {
	char *pnpbuf=NULL;
	char *start, *current, *ptr;
	struct isapnpDevice *dev;
	char pdev[10], pdesc[64];
	char buf[2048];
	int x, len=0;
	
	while ( (x=read(fd,buf,2048))==2048 ) {
		pnpbuf=realloc(pnpbuf,len+2048);
		strncpy(pnpbuf+len,buf,2048);
		len+=2048;
	}
	if (x && x!=-1) {
		pnpbuf=realloc(pnpbuf,len+x);
		strncpy(pnpbuf+len,buf,x);
		len+=x;
		pnpbuf[len]='\0';
	}
	if (!pnpbuf)  {
		close(fd);
		return devlist;
	}
	close(fd);
	
	start = pnpbuf;
	
	while (start && *start) {
		current = start;
		while (*current && *current != '\n') current++;
		if (*current) {
			*current = '\0';
			current++;
		}
		if (!strncmp("Card ",start,5)) {
			start+=8;
			if ( (ptr=strstr(start,":")) ) {
				*ptr='\0';
				strncpy(pdev,start,10);
				start=ptr+1;
				if ( (ptr=strstr(start,"'")) ) {
					*ptr='\0';
					strncpy(pdesc,start,64);
					start=ptr+1;
				}
			}
		} else if (!strncmp("  Logical device",start,16)) {
			dev = isapnpNewDevice(NULL);
			if (devlist) {
				dev->next = devlist;
			}
			devlist = (struct device *)dev;
			dev->pdeviceId = strdup(pdev);
			dev->ppnpdesc = strdup(pdesc);
			dev->driver = strdup("unknown");
			start += 20;
			if ( (ptr=strstr(start,":")) ) {
				*ptr='\0';
				dev->deviceId = strdup(start);
				start = ptr+1;
				if ( (ptr=strstr(start,"'")) ) {
					*ptr = '\0';
					dev->pnpdesc = strdup(start);
				}
			}
			dev->desc = malloc(strlen(dev->ppnpdesc)+strlen(dev->pnpdesc)+3);
			snprintf(dev->desc,strlen(dev->ppnpdesc)+strlen(dev->pnpdesc)+2,"%s:%s",dev->ppnpdesc,dev->pnpdesc);
		} else if (!strncmp("    Compatible device", start,21)) {
			dev->compat=strdup(start+22);
		} else if (!strncmp("    Active port", start, 15)) {
			dev->io=isapnpReadResources(start+16,16);
		} else if (!strncmp("    Active IRQ", start, 14)) {
			dev->irq=isapnpReadResources(start+15,10);
		} else if (!strncmp("    Active DMA", start, 14)) {
			dev->dma=isapnpReadResources(start+15,10);
		} else if (!strncmp("    Active memory", start, 17)) {
			dev->mem=isapnpReadResources(start+18,16);
		}
		start = current;
	}
	return devlist;
}

struct device * isapnpProbeOld(enum deviceClass probeClass, int probeFlags, struct device *devlist) {
	return devlist;
}

struct device * isapnpProbe(enum deviceClass probeClass, int probeFlags, struct device *devlist) {
	int fd;
	
	if (
	    (probeClass == CLASS_UNSPEC) ||
	    (probeClass == CLASS_OTHER) ||
	    (probeClass == CLASS_NETWORK) ||
	    (probeClass == CLASS_MODEM) ||
	    (probeClass == CLASS_AUDIO)
	    ) {
		
		fd = open("./isapnp",O_RDONLY);
		if (fd==-1) {
			fd = open("/proc/isapnp",O_RDONLY);
			if (fd==-1) {
				if (probeFlags & PROBE_SAFE) return devlist;
				access_type = 0;
				devlist = isapnpProbeOld(probeClass, probeFlags, devlist);
			}
		}
		if (fd != -1) {
			devlist = isapnpProbeKernel(probeClass, probeFlags, devlist, fd);
		}
	}
	return devlist;
}

