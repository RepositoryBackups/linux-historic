/* Copyright 1999, 2000 Red Hat, Inc.
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
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <errno.h>

#include <sys/mount.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/utsname.h>

#include <libintl.h>
#include <locale.h>

#include <newt.h>
#include <popt.h>

#define _(String) gettext((String))
#define N_(String) (String)

#include "kudzu.h"

#include "modules.h"

#define FILENAME "hwconf"

static int madebak=0;
static int quiet=0;
static int timeout=0;
static int safe=0;

struct device **storedDevs;
int numStored=0;
struct device **currentDevs;
int numCurrent=0;

void startNewt()
{
	char roottext[80];
	
	newtInit();
	newtCls();
	newtPushHelpLine(_(" <Tab>/<Alt-Tab> between elements   |   <Space> selects  |   <F12> next screen"));

	snprintf(roottext, 80,
		 _("Hardware Discovery Utility %s               "
		   "      (C) 2000 Red Hat, Inc."), VERSION);
	newtDrawRootText(0, 0, roottext);
}

void winStatus(int width, int height, char * title,
		char * text, ...) {
    newtComponent t, f;
    char * buf = NULL;
    int size = 0;
    int i = 0;
    va_list args;

    va_start(args, text);

    do {
	size += 1000;
	if (buf) free(buf);
	buf = malloc(size);
	i = vsnprintf(buf, size, text, args);
    } while (i == size);

    va_end(args);

    newtCenteredWindow(width, height, title);

    t = newtTextbox(1, 1, width - 2, height - 2, NEWT_TEXTBOX_WRAP);
    newtTextboxSetText(t, buf);
    f = newtForm(NULL, NULL, 0);

    free(buf);

    newtFormAddComponent(f, t);

    newtDrawForm(f);
    newtRefresh();
    newtFormDestroy(f);
}

int installXRPM(char *server) {
	int x,done=0;
	char path[512];
	char *dirname;
	struct newtWinEntry ne[2];
	
	while (!done) {
		/* OK, let's play Find the RPM! */
		x=newtWinTernary(_("RPM Required"),_("CD"),_("Filesystem"), _("Cancel"),
				_("The XFree86-%s RPM is needed to use this graphics card."
				"You can install it from your Red Hat Linux CD-ROM, "
				"from another location on the filesytem, or postpone "
				"it until later."), server); 
		switch (x) {
		 case 1:
			newtWinMessage(_("Insert CD-ROM"),_("Ok"),
				       _("Please insert your Red Hat Linux CD-ROM."));
			dirname=strdup("/mnt/kudzuXXXXXX");
			x=mkstemp(dirname);
			close(x);
			/* This is a race. :( */
			unlink(dirname);
			mkdir(dirname,0755);
			if (mount("/dev/cdrom",dirname,"iso9660",MS_RDONLY,NULL)) {
				newtWinMessage(_("Mount failed"),_("Ok"),
					       _("Could not mount CD-ROM."));
				unlink(dirname);
				break;
			}
			snprintf(path,512,"/bin/rpm -i %s/XFree86-%s*",dirname,server);
			newtSuspend();
			x=system(path);
			newtResume();
			if (!x) {
				done=1;
				openlog("kudzu",0,LOG_USER);
				syslog(LOG_NOTICE,"installed XFree86-%s server",server);
				closelog();
			} else {
				newtWinMessage(_("RPM Install Failed"),_("Ok"),
					       _("RPM install of XFree86-%s failed."),server);
			}
			umount(dirname);
			unlink(dirname);
			free(dirname);
			break;
		 case 2:
			while (1) {
				dirname=NULL;
				ne[0].text = strdup(_("Path:"));
				ne[0].value = &dirname;
				ne[0].flags = NEWT_FLAG_SCROLL;
				memset(&ne[1],'\0',sizeof(struct newtWinEntry));
			
				snprintf(path,512,_("Please enter the path where the XFree86-%s RPM can be found:"),server);
				x=newtWinEntries(_("Enter path to RPM"),
				       path,60,0,0,40,
				       ne,_("Ok"),_("Cancel"),NULL);
				if (x==2 || x==0) {
					if (dirname) free(dirname);
					break;
				}
				snprintf(path,512,"/bin/rpm -i %s/XFree86-%s*",dirname,server);
				newtSuspend();
				x=system(path);
				newtResume();
				if (!x) {
					done=1;
					free(dirname);
					openlog("kudzu",0,LOG_USER);
					syslog(LOG_NOTICE,"installed XFree86-%s server",server);
					closelog();
					break;
				} else {
					newtWinMessage(_("RPM Install Failed"),_("Ok"),
						       _("RPM install of XFree86-%s failed."),server);
				}
			}
			break;
		 case 3:
			return 1;
			break;
		 default:
		 	return 1;
		 	break;
		}
	}
	return 0;
}

