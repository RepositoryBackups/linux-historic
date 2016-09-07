/* this file has 'high-level' operations which only use the partition # */
/* For example, on a disk the first 4 partitions are the primary partitions */
/* and are numbered 1 thru 4. Then each subsequent logical paritition   */
/* is numbered sequentially starting with 5.                            */
/* The functions in this file use this partition number as the index of */
/* the operation.                                                       */
/* Recoded to use new HardDrive struct with single partition table      */


#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include "libfdisk.h"
#include "sunlabel.h"
#include "bsdlabel.h"

static int fdiskWritePartitions ( HardDrive *hd) ;
static int fdiskWriteSunPartitions( HardDrive *hd );

/* not sure where this guy should go, we'll leave it here for now */
/* make raw partition entry for the given partition               */
/* partition should have the start, size, type, active fields set */
/* rest are taken care of                                         */
int fdiskMakeRawEntry( HardDrive *hd, Partition *pt, RawPartition *p ) {
    unsigned int start, size, end;
    unsigned int s_cyl, s_hd, s_sec;
    unsigned int e_cyl, e_hd, e_sec;
    unsigned int cur, low, hi, act;

    /* read in the current values for the various parameters */
    fdiskGetConstraint( &pt->start,  &p->start,  &low, &hi, &act );
    fdiskGetConstraint( &pt->size,   &p->size,   &low, &hi, &act );
    fdiskGetConstraint( &pt->type,   &cur,       &low, &hi, &act );
    p->type   = cur;
    fdiskGetConstraint( &pt->active, &cur,       &low, &hi, &act );
    p->active = cur;
    
    /* convert from sector to cyl/hd/sector */
    start = p->start;
    size  = p->size;
    end   = start + size - 1;
    
    fdiskSectorToCHS( hd, start, &s_cyl, &s_hd, &s_sec );
    fdiskSectorToCHS( hd, end  , &e_cyl, &e_hd, &e_sec );
    
    /* we have to store CHS in funky way */
    /* handle cylinder limits on PC partitions correctly */
    s_cyl = (s_cyl > 1023) ? 1023 : s_cyl;
    p->start_cyl  = s_cyl & 0xff;
    p->start_head = s_hd;
    p->start_sec  = s_sec | ((s_cyl >> 2) &0xc0);

    e_cyl = (e_cyl > 1023) ? 1023 : e_cyl;
    p->end_cyl    = e_cyl & 0xff;
    p->end_head   = e_hd;
    p->end_sec    = e_sec | ((e_cyl >> 2) &0xc0);
    
    return FDISK_SUCCESS;
}



/* remove partition # n from drive hd. If it doesn't exist then a return */
/* code > 0 is returned, else 0 is returned                              */
/* First primary partition is number 1, not 0!                           */
int fdiskRemovePartition( HardDrive *hd, unsigned int n ) {
    
    if (n < 1)
	return FDISK_ERR_BADNUM;
    
    if (n <= hd->limits.maxPrimary) {
	return fdiskRemovePrimary( hd, n );
    } else {
	return fdiskRemoveLogical( hd, n );
    }
}

/*                                                                         */
/* not currently used, unclear what numbering scheme to use to index       */
/* partitions we wish to create beyond the first 4.  Logical partition     */
/* numbers are sequential, so only 'free' number to create a new partition */
/* at would be the number of the last logical plus one. Seems kludgey to   */
/* require user to figure out that number just to call the function.       */
/* Instead I will have a fdiskCreatePrimary() and fdiskAppendLogical()     */
/*                                                                         */
#if 0
/* create partition number n, where n can be 1-4 for primary,   */
/* or 5 or greater to append a logical partition (this is       */
/* because of screwy way logicals are numbered )                */ 
/* DO NOT USE THIS FUNCTION YET - UNCLEAR IF IT ACTS LIKE WE WANT!  */
/* DOES NOT MAKE EXTENDED PARTITIONS IF THEY ARE NEEDED FOR LOGICAL */
int fdiskCreatePartition( HardDrive *hd, unsigned int n,
			  unsigned int start, unsigned int size) {

    if (n < 1 )
	return FDISK_ERR_BADNUM;
    
    if (n <= hd->limits.maxPrimary) {
	return fdiskCreatePrimary(hd, n, start, size);
    } else {
	/* append a logical partition */
	return fdiskAppendLogical( hd, start, size );
    }
}
#endif


