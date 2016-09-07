#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "libfdisk.h"

#include "fstab.h"
#include "fsedit.h"
#include "newtfsedit.h"
#include <sys/utsname.h>

#include "isys.h"

/* this isn't const, as we have a loop which does i18n conversion on these */
struct parttypes allparttypes[] = {
    {0, "Empty"},
    {SUNPARTTYPE|1, "SunOS boot"},
    {SUNPARTTYPE|2, "SunOS root"},
    {SUNPARTTYPE|3, "SunOS swap"},
    {SUNPARTTYPE|4, "SunOS usr"},
    {SUNPARTTYPE|5, "Whole disk"},
    {SUNPARTTYPE|6, "SunOS stand"},
    {SUNPARTTYPE|7, "SunOS var"},
    {SUNPARTTYPE|8, "SunOS home"},
    {NONSUNPARTTYPE|1, "DOS 12-bit FAT"},
    {NONSUNPARTTYPE|2, "XENIX root"},
    {NONSUNPARTTYPE|3, "XENIX usr"},
    {NONSUNPARTTYPE|4, "DOS 16-bit <32M"},
    {NONSUNPARTTYPE|5, "Extended"},
    {NONSUNPARTTYPE|6, "DOS 16-bit >=32M"},
    {NONSUNPARTTYPE|7, "OS/2 HPFS"},               /* or QNX? */
    {NONSUNPARTTYPE|8, "AIX"},
    {9, "AIX bootable"},
    {10, "OS/2 Boot Manager"},
    {0xb, "Win95 FAT32"},
    {0xc, "Win95 FAT32"},
    {0xe, "Win95 FAT16"},
    {0xf, "Win95 Ext'd"},
    {0x12, "Compaq Setup"},
    {0x40, "Venix 80286"},
    {0x51, "Novell?"},
    {0x52, "Microport"},            /* or CPM? */
    {0x63, "GNU HURD"},             /* or System V/386? */
    {0x64, "Novell Netware 286"},
    {0x65, "Novell Netware 386"},
    {0x75, "PC/IX"},
    {0x80, "Old MINIX"},            /* Minix 1.4a and earlier */

    {0x81, "Linux/MINIX"}, /* Minix 1.4b and later */
    {0x82, "Linux swap"},
    {0x83, "Linux native"},
    {0x84, "OS/2 hidden C:"},
    {0x85, "Linux Extended"},
    {0xfd, "Linux RAID"},

    {0x93, "Amoeba"},
    {0x94, "Amoeba BBT"},           /* (bad block table) */
    {0xa5, "BSD/386"},
    {0xb7, "BSDI fs"},
    {0xb8, "BSDI swap"},
    {0xc7, "Syrinx"},
    {0xdb, "CP/M"},                 /* or Concurrent DOS? */
    {0xe1, "DOS access"},
    {0xe3, "DOS R/O"},
    {0xf2, "DOS secondary"},
    {0xff, "BBT"}                   /* (bad track table) */
};

struct repartitionInfo {
    HardDrive ** hdarr;
    int numDrives;
};

static int addNewPartition(HardDrive ** hdarr, int numhd, PartitionSpec * spec, 
		           char * where, int megs, int grow, int bootable, 
		           unsigned char type, int startCyl,
			   char * device, int partNum,
			   int maxSize);

int nparttypes = sizeof (allparttypes) / sizeof (struct parttypes);

static int HandleFdiskError( int status, char *errbody, char *y, char *n );
static int ErrorDialog(char *title, char *errbody, char *errmsg,
		       char *yesmsg, char *nomsg);
int ReadDrives( char **drives, int numdrives,
		HardDrive **hdarr, unsigned int *numhd,
		int forcezero, int readOnly, int testing, int autoClearMbr,
		int ignoreFailedDrives);
void setupGlobalContext(FseditContext * context);

char * nstrdup(const char * foo) {
    return foo ? strdup(foo) : NULL;
}

static struct attemptedPartition normPartitioning[] = {
#if defined(__i386__) || defined(__sparc__)
	{ "/boot",	16,	LINUX_NATIVE_PARTITION,	0, -1 },
#elif defined(__alpha__)
	{ "/dos",	2,	DOS_PRIMARY_lt32MEG_PARTITION,	0, 1 },
#endif
	{ "/",		400,	LINUX_NATIVE_PARTITION,	1, -1 },
	{ "Swap-auto",	64,	LINUX_SWAP_PARTITION,	0, -1 },
	{ NULL, 0, 0, 0 }
};

static struct attemptedPartition servPartitioning[] = {
#if defined(__i386__) || defined(__sparc__)
        { "/boot",      16,     LINUX_NATIVE_PARTITION, 0, -1 },
#elif defined(__alpha__)
        { "/dos",       2,      DOS_PRIMARY_lt32MEG_PARTITION,  0, 1 },
#endif
        { "/",          256,    LINUX_NATIVE_PARTITION, 0, -1 },
        { "/usr",       512,    LINUX_NATIVE_PARTITION, 1, -1 },
        { "/var",       256,    LINUX_NATIVE_PARTITION, 0, -1 },
        { "/home",      512,    LINUX_NATIVE_PARTITION, 1, -1 },
        { "Swap-auto",  64,     LINUX_SWAP_PARTITION,   0, -1 },
        { NULL, 0, 0, 0 } 
};

struct attemptedPartition *normalPartitioning = normPartitioning;
struct attemptedPartition *serverPartitioning = servPartitioning;



