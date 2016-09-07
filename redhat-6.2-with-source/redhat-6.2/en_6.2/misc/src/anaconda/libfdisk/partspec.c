/* handles the PartitionSpec type and operations we do on them */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "libfdisk.h"

/* some globals used to keep up with last allocation error (sortof like errno) */
enum allocReason LastAllocStat = ALLOC_UNDEF;

static char *reasons[] = { "Reason Undefined",
	"Allocation Succeeded",
	"Not enough free space",
	"Requested Start",
	"Boot partition too big",
	"Requested drive(s)",
	"No free primary",
	"Extended Failed",
	"No free slots",
	NULL
};

/* returns pointers to a const char string of why */
char *GetReasonString( enum allocReason reason ) {
    int i;
    
    for (i=0; i < (int) reason && reasons[i]; i++);
    if (!reasons[i])
	return "Unknown reason";
    else
	return reasons[i];
}




/* give priority to the constraints of a partition */
/* if priority is < 0 then there was an error      */
int fdiskGetConstraintPriority( Partition *p ) {
    int pri;
    
    pri = 0;

    if (p->immutable)
	pri += 16384;
    if (fdiskDriveSetIsActive( &p->drive ))
	pri += 128;
    if (fdiskConstraintIsActive( &p->endcyl))
	pri +=  64;
    if (fdiskConstraintIsActive( &p->start))
	pri +=  32;
    if (fdiskConstraintIsActive( &p->size))
	pri +=  16;

    return pri;
}


/* sort in order of priority                                   */
/* we now worry about <1024 cyl contraints if necessary        */
int fdiskSortPartitionSpec( PartitionSpec *spec ) {
    unsigned int i, j;
    int          f;
    unsigned int pri1, pri2;
    unsigned int lsize1, lsize2;
    unsigned int msize1, msize2;
    unsigned int csize1, csize2;
    unsigned int act1,   act2;
    Partition *p1, *p2;
    PartitionSpecEntry tmpe;


    /* now see if there is anything to sort */
    if (spec->num < 2)
	return FDISK_SUCCESS;

    for (i=0; i<spec->num-1; i++) {
	p1 = &spec->entry[i].partition;

#if 0
	if (p1->immutable)
	    continue;
#endif
	
	pri1 = fdiskGetConstraintPriority(p1);
	fdiskGetConstraint(&p1->size,&csize1,&lsize1,&msize1,&act1 );
	for (j=i+1; j < spec->num; j++) {
	    p2 = &spec->entry[j].partition;

	    /* we dont want to reorder immutable specs */
	    /* but we want to bubble them to top over  */
	    /* user specifified partitions             */
	    if (p1->immutable && p2->immutable)
		continue;
	    
	    pri2 = fdiskGetConstraintPriority(p2);

	    if (pri1 < pri2)
		f = 1;
	    else if (pri1 > pri2)
		f = -1;
	    else {
		fdiskGetConstraint(&p2->size,&csize2,&lsize2,&msize2,&act2 );
		f = (lsize1 < lsize2);
	    }

	    if (f > 0) {
		memcpy(&tmpe, &spec->entry[i], sizeof(PartitionSpecEntry));
		memcpy(&spec->entry[i], &spec->entry[j],
		       sizeof(PartitionSpecEntry));
		memcpy(&spec->entry[j], &tmpe, sizeof(PartitionSpecEntry));

	    }
	}
    }
    return FDISK_SUCCESS;
}

/* Uh oh, the limitation on SPARC is not a particular cylinder boundary,
   but 1GB from start of the disk on some more broken sparc32 PROMs
   and 2GB on all others. We'll play safe and require all sparc32
   bootable partitions to sit below 1GB. */