/* Set    partition #n into the current partition table */
/* DOES NOT create an extended partition if necessary   */
/* does not do any sanity checking on attributes        */
int fdiskSetAttrPartition( HardDrive *hd, unsigned int n, Partition *p ) {
    
    if (n < 1)
	return FDISK_ERR_BADNUM;
    
    if (n <= hd->limits.maxPrimary)
	return fdiskSetAttrPrimary(hd, n, p);
    else
	return fdiskSetAttrLogical(hd, n, p);
}


int fdiskGetAttrPartition( HardDrive *hd, unsigned int n, Partition **p ) {
    
    if (n < 1)
	return FDISK_ERR_BADNUM;
    
    if (n <= hd->limits.maxPrimary)
	return fdiskGetAttrPrimary(hd, n, p);
    else
	return fdiskGetAttrLogical(hd, n, p);
}

static void initializeHardDrive(HardDrive * hd) {
    int i;

    /* set all primaries as available and all extended as unavailable */
    for (i=1; i <= hd->limits.maxPartitions; i++) {
	if (i < (hd->limits.maxPrimary + 1))
	    hd->table[i].status = AVAILABLE;
	else
	    hd->table[i].status = UNAVAILABLE;

	/* set all eptable entries as UNAVAILABLE */
	hd->eptable[i].status = UNAVAILABLE;
    }
}

int fdiskInitSunLabel( HardDrive * hd) {
    struct sun_disklabel label;
    unsigned short * sp;

    memset(&label, 0, sizeof(label));

    label.magic = SUN_LABEL_MAGIC;
    label.rspeed = 5400;
    label.nacyl = 2;
    label.sparecyl = 0;
    label.ilfact = 1;
    label.pcylcount = hd->geom.cylinders;
    hd->geom.cylinders -= label.nacyl;
    label.ncyl = hd->geom.cylinders;
    label.ntrks = hd->geom.heads;
    label.nsect = hd->geom.sectors;

    label.partitions[2].num_sectors = label.ncyl * label.ntrks * label.nsect;
    label.partitions[2].start_cylinder = 0;
    label.infos[2].id = 5;

    strcpy (label.info, "DiskDruid created partition table");

    hd->totalsectors = hd->geom.sectors*hd->geom.heads*hd->geom.cylinders;

    label.csum = 0;
    for (sp = (unsigned short *) &label; sp < &label.csum; sp++)
	label.csum ^= *sp;

    if (lseek(hd->fd, 0, SEEK_SET)) return -1;
    if (write(hd->fd, &label, sizeof(label)) != sizeof(label)) return -1;

    return 0;
}

static int fdiskWriteSunPartitions( HardDrive *hd ) {
    struct sun_disklabel label;
    int i;
    Partition * p;
    unsigned short * sp;

    if (lseek(hd->fd, 0, SEEK_SET)) return -1;
    if (read(hd->fd, &label, sizeof(label)) != sizeof(label)) return -1;
    if (label.magic != SUN_LABEL_MAGIC) return FDISK_ERR_BADMAGIC;

    for (i = 0; i < 8; i++) {
	if (fdiskGetAttrPartition( hd, i + 1, &p ) == FDISK_SUCCESS) {
	    label.partitions[i].num_sectors = p->size.current;
	    label.partitions[i].start_cylinder = p->start.current /
			(hd->geom.sectors * hd->geom.heads);
            /* How's this for a hack? */
            if (p->type.current == 0xfd && !label.partitions[i].start_cylinder)
		label.partitions[i].start_cylinder++;
	    label.infos[i].id = p->type.current;
	} else {
	    label.partitions[i].start_cylinder = 0;
	    /* create the wholedisk partition if it does not exist. */
	    if (i == 2) {
		label.partitions[i].num_sectors = hd->totalsectors;
		label.infos[i].id = 5;
	    } else {
		label.partitions[i].num_sectors = 0;
		label.infos[i].id = 0;
	    }
	}
    }

    label.csum = 0;
    for (sp = (unsigned short *) &label; sp < &label.csum; sp++)
	label.csum ^= *sp;

    if (lseek(hd->fd, 0, SEEK_SET)) return -1;
    if (write(hd->fd, &label, sizeof(label)) != sizeof(label)) return -1;

    return fdiskReReadPartitions(hd);
}