/* XXXX - temporary debugging function */
void print_partitionspec( PartitionSpec *spec ) {
    unsigned int j;
    unsigned int drive;
    Partition   *p;

    if (!spec) {
	    printf ("Partition spec undefined.\n");
	    return;
    }
    
    printf("Requested partitions: \n\n");
    printf("Mount Point  Num    Drive            Size          Endcyl     Weight   Status\n");
    printf("-----------  ---  ----------    --------------   ----------   ------  ---------\n");

    for (j=0; j<spec->num; j++) {
	p = &spec->entry[j].partition;
	printf("%11s  ", spec->entry[j].name);

	printf("%2d   ",p->num.current);
	
	if (fdiskDriveSetIsActive(&p->drive)) {
	    fdiskInitWalkDriveSet(&p->drive,&drive);
	    printf("%2d", drive);
	    do {
		printf(",%2d", drive);
	    } while (fdiskWalkDriveSet(&p->drive,&drive)==FDISK_SUCCESS);
	} else
	    printf("Any Drive  ");

	if (p->size.active) {
	    printf("%8d", p->size.min);
	    if (p->size.max != p->size.min)
		printf("+");
	    else
		printf(" ");
	    printf("/%8d", p->size.current);
	}else
	    printf("       Not Active      ");

	if (p->endcyl.active)
	    printf("  <= %5d  ", p->endcyl.max);
	else
	    printf("  Not Active");

	printf("   %5d    ", fdiskGetConstraintPriority( p ));

	if (spec->entry[j].status == REQUEST_PENDING)
	    printf("PENDING");
	else if (spec->entry[j].status == REQUEST_DENIED)
	    printf("DENIED");
	else if (spec->entry[j].status == REQUEST_GRANTED)
	    printf("GRANTED");
	else if (spec->entry[j].status == REQUEST_ORIGINAL)
	    printf("ORIGINAL");
	else
	    printf("UNKNOWN");

	printf("\n");
    }
}

void print_user_partition_table(HardDrive *hd) {
    int i;
    Partition *up;
    unsigned int first, last, end;
    unsigned int s_cyl, s_hd, s_sec, e_cyl, e_hd, e_sec;

    
    printf("Nr AF     Start C/H/S     End C/H/S      Start      Size    ID\n");

    /* print primary partitions first */
    fdiskFirstPartition( hd, &first );
    fdiskLastPartition( hd, &last );
    for (i=first; i<=last; i++) {
        up = &hd->table[i];
        if (!(up->size.current) || !(up->type.current))
            continue;

        fdiskSectorToCHS( hd, up->start.current, &s_cyl, &s_hd, &s_sec );
        end = up->start.current + up->size.current - 1;
        fdiskSectorToCHS( hd, end, &e_cyl, &e_hd, &e_sec );
        printf("%2d %2x   [%4d/%3d/%3d]  [%4d/%3d/%3d] %7d   %7d    %2x\n",
               up->num.current,
               up->active.current,
               s_cyl, s_hd, s_sec,
               e_cyl, e_hd, e_sec,
               up->start.current,
               up->size.current,
               up->type.current);
    }

}

int validLoopbackSetup(PartitionSpec * spec) {
    int i;

    if (fdiskIndexPartitionSpec(spec, "/", &i))
	return 0;

    if (spec->entry[i].status == REQUEST_DENIED)
	return 0;

    if ((spec->entry[i].partition.type.current != 
		DOS_PRIMARY_gt32MEG_PARTITION) &&
	(spec->entry[i].partition.type.current != 
		WIN_VFAT32_PARTITION) &&
	(spec->entry[i].partition.type.current != 
		WIN_VFAT32_LBA_PARTITION))
	return 0;

    return 1;
}

int guessAtPartitioning(FseditContext *context, int flags, char *name,
			struct attemptedPartition * goals,
			int interactive) {
    int i, rc;
    int numhd;
    HardDrive *tmphdarr[MAX_HARDDRIVES];
    HardDrive *hdarr[MAX_HARDDRIVES];
    PartitionSpec *spec;

    /*struct newtColors colors = newtDefaultColorPalette;*/

    /* make local copies because we're going to do massive modification */
    numhd = context->numhd;
    for (i=0; i<numhd; i++) {
	hdarr[i] = (HardDrive *) alloca(sizeof(HardDrive));
	memcpy(hdarr[i], context->hdarr[i], sizeof(HardDrive));

	tmphdarr[i] = (HardDrive *) alloca(sizeof(HardDrive));
	memcpy(tmphdarr[i], context->hdarr[i], sizeof(HardDrive));
    }
    
    MergeFstabEntries( context->hdarr, context->numhd,
		       &context->spec, &context->fstab );

    spec = (PartitionSpec *) alloca(sizeof(PartitionSpec));
    memset(spec, 0, sizeof(PartitionSpec));
    spec->num = context->spec.num;
    for (i=0; i<spec->num; i++) {
	spec->entry[i].name = strdup(context->spec.entry[i].name);
	memcpy(&spec->entry[i].partition, 
	       &context->spec.entry[i].partition, 
	       sizeof(Partition));
	spec->entry[i].status = context->spec.entry[i].status;
    }

    /* check flags are remove requested partition classes from */
    /* existing partition spec and original partitions         */
    /* (original partitions are those which were on the drive  */
    /* before we started editting the partition tables)        */
    if (flags & (FSEDIT_CLEARLINUX | FSEDIT_CLEARALL)) {
	rc = -1;
	
	if (flags & FSEDIT_CLEARLINUX) {
	    deletePartitionClass(hdarr, numhd, spec, FSEDIT_CLEARLINUX);
	} else {
	    deletePartitionClass(hdarr, numhd, spec, 0);
	}
    }

    rc = tryGoal(hdarr, tmphdarr, numhd, spec, goals);
    if (rc) {
	/* release memory */ 
	fdiskWipePartitionSpec(spec);
	return INST_CANCEL;
    }

    for (i=0; i<numhd; i++) {
	*context->hdarr[i] = *tmphdarr[i];
	*context->newhdarr[i] = *tmphdarr[i];
    }
    
    /* erase old partition spec and replace with new */
    fdiskWipePartitionSpec (&context->spec);
    
    context->spec.num = spec->num;
    memcpy(context->spec.entry, spec->entry, 
	   MAX_PARTITION_SPEC*sizeof(PartitionSpecEntry));

    exitFsedit (context, NULL);

    return 0;
}