static int fdiskSpecialPartitionLimit( HardDrive **hdarr, unsigned int numhd,
				       Partition *p) {
#ifndef __sparc__
    return 1023;
#else
    int limitcyl = 0, hdidx, i;
    for (hdidx = 0; hdidx < numhd; hdidx++) {
	i = hdarr[hdidx]->geom.heads * hdarr[hdidx]->geom.sectors;
	if (hdarr[hdidx]->num == p->drive.current) {
	    limitcyl = i;
	    break;
	}
	if (limitcyl < i) limitcyl = i;
    }
    if (!limitcyl)
	limitcyl = 2;
    else
	limitcyl = 1024 * 1024 * 2 / limitcyl;
    return limitcyl;
#endif
}

/* given mntpt and hard drive data, sets cylinder limits and hard drive
 * constraints on partition corresponding to the mntpt so that it will 
 * be bootable
 */
void fdiskSetPartitionLimitsBootable (char *mntpt,
				     HardDrive **hdarr, unsigned int numhd,
				     PartitionSpec *spec )
{
    unsigned int j;
    unsigned int drv1act, drv2act;
    Partition *p2;

    j = fdiskReturnPartitionSpec( spec, mntpt, &p2 );
    if (j == FDISK_SUCCESS) {
	/* make backup of the user's preferences just in case */
	/* they change partition so it isn't bootable         */
	if (!p2->userdriveflag)
	    fdiskCopyDriveSet ( &p2->drive, &p2->userdrive );
	p2->userdriveflag = 1;

	drv1act = fdiskThisDriveSetIsActive ( &p2->drive, 1);
	drv2act = fdiskThisDriveSetIsActive ( &p2->drive, 2);

	fdiskSetConstraint(&p2->endcyl, 0,0,
			   fdiskSpecialPartitionLimit (hdarr, numhd, p2),1);

	/* now make sure at least one of drive 1 and drive 2 are    */
	/* active. If neither are active then just make both active */
	fdiskDeactivateAllDriveSet( &p2->drive );
	if (!drv1act && !drv2act) {
	    fdiskActivateDriveSet( &p2->drive, 1 );
	    fdiskActivateDriveSet( &p2->drive, 2 );
	} else {
	    if (drv1act)
		fdiskActivateDriveSet( &p2->drive, 1 );

	    if (drv2act)
		fdiskActivateDriveSet( &p2->drive, 2 );
	}

	fdiskModifyPartitionSpec( spec, mntpt, p2, REQUEST_PENDING );
	free(p2);
    }
}

/* given a mntpt and other hard data, unsets any special cylinder limits 
 * that exist
 */
void fdiskUnsetPartitionLimitsBootable (char *mntpt, PartitionSpec *spec ) {
    unsigned int i;
    Partition *p1;

    i = fdiskReturnPartitionSpec( spec, mntpt,  &p1 );
    if (i == FDISK_SUCCESS) {
	    fdiskSetConstraint(&p1->endcyl,
			       0,FDISK_ENDCYL_MIN,FDISK_ENDCYL_MAX,0);

	    /* restore user preferences from before this was deemed */
	    /* a bootable partition                                 */
	    if (p1->userdriveflag)
		fdiskCopyDriveSet ( &p1->userdrive, &p1->drive );
	    p1->userdriveflag = 0;

	    /* fdiskActivateAllDriveSet( &p1->drive ); */
	    fdiskModifyPartitionSpec( spec, mntpt, p1, REQUEST_PENDING );
	    free(p1);
    }
}

/* returns unallocated character string containing mount point */
/* of bootable partition, or NULL if none exists               */
char *fdiskBootablePartitionName (PartitionSpec *spec) {
    unsigned int i;
    Partition *p1;

    i = fdiskReturnPartitionSpec( spec, "/boot",     &p1 );
    if (i == FDISK_SUCCESS) {
	free(p1);
	return "/boot";
    }

    i = fdiskReturnPartitionSpec( spec, "/",     &p1 );
    if (i == FDISK_SUCCESS) {
	return "/";
	free(p1);
    }

    return NULL;
}