static int fdiskReadSunPartitions( HardDrive *hd ) {
    struct sun_disklabel label;
    int s, i;
    unsigned short *ush, csum;
    Partition p;

    if (lseek(hd->fd, 0, SEEK_SET)) return -1;
    if (read(hd->fd, &label, sizeof(label)) != sizeof(label)) return -1;
    if (label.magic != SUN_LABEL_MAGIC) return FDISK_ERR_BADMAGIC;
    ush = ((unsigned short *) (&label + 1)) - 1;
    for (csum = 0; ush >= (unsigned short *) &label;) csum ^= *ush--;
    /*  Checksum != 0, checksum fails */
    if (csum) return FDISK_ERR_BADMAGIC;

    hd->limits.maxPrimary = 8;
    hd->limits.maxPartitions = 8;
    hd->limits.skipPartition = 3;
    initializeHardDrive(hd);

    hd->geom.cylinders = label.ncyl;
    hd->totalsectors = hd->geom.cylinders * hd->geom.heads * hd->geom.sectors;
    hd->part_type = FDISK_PART_TYPE_SUN;
    hd->write_f = fdiskWriteSunPartitions;

    if (!label.partitions[2].num_sectors || !label.infos[2].id) {
	/* If whole disk partition is missing and we can create it, let's do it now */
	label.partitions[2].num_sectors = label.ncyl * label.ntrks * label.nsect;
	label.partitions[2].start_cylinder = 0;
	label.infos[2].id = 5;
    }

    for (i = 0; i < 8; i++) {
	if (label.partitions[i].num_sectors && label.infos[i].id) {
	    if ((s=fdiskCreatePrimary( hd, i + 1)) != 0)
		return s;

	    fdiskSetFixedConstraint(&p.size,    
				    label.partitions[i].num_sectors);
	    fdiskSetFixedConstraint(&p.start,   
		    label.partitions[i].start_cylinder * hd->geom.sectors * hd->geom.heads);
	
	    fdiskSetFixedConstraint(&p.offset,   0);
	    fdiskSetFixedConstraint(&p.type,    label.infos[i].id);
	    fdiskSetFixedConstraint(&p.active,  0		       );
	    fdiskSetFixedConstraint(&p.offset,  0                      );
	    fdiskDeactivateAllDriveSet( &p.drive );
	    fdiskActivateDriveSet( &p.drive, hd->num );
	    fdiskSetCurrentDriveSet( &p.drive, hd->num );
	    fdiskSetFixedConstraint( &p.num, i + 1);
	    
	    /* now to make sure NOTHING can happen to this partition */
	    p.immutable = 1;
	    p.status    = ALLOCATED;

	    /* store in the partition table */
	    fdiskSetAttrPartition( hd, i+ 1, &p );
	}
    }
    
    return 0;
}

static unsigned short xbsd_dkcksum (struct bsd_disklabel *lp) {
  unsigned short *start, *end;
  unsigned short sum = 0;
  
  lp->d_checksum = 0;
  start = (u_short *)lp;
  end = (u_short *)&lp->d_partitions[lp->d_npartitions];
  while (start < end)
    sum ^= *start++;
  return (sum);
}

void alpha_bootblock_checksum (char *boot) {
  u_int64_t *dp, sum;
  int i;
   
  dp = (u_int64_t *)boot;
  sum = 0;
  for (i = 0; i < 63; i++)
    sum += dp[i];
  dp[63] = sum;
}

/* from util-linux's bsd label code */
int fdiskInitLabel( HardDrive * hd) {
    struct bsd_disklabel label;
    char boot[512];

    memset(&label, 0, sizeof(label));

    label.d_magic = BSD_DISKMAGIC;
    label.d_type = BSD_DTYPE_SCSI;
    label.d_flags = 0;
    label.d_secsize = SECTORSIZE;       /* bytes/sector  */
    label.d_nsectors = hd->geom.sectors; /* sectors/track */
    label.d_ntracks = hd->geom.heads;    /* tracks/cylinder (heads) */
    label.d_ncylinders = hd->geom.cylinders;
    label.d_secpercyl  = hd->geom.sectors * hd->geom.heads; /* sectors/cylinder */
    label.d_secperunit = label.d_secpercyl * label.d_ncylinders;

    label.d_rpm = 3600;
    label.d_interleave = 1;
    label.d_trackskew = 0;
    label.d_cylskew = 0;
    label.d_headswitch = 0;
    label.d_trkseek = 0;
    
    label.d_magic2 = BSD_DISKMAGIC;
    label.d_bbsize = BSD_BBSIZE;
    label.d_sbsize = BSD_SBSIZE;

    label.d_npartitions = 0;
    label.d_checksum = xbsd_dkcksum (&label);

    if (lseek(hd->fd, BSD_LABEL_OFFSET, SEEK_SET) < 0) return -1;
    if (write(hd->fd, &label, sizeof(label)) != sizeof(label)) return -1;

    if (lseek(hd->fd, 0, SEEK_SET) < 0) return -1;
    if (read(hd->fd, &boot, sizeof(boot)) != sizeof(boot)) return -1;
    alpha_bootblock_checksum(boot);
    if (lseek(hd->fd, 0, SEEK_SET) < 0) return -1;
    if (write(hd->fd, &boot, sizeof(boot)) != sizeof(boot)) return -1;

    return 0;
}

