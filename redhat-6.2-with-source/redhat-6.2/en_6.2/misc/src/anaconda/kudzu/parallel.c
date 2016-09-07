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

#include "parallel.h"


struct parallelDevice *parallelDeviceList = NULL;
static int numParallelDevices = 0;

static char *procDirs[] = { "/proc/sys/dev/parport", "/proc/parport"
};

static int devCmp(const void * a, const void * b) {
	const struct parallelDevice * one = a;
	const struct parallelDevice * two = b;
	int x=0, y=0;
	
	x = strcmp(one->pnpmfr,two->pnpmfr);
	y = strcmp(one->pnpmodel,two->pnpmodel);
	if (x)
	  return x;
	else
	  return y;
}


static void parallelFreeDevice( struct parallelDevice *dev ) {
	if (dev->pinfo) {
		if (dev->pinfo->uniprint) free(dev->pinfo->uniprint);
		free(dev->pinfo);
	}
	if (dev->pnpmodel) free(dev->pnpmodel);
	if (dev->pnpmfr) free(dev->pnpmfr);
	if (dev->pnpmodes) free(dev->pnpmodes);
	if (dev->pnpdesc) free(dev->pnpdesc);
	freeDevice( (struct device *)dev);
}

static void parallelWriteDevice( FILE *file, struct parallelDevice *dev) 
{
	writeDevice(file, (struct device *)dev);
	if (dev->pnpmodel)
	  fprintf(file,"pnpmodel: %s\n",dev->pnpmodel);
	if (dev->pnpmfr)
	  fprintf(file,"pnpmfr: %s\n",dev->pnpmfr);
	if (dev->pnpmodes)
	  fprintf(file,"pnpmodes: %s\n",dev->pnpmodes);
	if (dev->pnpdesc)
	  fprintf(file,"pnpdesc: %s\n",dev->pnpdesc);
	if (dev->pinfo) {
		fprintf(file,"pinfo.xres: %d\npinfo.yres: %d\npinfo.color: %d\npinfo.ascii: %d\n",
			dev->pinfo->xres, dev->pinfo->yres, dev->pinfo->color, dev->pinfo->ascii);
		if (dev->pinfo->uniprint)
		  fprintf(file,"pinfo.uniprint: %s\n",dev->pinfo->uniprint);
	}
}

static int parallelCompareDevice( struct parallelDevice *dev1, struct parallelDevice *dev2)
{
	int x,y;

	x=compareDevice((struct device *)dev1, (struct device *)dev2);
	if (x && x!=2)
	  return x;
	if ((y=devCmp((void *)dev1, (void *)dev2)))
	  return y;
	return x;
}

struct parallelDevice *parallelNewDevice( struct parallelDevice *old) {
    struct parallelDevice *ret;
    
	ret = malloc( sizeof(struct parallelDevice) );
	memset(ret, '\0', sizeof (struct parallelDevice));
	ret=(struct parallelDevice *)newDevice((struct device *)old,(struct device *)ret);
	ret->bus = BUS_PARALLEL;
	ret->newDevice = parallelNewDevice;
	ret->freeDevice = parallelFreeDevice;
	ret->writeDevice = parallelWriteDevice;
	ret->compareDevice = parallelCompareDevice;
	ret->pinfo = malloc(sizeof (struct printerInfo));
	memset(ret->pinfo, '\0', sizeof(struct printerInfo));
	if (old && old->bus == BUS_PARALLEL) {
		if (old->pnpmodel)
		  ret->pnpmodel=strdup(old->pnpmodel);
		if (old->pnpmfr)
		  ret->pnpmfr=strdup(old->pnpmfr);
		if (old->pnpmodes)
		  ret->pnpmodes=strdup(old->pnpmodes);
		if (old->pnpdesc)
		  ret->pnpdesc=strdup(old->pnpdesc);
		if (old->pinfo) {
			ret->pinfo->xres = old->pinfo->xres;
			ret->pinfo->yres = old->pinfo->yres;
			ret->pinfo->color = old->pinfo->color;
			ret->pinfo->ascii = old->pinfo->ascii;
			if (old->pinfo->uniprint)
			  ret->pinfo->uniprint = strdup(old->pinfo->uniprint);
		}
	}
	return ret;
}