/* clean up that string */
void TrimWhitespace( char *s ) {
    char *f, *l, *p, *q;

    if (!(*s))
        return;
    
    for (f=s; *f && isspace(*f); f++) ;

    if (!*f) {
        *s = '\0';
        return;
    }
    
    for (l=f+strlen(f)-1; isspace(*l) ; l--)
    *l = '\0';
        
    q = s, p = f;
    while (*p)
        *q++ = *p++;
    
    *q = '\0';
}

/* see if anything really changed */
int DisksChanged( HardDrive **oldhd, HardDrive **newhd, unsigned int numhd ) {
    int i, j;
    
    /* see if partition tables are identical */
    for (i=0; i<numhd; i++)
	for (j=0; j<MAX_PARTITIONS; j++) {
	    if (memcmp(&oldhd[i]->table[j],&newhd[i]->table[j],
		       sizeof(Partition)))
		return 1;
	    if (memcmp(&oldhd[i]->eptable[j],&newhd[i]->eptable[j],
		       sizeof(Partition)))
		return 1;
	}

    return 0;
}
		
/* delete a partition spec */
int doDeletePartitionSpec( HardDrive **hdarr, unsigned int numhd,
			 PartitionSpec *spec, PartitionSpecEntry *entry){
    Partition *p;
    int      status;
    unsigned int c, l, m, n, t;
    char     *tmpstr;

    p = &entry->partition;

    tmpstr=strdup(entry->name);
    
    if (p->immutable) {
	fdiskGetCurrentConstraint(&p->num, &c);
	fdiskGetCurrentConstraint(&p->type, &t);
	fdiskGetCurrentDriveSet(&p->drive, &l);
	for (m=0; m<numhd; m++)
	    if (hdarr[m]->num == l)
		break;
	
	fdiskRemovePartition(hdarr[m], c);

	/* make it so we can delete this partition now */
	p->immutable = 0;
	fdiskModifyPartitionSpec( spec, tmpstr, p, REQUEST_PENDING );

	/* ok, see if this was the last immutable logical partition */
	/* in an immutable primary extended partition               */
	/* we pray that fdiskCleanOriginal... will get rid of the   */
	/* spec entry for the pep                                   */
	if (c > 4) {
	    if (fdiskLastLogical( hdarr[m], &n ) != FDISK_SUCCESS) {
		/* all logicals are gone, blow away pep */
		if (hdarr[m]->pep && hdarr[m]->table[hdarr[m]->pep].immutable){
		    fdiskRemovePartition(hdarr[m], hdarr[m]->pep);
		}
	    }
	}
    }
    
    status = fdiskDeletePartitionSpec( spec, tmpstr );
    fdiskHandleSpecialPartitions( hdarr, numhd, spec );
    free(tmpstr);

    return FDISK_SUCCESS;
}

/* converts a PartionSpec to an equivalent struct fstab */
/* Creates fstab from scratch                           */
int PartitionSpecToFstab( HardDrive **hdarr, int numhd,
			  PartitionSpec *spec, struct fstab *fstab ) {

    int i, j;
    struct fstabEntry entry;
    
    fstab->entries = malloc(sizeof(*fstab->entries) * spec->num);
    fstab->numEntries = 0;
    for (i = 0; i < spec->num; i++) {
	if (!spec->entry[i].name) continue;
	
	if (spec->entry[i].status != REQUEST_ORIGINAL &&
	    spec->entry[i].status != REQUEST_GRANTED)
	    continue;
	
	/* FIXME: hack, hack, hack */
	if (*spec->entry[i].name != '/' &&
	    *spec->entry[i].name != 'S') continue;
	
	for (j=0; j<numhd; j++)
	    if (hdarr[j]->num == spec->entry[i].partition.drive.current)
		break;
	
	if (j == numhd)
	    continue;

	initFstabEntry(&entry);
	entry.mntpoint = strdup(spec->entry[i].name);
	entry.size = spec->entry[i].partition.size.current / 2;

	entry.device = malloc(6);
	sprintf(entry.device, "%s%d",
		hdarr[j]->prefix, spec->entry[i].partition.num.current);
	
	switch (spec->entry[i].partition.type.current) {
	  case LINUX_NATIVE_PARTITION:
	    entry.type = PART_EXT2;
	    entry.tagName = "Linux native";
	    break;
	    
	  case LINUX_SWAP_PARTITION:
	    entry.type = PART_SWAP;
	    entry.tagName = "Linux swap";
	    break;
	    
	  case DOS_PRIMARY_lt32MEG_PARTITION:
	  case DOS_PRIMARY_gt32MEG_PARTITION:
	  case WIN_VFAT32_PARTITION:
	  case WIN_VFAT32_LBA_PARTITION:

	    if (hdarr[j]->part_type != FDISK_PART_TYPE_SUN) {
	        entry.type = PART_DOS;
	        entry.tagName = "DOS 16-bit >=32";
	        break;
	    }
	    
	  default:
	    entry.type = PART_OTHER;
	    entry.tagName = "Other";
	    break;
	}

	addFstabEntry(fstab, entry);
    }
    
    fstabSort(fstab);
    return 0;
}