static int fdiskWriteLabelPartitions( HardDrive *hd ) {
    struct bsd_disklabel label;
    int i, s;
    Partition * p;
    char boot[512];
    int maxPartition = 0;

    if (lseek(hd->fd, BSD_LABEL_OFFSET, SEEK_SET) < 0) return -1;
    if (read(hd->fd, &label, sizeof(label)) != sizeof(label)) return -1;
    if (label.d_magic != BSD_DISKMAGIC) return FDISK_ERR_BADMAGIC;

    for (i = 0; i < 8; i++) {
	if (fdiskGetAttrPartition( hd, i + 1, &p ) == FDISK_SUCCESS) {
	    label.d_partitions[i].p_size = p->size.current;
	    label.d_partitions[i].p_offset = p->start.current;
	    /* How's this for a hack? */
	    /* changed to LINUX_SWAP_PARTIITON, we need to make this at least 2 - msw*/
	    if (p->type.current == LINUX_SWAP_PARTITION && label.d_partitions[i].p_offset < 2)
		label.d_partitions[i].p_offset = 2;

	    switch (p->type.current) {
	      case LINUX_SWAP_PARTITION: s = 1; break;
	      case LINUX_NATIVE_PARTITION: s = 8; break;
	      default: s = p->type.current; break;
	    }

	    label.d_partitions[i].p_fstype = s;
	    maxPartition = i;
	} else {
	    label.d_partitions[i].p_size = 0;
	    label.d_partitions[i].p_offset = 0;
	    label.d_partitions[i].p_fstype = 0;
	}
    }

    label.d_npartitions = maxPartition + 1;
    label.d_checksum = xbsd_dkcksum (&label);

    if (lseek(hd->fd, BSD_LABEL_OFFSET, SEEK_SET) < 0) return -1;
    if (write(hd->fd, &label, sizeof(label)) != sizeof(label)) return -1;

    if (lseek(hd->fd, 0, SEEK_SET) < 0) return -1;
    if (read(hd->fd, &boot, sizeof(boot)) != sizeof(boot)) return -1;
    alpha_bootblock_checksum(boot);
    if (lseek(hd->fd, 0, SEEK_SET) < 0) return -1;
    if (write(hd->fd, &boot, sizeof(boot)) != sizeof(boot)) return -1;

    return fdiskReReadPartitions(hd);
}

static int fdiskReadLabelPartitions( HardDrive *hd ) {
    struct bsd_disklabel label;
    int i, s;
    Partition p;

    if (lseek(hd->fd, BSD_LABEL_OFFSET, SEEK_SET) < 0) return -1;
    if (read(hd->fd, &label, sizeof(label)) != sizeof(label)) return -1;
    if (label.d_magic != BSD_DISKMAGIC) return FDISK_ERR_BADMAGIC;

    hd->limits.maxPrimary = 8;
    hd->limits.maxPartitions = 8;
    hd->limits.skipPartition = -1;
    initializeHardDrive(hd);

    hd->totalsectors = hd->geom.cylinders * hd->geom.heads * hd->geom.sectors;
    hd->part_type = FDISK_PART_TYPE_BSD;
    hd->write_f = fdiskWriteLabelPartitions;

    if (label.d_npartitions > 8)
	label.d_npartitions = 8;
    
    for (i = 0; i < label.d_npartitions; i++) {
	if (label.d_partitions[i].p_size && label.d_partitions[i].p_fstype) {
	    if ((s=fdiskCreatePrimary( hd, i + 1)) != 0)
		return s;

	    fdiskSetFixedConstraint(&p.size,    
				    label.d_partitions[i].p_size);
	    fdiskSetFixedConstraint(&p.start,   
		    label.d_partitions[i].p_offset);
	
	    fdiskSetFixedConstraint(&p.offset,   0);

	    switch (label.d_partitions[i].p_fstype) {
	      case 1: s = LINUX_SWAP_PARTITION; break;
	      case 8: s = LINUX_NATIVE_PARTITION; break;
	      default: s = label.d_partitions[i].p_fstype; break;
	    }

	    fdiskSetFixedConstraint(&p.type,    s		       );

	    fdiskSetFixedConstraint(&p.active,  0		       );
	    fdiskSetFixedConstraint(&p.offset,  0                      );
	    fdiskDeactivateAllDriveSet( &p.drive );
	    fdiskActivateDriveSet( &p.drive, hd->num );
	    fdiskSetCurrentDriveSet( &p.drive, hd->num );
	    fdiskSetFixedConstraint( &p.num, i + 1);
	    
	    /* now to make sure NOTHING can happen to this partition */
	    p.immutable = 1;
	    p.status    = ALLOCATED;

	    /* store in the partition table */
	    fdiskSetAttrPartition( hd, i+ 1, &p );
	}
    }

    return 0;
}