int parallelReadDrivers(char *filename) {
	int dbfile,fieldno=0;
	struct stat sbuf;
	char *filebuf,*ptr;
	struct parallelDevice pdev, *tmpdev, key;
	int merge;
	  
	if (filename) {
		dbfile = open(filename, O_RDONLY);
		if (dbfile < 0)
		  return -1;
	} else {
		dbfile=open("/usr/share/kudzu/printertable",O_RDONLY);
		if (dbfile<0) {
			dbfile=open("/etc/printertable", O_RDONLY);
			if (dbfile<0) {
				dbfile=open("./printertable", O_RDONLY);
				if (dbfile<0)
				  return -1;
			}
		}
	}
	fstat(dbfile,&sbuf);
	filebuf=alloca(sbuf.st_size+1);
	if (read(dbfile,filebuf,sbuf.st_size)!=sbuf.st_size) { 
		perror("read"); 
		close(dbfile);
		return -1;
	}
	if (parallelDeviceList)
	  merge = 1;
	filebuf[sbuf.st_size] = '\0';
	ptr=filebuf;
	while (filebuf[0]!='\0') {
		char tmpstr[128],tmpstr2[128];
		if (!strncmp(ptr,"#section: drivers",17)) break;
		if (ptr[0] == '#') { 
			while (*ptr && *ptr != '\n') ptr++;
			if (*ptr) {
				*ptr='\0';
				ptr++;
			}
			filebuf=ptr;
			continue;
		}
		while (*ptr && *ptr != '\0' && *ptr != '\t' && *ptr !='\n') ptr++;
		if (*ptr) {
			*ptr='\0';
			ptr++;
		}
		switch (fieldno) {
		 case 0:
			strncpy(tmpstr,filebuf,128);
		   	memset(&pdev,'\0',sizeof(struct parallelDevice));
			pdev.bus=BUS_PARALLEL;
			pdev.newDevice=parallelNewDevice;
			pdev.freeDevice=parallelFreeDevice;
			pdev.writeDevice=parallelWriteDevice;
			pdev.compareDevice=parallelCompareDevice;
			pdev.pinfo=calloc(1,sizeof(struct printerInfo));
			break;
		 case 1:
			strncpy(tmpstr2,filebuf,128);
			pdev.desc=malloc(strlen(tmpstr)+strlen(tmpstr2)+2);
			snprintf(pdev.desc,strlen(tmpstr)+strlen(tmpstr2)+2,"%s %s",tmpstr,tmpstr2);
			break;
		 case 2:
			if (strlen(filebuf) && !strcasecmp(filebuf,"Color"))
			  pdev.pinfo->color = 1;
			else
			  pdev.pinfo->color = 0;
			break;
		 case 5:
			if (strlen(filebuf)) {
				if (!strcasecmp(filebuf,"N/A"))
				  pdev.driver=strdup("unknown");
				else
				  pdev.driver=strdup(filebuf);
			}
			break;
		 case 6:
			if (strlen(filebuf) && !strcasecmp(filebuf,"PostScript")) {
				if (pdev.driver) free(pdev.driver);
				pdev.driver=strdup("postscript");
			}
			break;
		 case 7:
			if (strlen(filebuf))
			  pdev.pinfo->xres=atoi(filebuf);
			break;
		 case 8:
			if (strlen(filebuf))
			  pdev.pinfo->yres=atoi(filebuf);
			break;
		 case 10:
			if (strlen(filebuf) && !strcasecmp(filebuf,"t")) 
			  pdev.pinfo->ascii=1;
			else
			  pdev.pinfo->ascii=0;
			break;
		 case 11:
			if (strlen(filebuf))
			  pdev.pnpmfr = strdup(filebuf);
			break;
		 case 12:
			if (strlen(filebuf))
			  pdev.pnpmodel = strdup(filebuf);
			break;
		 case 13:
			if (strlen(filebuf))
			  pdev.pnpmodes = strdup(filebuf);
			break;
		 case 14:
			if (strlen(filebuf))
			  pdev.pnpdesc = strdup(filebuf);
			break;
		 default:
			break;
		}
		fieldno++;
		if (fieldno==16) {
			fieldno=0;
			if (pdev.pnpmfr) {
				tmpdev = NULL;
				if (merge) {
					key.pnpmfr = pdev.pnpmfr;
					key.pnpmodel = pdev.pnpmodel;
					tmpdev = bsearch(&key,parallelDeviceList,numParallelDevices,
							 sizeof(struct parallelDevice), devCmp);
				} 
				if (!tmpdev) {
					numParallelDevices++;
					parallelDeviceList = realloc(parallelDeviceList,
								     (numParallelDevices *
								      sizeof(*parallelDeviceList)));
					parallelDeviceList[numParallelDevices-1]=pdev;
				} else {
					if (tmpdev->pnpdesc) free(tmpdev->pnpdesc);
					if (tmpdev->pnpmfr) free(tmpdev->pnpmfr);
					if (tmpdev->pnpmodel) free(tmpdev->pnpmodel);
					if (tmpdev->pnpmodes) free(tmpdev->pnpmodes);
					if (tmpdev->desc) free(tmpdev->desc);
					free(tmpdev->driver);
					free(tmpdev->pinfo);
					memcpy(tmpdev,&pdev,sizeof(pdev));
				}
			} else {
				if (pdev.pnpdesc) free(pdev.pnpdesc);
				if (pdev.pnpmfr) free(pdev.pnpmfr);
				if (pdev.pnpmodel) free(pdev.pnpmodel);
				if (pdev.pnpmodes) free(pdev.pnpmodes);
				if (pdev.desc) free(pdev.desc);
				free(pdev.driver);
				free(pdev.pinfo);
			}
		}
		filebuf=ptr;
	}
	/* Here we could read the driver descriptions. That's work for later, though. */
	qsort(parallelDeviceList, numParallelDevices, sizeof(struct parallelDevice),  devCmp);
	close(dbfile);
	return 0;
}