/* merges mount point info from existing struct fstab into a PartionSpec     */
/* PartitionSpec should already exist and be primed with existing partitions */
/* Note - remote fs ARE NOT stored in the PartitionSpec                      */
int MergeFstabEntries( HardDrive **hdarr, int numhd,
			      PartitionSpec *spec, struct fstab *fstab ) {

    int i, j, k;
    char device[15];
    
    for (j = 0; j < spec->num; j++) {
	for (k=0; k<numhd; k++)
	    if (hdarr[k]->num == spec->entry[j].partition.drive.current)
		break;
	
	if (k == numhd)
	    continue;

	sprintf(device, "%s%d",
		hdarr[k]->prefix, spec->entry[j].partition.num.current);
	
	for (i = 0; i < fstab->numEntries; i++)
	    if (!strcmp(fstab->entries[i].device, device))
		break;

	if ( i == fstab->numEntries )
	    continue;

	/* we found a matching entry in the PartitionSpec */
	/* see if the old fstab file had any info we need */
	/* to use (like mount point, etc)                 */
	if (spec->entry[j].name)
	    free(spec->entry[j].name);

	spec->entry[j].name = strdup(fstab->entries[i].mntpoint);
    }

    return 0;
}

/* suck out just the remote fs entries from an fstab */
/* pretty much CopyFstab, with filter on type        */
struct fstab copyRemoteFSFstab(struct fstab * fstab) {
    struct fstab newfstab;
    int i, j;

    if (!fstab->numEntries) {
        newfstab.numEntries = 0;
        newfstab.entries = malloc(1);
        return newfstab;
    }

    /* duplicate the current fstab */
    newfstab.numEntries = fstab->numEntries;
    newfstab.entries = malloc(fstab->numEntries * sizeof(struct fstabEntry));
    for (i = j = 0; i < newfstab.numEntries; i++) {
	if (fstab->entries[i].type != PART_NFS)
	    continue;
	
        if (fstab->entries[i].mntpoint) {
            newfstab.entries[j] = fstab->entries[i];
            newfstab.entries[j].mntpoint=nstrdup(fstab->entries[i].mntpoint);
            newfstab.entries[j].device = nstrdup(fstab->entries[i].device);
            newfstab.entries[j].netPath = nstrdup(fstab->entries[i].netPath);
            newfstab.entries[j].netHost = nstrdup(fstab->entries[i].netHost);
            j++;
        }
    }

    newfstab.numEntries = j;

    /* return the memory we don't actually need */
    newfstab.entries=realloc(newfstab.entries, j * sizeof(struct fstabEntry));

    return newfstab;
}


/* suck out just the remote fs entries from an fstab */
/* pretty much CopyFstab, with filter on type        */
void MergeRemoteFSFstab(struct fstab *oldfstab, struct fstab *newfstab) {
    int i, j;

    /* copy remote fs entries */
    for (i = 0; i < oldfstab->numEntries; i++) {
	if (oldfstab->entries[i].type != PART_NFS)
	    continue;
	
        if (oldfstab->entries[i].mntpoint) {
	    j = newfstab->numEntries;
	    newfstab->entries = realloc(newfstab->entries,
				     (j+1)*sizeof(struct fstabEntry));
	    newfstab->entries[j] = oldfstab->entries[i];
            newfstab->entries[j].mntpoint=nstrdup(oldfstab->entries[i].mntpoint);
            newfstab->entries[j].device=nstrdup(oldfstab->entries[i].device);
            newfstab->entries[j].netPath=nstrdup(oldfstab->entries[i].netPath);
            newfstab->entries[j].netHost=nstrdup(oldfstab->entries[i].netHost);
	    newfstab->numEntries = j+1;
        }
    }
}

int deletePartitionClass(HardDrive ** hd, int numhd, 
				PartitionSpec * spec, int justLinux) {
    int deleteit;
    int type;
    int i;

    i = 0;
    while (i < spec->num) {
	type = spec->entry[i].partition.type.current;

	deleteit = !justLinux;
	if (justLinux && (type == LINUX_SWAP_PARTITION || 
			  type == LINUX_NATIVE_PARTITION ||
			  type == LINUX_RAID_PARTITION))
	    deleteit = 1;

#ifdef __sparc__
	/* never ever delete the wholedisk partition */
	if (type == 5 && spec->entry[i].partition.num.current == 3)
	    deleteit = 0;
#endif
	
	if (deleteit) {
	    doDeletePartitionSpec(hd, numhd, spec, &spec->entry[i]);
	    fdiskCleanOriginalSpecs( hd, numhd, spec );
	    i=0; /* restart cause entries changed */
	    continue;
	} else {
	    i++;
	}
    }

    return 0;
}