/* give a hard drive hd, read in the partition data        */
/* COMPLETELY trashes all partition info in the hard drive */
int fdiskReadPartitions( HardDrive *hd ) {
    int i, s;
    unsigned int exttype;
    RawPartitionTable *pt;
    Partition         p, ept, *pti;
    
    s = fdiskReadPartitionTable(hd, 0L, &pt );
    if (s == FDISK_ERR_BADMAGIC) {
	s = fdiskReadSunPartitions(hd);
	if (s == FDISK_ERR_BADMAGIC)
	    s = fdiskReadLabelPartitions(hd);

	return s;
    } else if (s) {
	return s;
    }

    hd->limits.maxPrimary = 4;
    if (strstr (hd->name, "rd/"))
	/* DAC960 has a limit of 7 partitions per drive. */
	hd->limits.maxPartitions = 7;
    else
	hd->limits.maxPartitions = 16;      
    hd->limits.skipPartition = -1;

    initializeHardDrive(hd);
    hd->part_type = FDISK_PART_TYPE_DOS;
    hd->write_f = fdiskWritePartitions;

    /* move data from raw partition table into abstract data type */
    /* primary partitions store absolute sector offsets/sizes already */
    for (i=1; i <= hd->limits.maxPrimary; i++) {
	if ((s=fdiskCreatePrimary( hd, i )) != 0)
	    return s;

	/* get initial attr of the partition */
	fdiskGetAttrPartition( hd, i, &pti );
	memcpy(&p, pti, sizeof(Partition));
	free(pti);

	/* if the partition is defined                                */
	/* have to set this up so that we don't try to mangle this    */
	/* partition later when we are automatically allocating space */
	/* for newly created partitions                               */
	/* remember the rawPT index starts at 0, PT starts at 1       */
	if (pt->entry[i-1].size && pt->entry[i-1].type) {
	    fdiskSetFixedConstraint( &p.size,    pt->entry[i-1].size    );
	    fdiskSetFixedConstraint( &p.start,   pt->entry[i-1].start   );
	    fdiskSetFixedConstraint( &p.type,    pt->entry[i-1].type    );
	    fdiskSetFixedConstraint( &p.active,  pt->entry[i-1].active  );
	    fdiskSetFixedConstraint( &p.offset,  0                      );
	    fdiskDeactivateAllDriveSet( &p.drive );
	    fdiskActivateDriveSet( &p.drive, hd->num );
	    fdiskSetCurrentDriveSet( &p.drive, hd->num );
	    fdiskSetFixedConstraint( &p.num, i );
	    
	    /* now to make sure NOTHING can happen to this partition */
	    p.immutable = 1;
	    p.status    = ALLOCATED;
	} else {
	    p.status = AVAILABLE;
	}

	/* store in the partition table */
	fdiskSetAttrPartition( hd, i, &p );
    }

    /* now pursue extended partition */
    /* we do not handle more than one extended partition per drive */
    /* read in extended partition(s) if they exist */
    hd->pep = 0;
    for (i=1; i<=hd->limits.maxPrimary; i++) {
	exttype = hd->table[i].type.current;
	if (fdiskIsExtended(exttype)) {
	    
	    RawPartitionTable *extended;
	    unsigned int ext_start;
	    unsigned int ext_size;
	    unsigned int cur_start;
	    unsigned int cur_size;
	    unsigned int lp;
	    int sawext, sawlog;
	    int j;

	    /* if we also have seen the PEP, we have trouble */
	    if (hd->pep)
		return FDISK_ERR_TWOEXT;

	    /* mark which primary hold the PEP */
	    hd->pep = i;

	    /* start/size of the PEP */
	    ext_start  = hd->table[i].start.current;
	    ext_size   = hd->table[i].size.current;

	    /* start/size of the EP within the next LP must fall */
	    cur_start = ext_start;
	    cur_size  = ext_size;
	    
	    /* follow linked list of extended partitions              */
	    /* watch out for more than one logical partition          */
	    /* per EPT. This is bad and we will die and they will cry */
	    /* fdisk and cfdisk cant handle this either. You should   */
	    /* be using your OS's fdisk if this is the case, cause    */
	    /* it is insane and only it can understand the madness    */
	    while (1) {
		if ((s=fdiskReadPartitionTable(hd, cur_start, &extended)))
		    return s;

                /* see if this is a empty partition table or not   */
		/* can be empty if someone creates an extended     */
		/* partition which contains no logical partitions  */
		sawlog = 0;
		for (j=1; j<=hd->limits.maxPrimary; j++) {
		    RawPartition        *raw;

		    raw = &extended->entry[j-1];
		    if (!raw->size || !raw->type)
			continue;

		    sawlog = 1;
		}

		if (!sawlog)
		    break;

		/* insert this extended partition into the partition table */
		/* we also insert a logical partition as well              */
		/* since there is only 1 LP per EPT/EP, we store the type  */
		/* in the main partition table as the type of the LP.      */
		/* We also store information on the EP in the eptable[].   */
		/* Later when we write the entire                          */
		/* partition table out to disk, we'll remember that there  */
		/* is also an extended partition as well.                  */
		/*                                                         */
		/* The number assigned to the logical partition created is */
		/* passed back in the variable lp                          */
		if ((s=fdiskAppendLogical( hd, &lp )) != 0)
		    return s;

		/* start from scratch */
		fdiskGetAttrPartition( hd, lp, &pti );
		memcpy(&p, pti, sizeof(Partition));
		free(pti);
		fdiskGetAttrExtended( hd, lp, &pti );
		memcpy(&ept, pti, sizeof(Partition));
		free(pti);

		/* store the size/start of the EP                          */
		/* we DO NOT set end because we are interested in          */
		/* restricted the position on disk to the CURRENT position */
		/* endcyl is used if we are trying to restrict the         */
		/* placement of a NEW partition to a region of disk, like  */
		/* the first 1024 cylinders. Storing the start sector isnt */
		/* useful in that case since the mapping from sector->cyl  */
		/* depends upon the geom of the drive, and we may be       */
		/* considering several different drives of differring geom */
		/* for the placement of the NEW partition.                 */
		fdiskSetFixedConstraint( &ept.size,    cur_size   );
		fdiskSetFixedConstraint( &ept.start,   cur_start  );
		fdiskDeactivateAllDriveSet( &ept.drive );
		fdiskActivateDriveSet( &ept.drive, hd->num );
		fdiskSetCurrentDriveSet( &ept.drive, hd->num );
		fdiskSetFixedConstraint( &ept.num,     lp         );
		fdiskSetFixedConstraint( &ept.type,    exttype    );
	    
		/* now to make sure NOTHING can happen to this partition */
		ept.immutable = 1;
		ept.status    = ALLOCATED;

		/* setup the extended partition which corresponds to the */
		/* logical partition we are going to setup next          */
		fdiskSetAttrExtended( hd, lp, &ept );
		
		/* ok, now figure out what logical partitions are */
		/* in this extended partition */
		sawlog = 0;
		sawext = 0;
		for (j=1; j<=hd->limits.maxPrimary; j++) {
		    RawPartition        *raw;

		    raw = &extended->entry[j-1];

		    /* skip link to next in extended partition chain for now */
		    /* we put it in the logical partitions for future ref */
		    if (fdiskIsExtended(raw->type)) {
			if (sawext)
			    return FDISK_ERR_TWOEXT;
			sawext = j;
			exttype = raw->type;
			continue;
		    }

		    /* make sure the log partition exists */
		    if (!raw->size || !raw->type)
			continue;

		    if (sawlog)
			return FDISK_ERR_TWOLOG;
		    else
			sawlog = 1;
		    
		    /* make sure that numbers make sense */
		    /* we test that:                     */
		    /*  - the current partition isnt bigger than */
		    /*    the extended partition its in          */
		    /*  - start isnt before the start of the "primary" */
		    /*    extended partition                           */
		    /*  - end isnt past end of "primary" extended partition */
		    if (((raw->start+raw->size) > (cur_start+cur_size)) ||
			((cur_start+raw->start) < ext_start) ||
			((cur_start+raw->start+raw->size) >
			 (ext_start+ext_size)))
			return FDISK_ERR_CORRUPT;

		    fdiskSetFixedConstraint( &p.size,   raw->size   );
		    fdiskSetFixedConstraint( &p.start,  raw->start+cur_start );
		    fdiskDeactivateAllDriveSet( &p.drive );
		    fdiskActivateDriveSet( &p.drive, hd->num );
		    fdiskSetCurrentDriveSet( &p.drive, hd->num );
		    fdiskSetFixedConstraint( &p.num,    lp         );
		    fdiskSetFixedConstraint( &p.type,   raw->type    );
		    fdiskSetFixedConstraint( &p.active, raw->active  );

		    /* we currently set ALL offsets to 0       */
		    /* the start parameter stores the absolute */
		    /* starting position of the LP             */
		    /* get the offset by comparing start of LP */
		    /* to that of the EP who EPT it is in      */
		    fdiskSetFixedConstraint( &p.offset, 0 );

		    /* now to make sure NOTHING can happen to this partition */
		    p.immutable = 1;
		    p.status    = ALLOCATED;
		    
		    fdiskSetAttrPartition( hd, lp, &p );
		}

		/* see if we have another extended partition to follow */
		if (!sawext)
		    break;
		else {
		    cur_start  = extended->entry[sawext-1].start + ext_start;
		    cur_size   = extended->entry[sawext-1].size;
		}
	    }
	}
    }
    
    return FDISK_SUCCESS;
}