void parallelFreeDrivers() {
	int x;
	
	if (parallelDeviceList) {
		for (x=0;x<numParallelDevices;x++) {
			if (parallelDeviceList[x].pnpdesc) free(parallelDeviceList[x].pnpdesc);
			if (parallelDeviceList[x].pnpmfr) free(parallelDeviceList[x].pnpmfr);
			if (parallelDeviceList[x].pnpmodel) free(parallelDeviceList[x].pnpmodel);
			if (parallelDeviceList[x].pnpmodes) free(parallelDeviceList[x].pnpmodes);
			if (parallelDeviceList[x].desc) free(parallelDeviceList[x].desc);
			free(parallelDeviceList[x].driver);
			free(parallelDeviceList[x].pinfo);
		}
		free(parallelDeviceList);
		parallelDeviceList=NULL;
		numParallelDevices=0;
	}
}

struct parallelDevice *readProbeInfo(char *ppath) {
	int lpfile,bytes;
	char *probebuf,*ptr;
	char *mfr,*model,*desc,*func,*modes;
	struct parallelDevice *pardev,key;
	
	lpfile=open(ppath,O_RDONLY);
	if (lpfile==-1) return NULL;
	probebuf=calloc(8192,sizeof(char));
	bytes=read(lpfile,probebuf,8192);
	if (bytes<=0) {
		close(lpfile);
		return NULL;
	}
	