void Xconfig(struct device *dev)
{
	int serverfile;
	int status;
	int cardindex=0;
	struct device *mousedev = NULL;
	int fd;
	int x;
	char path[512];
	char buf[40];
	char servername[40];
	
	for (x=0 ; currentDevs[x] ; x++) {
		if (currentDevs[x]->class == CLASS_MOUSE)
		  mousedev = currentDevs[x];
	}
	memset(buf,'\0',40);
	memset(servername,'\0',40);
	if (!strncmp(dev->driver,"Card:",5)) {
		snprintf(path,512,"/usr/X11R6/bin/Xconfigurator --pick --card \"%s\" --kickstart",
			 dev->driver+5);
		newtSuspend();
		system(path);
		newtResume();
		serverfile=open("/tmp/SERVER",O_RDONLY);
		if (serverfile==-1) return;
		memset(path,'\0',512);
		read(serverfile,path,512);
		close(serverfile);
		strncpy(buf,path,strstr(path," ")-path);
		cardindex=atoi(strstr(path," ")+1);
	} else {
		strncpy(buf,strstr(dev->driver,":")+1,40);
		unlink("/tmp/SERVER");
		serverfile=open("/tmp/SERVER",O_WRONLY|O_CREAT|O_EXCL,0644);
		if (serverfile==-1) return;
		snprintf(path,512,"%s -1\n",buf);
		write(serverfile,path,strlen(path));
		close(serverfile);
	}
	snprintf(path,512,"/bin/rpm -q XFree86-%s --qf \"\" 2>/dev/null",buf);
	newtSuspend();
	status = system(path);
	newtResume();
	if (status) {
		if (installXRPM(buf)) return;
	}
	if (mousedev) 
	  snprintf(path,512,"/usr/X11R6/bin/Xconfigurator --continue --mouse %s --mousedevice %s",
		   mousedev->driver,mousedev->device);
	else
	  snprintf(path,512,"/usr/X11R6/bin/Xconfigurator --continue");
	newtSuspend();
	x=system(path);
	newtResume();
	openlog("kudzu",0,LOG_USER);
	syslog(LOG_NOTICE,"ran Xconfigurator for %s",dev->driver);
	if (x==0) {
		/* This is a sick and gross hack. */
		fd=open("/tmp/ranXconfig",O_CREAT|O_EXCL,0644);
		if (fd!=-1) close(fd);
	}
	closelog();
}

char *checkConfFile()
{
	char path[_POSIX_PATH_MAX];
	struct stat sbuf;
	
	snprintf(path,_POSIX_PATH_MAX,"/etc/sysconfig/%s",FILENAME);
	if (stat(path,&sbuf)==-1) {
		snprintf(path,_POSIX_PATH_MAX,"./%s",FILENAME);
		if (stat(path,&sbuf)==-1) {
			return NULL;
		}
	}
	return strdup(path);
}

int makeLink(struct device *dev, char *name)
{
	char oldfname[256],newfname[256];
	
	if (!dev->device || !name) return 1;
	snprintf(oldfname,256,"/dev/%s",dev->device);
	if (dev->index > 0) {
		snprintf(newfname,256,"/dev/%s%d",name,dev->index);
	} else {
		snprintf(newfname,256,"/dev/%s",name);
	}
	openlog("kudzu",0,LOG_USER);
	syslog(LOG_NOTICE,_("linked %s to %s"),newfname,oldfname);
	closelog();
	return symlink(oldfname,newfname);
}

int removeLink(struct device *dev, char *name)
{
	char newfname[256];
	char oldfname[256];
	int x;
	
	if (!name) return 1;
	if (dev->index > 0) {
		snprintf(newfname,256,"/dev/%s%d",name,dev->index);
	} else {
		snprintf(newfname,256,"/dev/%s",name);
	}
	memset(oldfname,'\0',256);
	x=readlink(newfname,oldfname,255);
	openlog("kudzu",0,LOG_USER);
	if (x!=-1)
	  syslog(LOG_NOTICE,_("unlinked %s (was linked to %s)"),newfname,oldfname);
	else
	  syslog(LOG_NOTICE,_("unlinked %s"),newfname);
	closelog();
	return(unlink(newfname));
}

int isLinked(struct device *dev, char *name)
{
	char path[256],path2[256];
	
	memset(path,'\0',256);
	memset(path2,'\0',256);
	if (!name) return 0;
	if (!dev->device) return 0;
	if (dev->index) 
	  snprintf(path,256,"/dev/%s%d",name,dev->index);
	else 
	  snprintf(path,256,"/dev/%s",name);
	if (readlink(path,path2,256)>0) {
		if (!strcmp(basename(path2),dev->device))
		  return 1;
	}
	return 0;
}

#ifdef __sparc__
/* We load a default keymap so that the user can even
   move around and select what he wants when changing from
   pc to sun keyboard or vice versa. */
int installDefaultKeymap(int sunkbd)
{
	char buf[256], *keymap;
	sprintf (buf, "/bin/loadkeys %s < /dev/tty0 > /dev/tty0 2>/dev/null",
		 sunkbd ? "sunkeymap" : "us");
	system (buf);
	openlog("kudzu",0,LOG_USER);
	syslog(LOG_NOTICE,_("ran loadkeys %s"), sunkbd ? "sunkeymap" : "us");
	closelog();
}

/* Return 0 if /etc/sysconfig/keyboard is ok, -1 if not */
int checkKeyboardConfig(struct device *dev)
{
	char buf[256], *p;
	FILE *f = fopen("/etc/sysconfig/keyboard", "r");

	if (!f) {
		if (errno == ENOENT && dev->device && 
		    !strcmp (dev->device, "console"))
			return 0;
		return -1;
	}
	if (dev->device && !strcmp (dev->device, "console"))
		return -1;
	while (fgets(buf, sizeof(buf), f) != NULL) {
		p = strstr(buf, "KEYBOARDTYPE=");
		if (p == NULL) continue;
		if (strstr (p, "pc") != NULL) {
			if (dev->device == NULL)
				return 0;
			installDefaultKeymap(1);
			return -1;
		} else if (strstr (p, "sun") != NULL) {
			if (dev->device != NULL)
				return 0;
			installDefaultKeymap(0);
			return -1;
		}
		return -1;
	}
	fclose(f);
	return -1;
}

