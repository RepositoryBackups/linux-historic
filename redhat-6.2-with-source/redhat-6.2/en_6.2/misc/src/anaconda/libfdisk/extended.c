/* these routines handle extended and logical partitions - tricky stuff! */
/* converted to use new HardDrive structure                              */

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "libfdisk.h"

/* ultra-simple but useful utility routines */

/* given a type, is it an extended partition */
int fdiskIsExtended( unsigned int t ) {
    return  (t == DOS_EXTENDED_PARTITION ||
	     t == WIN98_EXTENDED_PARTITION ||
	     t == LINUX_EXTENDED_PARTITION);
}

/* go thru and make update numbers of all logical partitions */
/* probably of no use anymore with new HardDrive structure   */
int fdiskRenumberLogical( HardDrive *hd ) {
    int                     i;

    for (i=hd->limits.maxPrimary+1; i <= hd->limits.maxPartitions; i++)
	if (hd->table[i].type.current && hd->table[i].status == ALLOCATED) {
	    hd->table[i].num.current = i;
	    hd->eptable[i].num.current = i;
	}

    return FDISK_SUCCESS;
}

/* allocates a logical partition at end of  the EPT chain      */
/* user has to set attributes with a different function        */
/* returns allocated slot number if logical allocated          */
/* if PEP does not exist, returns FDISK_ERR_BADNUM             */
int fdiskAppendLogical( HardDrive *hd, unsigned int *n ) {
    int i;
    
    /* make sure the PEP exists */
    if (!hd->pep)
	return FDISK_ERR_BADNUM;
    
    /* find a free slot */
    for (i=hd->limits.maxPrimary+1; i <= hd->limits.maxPartitions; i++)
	if (hd->table[i].status != ALLOCATED &&
	    hd->table[i].status != UNUSABLE)
	    break;
    
    if (i > hd->limits.maxPartitions) {
	LastAllocStat = ALLOC_FREEPART;
	return FDISK_ERR_NOFREE;
    }

    memset(&hd->table[i], 0, sizeof(Partition));
    memset(&hd->eptable[i], 0, sizeof(Partition));
    hd->table[i].status = ALLOCATED;
    hd->eptable[i].status = ALLOCATED;
    *n = i;
    
    return FDISK_SUCCESS;
}

/* delete the logical partition into slot n of the partition table  */
int fdiskRemoveLogical( HardDrive *hd, unsigned int num ) {
    unsigned int i;

    /* see if there is a PEP */
    if (!hd->pep)
	return FDISK_ERR_BADNUM;

    /* move all other partitions up one */
    for ( i = num; i < hd->limits.maxPartitions; i++) {
	memcpy(&hd->table[i], &hd->table[i+1], sizeof(Partition));
	memcpy(&hd->eptable[i], &hd->eptable[i+1], sizeof(Partition));
    }
    
    memset(&hd->table[hd->limits.maxPartitions], 0, sizeof(Partition));
    memset(&hd->eptable[hd->limits.maxPartitions], 0, sizeof(Partition));
    hd->table[hd->limits.maxPartitions].status = AVAILABLE;
    hd->eptable[hd->limits.maxPartitions].status = AVAILABLE;

    /* renumber the logical partitions just like Dos/Linux look at them */
    fdiskRenumberLogical( hd );
    
    return FDISK_SUCCESS;
}

/* set attr of the logical partition into slot n of the partition table   */
/* does not check values of attributes, this must happen higher up        */
int fdiskSetAttrLogical( HardDrive *hd,	 unsigned int n, Partition *p ) {
    
    /* see if PEP exists */
    if (!hd->pep)
	return FDISK_ERR_BADNUM;

    if (n <= hd->limits.maxPrimary)
	return FDISK_ERR_BADNUM;
    
    if (hd->table[n].status != ALLOCATED)
	return FDISK_ERR_BADNUM;
    
    memcpy(&hd->table[n], p, sizeof(Partition));
    
    return FDISK_SUCCESS;
}	