/* give a hard drive hd, write the partition data */
static int fdiskWritePartitions ( HardDrive *hd ) {
    int i;
    int n;
    int error=0;
    unsigned int ext_start=0, ext_size, cur_start=0, cur_size;
    unsigned int next_start;
    unsigned int low, hi, act;
    unsigned int lpart;
    RawPartitionTable rpt;
    RawPartition *p;
    Partition *pt, *ept;
    
    /* move data to raw partition table from the abstract data type  */
    /* first we handle the primary partitions                        */
    /* we write them NO MATTER what they hold, since they MUST exist */
    /* in the MBR of the hard drive.                                 */
    memset(&rpt, 0, sizeof(RawPartitionTable));
    for (i=1; i <= hd->limits.maxPrimary ; i++) {
	p = &rpt.entry[i-1];
	if (fdiskGetAttrPartition( hd, i, &pt ) == FDISK_SUCCESS) {
	    fdiskMakeRawEntry( hd, pt, p );
	    free(pt);
	} else {
	    memset( p, 0, sizeof(RawPartition) );
	}
    }

    /* Set first primary partition active if no primary partitions
       are marked as active. */
    n = 0;
    for (i=0; i < hd->limits.maxPrimary ; i++) {
	if (rpt.entry[i].active)
	    n = 1;
    }
    
    if (!n)
	rpt.entry[0].active = 0x80;

    
    /* write the primary partition */
    fdiskWritePartitionTable(hd, 0, &rpt);
    
    /* now pursue extended partition                               */
    /* we do not handle more than one extended partition per drive */
    /* we do not handle more than one logical partition per EP     */
    /* we do not handle a tree of EP, just a EP chain              */
    if (hd->pep) {
	fdiskGetAttrPartition( hd, hd->pep, &pt );
	fdiskGetConstraint( &pt->start, &ext_start, &low, &hi, &act );
	fdiskGetConstraint( &pt->size,  &ext_size, &low, &hi, &act );
	free(pt);
	
	cur_start = ext_start;
	cur_size  = ext_size;
    }

    /* now we loop over all logical partitions */
    if (fdiskLastPartition( hd, &lpart ) != FDISK_SUCCESS)
	lpart = 0;
    
    for (n=hd->limits.maxPrimary + 1; n <= lpart; n++) {
	/* start with a clean table */
	memset(&rpt, 0, sizeof(RawPartitionTable));

	/* move data to raw partition table from the abstract data type */
	/* the raw partition table can hold up to 4 entries, just like  */
	/* the one in the MBR. However, we will have at most 2 entries. */
	/* If there is an EP following the current in the chain, it will*/
	/* have an entry. And there will ALWAYS be an entry for the     */
	/* current LP.                                                  */
	/*                                                              */
	/* stick the logical partition first, then the extended         */
	p = &rpt.entry[0];

	/* now do the logical partition */
	fdiskGetAttrPartition( hd, n, &pt );
	fdiskMakeRawEntry( hd, pt, p );
	free(pt);

	/* HACK - we have to translate start sector   */
	/*        relative to the start of the current*/
	/*        extended partition                  */
	/*                                            */
	/* NOTE - CHS appears to NOT be translated    */
	/*        Not sure why this is...             */
	/* HUH? - This code appears to do *nothing*,  */
	/*	  but I'm scared to remove it until I */
	/*	  have time to actually look over it  */
	/*	  (ewt)				      */
	p->start  -= cur_start;
	p++;
	    
	/* see if there is an extended partition following this one in */
	/* the EPT chain.                                              */
	if (fdiskGetAttrExtended( hd, n+1, &ept ) == FDISK_SUCCESS) {
	    fdiskMakeRawEntry( hd, ept, p);
	    free(ept);

	    /* HACK - we have to translate start sector   */
	    /*        to a value relative to the start of */
	    /*        the PEP. We use absolute values     */
	    /*        up until we write to disk (like now)*/
	    /*                                            */
	    /* NOTE - CHS appears to NOT be translated    */
	    /*        Not sure why this is...             */
	    next_start = p->start; /* save for later use */
	    p->start  -= ext_start;

	    /* move pointer to next entry in partition table */
	} else
	    next_start = 0;

	/* write the partition */
	fdiskWritePartitionTable(hd, cur_start, &rpt);

	/* point to next partition table, if it exists */
	if (next_start)
	    cur_start = next_start;
    }

    /* now sync disk and re-read the partition table */
    sync();
    if ((i = fdiskReReadPartitions( hd ))<0) {
	error = errno;
    } else {
	/* some kernel versions (1.2.x) seem to have trouble
	   rereading the partition table, but if asked to do it
	   twice, the second time works. - biro@yggdrasil.com */

	/* FIXME: is this really necessary -- ewt */
	sync();
	if ((i=fdiskReReadPartitions( hd ))<0)
	    error = errno;
    }
    
    if (i < 0) {
	close(hd->fd);
	
	sync();
    }
    
    return i;
}