/* Return 0 if /etc/inittab is ok, non-zero if not.
   Bit 0 is set if at least one line needs commenting out,
   bit 1 if the /sbin/getty console line is present, but commented out.
 */
int checkInittab(struct device *dev)
{
	char buf[1024], *p;
	int ret = 0, comment;
	FILE *f = fopen("/etc/inittab", "r");

	while (fgets(buf, sizeof(buf), f) != NULL) {
		for (p = buf; isspace (*p); p++);
		comment = *p == '#';
		if (comment) p++;
		if (*p == '#') p++;
		while (*p && *p != ':') p++;
		if (!*p) continue;
		p++;
		while (*p && *p != ':') p++;
		if (strncmp (p, ":respawn:", 9)) continue;
		p += 9;
		while (*p && isspace (*p)) p++;
		if (!strncmp (p, "/sbin/mingetty", 14)) {
			if (!comment && dev->device && !strcmp (dev->device, "console"))
				ret |= 1;
		} else if (!strncmp (p, "/sbin/getty", 11)) {
			p += 11;
			if (!isspace (*p)) continue;
			if (strstr (p, "console")) {
				if (!comment && (!dev->device || strcmp (dev->device, "console")))
					ret |= 1;
				else if (comment && dev->device && !strcmp (dev->device, "console"))
					ret |= 2;
			}
		}
	}
	fclose(f);
	return ret;
}

int rewriteInittab(struct device *dev)
{
	char buf[1024], *p;
	int ret, check;
	char *comment;
	int isSerial = dev->device && !strcmp (dev->device, "console");
	FILE *f;
	FILE *g;
	char speed[10];	

	check = checkInittab(dev);
	if (!check)
		ret = 2;
	else if (quiet)
		ret = 0;
	else
		ret = newtWinChoice(_("Update /etc/inittab"),_("Yes"),_("No"),
				    isSerial ? _("Your /etc/inittab is not suitable for serial console operation. "
						 "Would you like to update it?") :
					       _("Your /etc/inittab is not suitable for video console operation. "
						 "Would you like to update it?"));
	if (ret == 2)
		return 0;
	if (isSerial) {
		for (p = dev->desc; *p && !isdigit(*p); p++);
		ret = atoi (p);
		switch (ret) {
		default:
			ret = 9600;
			/* Fall through */
		case 9600:
		case 19200:
		case 38400:
			sprintf (speed, "DT%d", ret);
			break;
		case 2400:
		case 57600:
		case 115200:
		case 230400:
			sprintf (speed, "%d", ret);
			break;
		}
	}
	f = fopen("/etc/inittab", "r");
	if (!f) return -1;
	g = fopen("/etc/inittab-", "w");
	if (!g) {
		fclose (f);
		return -1;
	}
	while (fgets(buf, sizeof(buf), f) != NULL) {
		for (p = buf; isspace (*p); p++);
		comment = NULL;
		if (*p == '#') comment = p++;
		if (*p == '#') {
			/* To protect /sbin/mingetty and /sbin/getty respawn lines from being
			   automagically uncommented, just add two ## as in
			   ##7:2345:respawn:/sbin/mingetty tty7
			 */
			fputs (buf, g);
			continue;
		}
		if (isSerial && !strncmp(p, "id:5:", 5)) {
			/* Running X from serial console is generally considered a bad idea. */
			fputs ("id:3:initdefault:\n", g);
			continue;
		}
		while (*p && *p != ':') p++;
		if (!*p) {
			fputs (buf, g);
			continue;
		}
		p++;
		while (*p && *p != ':') p++;
		if (strncmp (p, ":respawn:", 9)) {
			fputs (buf, g);
			continue;
		}
		p += 9;
		while (*p && isspace (*p)) p++;
		if (!strncmp (p, "/sbin/mingetty", 14)) {
			if (isSerial) {
				if (!(check & 2)) {
					fprintf (g, "co:2345:respawn:/sbin/getty console %s vt100\n", speed);
					check |= 2;
				}
				if (!comment) {
					fprintf (g, "#%s", buf);
					continue;
				}
			} else if (comment) {
				if (comment != buf)
					fwrite (buf, 1, comment - buf, g);
				fputs (comment + 1, g);
				continue;
			}
		} else if (!strncmp (p, "/sbin/getty", 11)) {
			p += 11;
			if (!isspace (*p)) {
				fputs (buf, g);
				continue;
			}
			if ((p = strstr (p, "console")) != NULL) {
				if (!isSerial && !comment) {
					fprintf (g, "#%s", buf);
					continue;
				} else if (isSerial && comment) {
					char *q;
					if (comment != buf)
						fwrite (buf, 1, comment - buf, g);
					fwrite (comment + 1, 1, p + 7 - comment - 1, g);
					while (isspace (*p)) p++;
					for (q = p; *q && strchr ("DTF0123456789", *q); q++);
					while (isspace (*q)) q++; 
					if (q != p && *q)
						fprintf (g, " %s %s", speed, q);
					else
						fprintf (g, " %s vt100\n", speed);
					continue;
				}
			}
		}
		fputs (buf, g);
	}
	fclose(f);
	fclose(g);
	unlink("/etc/inittab");
	rename("/etc/inittab-", "/etc/inittab");
	return 0;
}

