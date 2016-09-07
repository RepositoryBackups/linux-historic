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

#include "keyboard.h"

#ifdef __sparc__
#include <sys/ioctl.h>
#include <asm/kbio.h>
#include <asm/openpromio.h>
#include <linux/serial.h>
#endif

static void keyboardFreeDevice(struct keyboardDevice *dev)
{
	freeDevice((struct device *) dev);
}

static void keyboardWriteDevice(FILE *file, struct keyboardDevice *dev)
{
	writeDevice(file, (struct device *)dev);
}

static int keyboardCompareDevice(struct keyboardDevice *dev1, struct keyboardDevice *dev2)
{
	return compareDevice( (struct device *)dev1, (struct device *)dev2);
}

struct keyboardDevice *keyboardNewDevice(struct keyboardDevice *old)
{
	struct keyboardDevice *ret;

	ret = malloc(sizeof(struct keyboardDevice));
	memset(ret, '\0', sizeof(struct keyboardDevice));
	ret = (struct keyboardDevice *) newDevice((struct device *) old, (struct device *) ret);
	ret->bus = BUS_KEYBOARD;
	ret->newDevice = keyboardNewDevice;
	ret->freeDevice = keyboardFreeDevice;
	ret->writeDevice = keyboardWriteDevice;
	ret->compareDevice = keyboardCompareDevice;
	return ret;
}

struct device *keyboardProbe(enum deviceClass probeClass, int probeFlags,
			struct device *devlist)
{
#ifdef __sparc__
	int fd;
	char buf[256];
	struct keyboardDevice *kbddev;
	
	if (
	    (probeClass == CLASS_UNSPEC) ||
	    (probeClass == CLASS_KEYBOARD) 
	    ) {
		static struct {
		  int layout0, layout1, layout2;
		  char *symbols;
		  int use_iso9995_3;
		} sunkbd_translate[] = {
		  { 0, 33, 80, "",      0, },
		  { 1, 34, 81, "",      0, },
		  { 2, -1, -1, "fr_BE", 0, },
		  { 3, -1, -1, "ca",    0, },
		  { 4, 36, 83, "dk",    1, },
		  { 5, 37, 84, "de",    1, },
		  { 6, 38, 85, "it",    1, },
		  { 7, 39, 86, "nl",    0, },
		  { 8, 40, 87, "no",    1, },
		  { 9, 41, 88, "pt",    1, },
		  { 10, 42, 89, "es",   1, },
		  { 11, 43, 90, "se",   1, },
		  { 12, 44, 91, "fr_CH",1, },
		  { 13, 45, 92, "de_CH",1, },
		  { 14, 46, 93, "gb",   1, },
		  { 16, 47, 94, "ko",   0, },
		  { 17, 48, 95, "tw",   0, },
		  { 32, 49, 96, "jp",   0, },
		  { 50, 97, -1, "fr_CA",0, },
		  { 51, -1, -1, "hu",   0, },
		  { 52, -1, -1, "pl",   0, },
		  { 53, -1, -1, "cs",   0, },
		  { 54, -1, -1, "ru",   0, },
		  { -1, -1, -1, NULL,   0, }
		};

		char twelve = 12;
		int fdstd = 0;
		char buf[4096];

		for (fd = 0; fd <= 2; fd++) {
		    sprintf (buf, "/proc/self/fd/%d", fd);
		    if (readlink (buf, buf, 4096) == 12 &&
			!strncmp (buf, "/dev/console", 12)) {
			fdstd = 1;
			break;
		    }
		}
		if (!fdstd) {
		    fd = open("/dev/console", O_RDWR);
		    if (fd < 0) return devlist;
		}
		
		kbddev=keyboardNewDevice(NULL);
		kbddev->driver=strdup("ignore");
		kbddev->class=CLASS_KEYBOARD;
		if (devlist)
			kbddev->next = devlist;
		devlist = (struct device *) kbddev;

		if (ioctl (fd, TIOCLINUX, &twelve) < 0) {
			/* Serial console */
			char desc[64];
			struct serial_struct si;
			int fd, line = 0;

			if (ioctl (fd, TIOCGSERIAL, &si) >= 0) {
			    if (si.line & 1)
				/* ttyb */
				line = 1;
			}
			if (!fdstd) close(fd);
			sprintf (desc, "Serial console tty%c", line + 'a');
			fd = open("/dev/openprom", O_RDONLY);
			if (fd >= 0) {
				struct openpromio *op = (struct openpromio *)buf;
				sprintf (op->oprom_array, "tty%c-mode", line + 'a');
				op->oprom_size = 4096-128-4;
				if (ioctl (fd, OPROMGETOPT, op) >= 0 &&
				    op->oprom_size > 0 && op->oprom_size < 40) {
					strcat (desc, " ");
					op->oprom_array [op->oprom_size] = 0;
					strcat (desc, op->oprom_array);
				}
				close (fd);
			}
			kbddev->desc=strdup(desc);
			kbddev->device=strdup("console");
			return devlist;
		}

		if (!fdstd) close(fd);
		fd=open("/dev/kbd", O_RDWR);
		if (fd < 0) {
			/* PS/2 keyboard */
			kbddev->desc=strdup("Generic PS/2 Keyboard");
		} else {
			/* Sun keyboard */
			int kbdtype, kbdlayout, i;
			char *desc, *desclayout = NULL;

			kbddev->device=strdup("kbd");
			desc = "Sun Type4 ";
			if (ioctl(fd, KIOCTYPE, &kbdtype) >= 0)
				switch (kbdtype) {
				case 2: desc = "Sun Type2 "; break;
				case 3: desc = "Sun Type3 "; break;
				case 4: ioctl(fd, KIOCLAYOUT, &kbdlayout);
					if (kbdlayout < 33)
						desc = "Sun Type4 ";
					else switch (kbdlayout) {
					case 33: case 47: case 48: case 49:
					case 80: case 94: case 95: case 96:
						desc = "Sun Type5 "; break;
					case 34: case 81:
						desc = "Sun Type5 Unix "; break;
					default:
						desc = "Sun Type5 Euro "; break;
					}
					for (i = 0; sunkbd_translate[i].layout0 != -1; i++) {
						if (sunkbd_translate[i].layout0 == kbdlayout ||
						    sunkbd_translate[i].layout1 == kbdlayout ||
						    sunkbd_translate[i].layout2 == kbdlayout)
						break;
					}
					if (sunkbd_translate[i].layout0 != -1 &&
					    sunkbd_translate[i].symbols[0])
						desclayout = sunkbd_translate[i].symbols;
					break;
				}
			if (desclayout) {
				kbddev->desc = malloc(strlen(desc) + strlen(desclayout) + 1 + strlen("Keyboard") + 1);
				strcpy (kbddev->desc, desc);
				strcat (kbddev->desc, desclayout);
				strcat (kbddev->desc, " Keyboard");
			} else {
				kbddev->desc = malloc(strlen(desc) + strlen("Keyboard") + 1);
				strcpy (kbddev->desc, desc);
				strcat (kbddev->desc, "Keyboard");
			}
		}
		close (fd);
	}
#endif
	return devlist;
}