/* check for special partitions and setup constraints if they exist */
int fdiskHandleSpecialPartitions( HardDrive **hdarr, unsigned int numhd,
				  PartitionSpec *spec ) {
    unsigned int i, j;
    Partition *p1, *p2;

    /* oh man are PCs braindead */
    /* oh man, SPARCs with buggy PROMs are braindead too (fortunately
       Ultras do it right) */
#if defined(__i386__) || defined(__sparc__)
#if defined(__sparc__)
    if (fdiskIsSparc64) return FDISK_SUCCESS;
#endif
    /* see if we have any special partitions (like bootable) */
    i = fdiskReturnPartitionSpec( spec, "/",     &p1 );
    if (i == FDISK_SUCCESS)
	free(p1);

    j = fdiskReturnPartitionSpec( spec, "/boot", &p2 );
    if (j == FDISK_SUCCESS)
        free(p2);
    
    if (j == FDISK_SUCCESS) {
	/* if "/" exists make it a normal partition again */
	if (i==FDISK_SUCCESS)
	    fdiskUnsetPartitionLimitsBootable("/", spec);

	/* set "/boot" as bootable partition */
	fdiskSetPartitionLimitsBootable("/boot", hdarr, numhd, spec);
    } else if (i == FDISK_SUCCESS) {
	/* make "/" bootable */
	fdiskSetPartitionLimitsBootable("/", hdarr, numhd, spec);
    }

    fdiskSortPartitionSpec( spec );
#endif
    return FDISK_SUCCESS;
}


/* get index of partition spec requested */
/* return value is non-zero if not found */
int fdiskIndexPartitionSpec(PartitionSpec *spec, char *name,
			    unsigned int *index ) {

    unsigned int j, found;
    
    /* see if it already exists */
    found = 0;
    for (j=0; j<spec->num && !found;)
	if (!strcmp(spec->entry[j].name, name))
	    found = 1;
	else
	    j++;

    if (found) {
	*index = j;
	return FDISK_SUCCESS;
    } else {
	return FDISK_ERR_BADNUM;
    }
}

/* insert new specification into existing PartitionSpec */
int fdiskInsertPartitionSpec( PartitionSpec *spec,
			      char          *name,
			      Partition     *p,
			      unsigned int  status) {
    unsigned int num;
    unsigned int j, found;

    if ((num=spec->num) >=MAX_PARTITION_SPEC)
	return FDISK_ERR_NOFREE;

    /* see if it already exists */
    found = 0;
    for (j=0; j<spec->num && !found;)
	if (!strcmp(spec->entry[j].name, name))
	    found = 1;
	else
	    j++;

    if (found)
	return FDISK_ERR_INUSE;
    
    /* Ok insert the bugger */
    spec->entry[num].name  = strdup(name);
    memcpy(&spec->entry[num].partition, p, sizeof(Partition));
    spec->entry[num].status = status;
    spec->num++;

    fdiskSortPartitionSpec( spec );
    return FDISK_SUCCESS;
}

/* delete specification based on mount point */
int fdiskDeletePartitionSpec( PartitionSpec *spec,
			      char          *name ) {

    unsigned int j, k, found;

    found = 0;
    for (j=0; j<spec->num && !found;)
	if (!strcmp(spec->entry[j].name, name))
	    found = 1;
	else
	    j++;

    if (!found)
	return FDISK_ERR_BADNUM;
    else {
	/* cant delete immutable partitions */
	if (spec->entry[j].partition.immutable)
	    return FDISK_ERR_BADNUM;
	
	if (spec->entry[j].name)
	    free(spec->entry[j].name);
	for (k=j; k < spec->num-1; k++)
	    memcpy(&spec->entry[k], &spec->entry[k+1],
		   sizeof(PartitionSpecEntry));

	memset(&spec->entry[spec->num-1], 0, sizeof(PartitionSpecEntry));
	spec->num--;
    }

    return FDISK_SUCCESS;
}