int rewriteSecuretty(void)
{
	char buf[256], *p;
	int ret;
	FILE *f;

	f = fopen("/etc/securetty", "r+");
	if (!f) return -1;
	while (fgets (buf, sizeof(buf), f) != NULL) {
		if (!strcmp (buf, "console\n")) {
			fclose(f);
			return 0;
		}
		p = strchr (buf, '\n');
	}
	if (quiet)
		ret = 0;
	else
		ret = newtWinChoice(_("Update /etc/securetty"),_("Yes"),_("No"),
				    _("Your /etc/securetty does not contain `console' device, which means "
				      "root won't be able to log in on console. Would you like to update it?"));
	if (ret != 2) {
		if (p)
			fputs ("console\n", f);
		else
			fputs ("\nconsole\n", f);
	}
	fclose(f);
	return 0;
}
#endif

int isAvailable(char *modulename)
{
	struct utsname utsbuf;
	struct stat sbuf;
	char path[512];
	int x;
	char *searchdir[] = {
		"fs",
		"net",
		"scsi",
		"block",
		"cdrom",
		"ipv4",
		"ipv6",
		"sound",
		"fc4",
		"video",
		"misc",
		"pcmcia",
		"atm",
		"usb",
		NULL
	};
	
	uname(&utsbuf);
	/* First, try the current kernel */
	for (x = 0; searchdir[x]; x++) {
		snprintf(path,512,"/lib/modules/%s/%s/%s.o",
			 utsbuf.release, searchdir[x],modulename);
		if (!stat(path,&sbuf))
		  return 1;
	}
	/* If they're running a -BOOT kernel, try the original. */
	if (strstr(utsbuf.release,"BOOT")) {
		char kernelver[64];
		int len;
		
		len = strstr(utsbuf.release,"BOOT")-utsbuf.release;
		strncpy(kernelver,utsbuf.release,len);
		kernelver[len]='\0';
		for (x = 0; searchdir[x]; x++) {
			snprintf(path,512,"/lib/modules/%s/%s/%s.o",
				 kernelver, searchdir[x],modulename);
			if (!stat(path,&sbuf))
			  return 1;
		}
	}
	/* Finally, try non-version specific directories. */
	for (x = 0; searchdir[x]; x++) {
		snprintf(path,512,"/lib/modules/%s/%s.o",
			 searchdir[x],modulename);
		if (!stat(path,&sbuf))
		  return 1;
	}
	return 0;
}


int isConfigured(struct device *dev)
{
	struct confModules *cf;
	char path[256],path2[256];
	struct stat sbuf;
	int ret=0;
	
	memset(path,'\0',256);
	memset(path2,'\0',256);
	cf = readConfModules("/etc/conf.modules");
	switch (dev->class) {
	 case CLASS_NETWORK:
		if (!strcmp(dev->driver,"unknown") ||
		    !strcmp(dev->driver,"ignore"))
		  ret=1;
		if (cf)
		  if (isAliased(cf,dev->device,dev->driver)!=-1)
		    ret = 1;
		break;
	 case CLASS_SCSI:
		if (!strcmp(dev->driver,"unknown") ||
		    !strcmp(dev->driver,"ignore"))
		  ret=1;
		if (cf)
		  if (isAliased(cf,"scsi_hostadapter",dev->driver)!=-1)
		    ret=1;
		break;
	 case CLASS_VIDEO:
		/* Assume on initial runs that if X is configured, we got the right card */
#ifndef __sparc__
		if (!stat("/etc/X11/XF86Config",&sbuf))
		  ret = 1;
#else
		ret = 1;
#endif
		if (!strcmp(dev->driver,"unknown") ||
		    !strcmp(dev->driver,"ignore"))
		  ret=1;
		break;
	 case CLASS_CAPTURE:
		if (!strcmp(dev->driver,"unknown") ||
		    !strcmp(dev->driver,"ignore"))
		  ret=1;
		if (cf)
		  if (isAliased(cf,"char-major-81",dev->driver)!=-1)
		    ret = 1;
		break;
	 case CLASS_AUDIO:
		if (!strcmp(dev->driver,"unknown") ||
		    !strcmp(dev->driver,"ignore"))
		  ret=1;
		if (cf) {
			if (isAliased(cf,"sound",dev->driver)!=-1)
			  ret = 1;
			if (isAliased(cf,"sound-card-",dev->driver)!=-1)
			  ret = 1;
			if (isAliased(cf,"sound-slot-",dev->driver)!=-1)
			  ret = 1;
		}
		break;
	 case CLASS_MOUSE:
		ret = isLinked(dev,"mouse");
		break;
	 case CLASS_MODEM:
		ret = isLinked(dev,"modem");
		break;
	 case CLASS_CDROM:
		ret = isLinked(dev,"cdrom");
		break;
	 case CLASS_SCANNER:
		ret = isLinked(dev,"scanner");
		break;
	 case CLASS_KEYBOARD:
#ifdef __sparc__
		if (!checkKeyboardConfig(dev) && !checkInittab(dev))
			ret = 1;
#endif
		break;	 
	 case CLASS_PRINTER:
#ifdef _we_need_a_printer_tool_
		/* fairly braindead printcap parser */
		{
			char *buf,*ptr,*tmp;
			int fd;
			
			fd = open("/etc/printcap",O_RDONLY);
			if (fd==-1) break;
			fstat(fd,&sbuf);
			buf=malloc(sbuf.st_size+1);
			if (read(fd,buf,sbuf.st_size)!=sbuf.st_size) break;
			buf[sbuf.st_size] = '\0';
			ptr=buf;
			while (buf[0]!='\0') {
				if (ptr[0] == '#') {
					while (*ptr && *ptr != '\n') ptr++;
					if (*ptr) {
						*ptr='\0';
						ptr++;
					}
					buf=ptr;
		                        continue;
				}
				while (*ptr && *ptr != '\0' && *ptr !='\n') ptr++;
				if (*ptr) {
					*ptr='\0';
					ptr++;
				}
				if ((tmp=strstr(buf,"lp=/dev/"))) {
					while (*tmp && *tmp != ':') tmp++;
					if (*tmp) {
						*tmp = '\0';
					}
					if (!strcmp(buf+8,dev->device))
					  ret = 1;
				}
				buf = ptr;
			}
			break;
		}
#endif
	 default:
		/* If we don't know how to configure it, assume it's configured. */
		ret = 1;
		break;
	}
	if (cf)
	  freeConfModules(cf);
	return ret;
}