/* return value of first partition, starting with primaries and working */
/* down thru logical  partitions                                        */
/* returns FDISK_ERR_BADNUM if NO partitions exists                     */
int fdiskFirstPartition( HardDrive *hd, unsigned int *first ) {
    if (fdiskFirstPrimary( hd, first ) == 0 )
	return FDISK_SUCCESS;

    if (fdiskFirstLogical( hd, first ) == 0 )
	return FDISK_SUCCESS;

    return FDISK_ERR_BADNUM;
}

/* return value of Last partition, starting with logical and working    */
/* down thru primary partitions                                         */
/* returns FDISK_ERR_BADNUM if NO partitions exists                     */
int fdiskLastPartition( HardDrive *hd, unsigned int *last ) {
    if (fdiskLastLogical( hd, last ) == 0 )
	return FDISK_SUCCESS;

    if (fdiskLastPrimary( hd, last ) == 0 )
	return FDISK_SUCCESS;

    *last = 0;
    return FDISK_ERR_BADNUM;
}

/* return pointer to partition #n if it exists            */
/* returns null pointer and FDISK_ERR_BADNUM if it doesnt */
int fdiskFindPartition( HardDrive *hd, unsigned int n, Partition **p ) {
    if (n <= hd->limits.maxPrimary)
	return fdiskFindPrimary( hd, n, p );
    else
	return fdiskFindLogical( hd, n, p );
}	
    