static int addNewPartition(HardDrive ** hdarr, int numhd, PartitionSpec * spec, 
		           char * where, int megs, int grow, int bootable, 
		           unsigned char type, int startCyl,
			   char * device, int partNum,
			   int maxSize) {
    Partition template;
    int i;

    /* create a template partitionspec to send to editpartition */
    memset(&template, 0, sizeof(Partition));
    fdiskSetFixedConstraint(&template.type, type);

    if (bootable) {
	fdiskDeactivateAllDriveSet( &template.drive );
	fdiskActivateDriveSet(&template.drive, 1);
	fdiskActivateDriveSet(&template.drive, 2);
	fdiskSetConstraint(&template.endcyl, 0, 0, 1023, 1);
    } else {
	fdiskActivateAllDriveSet( &template.drive );
    }

    if (device) {
	fdiskDeactivateAllDriveSet( &template.drive );
	
	for (i=0; i<numhd; i++)
	    if (!strcmp(hdarr[i]->prefix, device)) break;
	if (i == numhd) return -1;

	fdiskActivateDriveSet(&template.drive, i + 1);
    }

    if (partNum != -1) {
	fdiskSetFixedConstraint(&template.num, partNum);
    }

/*  For now we'll make this alpha only code  */
#ifdef __alpha__

    if (startCyl != -1) {
	/* XXX HACK */
	/* Add a little buffer in case we need to skip a head */
	fdiskSetConstraint(&template.start, 0, startCyl, startCyl + 100, 1);
    }

#endif

    if (maxSize == -1)
	maxSize = grow ? FDISK_SIZE_MAX : SECPERMEG * megs;
    else
	maxSize = SECPERMEG * maxSize;

    fdiskSetConstraint(&template.size, SECPERMEG * megs, SECPERMEG * megs,
			maxSize, 1);

    /* insert with a name we know to mean its a new partition */
    fdiskInsertPartitionSpec(spec, where, &template, 
				  REQUEST_PENDING);
    
    return 0;
}

int tryGoal(HardDrive ** hdarr, HardDrive ** tmphdarr, int numhd, 
		   PartitionSpec * spec, struct attemptedPartition * goals) {
    int i;

    for (i = 0; goals[i].mount; i++)
	if (addNewPartition(hdarr, numhd, spec, goals[i].mount, goals[i].size, 
			goals[i].grow, !strcmp(goals[i].mount, "/boot"),
			goals[i].type, goals[i].start, goals[i].device,
			goals[i].partNum, goals[i].maxSize))
	    return 1;

    /* insert the new partition spec */
    fdiskAutoInsertPartitions(hdarr, numhd, tmphdarr, spec );
    fdiskGrowPartitions(hdarr, numhd, tmphdarr, spec);

    for (i = 0; i < spec->num; i++)
	if (spec->entry[i].status == REQUEST_DENIED)
	    break;

    return (i < spec->num);
}

static char ** createDevices(char ** drives, int numDrives) {
    int i, rc;
    char ** deviceList;

    deviceList = malloc(numDrives * sizeof(char *));

    for (i = 0; i < numDrives; i++) {
	deviceList[i] = malloc(15);

	strcpy(deviceList[i], "/dev/");
	strcat(deviceList[i], drives[i]);
	if (access(deviceList[i], R_OK)) {
	    strcpy(deviceList[i], "/tmp/");
	    strcat(deviceList[i], drives[i]);
	    if ((rc = devMakeInode(drives[i], deviceList[i]))) return NULL;
	}
    }

    return deviceList;
}

#if 0 /* not used */
static void removeDevices(char ** deviceList, int numDrives) {
    int i;

    for (i = 0; i < numDrives; i++) {
	unlink(deviceList[i]);
    	free(deviceList[i]);
    }

    free(deviceList);
}
#endif

/* check a mount point to make sure its valid */
/* returns non-zero if bad mount point        */
int badMountPoint(unsigned int type, char * item) {
    char * chptr = item;

    if (!strcmp(item, "/dev") ||
        !strcmp(item, "/bin") ||
        !strcmp(item, "/sbin") ||
        !strcmp(item, "/etc") ||
        !strcmp(item, "/lib")) {
        ddruidShowMessage(_("Bad Mount Point"), _("Ok"),
                    _("The %s directory must be on the root filesystem."),
		    item);
	return 1;
    }

    if (*chptr != '/') {
        ddruidShowMessage(_("Bad Mount Point"), _("Ok"),
                    _("The mount point %s is illegal.\n\n"
		    "Mount points must begin with a leading /."), item);
        return 1;
    } 

    if (*(chptr + 1) && *(chptr + strlen(chptr) - 1) == '/') {
        ddruidShowMessage(_("Bad Mount Point"), _("Ok"),
                    _("The mount point %s is illegal.\n\n"
                      "Mount points may not end with a /."), item);
        return 1;
    } 

    while (*chptr && isprint(*chptr)) chptr++;

    if (*chptr) {
        ddruidShowMessage(_("Bad Mount Point"), _("Ok"),
                    _("The mount point %s is illegal.\n\n"
                    "Mount points may only printable characters."), item);
        return 1;
    }

    /* This means the user wants to do that odd loopback-root thingy */
    if (!strcmp(item, "/") && (type == DOS_PRIMARY_gt32MEG_PARTITION ||
			       type == WIN_VFAT32_PARTITION ||
			       type == WIN_VFAT32_LBA_PARTITION)) {
	if (ddruidYesNo("FAT-hosted install", _("Yes"), _("No"),
	    _("You've asked to put your root (/) filesystem on a DOS-style "
	      "FAT partition. You can do this, but you may not use any "
	      "other filesystems for your Linux system. Additionally, there "
	      "will be a speed penalty for not using Linux-native partitions."
	      " Do you want to continue?"), 0))
	    return 0;
	else
	    return 1;
    }

    if (type != LINUX_NATIVE_PARTITION && (
         !strncmp(item, "/var", 4) ||
         !strncmp(item, "/tmp", 4) ||
         !strncmp(item, "/boot", 4) ||
         !strcmp(item, "/") ||
         !strncmp(item, "/root", 4))) {
        ddruidShowMessage(_("Bad Mount Point"), _("Ok"),
                    _("The mount point %s is illegal.\n\n"
                      "System partitions must be on Linux Native "
                      "partitions."), item);
        return 1;
    }

    if (type != LINUX_NATIVE_PARTITION &&
	type != NFS_REMOTE_PARTITION &&
        !strncmp(item, "/usr", 4)) {
        ddruidShowMessage(_("Bad Mount Point"), _("Ok"),
                    _("The mount point %s is illegal.\n\n"
                      "/usr must be on a Linux Native partition "
                      "or an NFS volume."), item);
        return 1;
    }

    return 0;
}