/* complete erase contents of a PartitionSpec - frees up names */
int fdiskWipePartitionSpec( PartitionSpec *spec ) {
    unsigned int i;

    for (i=0; i<spec->num; i++)
	free(spec->entry[i].name);

    memset(spec, 0, sizeof(PartitionSpec));
    spec->num = 0;
    return FDISK_SUCCESS;
}


	
/* update 'original' drives in partitionspec, in case any were removed  */
/* leave specs the user has added since we started alone                */
int fdiskCleanOriginalSpecs( HardDrive **hdarr, unsigned int numhd,
			     PartitionSpec *spec ) {
    unsigned int j, k, l;
    unsigned int drive, start, size;
    unsigned int tsize, tstart;
    unsigned int first, last;
    unsigned int remove;
    unsigned int type, ttype;
    unsigned int found;
    Partition *p;

    for (j=0; j<spec->num; )
	if (spec->entry[j].status != REQUEST_ORIGINAL) {
	    j++;
	    continue;
	} else {
	    p = &spec->entry[j].partition;
	    
	    fdiskGetCurrentDriveSet(&p->drive, &drive );
	    fdiskGetCurrentConstraint(&p->size , &size  );
	    fdiskGetCurrentConstraint(&p->start, &start );
	    fdiskGetCurrentConstraint(&p->type, &type );

	    /* find the HardDrive containing the drive we want */
	    for ( l=0; l<numhd; l++)
		if (hdarr[l]->num == drive)
		    break;

	    /* shouldnt happen */
	    if (l == numhd)
		return FDISK_ERR_BADNUM;
	    else
		drive = l;
	    
	    /* see if the partition still exists */
	    remove = 0;
	    found  = 0;
	    if (fdiskFirstPartition(hdarr[drive], &first)) {
		remove = 1;
	    } else {
		fdiskLastPartition(hdarr[drive], &last);

		for (l=first; l <=last && !found; l++) {
		    if (fdiskGetAttrPartition(hdarr[drive], l, &p) ==
			FDISK_SUCCESS) {
			fdiskGetCurrentConstraint(&p->size , &tsize  );
			fdiskGetCurrentConstraint(&p->start, &tstart );
			if (tstart == start && tsize == size) {
			    if (hdarr[drive]->part_type ==  FDISK_PART_TYPE_SUN) {
				fdiskGetCurrentConstraint(&p->type, &ttype );
				if ((type == 5) ^ (ttype == 5)) {
				    /* Never allow merging of a WHOLE_DISK Sun partition
				       with some non-WHOLE_DISK */
				    free (p);
				    continue;
				}
			    }
			    found = 1;
			    /* lets update partition info */
			    memcpy(&spec->entry[j].partition, p,
				   sizeof(Partition));
			}

			free(p);
		    }
		}
		if (!found)
		    remove = 1;
	    }
		    
	    if (remove) {
		if (spec->entry[j].name)
		    free(spec->entry[j].name);
		for (k=j; k < spec->num-1; k++) 
		    memcpy(&spec->entry[k], &spec->entry[k+1],
			   sizeof(PartitionSpecEntry));
		memset(&spec->entry[spec->num-1],0,sizeof(PartitionSpecEntry));
		spec->num--;
	    } else {
		j++;
	    }
	}
    fdiskSortPartitionSpec( spec );
    return FDISK_SUCCESS;
}