#ifdef NEED_WALK_ROUTINES
/* these routines walk through the list of partitions on a hard drive */
/* Point to 'first' partition on drive                                */
/* returns FDISK_ERR_BADNUM if no partitions exist                    */
/* also sets key to NULL                                              */
int fdiskWalkReset( HardDrive *hd, WalkKey **key ) {
    unsigned int first;
    
    /* find first partition and initialize the key used to walk list */
    if (fdiskFirstPartition( hd, &first ) != 0)
	return FDISK_ERR_BADNUM;

    *key = (WalkKey *) malloc( sizeof(WalkKey) );
    *key->value = first;
}

/* return partition information for where key is currently pointing */
/* and increment key to next position                               */
/* if key is NULL then we're at end of list                         */
int fdiskWalkNext( HardDrive *hd, WalkKey **key, Partition **p ) {

    Partition **q;
    int s, next, last;

    /* find current partition */
    s = fdiskFindPartition( hd, *key->value, p );
    if (s != 0) {
	free(*key);
	*key = NULL;
	return s;
    }

    /* point to next */
    fdiskLastPartition( hd, &last );
    next = *key->value+1;

    for (; next <= last; next++)
	s = fdiskFindPartition( hd, next, q );
	if (!s)
	    break;
	
    /* we've reached the end */
    if (next >= last) {
	free(*key);
	*key = NULL;
    }

    return FDISK_SUCCESS;
}

#endif