int newFseditContext (char ** drives,
		      struct fstab * fstab,
		      int flags,
		      FseditContext ** new)
{
    FseditContext * context;
    char ** deviceList;
    int numDrives;
    struct fstabEntry entry;
    int i, rc;

    *new = (FseditContext *) malloc (sizeof (FseditContext));
    context = *new;
    memset(context, 0, sizeof (FseditContext));

    numDrives = 0;
    for (i = 0; drives[i]; i++, numDrives++);

    if (numDrives >= MAX_HARDDRIVES) {
	ddruidShowMessage(_("Too Many Drives"), _("Ok"),
		  _("You have more drives than this program supports. "
		    "Please use the standard fdisk program to setup your "
		    "drives and please notify Red Hat Software that you "
		    "saw this message."));
	return INST_ERROR;
    }

    deviceList = createDevices(drives, numDrives);

    memset(context->hdarr, 0, MAX_HARDDRIVES * sizeof(HardDrive *));
    rc = ReadDrives(deviceList, numDrives, context->hdarr, &context->numhd, 0, 
		    flags & FSEDIT_READONLY, flags & FSEDIT_TESTING,
		    flags & FSEDIT_AUTOCLEARMBR,
		    flags & FSEDIT_IGNOREFAILEDDRIVES);

    if (context->numhd < 1) {
	ddruidShowMessage(_("No Drives Found"), _("Ok"),
		  _("An error has occurred - no valid devices were found "
		    "on which to create new filesystems.  Please check "
		    "your hardware for the cause of this problem."));
	
	for (i = 0; i < numDrives; i++)
	    unlink(deviceList[i]);
	return INST_ERROR;
    }

    /* copy the fstab we were given */
    for (i = 0; i < fstab->numEntries; i++) {
	memset(&entry, 0, sizeof(entry));
	entry.device = strdup(fstab->entries[i].device);
	entry.mntpoint = strdup(fstab->entries[i].mntpoint);
	addFstabEntry(&context->fstab, entry);
    }

    /* make a prestine copy of the hdarr */
    for (i = 0; i < context->numhd; i++) {
	context->prestinehdarr[i] = (HardDrive *) malloc(sizeof(HardDrive));
	*context->prestinehdarr[i] = *context->hdarr[i];
    }

    /* make backup of hdarr if necessary before user mucked with it */
    /* copy original hard drive configurations into work spaces */
    for (i=0; i < context->numhd; i++) {
	context->newhdarr[i] = (HardDrive *) malloc(sizeof(HardDrive));
	*context->newhdarr[i] = *context->hdarr[i];
    }

    fdiskSetupPartitionSpec(context->hdarr, context->numhd, &context->spec );

    context->flags = flags;
    context->cbi.function = NULL;
    context->cbi.data = NULL;

    context->raid = NULL;
    
    return INST_OKAY;
}

void delFseditContext (FseditContext * context)
{
    int i;
    
    freeFstab(context->fstab);
    
    for (i=0; i < context->numhd; i++)
	fdiskCloseDevice(context->hdarr[i]);

    for (i=0; i < context->numhd; i++)
	if (context->newhdarr[i])
	    free (context->newhdarr[i]);

    for (i=0; i < context->numhd; i++)
	if (context->prestinehdarr[i])
	    free (context->prestinehdarr[i]);

/*      removeDevices(deviceList, numDrives); */
}


void exitFsedit (FseditContext *context, struct raidInstance * raidInfo)
{
    struct repartitionInfo * repartInfo;
    int i, j;

    context->repartInfo = NULL;
    if (DisksChanged(context->prestinehdarr, context->newhdarr, context->numhd)) {
	repartInfo = malloc(sizeof(*repartInfo));
	repartInfo->hdarr = context->newhdarr;
	repartInfo->numDrives = context->numhd;
	context->repartInfo = repartInfo;
    }
    
    /* free up old fstab */
/*      freeFstab(context->fstab); */
    PartitionSpecToFstab(context->newhdarr, context->numhd,
			 &context->spec, &context->fstab);
/*      MergeRemoteFSFstab( &remotefstab, fstab ); */
/*      freeFstab(remotefstab); */

    /* free old raid information, if any */
    if (context->raid) {
	i = 0;
	while (context->raid[i].mntpt) {
	    free(context->raid[i].mntpt);
	    free(context->raid[i].device);
	    j = 0;
	    while (context->raid[i].devices[j]) {
		free(context->raid[i].devices[j]);
		j++;
	    }
	    free(context->raid[i].devices);

	    i++;
	}
    }

    context->raid = raidInfo;
}

void FSEditInitialize(FseditContext * context) {
    setupGlobalContext(context);
}

