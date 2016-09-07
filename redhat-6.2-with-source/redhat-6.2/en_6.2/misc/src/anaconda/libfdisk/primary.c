/* handles primary partitions */
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include "libfdisk.h"

/* delete the primary partition into slot n of the partition table  */
/* primaries are numbered 1 to hd->limits.maxPrimary                    */
int fdiskRemovePrimary( HardDrive *hd, unsigned int n ) {
    int i;
    unsigned int last;
    
    /* we only can have hd->limits.maxPrimary primary partitions */
    if (n < 1 || n > hd->limits.maxPrimary)
	return FDISK_ERR_BADNUM;

    /* if this primary is the top extended partition, we have more to do */
    if (fdiskIsExtended(hd->table[n].type.current)) {
	fdiskLastPartition(hd, &last);
	for (i=last; i > hd->limits.maxPrimary; i--)
	    fdiskRemoveLogical( hd, i );

	hd->pep = 0;
    }
    
    memset(&hd->table[n], 0, sizeof(Partition));
    hd->table[n].status = AVAILABLE;
    
    return FDISK_SUCCESS;
}	


/* create the primary partition into slot n of the partition table  */
/* primaries are numbered 1 to hd->limits.maxPrimary                    */
/* User must set attributes using fdiskSetAttrPrimary()             */
/* Just marks the partition table entry as allocated                */
int fdiskCreatePrimary( HardDrive *hd, unsigned int n ) {

    /* we only can have hd->limits.maxPrimary primary partitions */
    if (n < 1 || n > hd->limits.maxPrimary)
	return FDISK_ERR_BADNUM;

    /* does a partition already exist here */
    if (hd->table[n].status != AVAILABLE)
	return FDISK_ERR_INUSE;
    
    memset(&hd->table[n], 0, sizeof(Partition));
    hd->table[n].status = ALLOCATED;
    
    return FDISK_SUCCESS;
}	

/* sets attr of primary partition into slot n of the partition table  */
/* primaries are numbered 1 to hd->limits.maxPrimary                      */
/* this routine just copies data into partition table. Up to higher   */
/* level routines to check if this operation is allowable!            */
int fdiskSetAttrPrimary( HardDrive *hd, unsigned int n, Partition *p) {

    /* we only can have hd->limits.maxPrimary primary partitions */
    if (n < 1 || n > hd->limits.maxPrimary)
	return FDISK_ERR_BADNUM;

    if (hd->table[n].status != ALLOCATED)
	return FDISK_ERR_BADNUM;

    memcpy(&hd->table[n], p, sizeof(Partition));
    
    return FDISK_SUCCESS;
}	

/* allocates a Partition struct and returns contents of request partition */
int fdiskGetAttrPrimary( HardDrive *hd, unsigned int n, Partition **p ) {

    /* we only can have hd->limits.maxPrimary primary partitions */
    if (n < 1 || n > hd->limits.maxPrimary)
	return FDISK_ERR_BADNUM;

    if (hd->table[n].status != ALLOCATED)
	return FDISK_ERR_BADNUM;
    
    *p = (Partition *) malloc( sizeof(Partition) );
    memcpy(*p, &hd->table[n], sizeof(Partition));
    
    return FDISK_SUCCESS;
}	

/* find the primary of the given number                                      */
/* if partition doesn't exists return NULL pointer and FDISK_ERR_BADNUM      */
/* Not sure this function has any use now I moved to a single partition table*/
int fdiskFindPrimary( HardDrive *hd, unsigned int n, Partition **p ) {
    if (hd->table[n].status == ALLOCATED) {
	*p = &hd->table[n];
	return FDISK_SUCCESS;
    } else {
	*p = NULL;
	return FDISK_ERR_BADNUM;
    }
}

/* return value of first primary that exists */
/* return FDISK_ERR_BADNUM if none do        */
int fdiskFirstPrimary( HardDrive *hd, unsigned int *first ) {
    int i;

    for (i=1; i <= hd->limits.maxPrimary; i++)
#if 0
	/* I think we want to consider Primary Partitions which have */
	/* a type of extended, so this is #if'd out for now          */
	if (!fdiskIsExtended(hd->table[i].type.current) && 
	    hd->table[i].status == ALLOCATED)
#else
        if (hd->table[i].status == ALLOCATED)
#endif
	    break;

    if (i > hd->limits.maxPrimary)
	return FDISK_ERR_BADNUM;

    *first = i;
    return FDISK_SUCCESS;
}

/* return value of last primary that exists */
/* return FDISK_ERR_BADNUM if none do       */
int fdiskLastPrimary( HardDrive *hd, unsigned int *last ) {
    int i;

    for (i=hd->limits.maxPrimary; i>=1; i--)
#if 0
	/* I think we want to consider Primary Partitions which have */
	/* a type of extended, so this is #if'd out for now          */
	if (!fdiskIsExtended(hd->table[i].type.current) && 
	    hd->table[i].status == ALLOCATED)
#else
        if (hd->table[i].status == ALLOCATED)
#endif
	    break;

    if (i < 1)
	return FDISK_ERR_BADNUM;

    *last = i;
    return FDISK_SUCCESS;
}


/* return value of the first free primary slot */
/* return FDISK_ERR_NOFREEPRIM if none do          */
int fdiskFindFreePrimary( HardDrive *hd, unsigned int *free ) {
    int i;

    for (i=1; i<=hd->limits.maxPrimary; i++)
	if (hd->table[i].status == AVAILABLE &&
	    hd->limits.skipPartition != i)
	    break;
    if (i > hd->limits.maxPrimary)
	return FDISK_ERR_NOFREEPRIM;

    *free = i;
    return FDISK_SUCCESS;
}