	mfr=model=desc=func=modes=NULL;
	ptr = probebuf;
	while (probebuf[0]!='\0') {
		while (*ptr && *ptr !='\n') ptr++;
		if (*ptr) {
			*ptr='\0';
			/* strip trailing semicolon, too */
			*(ptr-1)='\0';
			ptr++;
		}
		if (!strncmp(probebuf,"MFG:",4) || !strncmp(probebuf,"MANUFACTURER:",13)) {
			mfr=strdup(strstr(probebuf,":")+1);
		}
		if (!strncmp(probebuf,"MDL:",4) || !strncmp(probebuf,"MODEL:",6)) {
			model=strdup(strstr(probebuf,":")+1);
		}
		if (!strncmp(probebuf,"CLS:",4) || !strncmp(probebuf,"CLASS:",6)) {
			func=strdup(strstr(probebuf,":")+1);
		}
		if (!strncmp(probebuf,"CMD:",4) || !strncmp(probebuf,"COMMAND SET:",12)) {
			modes=strdup(strstr(probebuf,":")+1);
		}
		if (!strncmp(probebuf,"DES:",4) || !strncmp(probebuf,"DESCRIPTION:",12)) {
			desc=strdup(strstr(probebuf,":")+1);
		}
		probebuf=ptr;
	}
	if (!strcmp(mfr,"Unknown vendor")&&!strcmp(model,"Unknown device")) return NULL;
	key.pnpmfr = mfr;
	key.pnpmodel = model;
	pardev = bsearch(&key,parallelDeviceList,numParallelDevices,
			 sizeof(struct parallelDevice), devCmp);
	if (!pardev) {
		pardev = parallelNewDevice(NULL);
		if (desc)
		  pardev->desc=strdup(desc);
		else {
			desc=malloc(strlen(mfr)+strlen(model)+2);
			if (mfr && model) {
				snprintf(desc,strlen(mfr)+strlen(model)+2,"%s %s",mfr,model);
			}
			pardev->desc=strdup(desc);
		}
		pardev->driver=strdup("unknown");
		pardev->pnpmfr = strdup(mfr);
		pardev->pnpmodel = strdup(model);
		if (modes)
		  pardev->pnpmodes = strdup(modes);
		if (desc)
		  pardev->pnpdesc = strdup(desc);
	} else {
		pardev = parallelNewDevice(pardev);
	}
	if (func) {
		if (!strcmp(func,"PRINTER")) {
			pardev->class=CLASS_PRINTER;
		} else if (!strcmp(func,"MODEM")) {
			pardev->class=CLASS_MODEM;
		} else if (!strcmp(func,"NET")) {
			pardev->class=CLASS_NETWORK;
		} else if (!strcmp(func,"HDC")) {
			pardev->class=CLASS_HD;
		} else if (!strcmp(func,"FDC")) {
			pardev->class=CLASS_FLOPPY;
		} else if (!strcmp(func,"SCANNER")) {
			pardev->class=CLASS_SCANNER;
		} else {
			pardev->class=CLASS_OTHER;
		}
	} else pardev->class=CLASS_OTHER;
	
	if (mfr) free(mfr);
	if (model) free(model);
	if (func) free(func);
	if (modes) free(modes);
	if (desc) free(desc);
	return pardev;
}

struct device *parallelProbe( enum deviceClass probeClass, int probeFlags,
			 struct device *devlist) {
	DIR *dir;
	struct parallelDevice *pardev;
	struct dirent *dent;
	char path[256];
	int init_list = 0;
	int procdir=0;
    
	if (
	    (probeClass == CLASS_UNSPEC) ||
	    (probeClass == CLASS_OTHER) ||
	    (probeClass == CLASS_NETWORK) ||
	    (probeClass == CLASS_FLOPPY) || 
	    (probeClass == CLASS_CDROM) ||
	    (probeClass == CLASS_HD) ||
	    (probeClass == CLASS_TAPE) ||
	    (probeClass == CLASS_SCANNER)  ||
	    (probeClass == CLASS_PRINTER)
	    ) {
		dir=opendir(procDirs[0]);
		if (!dir) {
			dir=opendir(procDirs[1]);
			if (!dir)
			  return devlist;
			else
			  procdir=1;
		} else {
			procdir=1;
		}
		if (!parallelDeviceList) {
			parallelReadDrivers(NULL);
			init_list = 1;
		}
		while ((dent=readdir(dir))) {
			if (dent->d_name[0]=='.') continue;
			snprintf(path,256,"%s/%s/autoprobe",procDirs[procdir],dent->d_name);
			pardev=readProbeInfo(path);
			if (!pardev) {
				continue;
			}
			if (pardev->class == probeClass || probeClass == CLASS_UNSPEC) {
				snprintf(path,256,"/dev/lp%s",dent->d_name);
				pardev->device = strdup(path);
				if (devlist)
				  pardev->next = devlist;
				devlist = (struct device *) pardev;
			} else {
				pardev->freeDevice(pardev);
			}
		}
		    
	}
	if (parallelDeviceList && init_list)
	  parallelFreeDrivers();
	return devlist;
}