/* main program */
int FSEditPartitions(FseditContext *context) {
    struct fstab  remotefstab;
    int rc, ourrc = 0;
    static int beenManual = 0;
    int keepChanges = 0;
    static int where = 0;
    int dir;
#ifdef __sparc__
    struct utsname my_utsname;

    if (uname(&my_utsname) == 0 && !strcmp(my_utsname.machine, "sparc64")) {
	fdiskIsSparc64 = 1;
	normalPartitioning = normPartitioning + 1;
	serverPartitioning = servPartitioning + 1;
    }
#endif

    if (where == 0)
	dir = 1;
    else
	dir = -1;

    /* if they dont cancel it will be new fstab                  */
    remotefstab = copyRemoteFSFstab( &context->fstab ); 
    MergeFstabEntries( context->hdarr, context->numhd,
		       &context->spec, &context->fstab );
    
    if (1 || beenManual) {
	/* Goto master screen */
	rc = StartMaster(context,
			 &remotefstab, 
			 context->flags & FSEDIT_READONLY,
			 &keepChanges);
	if (rc == FDISK_ERR_USERABORT)
	    ourrc = INST_CANCEL;
	else if (rc) 
	    ourrc = INST_ERROR;
	else
	    ourrc = 0;
    }

    return ourrc;
}

int FSWritePartitions(char ** drives, void * partList) {
    struct repartitionInfo * repartInfo = partList;
    char ** deviceList;
    int numDrives;
    int rc = 0;
    int i, error = 0;

    numDrives = 0;
    for (i = 0; drives[i]; i++, numDrives++);
    deviceList = createDevices(drives, numDrives);
    if (!deviceList) return -1;

    for (i=0; i<repartInfo->numDrives; i++) {
	if (!error) 
	    if (repartInfo->hdarr[i]->write_f(repartInfo->hdarr[i])) {
		rc = i + 1;
		error = errno;
	    }
	fdiskCloseDeviceFd(repartInfo->hdarr[i]);
    }
    
    errno = error;
    return rc;
}

/* Keep this in sync with balkan.h */
#define BALKAN_PART_DOS		1
#define BALKAN_PART_EXT2	2
#define BALKAN_PART_OTHER	3
#define BALKAN_PART_NTFS	4
#define BALKAN_PART_SWAP	5
#define BALKAN_PART_UFS		6
#define BALKAN_PART_RAID	7

#define UFS_SUPER_MAGIC		0x00011954

partitionList * FSPartitionList(FseditContext *context) {
    int i, hdnum;
    PartitionSpec * spec = &context->spec;
    partitionList * result = malloc(sizeof(partitionList) * (spec->num + 1));
    int num = 0;
    
    for (i=0; i<spec->num; i++) {
	if (spec->entry[i].partition.type.current == DOS_EXTENDED_PARTITION ||
	    spec->entry[i].partition.type.current == WIN98_EXTENDED_PARTITION)
	    continue;

	result[num].name = strdup(spec->entry[i].name);
	result[num].start = spec->entry[i].partition.start.current;
	result[num].size = spec->entry[i].partition.size.current;
	result[num].device = malloc(20);
	hdnum = spec->entry[i].partition.drive.current - 1;
	sprintf (result[num].device, "%s%d", 
		 context->hdarr[hdnum]->prefix,
		 spec->entry[i].partition.num.current);
	switch (spec->entry[i].partition.type.current) {
	case 0x01:
	case 0x04:
	case 0x06:
	case 0x0b:
	case 0x0c:
	case 0x0e:
	case 0x0f:
	    result[num].type = BALKAN_PART_DOS;
	    break;
	case 0x7:
	    result[num].type = BALKAN_PART_NTFS;
	    break;
	case 0x83:
	    result[num].type = BALKAN_PART_EXT2;
	    break;
	case 0x82:
	    result[num].type = BALKAN_PART_SWAP;
	    break;
	case 0xfd:
	    result[num].type = BALKAN_PART_RAID;
	    break;
	default:
	    result[num].type = BALKAN_PART_OTHER;
	    break;
	}
	if (context->hdarr[hdnum]->part_type == FDISK_PART_TYPE_SUN) {
	    if (result[num].type == BALKAN_PART_DOS)
		result[num].type = BALKAN_PART_OTHER;
	    if (spec->entry[i].partition.type.current >= 1 &&
		spec->entry[i].partition.type.current <= 8 &&
		spec->entry[i].partition.type.current != 3 && /* SunOS swap */
		spec->entry[i].partition.type.current != 5) {
		result[num].type = BALKAN_PART_UFS;
	    }
	}
	num++;
    }

    result[num].device = NULL;

    return result;
}

int FSFreeState(void * partList) {
    struct repartitionInfo * repartInfo = partList;
    int i;

    for (i = 0; i < repartInfo->numDrives; i++) {
    	fdiskCloseDevice(repartInfo->hdarr[i]);
    }

    return 0;
}

int onMilo (void)
{
    int ismilo = 0;
    FILE *f;
    
    f = fopen("/proc/cpuinfo", "r");
    if (f) {     
	char buff[1024];
	
	while (fgets (buff, 1024, f) != NULL) {
	    if (strstr (buff, "MILO")) {
		ismilo = 1;
	    }
	}
	fclose(f);
    } else
	return -1;
    
    return ismilo;
}

void initPartitions (HardDrive *hd)
{
#ifdef __sparc__
    fdiskInitSunLabel(hd);
#elif __alpha__
    if (onMilo())
	fdiskZeroMBR(hd);
    else
	fdiskInitLabel(hd);
#else /* i386 */
    fdiskZeroMBR(hd);
#endif
}