int fdiskGetAttrLogical( HardDrive *hd, unsigned int n, Partition **p ) {

    if (!hd->pep)
	return FDISK_ERR_BADNUM;

    if (n <= hd->limits.maxPrimary)
	return FDISK_ERR_BADNUM;

    if (hd->table[n].status != ALLOCATED)
	return FDISK_ERR_BADNUM;
    
    *p = (Partition *) malloc(sizeof(Partition));
    memcpy(*p, &hd->table[n], sizeof(Partition));
    
    return FDISK_SUCCESS;
}	


/* set attr of the extended partition into slot n of the partition table  */
/* each logical partition has a matching extended partition which contains*/
/* information on the size of the area of disk in which the LP lives.     */
/* does not check values of attributes, this must happen higher up        */
int fdiskSetAttrExtended( HardDrive *hd, unsigned int n, Partition *p ) {
    
    /* see if PEP exists */
    if (!hd->pep)
	return FDISK_ERR_BADNUM;

    if (n <= hd->limits.maxPrimary)
	return FDISK_ERR_BADNUM;
    
    if (hd->eptable[n].status != ALLOCATED)
	return FDISK_ERR_BADNUM;
    
    memcpy(&hd->eptable[n], p, sizeof(Partition));
    
    return FDISK_SUCCESS;
}	


int fdiskGetAttrExtended( HardDrive *hd, unsigned int n, Partition **p ) {

    if (!hd->pep)
	return FDISK_ERR_BADNUM;

    if (n <= hd->limits.maxPrimary)
	return FDISK_ERR_BADNUM;

    if (hd->eptable[n].status != ALLOCATED)
	return FDISK_ERR_BADNUM;
    
    *p = (Partition *) malloc(sizeof(Partition));
    memcpy(*p, &hd->eptable[n], sizeof(Partition));
    
    return FDISK_SUCCESS;
}	

/* return the start and size of the primary extended partition */
int fdiskQueryPEP( HardDrive *hd, unsigned int *start, unsigned int *size ) {
    Partition *pt;

    if (!hd->pep)
	return FDISK_ERR_NOPEP;

    fdiskGetAttrPartition( hd, hd->pep, &pt );
    *start = pt->start.current;
    *size  = pt->size.current;
    return FDISK_SUCCESS;
}

/* find the primary of the given number                                      */
/* if partition doesn't exists return NULL pointer and FDISK_ERR_BADNUM      */
/* Not sure this function has any use now I moved to a single partition table*/
/* difference from GetAttr is this one DOES NOT allocate a Partition struct */
/* use this one to see if the partition actually exists                     */
int fdiskFindLogical( HardDrive *hd, unsigned int n, Partition **p ) {
    if (!hd->pep)
	return FDISK_ERR_BADNUM;
	
    if (n <= hd->limits.maxPrimary)
	return FDISK_ERR_BADNUM;
    
    if (hd->table[n].status == ALLOCATED) {
	*p = &hd->table[n];
	return FDISK_SUCCESS;
    } else {
	*p = NULL;
	return FDISK_ERR_BADNUM;
    }
}
    

/* return value of first partition */
int fdiskFirstLogical( HardDrive *hd, unsigned int *first ) {

    /* see if any logicals exist */
    if (!hd->pep)
	return FDISK_ERR_BADNUM;

    if (hd->limits.maxPrimary == hd->limits.maxPartitions)
	return FDISK_ERR_BADNUM;

    if (hd->table[hd->limits.maxPrimary+1].status != ALLOCATED)
	return FDISK_ERR_BADNUM;
    
    /* ok some logicals exists, so first possible is hd->limits.maxPrimary */
    *first = hd->limits.maxPrimary+1;
    return FDISK_SUCCESS;
}

/* return value of last logical partition # */
/* if none exist, returns FDISK_ERR_BADNUM  */
int fdiskLastLogical( HardDrive *hd, unsigned int *last ) {
    unsigned int i;

    if (!hd->pep)
	return FDISK_ERR_BADNUM;

    for (i=hd->limits.maxPartitions; i > hd->limits.maxPrimary; i--)
	if (hd->table[i].status == ALLOCATED)
	    break;

    if (i > hd->limits.maxPrimary) {
	*last = i;
	return FDISK_SUCCESS;
    } else {
	*last = 0;
	return FDISK_ERR_BADNUM;
    }
}