int configure(struct device *dev)
{
	struct confModules *cf;
	char path[256];
	struct stat sbuf;
	int x,index;
	char *tmpalias;
	
	if (!quiet) {
		snprintf(path,256,_("Configuring %s"),dev->desc);
		winStatus(50,3,_("Configuring"),path);
		sleep(1);
	}
	switch (dev->class) {
	 case CLASS_NETWORK:
		cf = readConfModules("/etc/conf.modules");
		if (!cf) 
		  cf = newConfModules();
		cf->madebackup = madebak;
		if (isAliased(cf,dev->device,dev->driver)==-1) {
			index=0;
			while (1) {
				snprintf(path,256,"%s%d",dev->device,index);
				if ( (tmpalias=getAlias(cf,path)) ) {
					int x, num = 0;
					
					for (x=0;currentDevs[x];x++) {
						if (!strcmp(currentDevs[x]->driver, tmpalias)) {
							num++;
						}
					}
					num ? index += num : index++;
				} else
				  break;
			}
			addAlias(cf,path,dev->driver,CM_REPLACE);
			writeConfModules(cf,"/etc/conf.modules");
			madebak = cf->madebackup;
			openlog("kudzu",0,LOG_USER);
			syslog(LOG_NOTICE,_("aliased %s as %s"),path,dev->driver);
			closelog();
		}
		freeConfModules(cf);
		if (!quiet) {
			snprintf(path,256,"/etc/sysconfig/network-scripts/ifcfg-%s%d",dev->device,index);
			x=0;
			if (!stat(path,&sbuf)) {
				x=newtWinChoice(_("Existing Configuration Detected"),_("Yes"),_("No"),
						_("Migrate existing network configuration?"));
			}
			if (x==2 || x==0)  {
				snprintf(path,256,"/usr/sbin/netconfig --device %s%d",dev->device,index);
				newtSuspend();
				system(path);
				newtResume();
				openlog("kudzu",0,LOG_USER);
				syslog(LOG_NOTICE,_("ran netconfig for %s%d"),dev->device,index);
				closelog();
			}
		}
		break;
	 case CLASS_SCSI:
		cf = readConfModules("/etc/conf.modules");
		if (!cf)
		  cf = newConfModules();
		cf->madebackup = madebak;
		if (isAliased(cf,"scsi_hostadapter",dev->driver)==-1) {
			index=0;
			while (1) {
				if (index)
				  snprintf(path,256,"scsi_hostadapter%d",index);
				else
				  snprintf(path,256,"scsi_hostadapter");
				if (getAlias(cf,path))
				  index++;
				else
				  break;
			}
			addAlias(cf,path,dev->driver,CM_REPLACE);
			writeConfModules(cf,"/etc/conf.modules");
			madebak = cf->madebackup;
			openlog("kudzu",0,LOG_USER);
			syslog(LOG_NOTICE,_("aliased %s as %s"),path,dev->driver);
			closelog();
		}
		freeConfModules(cf);
		break;
	 case CLASS_VIDEO:
		if (!quiet) {
			Xconfig(dev);
		}
		break;
	 case CLASS_CAPTURE:
		cf = readConfModules("/etc/conf.modules");
		if (!cf)
		  cf = newConfModules();
		cf->madebackup = madebak;
		if (isAliased(cf,"char-major-81",dev->driver)==-1) {
			snprintf(path,256,"char-major-81");
			addAlias(cf,path,dev->driver,CM_REPLACE);
			writeConfModules(cf,"/etc/conf.modules");
			madebak = cf->madebackup;
			openlog("kudzu",0,LOG_USER);
			syslog(LOG_NOTICE,_("aliased %s as %s"),path,dev->driver);
			closelog();
		}
		freeConfModules(cf);
		break;		
	 case CLASS_AUDIO:
		if (!quiet) {
			newtSuspend();
			system("sndconfig");
			newtResume();
			openlog("kudzu",0,LOG_USER);
			syslog(LOG_NOTICE,_("ran soundconfig for %s"),dev->driver);
			closelog();
		} else {
			int fd;
			
			if (strcmp(dev->driver,"alsa")) {
			cf = readConfModules("/etc/conf.modules");
			if (!cf)
			  cf = newConfModules();
			cf->madebackup = madebak;
			if (isAliased(cf,"sound",dev->driver)==-1 &&
			    isAliased(cf,"sound-slot-",dev->driver)==-1 &&
			    isAliased(cf,"sound-card-",dev->driver)==-1 &&
			    /* The neomagic driver still has some issues */
			    strcmp(dev->driver,"nm256")) {
				snprintf(path,256,"sound-slot-%d",dev->index);
				addAlias(cf,path,dev->driver,CM_REPLACE);
				writeConfModules(cf,"/etc/conf.modules");
				madebak = cf->madebackup;
				openlog("kudzu",0,LOG_USER);
				syslog(LOG_NOTICE,_("aliased %s as %s"),path,dev->driver);
				closelog();
				unlink("/etc/sysconfig/soundcard");
				fd=open("/etc/sysconfig/soundcard",O_WRONLY|O_CREAT|O_EXCL,0644);
				if (fd!=-1) {
					snprintf(path,256,"%s",dev->driver);
					for (x=0;x<strlen(dev->driver);x++)
					  path[x]=toupper(path[x]);
					write(fd,"CARDTYPE=",9);
					write(fd,path,strlen(path));
					write(fd,"\n",1);
					close(fd);
				}
			}
			}
		}
		break;
	 case CLASS_MOUSE:
		makeLink(dev,"mouse");
		if (!quiet) {
			newtSuspend();
			snprintf(path,256,"/usr/sbin/mouseconfig --device %s %s",dev->device,dev->driver);
			system(path);
			newtResume();
			openlog("kudzu",0,LOG_USER);
			syslog(LOG_NOTICE,_("ran mouseconfig for %s"),dev->device);
			closelog();
		}
		break;
	 case CLASS_KEYBOARD:
#ifdef __sparc__
		rewriteInittab(dev);
		if (dev->device && !strcmp (dev->device, "console"))
			rewriteSecuretty();
#endif
		if (!quiet) {
			newtSuspend();
			system("/usr/sbin/kbdconfig");
			newtResume();
			openlog("kudzu",0,LOG_USER);
			syslog(LOG_NOTICE,_("ran kbdconfig for `%s'"),dev->desc);
			closelog();
		}
		break;
	 case CLASS_MODEM:
		makeLink(dev,"modem");
		break;
	 case CLASS_CDROM:
		makeLink(dev,"cdrom");
		break;
	 case CLASS_SCANNER:
		makeLink(dev,"scanner");
		break;
	 case CLASS_PRINTER:
#ifdef _we_need_a_printer_tool_
		if (!quiet) {
			newtWinMessage( _("Run printtool"), _("Ok"),
			       _("To properly configure your %s, you need"
				 "to run the 'printtool' command from"
				 "the X window system"), dev->desc);
		}
		break;
#endif
	 case CLASS_TAPE:
	 case CLASS_FLOPPY:
	 case CLASS_HD:
	 case CLASS_RAID:
	 default:
		break;
	}
	if (!quiet)
	  newtPopWindow();
	return 0;
}