/* read in the requested drives                                 */
/* pass an array of names of block devices, returns 0 if ok     */
int ReadDrives( char **drives, int numdrives,
		       HardDrive **hdarr, unsigned int *numhd, int forcezero, 
		       int readOnly, int testing, int autoClearMbr,
		       int ignoreFailedDrives) {

    char errbody[250];
    int  i, done, status;
    char *ptr;
    
    /* loop over all specified block devices */
    *numhd = 0;
    for (i=0; i < numdrives; ) {
	status = fdiskOpenDevice(drives[i], *numhd+1, &hdarr[*numhd]);
	if (status != FDISK_SUCCESS) {
	    snprintf(errbody, sizeof(errbody),
		     _("An error occurred reading the partition table for the "
		       "block device %s.  The error was"), drives[i]);
	    if (!ignoreFailedDrives && 
		    HandleFdiskError( status, errbody, "Retry", "Skip Drive" ))
		continue;
	    else {
		i++;
		continue;
	    }
	} else {
	    done = 0;

	    /* set up drive prefix */
	    if ((ptr = (strstr(hdarr[*numhd]->name, "tmp/"))))
		strcpy(hdarr[*numhd]->prefix, ptr + 4);
	    else if ((ptr = (strstr(hdarr[*numhd]->name, "dev/"))))
		strcpy(hdarr[*numhd]->prefix, ptr + 4);

	    /* for RAID arrays of format c0d0p1 */
	    if (strstr(ptr + 4, "rd/") || strstr(ptr + 4, "ida/"))
		strcat(hdarr[*numhd]->prefix, "p");
	    
	    while (!done) {
		status = fdiskReadPartitions( hdarr[*numhd] );
		if (status != FDISK_SUCCESS) {
		    int rc;
		    
		    if (status == FDISK_ERR_BADMAGIC) {
			if (forcezero) {
			    if (!testing)
				initPartitions(hdarr[*numhd]);
			    fdiskCloseDevice(hdarr[*numhd]);
			    done = 1;
			} else {
			    char text[200];

			    if (autoClearMbr) {
				rc = 1;
			    } else {
				sprintf(text,
				   _("The partition table on device %s is "
				     "corrupted.  To create new partitions "
				     "it must be initialized,"
				     " causing the loss of ALL DATA on "
				     "this drive."), drives[i]+5);
				rc = ddruidYesNo(_("Bad Partition Table"),
					 _("Initialize"), _("Skip Drive"), 
					 text, 0);
			    }
			    
			    if (rc == 1) {
				if (!testing)
				    initPartitions(hdarr[*numhd]);
				fdiskCloseDevice(hdarr[*numhd]);
				done = 1;
			    } else {
				i++;
				fdiskCloseDevice(hdarr[*numhd]);
				done = 1;
			    }
			}
		    } else {			
			snprintf(errbody, sizeof(errbody),
			 _("An error occurred reading the partition table "
			   "for the block device %s.  The error was"),
				 drives[i]+5);
			if (HandleFdiskError(status,errbody,
					     _("Retry"), _("Skip Drive"))){
			    fdiskCloseDevice(hdarr[*numhd]);
			    done = 1;
			} else {
			    i++;
			    fdiskCloseDevice(hdarr[*numhd]);
			    done = 1;
			}
		    }
		/* THIS IS A HORRIBLE NASTY HACK */
#if 0 && defined (__alpha__)
		} else if (hdarr[i]->limits.maxPrimary > 4 && !readOnly) {
		    ddruidShowMessage(_("BSD Disklabel"), _("Ok"), _("A disk with "
				"a BSD disklabel has been found. The Red Hat "
				"installation only supports BSD Disklabels in "
				"read-only mode, so you must use a custom install "
				"and fdisk (instead of Disk Druid) for "
				"machines with BSD Disklabels."));
		    return INST_CANCEL;
#endif
		} else {
		    *numhd += 1;
		    i++;
		    done = 1;
		}
	    }
	}
    }

    return FDISK_SUCCESS;
    
}
		
static int HandleFdiskError( int status, char *errbody, char *y, char *n ) {
    char errmsg[250];
    char yesmsg[]="Yes";
    char nomsg[] ="No";
    
    if (status < 0) {
	if (errno < sys_nerr-1)
	    strncpy(errmsg,sys_errlist[errno],sizeof(errmsg));
	else
	    snprintf(errmsg,sizeof(errmsg), _("System error %d"), errno);
    } else {
	if (status < fdisk_nerr)
	    strcpy(errmsg, fdisk_errlist[status]);
	else
	    snprintf(errmsg,sizeof(errmsg), "libfdisk error %d",errno);
    }

    if (y != NULL && n != NULL)
	return ErrorDialog( _("Fdisk Error"), errbody, errmsg, y, n);
    else
	return ErrorDialog( _("Fdisk Error"), errbody,errmsg,yesmsg,nomsg);
}

/* handles standard fdisk type errors */
/* returns non-zero if user picked "yes" response, and zero if "no" */
static int ErrorDialog(char *title, char *errbody, char *errmsg,
		       char *yesmsg, char *nomsg) {

    char *buf;

    /* I don't know if this is what msf intended, but it is a whole lot 
       easier. */
    buf = alloca(strlen(errbody)+strlen(errmsg) + 10);
    sprintf(buf, "%s: %s", errbody, errmsg);
    return ddruidYesNo(title, yesmsg, nomsg, buf, 0);
}