/* given a hard drive with original partitions, add to partition spec */
/* any user added partition specs are untouched                       */
int fdiskSetupPartitionSpec( HardDrive **hdarr, unsigned int numhd,
			     PartitionSpec *spec ) {
    unsigned int first, last, i, j;
    int          status;
    Partition    *p;
    char         *name;


    /* make sure there are any partitions to process */
    status = 0;
    for (i=0; i<numhd; i++)
	if (fdiskFirstPartition(hdarr[i], &first ) == FDISK_SUCCESS) {
	    status = 1;
	    break;
	}

    /* no partitions to process */
    if (status == 0)
	return FDISK_SUCCESS;
    
    /* go thru all drives and insert all pre-existing partitions */
    for (i=0; i<numhd; i++) {
	if (fdiskFirstPartition(hdarr[i], &first ) != FDISK_SUCCESS) 
	    continue;
	fdiskLastPartition(hdarr[i], &last );
	/* insert existing partitions using a temporary name */
	for (j=first; j <= last; j++) {
	    status = fdiskGetAttrPartition(hdarr[i], j, &p);
	    if (status == FDISK_SUCCESS) {
		if (p->type.current == LINUX_SWAP_PARTITION)
		    fdiskMakeSwapSpecName( spec, &name);
		else if (p->type.current == LINUX_RAID_PARTITION)
		    fdiskMakeRaidSpecName( spec, &name);
		else {
		    name = malloc(16);
		    sprintf(name, "Exist%03d%03d", hdarr[i]->num, j);
		}
		fdiskInsertPartitionSpec( spec, name, p, REQUEST_ORIGINAL );
		free(name);
		free(p);
	    }
	}
    }

    fdiskSortPartitionSpec( spec );
    
    return FDISK_SUCCESS;
}
	    

/* return specification based on mount point */
int fdiskReturnPartitionSpec( PartitionSpec *spec,
			      char          *name,
			      Partition     **p) {

    unsigned int j, found;

    found = 0;
    for (j=0; j<spec->num && !found;)
	if (!strcmp(spec->entry[j].name, name))
	    found = 1;
	else
	    j++;

    if (!found)
	return FDISK_ERR_BADNUM;
    else {
	*p = (Partition *) malloc( sizeof(Partition) );
	memcpy(*p, &spec->entry[j].partition, sizeof(Partition));
	return FDISK_SUCCESS;
    }
}

/* rename a spec */
int fdiskRenamePartitionSpec( PartitionSpec *spec,
			      char          *name,
			      char          *newname ) {
    unsigned int j, found;

    found = 0;
    for (j=0; j<spec->num && !found;)
	if (!strcmp(spec->entry[j].name, name))
	    found = 1;
	else
	    j++;

    if (!found)
	return FDISK_ERR_BADNUM;
    else {
	free(spec->entry[j].name);
	spec->entry[j].name = strdup(newname);
	return FDISK_SUCCESS;
    }
}
    
/* return specification based on mount point */
int fdiskModifyPartitionSpec( PartitionSpec *spec,
			       char          *name,
			       Partition     *p,
			       unsigned int  status) {

    unsigned int j, found;

    found = 0;
    for (j=0; j<spec->num && !found;)
	if (!strcmp(spec->entry[j].name, name))
	    found = 1;
	else
	    j++;

    if (!found)
	return FDISK_ERR_BADNUM;
    else {
	/* dont change immutable partitions! */
	if (p->immutable)
	    return FDISK_ERR_BADNUM;
	else {
	    memcpy(&spec->entry[j].partition, p, sizeof(Partition));
	    spec->entry[j].status = status;
	    fdiskSortPartitionSpec( spec );
	    return FDISK_SUCCESS;
	}
    }
}

/* make a unique name for a partition spec */
int fdiskMakeUniqSpecName( PartitionSpec *spec, char *base, char **name ) {
    int  i;
    char *s;
    unsigned int j;

    s = malloc(4+strlen(base));
    for (i=0; i < 1000; i++) {
	snprintf(s, 4+strlen(base), "%s%03d", base, i);
	if (fdiskIndexPartitionSpec(spec, s, &j) != FDISK_SUCCESS)
	    break;
    }
	
    *name = s;
    return FDISK_SUCCESS;
}


/* make a unique name for a swap partition spec */
int fdiskMakeSwapSpecName( PartitionSpec *spec, char **name ) {
    int rc;
    
    rc = fdiskMakeUniqSpecName( spec, "Swap", name );
    return rc;
}
	
/* make a unique name for a raid partition spec */
int fdiskMakeRaidSpecName( PartitionSpec *spec, char **name ) {
    int rc;
    
    rc = fdiskMakeUniqSpecName( spec, "Raid", name );
    return rc;
}
	