int unconfigure(struct device *dev)
{
	struct confModules *cf;
	char path[256];
	char *tmpalias;
	int index,needed;
	
	if (!quiet) {
		snprintf(path,256,_("Unconfiguring %s"),dev->desc);
		winStatus(50,3,_("Removing Configuration"),path);
		sleep(1);
	}
	switch (dev->class) {
	 case CLASS_NETWORK:
		cf = readConfModules("/etc/conf.modules");
		if (!cf)
		  cf = newConfModules();
		cf->madebackup = madebak;
		index = 0;
		while (1) {
			snprintf(path,256,"%s%d",dev->device,index);
			tmpalias=getAlias(cf,path);
			if (tmpalias && !strcmp(tmpalias,dev->driver)) {
				int x;
				
				needed = 0;
				
				for (x=0;currentDevs[x];x++) { 
					if (currentDevs[x]->driver &&
					    !strcmp(currentDevs[x]->driver,dev->driver)) {
						needed = 1;
						break;
					}
				}
				if (!needed)
				  removeAlias(cf,path,CM_REPLACE);
			} else
			  break;
			index++;
		}
		writeConfModules(cf,"/etc/conf.modules");
		madebak = cf->madebackup;
		freeConfModules(cf);
		break;
	 case CLASS_SCSI:
		cf = readConfModules("/etc/conf.modules");
		if (!cf)
		  cf = newConfModules();
		cf->madebackup = madebak;
		index = 0;
		while (1) {
			if (index) 
			  snprintf(path,256,"scsi_hostadapter%d",index);
			else
			  snprintf(path,256,"scsi_hostadapter");
			tmpalias=getAlias(cf,path);
			if (tmpalias && !strcmp(tmpalias,dev->driver)) {
				int x;
				
				needed = 0;
				
				for (x=0;currentDevs[x];x++) { 
					if (currentDevs[x]->driver &&
					    !strcmp(currentDevs[x]->driver,dev->driver)) {
						needed = 1;
						break;
					}
				}
				if (!needed)
				  removeAlias(cf,path,CM_REPLACE);
			} else
			  break;
			index++;
		}
		writeConfModules(cf,"/etc/conf.modules");
		madebak = cf->madebackup;
		freeConfModules(cf);
		break;
	 case CLASS_VIDEO:
		break;
	 case CLASS_AUDIO:
		break;
	 case CLASS_MOUSE:
		removeLink(dev,"mouse");
		break;
	 case CLASS_MODEM:
		removeLink(dev,"modem");
		break;
	 case CLASS_CDROM:
		removeLink(dev,"cdrom");
		break;
	 case CLASS_SCANNER:
		removeLink(dev,"scanner");
		break;
	 case CLASS_PRINTER:
	 case CLASS_TAPE:
	 case CLASS_FLOPPY:
	 case CLASS_HD:
	 case CLASS_RAID:
	 case CLASS_KEYBOARD:
	 default:
		break;
	}
	if (!quiet)
	  newtPopWindow();
	return 0;
}

void showWelcome(int timeout) {
	int x=timeout;
	int y=0;
	struct pollfd pfd;
	newtComponent textbox, form;
	char message[2048];
	
	pfd.fd = 0;
	pfd.events = POLLIN | POLLPRI;
	if (x) {
		newtCenteredWindow(60,11,_("Welcome to Kudzu"));
		textbox = newtTextbox(1,1,58,9,NEWT_TEXTBOX_WRAP);
	} else {
		newtCenteredWindow(60,9,_("Welcome to Kudzu"));
		textbox = newtTextbox(1,1,58,7,NEWT_TEXTBOX_WRAP);
	}
	form = newtForm(NULL,NULL,0);
	newtFormAddComponent(form,textbox);
	newtDrawForm(form);
	do {
		if (x>0) {
			snprintf(message,2048,
			  _("Welcome to Kudzu, the Red Hat Linux hardware "
			    "detection and configuration tool.\n\n"
			    "On the following screens you will be able to "
			    "configure any new or removed hardware for your "
			    "computer.\n\n"
			    "                Press any key to continue.\n\n"
			    "         Normal bootup will continue in %d seconds."), x );
		} else {
			snprintf(message,2048,
			  _("Welcome to Kudzu, the Red Hat Linux hardware "
			    "detection and configuration tool.\n\n"
			    "On the following screens you will be able to "
			    "configure any new or removed hardware for your "
			    "computer.\n\n"
			    "                Press any key to continue."));
		}
		newtTextboxSetText(textbox,message);
		newtDrawForm(form);
		newtRefresh();
		y=poll(&pfd,1,1000);
		if (y>0 && pfd.revents & (POLLIN | POLLPRI))
		  break;
		x--;
	} while (x!=0);
	if (x==0 && y<=0) {
		winStatus(60,11,_("Welcome to Kudzu"),
		    _("Welcome to Kudzu, the Red Hat Linux hardware "
		      "detection and configuration tool.\n\n\n\n"
		      "                             Timeout exceeded."));
		sleep(1);
		newtPopWindow();
		newtPopWindow();
		newtFinished();
		exit(5);
	}
	newtPopWindow();
}

static struct device *listRemove(struct device *devlist, struct device *dev) {
	struct device *head,*ptr,*prev;

	head = ptr = devlist;
	prev = NULL;
	while (ptr != NULL) {
		if (!ptr->compareDevice(ptr,dev)) {
			if (ptr == head) {
				head = head->next;
			} else {
				prev->next = ptr->next;
			}
		}
		prev = ptr;
		ptr = ptr->next;
	}
	return head;
}

void configMenu(struct device *oldDevs, struct device *newDevs, int runFirst)
{
	int y, z, rc;
	struct device *dev, *tmpdev;
	
	/* First, make sure we have work to do... */
	dev = oldDevs;
	for ( ; dev; dev=dev->next) {
		if (dev->class != CLASS_OTHER &&
		    !(dev->bus == BUS_PCI &&
		      (!strcmp(dev->driver, "ignore") ||
		       !strcmp(dev->driver, "unknown")))) {
			if (!dev->detached) {
				/* If the device only changed in the driver used, ignore it */
				tmpdev = newDevs;
				for ( ; tmpdev ; tmpdev = tmpdev->next) {
					if (tmpdev->compareDevice(tmpdev,dev) == 2) {
						oldDevs = listRemove(oldDevs,dev);
						newDevs = listRemove(newDevs,tmpdev);
						continue;
					}
				}
				continue;
			} else {
				/* Add detached devices to current list */
				currentDevs = realloc(currentDevs,(numCurrent+2)*sizeof(struct device *));
				currentDevs[numCurrent] = dev;
				currentDevs[numCurrent+1] = NULL;
				numCurrent++;
			}
		}
		oldDevs = listRemove(oldDevs, dev);
	}
	dev = newDevs;
	for ( ; dev; dev=dev->next) {
		if (dev->class != CLASS_OTHER &&
		    !(dev->bus == BUS_PCI && 
		      (!strcmp(dev->driver, "ignore") || 
		       !strcmp(dev->driver, "unknown"))) )
		  if (!runFirst || !isConfigured(dev)) {
			  switch (dev->class) {
			   case CLASS_NETWORK:
			   case CLASS_SCSI:
			   case CLASS_CAPTURE:
			   case CLASS_AUDIO:
				  if (isAvailable(dev->driver))
				    continue;
				  break;
			   default:
				  continue;
			  }

		  }
		newDevs = listRemove(newDevs, dev);
	}
	if (!oldDevs && !newDevs)
	  return;
	
	/* Now, do it... */

	if (!quiet) {
		startNewt();
		showWelcome(timeout);
	}
	dev = oldDevs;
	for ( ; dev ; dev = dev->next ) {
		if (!quiet)
		  rc=newtWinTernary(_("Hardware Removed"),_("Remove Configuration"),
				    _("Keep Configuration"),_("Do Nothing"),
				    _("The following hardware has been removed from "
				      "your system:\n        %s\n\n"
				      "You can choose to:\n\n"
				      "1) Remove any existing "
				      "configuration for the device.\n"
				      "2) Keep the existing configuration. "
				      "You will not be prompted "
				      "again if the device seems to be missing.\n"
				      "3) Do nothing. The configuration will "
				      "not be removed, but if the device is found missing on "
				      "subsequent reboots, you will be prompted again."),
				    dev->desc);
		else
		  rc=0;
		switch (rc) {
		 case 0:
		 case 1:
			unconfigure(dev);
			break;
		 case 2:
		 case 3:
		 default:
			if (rc==2)
			  dev->detached = 1;
			currentDevs = realloc(currentDevs,(numCurrent+2)*sizeof(struct device *));
			currentDevs[numCurrent] = dev;
			currentDevs[numCurrent+1] = NULL;
			numCurrent++;
			break;
		}
	}
	dev = newDevs;
	for ( ; dev ; dev = dev->next) {
		if (!quiet)
		  rc=newtWinTernary(_("Hardware Added"),_("Configure"),
				    _("Ignore"), _("Do Nothing"),
				    _("The following hardware has been added to "
				      "your system:\n        %s\n\n"
				      "You can choose to:\n\n"
				      "1) Configure the device.\n"
				      "2) Ignore the device. No configuration will "
				      "be added, but you will not be prompted if "
				      "the device is detected on subsequent reboots.\n"
				      "3) Do nothing. No configuration will be "
				      "added, and the device will show up as new if "
				      "it is detected on subsequent reboots."),
				    dev->desc);
		else
		  rc=0;
		switch (rc) {
		 case 0:
		 case 1:
			configure(dev);
			break;
		 case 2:
			break;
		 case 3:
		 default:
			y=0;
			while (currentDevs[y]) {
				if (currentDevs[y]==dev) {
					for (z=y;z<numCurrent;z++)
					  currentDevs[z]=currentDevs[z+1];
					numCurrent--;
					break;
				}
				y++;
			}
			break;
		}
	}
}

int main(int argc, char **argv) 
{
	char *confFile;
	char *debugFile=NULL;
	int runFirst=0;
	int ret;
	int rc;
	int x;
	poptContext context;
	struct device **oldDevs, **newDevs;
	struct poptOption options[] = {
		POPT_AUTOHELP
		{ "quiet", 'q', POPT_ARG_NONE, &quiet, 0,
		  _("do configuration that doesn't require user input"), 
		  NULL
		},
		{ "safe", 's', POPT_ARG_NONE, &safe, 0,
		  _("do only 'safe' probes that won't disturb hardware"),
		  NULL
		},
		{ "timeout", 't', POPT_ARG_INT, &timeout, 0,
		  _("set timeout in seconds"), NULL
		},
		{ "file", 'f', POPT_ARG_STRING, &debugFile, 0,
			_("read probed hardware from a file"),
			_("file to read hardware info from")
		},
		{ 0, 0, 0, 0, 0, 0 }
	};
	
	context = poptGetContext("kudzu", argc, argv, options, 0);
	while ((rc = poptGetNextOpt(context)) > 0) {
	}
	if (( rc < -1)) {
		fprintf(stderr, "%s: %s\n",
			poptBadOption(context, POPT_BADOPTION_NOALIAS),
			poptStrerror(rc));
		exit(-1);
	}
	
	if (!(confFile=checkConfFile())) {
		runFirst=1;
	}
	
	initializeDeviceList(BUS_UNSPEC);
	if (runFirst) {
		storedDevs = malloc(sizeof(struct device *));
		storedDevs[0] = NULL;
			       
	} else {
		storedDevs = readDevices(confFile);
		if (!storedDevs) {
			storedDevs = malloc(sizeof(struct device *));
			storedDevs[0] = NULL;
		}
	}
	while (storedDevs[numStored]) numStored++;
	if (debugFile)
	  currentDevs = readDevices(debugFile);
	else {
		if (safe)
		  currentDevs = probeDevices(CLASS_UNSPEC, BUS_UNSPEC, (PROBE_ALL|PROBE_SAFE));
		else
		  currentDevs = probeDevices(CLASS_UNSPEC, BUS_UNSPEC, PROBE_ALL);
	}
	while (currentDevs[numCurrent]) numCurrent++;
	ret = listCompare(storedDevs, currentDevs, &oldDevs, &newDevs);
	if (!ret) {
		writeDevices(confFile,currentDevs);
		exit(0);
	} else {
		/* List-ify oldDevs, newDevs */
		if (oldDevs[0]) {
			oldDevs[0]->next=NULL;
			for (x=1;oldDevs[x];x++)
				oldDevs[x-1]->next = oldDevs[x];
			oldDevs[x-1]->next = NULL;
		}
		if (newDevs[0]) {
			newDevs[0]->next = NULL;
			for (x=1;newDevs[x];x++) 
				newDevs[x-1]->next = newDevs[x];
			newDevs[x-1]->next = NULL;
		}
	        configMenu((*oldDevs),(*newDevs),runFirst);
	}
	if (!runFirst)
	  writeDevices(confFile,currentDevs);
	else
	  writeDevices("/etc/sysconfig/hwconf",currentDevs);
	newtFinished();
	return 0;
}
