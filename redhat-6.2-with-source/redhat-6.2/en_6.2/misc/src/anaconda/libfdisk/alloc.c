/* handles automatic allocation of space for requested new partitions */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>

#include "libfdisk.h"

struct chunkFree {
    int sizeFree;	    /* chunk size, not total size */
    int driveNum;
};

/* Sorts largest sizeFree's first, smaller driveNum breaks tie */
static int chunkCompare(const void * one, const void * two) {
    const struct chunkFree * a = one;
    const struct chunkFree * b = two;

    if (a->sizeFree > b->sizeFree)
	return -1;
    if (a->sizeFree < b->sizeFree)
	return 1;
    if (a->driveNum < b->driveNum)
	return -1;
    if (a->driveNum > b->driveNum)
	return 1;

    return 0;
}

/* these routines manipulate a space map - can be used or free space */
/* the space map is a array of structures, each structure indicating */
/* the start and size of a chunk of  space */
/* if the size is 0, then it is an UNUSED entry in the free space table */

/* initialze a space map data type */
int fdiskSpaceMapInit( SpaceMap **map ) {
    *map = (SpaceMap *) malloc( sizeof(SpaceMap) );
    (*map)->len = 0;
    (*map)->num = 0;
    (*map)->entry = NULL;
    return FDISK_SUCCESS;
}

int fdiskIsSparc64;

unsigned long long fdiskMaxSwap( void ) {
#if defined(__alpha__)
    const unsigned long long maxswap = 128ULL*1024*1024*1024;
#elif defined(__sparc__)
    unsigned long long maxswap = 1073741824ULL;
    if (fdiskIsSparc64) maxswap = 3072ULL*1024*1024*1024;
#else
    const unsigned long long maxswap = 2147483640;
#endif
    return maxswap;
}


/* reduce # entries to smallest possible number by combining */
/* touching/overlapping entries                              */
/* fuzz introduced because can't allocate space in less than */
/* that size chunk ususally                                  */
int fdiskSpaceMapKrunch( SpaceMap *map, unsigned int fuzz ) {
    unsigned int start;
    unsigned int end;
    unsigned int maxend;
    unsigned int tmpend;
    unsigned int i, j;
    
    for (i=0; i < map->num-1; ) {

	start  = map->entry[i].start;
	end    = start + map->entry[i].size;
	maxend = end;

	/* see if next partition can merge with existing */
	if (map->entry[i+1].start <= end+fuzz) {
	    /* find last space which starts inside existing space */
	    for (j=i+1; j < map->num; )
		if (map->entry[j].start <= end+fuzz) {
		    tmpend=map->entry[j].start+map->entry[j].size;
		    if (tmpend > maxend)
			maxend = tmpend;
		    
		    fdiskSpaceMapDel( map, j );
		    continue;
		} else
		    break;

	    /* adjust first space to contain all */
	    map->entry[i].size = maxend - map->entry[i].start;
	} else {
	    /* move to the next entry and keep going */
	    i++;
	}
    }
    
    return FDISK_SUCCESS;
}

/* add an entry to a map */
/* fuzz is the smallest chunk we can allocate, used to close up holes */
/* in the space map which are not usable                              */
int fdiskSpaceMapAdd( SpaceMap *map, SpaceMapEntry *entry, unsigned int fuzz ){
    int i;
    int spot;
    int num, len;

    /* see if anything exists yet */
    if (map->entry) {
	/* we want to keep map space sorted by start */
	num = map->num;
	len = map->len;
	if (num == 0) {
	    spot = -1;
	} else if (entry->start < map->entry[0].start) {
	    spot = -1;
	} else if (entry->start > map->entry[num-1].start) {
	    spot = num;
	} else {
	    /* NEW CODE (Nov 7 1997) WATCH CLOSELY! */
	    for (spot=0; spot < num-1; spot++)
		if (entry->start >= map->entry[spot].start &&
		    entry->start <= map->entry[spot+1].start)
		    break;
	}
    } else {
	/* new list */
	num = 0;
	len = 0;
	spot = -1;
    }

    /* are we simply adjusting an existing chunk, or creating a new one ? */
    /* if its a new entry at end of list we skip this check altogether    */
    if (spot > -1 && spot != num && entry->start == map->entry[spot].start){
	/* editting existing entry */
	if (entry->size > map->entry[spot].size)
	    map->entry[spot].size = entry->size;
    } else {
        /* insert into list */
	map->num++;
	if (map->num > len) {
	    len += SpaceMapChunk;
	    map->len = len;
	    map->entry = (SpaceMapEntry *) realloc(map->entry,
						   len*sizeof(SpaceMapEntry));
	    memset(&map->entry[map->num-1], 0, len*sizeof(SpaceMapEntry));
	}
	
	for (i=map->num-1; i >= 0 && i > spot; i--)
	    memcpy(&map->entry[i], &map->entry[i-1], sizeof(SpaceMapEntry));

	if (spot != num)
	    memcpy(&map->entry[spot+1], entry, sizeof(SpaceMapEntry));
	else
	    memcpy(&map->entry[spot], entry, sizeof(SpaceMapEntry));
    }

    /* go thru and consolidate to smallest set of entries          */
    /* for now lets not use fuzz, as it erases spaces where we may */
    /* actually want to put an EPT, for example                    */
    /* fdiskSpaceMapKrunch( map, fuzz );                           */
    fdiskSpaceMapKrunch( map, 0 );
    
    return FDISK_SUCCESS;
}


/* delete an entry from a map, index by position in list */
int fdiskSpaceMapDel( SpaceMap *map, unsigned int n ) {
    int i;

    /* see if anything exists yet */
    if (!map->entry || n < 0 || n > map->num-1)
	return FDISK_ERR_BADNUM;

    for (i=n; i<map->num-1; i++)
	memcpy(&map->entry[i], &map->entry[i+1], sizeof(SpaceMapEntry));

    memset(&map->entry[map->num-1], 0, sizeof(SpaceMapEntry));
    map->num--;
    return FDISK_SUCCESS;
}


int fdiskSpaceMapFree( SpaceMap *map ) {

    if (!map)
	return FDISK_ERR_BADPTR;

    if (map->entry)
	free(map->entry);

    free(map);
    return FDISK_SUCCESS;
}

	    

/*                                                                        */
/* END OF SPACE MAP MANAGEMENT ROUTINES                                   */
/*                                                                        */



/*                                                                        */
/* These routines handle making maps of used and free space on a given hd */
/*                                                                        */

/*                                                                        */
/* Routine  to build a 'used' space map for the given hard drive          */
/*                                                                        */
int fdiskUsedMapGen( HardDrive *hd, SpaceMap **map ) {
    unsigned int i, first, last;
    int          status;
    unsigned int diskempty=0;
    Partition    *p, *ep;
    SpaceMapEntry m;
    
    /* find range of partitions to include                  */
    /* if no partitions then its easy to compute free space */
    if ((status=fdiskFirstPartition( hd, &first )) ) {
	if (status != FDISK_ERR_BADNUM)
	    return status;
	else
	    diskempty = 1;
    }

    if (diskempty) {
	first = 0;
	last  = 0;
    } else {
	if ((status=fdiskLastPartition( hd, &last   )))
	    return status;
    }
    
    fdiskSpaceMapInit( map );

    /* insert the Master Boot record */
    m.start = 0;
    m.size  = hd->geom.sectors;
    fdiskSpaceMapAdd( *map, &m, hd->geom.sectors );

    /* Set aside some space for aboot */
    if (hd->part_type == FDISK_PART_TYPE_BSD) {
	m.start = 0;
	m.size  = hd->geom.sectors * hd->geom.heads;
	fdiskSpaceMapAdd( *map, &m, hd->geom.sectors );
    }
    
    /* if no partitions we're free to go */
    if (diskempty)
	return FDISK_SUCCESS;

    /* otherwise go thru list and figure allocated space */
    for (i=first; i <= last; i++) {
	status = fdiskGetAttrPartition( hd, i, &p );
	if (!status) {
	    if ((i<hd->limits.maxPrimary + 1)) {
		/* this is a primary partition */
		/* is it an extended partition? */
		if (!fdiskIsExtended( p->type.current )) {
		    m.start = p->start.current;
		    m.size  = p->size.current;
		} else {
		    /* this is the PEP */
		    /* we insert a used region for the PEPT */
		    m.start = p->start.current;
		    m.size  = hd->geom.sectors;
		}
		fdiskSpaceMapAdd( *map, &m, hd->geom.sectors );
	    } else {
		/* this is a  logical/extended partition pair.     */
		/* We take into account the space used by the EPT  */
		/* and the logical partition                       */

		/* first the EPT */
		fdiskGetAttrExtended( hd, i, &ep );
		m.start = ep->start.current;
		m.size  = hd->geom.sectors;  /* EPT takes 1 sector but */
                                             /* usually takes 1 head   */
		fdiskSpaceMapAdd( *map, &m, hd->geom.sectors );

		/* now the logical partition */
		/* see if the offset is due to a true offset, or because */
		/* of cyl/head/sector stuff  */
		/* could probably use p->offset.current but not sure */
		/* that is setup correctly (yet) */
		if ((p->start.current-ep->start.current)<=hd->geom.sectors) {
		    m.start = ep->start.current;
		    m.size  = p->size.current +
			(p->start.current-ep->start.current);
		} else {
		    m.start = p->start.current;
		    m.size  = p->size.current;
		}
		fdiskSpaceMapAdd( *map, &m, hd->geom.sectors );
		free(ep);
	    }
	    free(p);
	}
    }
		
    return FDISK_SUCCESS;
}

/*                                                                        */
/* Routine  to build a 'free' space map for the given hard drive          */
/*                                                                        */
int fdiskFreeMapGen( HardDrive *hd, SpaceMap **map ) {
    unsigned int    i, first, diff;
    int             status;
    SpaceMapEntry   m;
    SpaceMap        *umap;

    /* make a 'used' map, and invert it */
    if ((status=fdiskUsedMapGen( hd, &umap )))
	return status;
    
    fdiskSpaceMapInit( map );
    
    /* start with first sector, find first used sector */
    /* and keep building free space map that way.      */
    first = 0;
    for (i=0; i < umap->num; i++) {
	diff = umap->entry[i].start - first;
	if (diff) {
	    m.start = first;
	    m.size  = diff;
	    fdiskSpaceMapAdd( *map, &m, 0 ); /* no fuzz needed */
	}

	/* move over used block to next possible free space */
	first = umap->entry[i].start + umap->entry[i].size;
    }

    /* now handle the end */
    diff = hd->totalsectors - first;
    if (diff) {
	m.start = first;
	m.size  = diff;
	fdiskSpaceMapAdd( *map, &m, 0 ); /* no fuzz needed */
    }

    return FDISK_SUCCESS;
}


/* given a partition describing a request and a candidate freespace entry  */
/* determine if the space will work                                        */
/* Handles aligning to cylinder boundaries                                 */
/* modifies partition p if the constraints worked out ok                   */
/* Returns FDISK_ERR_NOFREE if nothing found                               */
int fdiskCheckConstraints( HardDrive *hd, SpaceMapEntry *freespace,
			   Partition *p, unsigned int type ) {

    unsigned int lsize,  msize,  csize, size;
    unsigned int lstart, mstart, cstart, start;
    unsigned int lcyl,   mcyl,   ccyl, cyl;
    unsigned int end;
    unsigned int extstart, extsize, extend;
    unsigned int tmp1, tmp2;
    unsigned int fuzz;
    unsigned int inpep;
    unsigned int satisfied, satis1, satis2, pass;

    /* see if we're looking for a logical partition or not */
    fuzz  = 0;
    inpep = 0;
    if (type & LOGICAL)
	if (!hd->pep)
	    return FDISK_ERR_NOPEP;
	else {
	    fuzz  = hd->geom.sectors;
	    inpep = 1;
	    fdiskQueryPEP( hd, &extstart, &extsize );
	    extend = extstart + extsize - 1;
	}
    else if (type & PRIMARY_EXTENDED)
	fuzz = 32;
    
    /* setup all of the constrains                                     */
    fdiskQueryConstraint(&p->size,  &csize, &lsize,  &msize,
		       FDISK_SIZE_MIN, FDISK_SIZE_MAX );
    fdiskQueryConstraint(&p->endcyl, &ccyl, &lcyl,   &mcyl,
		       FDISK_ENDCYL_MIN,  FDISK_ENDCYL_MAX );
    fdiskQueryConstraint(&p->start,  &cstart, &lstart,  &mstart,
		       FDISK_START_MIN,  FDISK_START_MAX );
    
    /* we have several constraints to satisfy, here we are interested  */
    /* in the size and start requirements of the partition             */
    /* First see if we have a big enough chunk of free space           */
    size  = freespace->size;
    start = freespace->start;
    end   = start + size - 1;
	
    /* handle cylinder boundaries here */
    fdiskRoundStartToCylinder(hd, &start);
    fdiskRoundEndToCylinder(hd, &end);
    size = end - start + 1;
    
    pass = 0;
    satis1 = satis2 = 0;
    while (pass < 2) {
	/* make sure its big enough and is inside PEP if so desired */
	satisfied = 0;
	if (size  >= lsize && (!inpep || (start>=extstart && start<=extend))) {
	    end = start + lsize - 1;

	    /* round end to the cylinder boundary */
	    fdiskRoundEndToCylinder( hd, &end );
	    
	    /* now check that we can satisfy and start/end contraints */
	    satisfied = 1;
	    if (fdiskConstraintIsActive( &p->endcyl )) {
		fdiskSectorToCHS(hd,end,&cyl,&tmp1,&tmp2);
		satisfied = satisfied && (cyl >= lcyl) && (cyl <= mcyl );
		if (!pass && !satisfied && LastAllocStat == ALLOC_UNDEF)
		    LastAllocStat = ALLOC_CYL;
	    }
	    
	    if (fdiskConstraintIsActive( &p->start )) {
		satisfied = satisfied && (start>=lstart) && (start<=mstart);
		if (!pass && !satisfied && LastAllocStat == ALLOC_UNDEF)
		    LastAllocStat = ALLOC_START;
	    }
	    
	    if (inpep) {
		satisfied = satisfied && (end <= extend);
	    }

            /* 2.1.x allows larger swap files*/
	    /* cant let swap get too big! */
	    if (p->type.current == LINUX_SWAP_PARTITION) {
		unsigned int maxswap = fdiskMaxSwap () / SECTORSIZE;
		satisfied = satisfied && ((end-start+1) <= maxswap);
	    }
	} else {
	    if (!pass && LastAllocStat == ALLOC_UNDEF)
		LastAllocStat = ALLOC_SIZE;
	}

	/* did we pass all the tests? */
	if (pass == 0) {
	    satis1 = satisfied;
	    if (satisfied) {
		/* save working params into partition entry */
		p->size.current  = end - start + 1;
		p->start.current = start;

		LastAllocStat = ALLOC_OK;
		
		/* now lets get greedy and grab extra cylinder    */
		/* this makes up for the fact that we almost      */
		/* always grab a little less space than requested */
		/* due to overhead of partition tables            */
		pass++;
		lsize += hd->geom.heads*hd->geom.sectors;
		if (lsize > size)
		    lsize = size;
	    } else {
		pass = 3;
	    }
	} else {
	    satis2 = satisfied;
	    if (satisfied) {
		/* save working params into partition entry */
		p->size.current  = end - start + 1;
		p->start.current = start;

		/* we're done */
		pass = 3;
	    } else {
		/* we're done */
		pass = 3;
	    }
	}
    }
    
    if (satis1 || satis2) {
	/* hack - did we allocate primary partition inside of PEP? */
	if ((type & PRIMARY) && hd->pep) {
	    unsigned int pstart, pend;

	    fdiskQueryPEP( hd, &extstart, &extsize );
	    extend = extstart + extsize - 1;
	    pstart = p->start.current;
	    if (pstart >= extstart && pstart <= extend) {
		return FDISK_ERR_NOFREE;
	    }
	    pend = p->start.current + p->size.current;
	    if (pend >= extstart && pend <= extend) {
		return FDISK_ERR_NOFREE;
	    }
	}

	return FDISK_SUCCESS;
    } else {
	return FDISK_ERR_NOFREE;
    }
}

/* Looking at current partition table of drive hd, find a chunk of         */
/* free space that will contain the partition p with all its constraints   */
/* if inpep!=0, we look in pep area only, and add some space required      */
/* for logical partitions over their requested size.                       */
/* Handles aligning to cylinder boundaries                                 */
/* Returns FDISK_ERR_NOFREE if nothing found                               */
int fdiskFindFreeSlot( HardDrive *hd, SpaceMap *freespace, Partition *p,
		       unsigned int type, unsigned int *freeslot ) {

    unsigned int done, j;
    
    /* we have several constraints to satisfy, here we are interested  */
    /* in the size and start requirements of the partition             */
    /* First see if we have a big enough chunk of free space           */
    done = 0;
    for (j=0; j<freespace->num && !done; j++) {
	if (fdiskCheckConstraints(hd, &freespace->entry[j],
				  p, type ) == FDISK_SUCCESS) {
	    done = 1;
	    *freeslot = j;
	}
    }
    
    if (done) {
	return FDISK_SUCCESS;
    } else {
	/* guess error if we didn't set it above */
	if (LastAllocStat == ALLOC_UNDEF)
	    LastAllocStat = ALLOC_SIZE;
	return FDISK_ERR_NOFREE;
    }
}

/* characteristics of primary are handled in constraints in p        */
/* if pri < 0, autoallocate primary slot                             */
int fdiskMakeNewPrimary( HardDrive *hd, int pri, Partition *p ) {
    
    unsigned int lsize,  msize,  csize;
    unsigned int lstart, mstart, cstart;
    unsigned int lcyl,   mcyl,   ccyl;
    int          status;
    
    Partition *pt;

    /* See if we need to auto-allocate the partition */
    if (pri < 0)
	if (fdiskFindFreePrimary( hd, &pri ) == FDISK_ERR_NOFREEPRIM) {
	    LastAllocStat = ALLOC_FREEPRI;
	    return FDISK_ERR_NOFREEPRIM;
	}
	
    /* setup all of the constrains                                     */
    fdiskQueryConstraint(&p->size,  &csize, &lsize,  &msize,
		       FDISK_SIZE_MIN, FDISK_SIZE_MAX );
    fdiskQueryConstraint(&p->endcyl, &ccyl, &lcyl,   &mcyl,
		       FDISK_ENDCYL_MIN,  FDISK_ENDCYL_MAX );
    fdiskQueryConstraint(&p->start,  &cstart, &lstart,  &mstart,
		       FDISK_START_MIN,  FDISK_START_MAX );
    
    status=fdiskCreatePrimary( hd, pri );
    if (status != FDISK_SUCCESS) {
	LastAllocStat = ALLOC_FREEPRI;
	return status;
    }
    
    /* msf - second argument was 1, was changed to pri */
    fdiskGetAttrPartition( hd, pri, &pt );
    fdiskSetConstraint( &pt->size,  csize,  lsize,  msize,
			fdiskConstraintIsActive(&p->size) );
    fdiskSetConstraint( &pt->start, cstart, lstart, mstart,
			fdiskConstraintIsActive(&p->start) );
    fdiskSetFixedConstraint( &pt->type,   p->type.current    );
    fdiskSetFixedConstraint( &pt->active, p->active.current  );
    fdiskSetFixedConstraint( &pt->offset, 0                  );
    fdiskDeactivateAllDriveSet( &pt->drive );
    fdiskActivateDriveSet( &pt->drive,  hd->num );
    fdiskSetCurrentDriveSet( &pt->drive, hd->num );
    fdiskSetFixedConstraint( &pt->num,    pri                );
    pt->status = ALLOCATED;

    fdiskSetAttrPartition( hd, pri, pt );

    /* store current value in the partition we were passed */
    fdiskSetCurrentConstraint(&p->num, pri);
    fdiskSetCurrentDriveSet(&p->drive, hd->num);
    fdiskSetCurrentConstraint(&p->size, csize);
    fdiskSetCurrentConstraint(&p->start, cstart);
    
    free(pt);

    return FDISK_SUCCESS;
}    

/* if no primary extended partition exists, this routine will do it */
int fdiskMakeNewPrimaryExtended(HardDrive *hd,
				int          pep,
				unsigned int freestart,
				unsigned int freesize ) {

    unsigned int extstart, extend, extsize;
    int          status;

    Partition *pt;
    
    /* no primary extended partition (yet)                            */
    /* lets make one the size of the free space block we're putting   */
    /* the requested partition in                                     */
    /* then we make our logical partition                             */
    if (pep < 0)
	if (fdiskFindFreePrimary( hd, &pep ) == FDISK_ERR_NOFREEPRIM) {
	    LastAllocStat = ALLOC_FREEPRI;
	    return FDISK_ERR_NOFREEPRIM;
	}
    
    hd->pep = pep;
    if ((status=fdiskCreatePrimary( hd, pep )) != FDISK_SUCCESS) {
	LastAllocStat = ALLOC_FREEPRI;
	return status;
    }
    
    fdiskGetAttrPartition( hd, pep, &pt );
    
    /* we need to make size/start that of the free block we're using */
    /* and NOT the size of the logical partition we want to make     */
    /* We DO NOT set the size/start constraints ACTIVE, since we may */
    /* want to grow them later.                                      */
    extstart = freestart;
    extend   = freestart + freesize - 1;
    
    /* handle cylinder boundaries here */
    fdiskRoundStartToCylinder(hd, &extstart);
    fdiskRoundEndToCylinder(hd, &extend);
    extsize = extend - extstart + 1;
    
    fdiskSetConstraint( &pt->size, extsize,  extsize,  FDISK_SIZE_MAX, 0 );
    fdiskSetConstraint( &pt->start,extstart, extstart, FDISK_START_MAX, 0);
    fdiskSetFixedConstraint( &pt->type,   DOS_EXTENDED_PARTITION );
    fdiskSetFixedConstraint( &pt->active, 0 );
    fdiskSetFixedConstraint( &pt->offset, 0 );
    fdiskDeactivateAllDriveSet( &pt->drive );
    fdiskActivateDriveSet( &pt->drive,  hd->num  );
    fdiskSetCurrentDriveSet( &pt->drive, hd->num );
    fdiskSetFixedConstraint( &pt->num,    pep );
    pt->status = ALLOCATED;
    
    fdiskSetAttrPartition( hd, pep, pt );
    free(pt);

    return FDISK_SUCCESS;
}

/* Make an extended partition within the specified region */
/* Must have a PEP before this call will work.            */
/* The size and start of the partition p are used         */
/* However, the user data is offset because there is a EPT*/
/* at the start of this space                             */
int fdiskMakeNewLogical( HardDrive *hd, Partition *p ) {
    int status;

    unsigned int lp, pep;

    unsigned int cstart, lstart, mstart;
    unsigned int csize,  lsize,  msize;
    unsigned int extstart, extend, extsize;
    unsigned int sector_offset;
    
    Partition *pt, *ept;

    /* see if pep exists */
    pep = hd->pep;
    if (!pep)
	return FDISK_ERR_NOPEP;
    
    /* use it as a template for the extented partition part of the */
    /* EP/LP pair we are creating                                  */
    if ((status=fdiskAppendLogical( hd, &lp )) != FDISK_SUCCESS)
	return status;


    /* setup all of the constrains                              */
    fdiskQueryConstraint(&p->size,  &csize, &lsize,  &msize,
		       FDISK_SIZE_MIN, FDISK_SIZE_MAX );
    fdiskQueryConstraint(&p->start, &cstart, &lstart,  &mstart,
		       FDISK_START_MIN,  FDISK_START_MAX );
    
    /* get initial starting points for extended and logical entries */
    fdiskGetAttrPartition( hd, lp,  &pt );
    fdiskGetAttrPartition( hd, pep, &ept );
    
    /* since we are creating a logical partition from scratch,  */
    /* we can make the offset anything we like. The existing    */
    /* 'fdisk' program makes the offset equal to one head       */
    sector_offset = hd->geom.sectors;

    /* setup the extended partition first */
    /* it describes the partition which the logical is IN       */
    extstart = cstart;
    extend   = cstart + csize + sector_offset - 1;
    
    /* handle cylinder boundaries here */
    fdiskRoundStartToCylinder(hd, &extstart);
    fdiskRoundEndToCylinder(hd, &extend);
    extsize = extend - extstart + 1;
    
    fdiskSetConstraint( &ept->size, extsize,  extsize,  FDISK_SIZE_MAX, 0 );
    fdiskSetConstraint( &ept->start,extstart, extstart, FDISK_START_MAX, 0);
    fdiskSetFixedConstraint( &ept->type,   DOS_EXTENDED_PARTITION );
    fdiskSetFixedConstraint( &ept->active, 0 );
    fdiskSetFixedConstraint( &ept->offset, 0 );
    fdiskDeactivateAllDriveSet( &ept->drive );
    fdiskActivateDriveSet( &ept->drive,  hd->num  );
    fdiskSetCurrentDriveSet( &ept->drive, hd->num );
    fdiskSetFixedConstraint( &ept->num,  lp );
    ept->status = ALLOCATED;
    
    fdiskSetAttrExtended( hd, lp, ept );
    free(ept);
    
    /* now the logical partition                                */
    fdiskSetConstraint( &pt->size,
			extsize-sector_offset,
			lsize, msize, fdiskConstraintIsActive(&p->size) );
    fdiskSetConstraint( &pt->start,
			extstart+sector_offset,
			lstart, mstart,fdiskConstraintIsActive(&p->start) );
    fdiskSetFixedConstraint( &pt->type,   p->type.current );
    fdiskSetFixedConstraint( &pt->active, p->active.current );
    fdiskSetFixedConstraint( &pt->offset, 0 );
    fdiskDeactivateAllDriveSet( &pt->drive );
    fdiskActivateDriveSet( &pt->drive,  hd->num  );
    fdiskSetCurrentDriveSet( &pt->drive, hd->num );
    fdiskSetFixedConstraint( &pt->num,    lp );
    pt->status = ALLOCATED;
    
    /* store current value in the partition we were passed */
    fdiskSetCurrentConstraint(&p->num, lp);
    fdiskSetCurrentDriveSet(&p->drive, hd->num);
    fdiskSetCurrentConstraint(&p->size, extsize-sector_offset);
    fdiskSetCurrentConstraint(&p->start, extstart+sector_offset);
    
    fdiskSetAttrPartition( hd, lp, pt );
    free(pt);

    return FDISK_SUCCESS;
}


/*                                                                        */
/* These routines handle inserting a desired partition into an existing   */
/* partition table on a hard drive.                                       */
/*                                                                        */
/* We pass an array of HardDrive's, each of which is considered for the   */
/* possible home of the partition.                                        */
/* HardDrive's are assumed to be in arranged in increasing number         */
/* Note that the index of the drive in the HardDrive array IS NOT the same*/
/* as its actual 'num'.                                                   */
int fdiskAutoInsertPartition(HardDrive **hdarr,
			     unsigned int nhd,
			     Partition *p,
			     int useBiggestChunk) {
    
    unsigned int drive;
    unsigned int extstart, extsize;
    unsigned int i;
    unsigned int freeslot, freedrive;

    unsigned int extExists, priExists, noneExists;
    unsigned int extCreate, priCreate, logCreate;
    unsigned int extTried,  priTried, logTried;
    unsigned int tryExtNext;
    unsigned int done, donesearch, errsearch, lasttry, trynext;
    int status;
    int chunk;

    HardDrive *hd;
    struct chunkFree * driveOrder;
    SpaceMap  **freespace;
    DriveSet  drives;
    Partition *ptmp;

    /* Rather then go through the drives in the order we're given then, we
       consider them in the order of their largest available free space
       map. */

    /* first lets generate free space maps for all possible drives */
    /* also figure out what range of drives exist                  */
    /* index of freespace[] will be the same as for accessing      */
    /* the entries of the hdarr[] of HardDrive's                   */
    freespace = (SpaceMap **) alloca((nhd)*sizeof(SpaceMap));
    driveOrder = alloca(nhd * sizeof(*driveOrder));
    fdiskDeactivateAllDriveSet( &drives );
    for (drive=0; drive < nhd; drive++) {
	fdiskFreeMapGen( hdarr[drive], &freespace[drive] );
	fdiskActivateDriveSet( &drives, hdarr[drive]->num );

	driveOrder[drive].driveNum = drive;
	driveOrder[drive].sizeFree = 0;
	for (i = 0; i < freespace[drive]->num; i++) {
	    if (freespace[drive]->entry[i].size > driveOrder[drive].sizeFree) {
		driveOrder[drive].sizeFree = freespace[drive]->entry[i].size;
	    }
	}
    }

    /* if there is a drive constraint, use it. Otherwise sort by largest */
    /* free drive first                                                  */
    if (useBiggestChunk) {
/*	printf ("sorting freespace\n"); */
	qsort(driveOrder, nhd, sizeof(*driveOrder), chunkCompare);
    } else {
/*	printf ("not sorting freespace\n"); */
    }	

    /* loop over all the drives which are valid to consider            */
    /* For each drive, go through a list of 'preferable' places to put */
    /* the new partition.                                              */
    /*                                                                 */
    /* Rules:                                                          */
    /*                                                                 */
    /*     If No Partitions Exist                                      */
    /*        Create New Primary                                       */
    /*        DONE                                                     */
    /*     Else If No Extended Exists                                  */
    /*        Create Extended in largest hole fitting all constraints  */
    /*        DONE                                                     */
    /*     Else                                                        */
    /*        Try to create a logical partition                        */
    /*        If Success                                               */
    /*           DONE                                                  */
    /*        Else                                                     */
    /*           Try to create a primary partition                     */
    /*           If Success                                            */
    /*              DONE                                               */
    /*           Else                                                  */
    /*              FAIL                                               */
    /*                                                                 */
    done = 0;
    for (chunk = 0; chunk < nhd && !done; chunk++) {
	drive = driveOrder[chunk].driveNum;

	hd = hdarr[drive];
	if (!fdiskThisDriveSetIsActive( &p->drive, hd->num ))
	    continue;
	
	LastAllocStat = ALLOC_UNDEF;
	/* Figure out what partitions currently exist on this drive */
	extExists  = (hd->pep != 0);
	priExists = 0;
	for (i=1; i< hd->limits.maxPrimary; i++) {
	    if (fdiskGetAttrPrimary(hd, i, &ptmp) != FDISK_SUCCESS)
		continue;

	    /* we only count an existing primary partition if it is */
	    /* a Linux RAID or ext2 partition. Otherwise we keep    */
	    /* searching because we want to make sure the first     */
	    /* LINUX partition we allocate (which will be the boot  */
	    /* partition) goes onto a primary partition.            */

	    /* XXX - this broke hjl's dac960 patch -- I'm not sure  */
            /* what his patch was trying to do here                 */
            /* he had:                                              */
            /*    if (!fdiskIsExtended(ptmp->type.current))         */
            /*        priExists++;                                  */
	    priExists |= !fdiskIsExtended(ptmp->type.current) &&
		         (ptmp->type.current == LINUX_NATIVE_PARTITION ||
			  ptmp->type.current == LINUX_RAID_PARTITION);
	}

	noneExists = !(extExists || priExists);

	/* Keeps trying strategies for placing new partition in existing */
	/* scheme until we hit a solution we like                        */
	donesearch = 0;
	errsearch  = 0;
	lasttry    = 0;
	trynext    = 0;
	extTried   = (hd->limits.maxPrimary == hd->limits.maxPartitions);
	logTried   = 0;
	priTried   = 0;
	tryExtNext = 0;
	while (!donesearch && !errsearch && !lasttry && !trynext) {
	    priCreate  = 0;
	    extCreate  = 0;
	    logCreate  = 0;

	    /* XXX fixme -- dac960 patch from hjl */
/*  	    if (noneExists || priExists < hd->limits.maxPrimary - 1) { */
	    if ((noneExists || !priExists) && !tryExtNext) { 
		/* gonna make a primary partition */
		priCreate = 1;
		if ((status=fdiskFindFreeSlot(hd,freespace[drive],p,
				      PRIMARY,&freeslot)) != FDISK_SUCCESS) {
		    /* if extended partition exists lets try it next */
		    /* else we fail to trying next drive             */
		    if (!extExists) {
			trynext = 1;
		    } else {
			tryExtNext = 1;
			continue;
		    }
		} else {
		    freedrive  = drive;
		    lasttry = 1;
		}
	    } else if (!extExists) {
		tryExtNext = 0;
		if (!extTried) {
		    /* let try to make a primary extended partition */
		    extCreate = 1;
		    if ((status=fdiskFindFreeSlot(hd,freespace[drive],p,
						  PRIMARY_EXTENDED,
						  &freeslot)) != FDISK_SUCCESS)
			trynext = 1;
		    else {
			freedrive  = drive;
		    }
		} else if (!priTried) {
		    /* couldnt make extended work, try to make another PP */
		    priCreate = 1;
		    if ((status=fdiskFindFreeSlot(hd,freespace[drive],p,
					  PRIMARY,&freeslot)) != FDISK_SUCCESS)
			trynext = 1;
		    else {
			freedrive  = drive;
			lasttry    = 1;
		    }
		} else
		    /* nothing else to try */
		    trynext = 1;
	    } else {
		/* try to make a logical first, then a primary */
		if (!logTried) {
		    logCreate = 1; 
		    if ((status=fdiskFindFreeSlot(hd,freespace[drive],p,
					  LOGICAL,&freeslot)) != FDISK_SUCCESS) {
			if (!priTried) {
			    priCreate = 1;
			    if ((status=fdiskFindFreeSlot(hd,freespace[drive],p,
							  PRIMARY,&freeslot)) != FDISK_SUCCESS)
				trynext = 1;
			    else {
				freedrive  = drive;
				lasttry    = 1;
			    }
			} else {
			trynext = 1;
			}
		    } else {
			freedrive  = drive;
		    }
		} else
		    /* nothing else to try */
		    trynext = 1;
	    }

	    /* move on to next drive */
	    if (trynext)
		continue;
	    
	    if (!errsearch) {
		/* now try to create whatever we decided was the best choice */
		/* parameters of the allocated partition are in the partition*/
		/* structure 'p'                                             */
		if (priCreate) {
		    priTried = 1;
		    if ((status=fdiskMakeNewPrimary(hd,-1,p))==FDISK_SUCCESS)
			donesearch = 1;
		} else if (extCreate) {
		    extTried = 1;
		    if (!extExists) {
			/* gotta make a PEP */
			extstart = freespace[drive]->entry[freeslot].start;
			extsize  = freespace[drive]->entry[freeslot].size;
			status=fdiskMakeNewPrimaryExtended(hd, -1,
							   extstart,extsize);
			if (status == FDISK_SUCCESS) {
			    extExists = 1;
			    status=fdiskMakeNewLogical(hd, p);
			    if (status == FDISK_SUCCESS)
				donesearch = 1;
			}
		    }
		} else if (logCreate) {
		    logTried = 1;
		    if (!extExists) {
			status = FDISK_ERR_NOPEP;
			errsearch = 1;
		    } else {
			status=fdiskMakeNewLogical(hd, p);
			if (status == FDISK_SUCCESS)
			    donesearch = 1;
		    }
		}			
	    } else {
		printf("Error searching was %d, aborting\n", status);
		return status;
	    }
	}

	if (donesearch)
	    done = 1;
    }
	    
    /* did we find an acceptable location?                  */
    /* if not we SHOULD try reshuffling existing partitions */
    /* but for now we just give up                          */
    if (!done) {
	return FDISK_ERR_NOFREE;
    }

    return FDISK_SUCCESS;
}


/* not sure what this will be good for yet */
#if 0

/* moves a non-extended primary partition to new location specified in */
/* the partition p                                                     */
int fdiskMovePrimary( HardDrive *hd, unsigned int num, Partition *p ) {

    fdiskSetAttrPartition( hd, num, &p);

}

/* krunch partition table so all moveable partitions form contiguous block */
/* will not slide logical partitions out of an extended partition, instead */
/* the extended partition start is slid and logical partitions move inside */
/* the extended partition. This way the # of primary partition slots used  */
/* is kept constant.                                                       */
/* we try to push available free space towards the start of the disk       */
int fdiskKrunchPartitionTable( HardDrive *hd ) {

    unsigned int last, first, cur;
    unsigned int csize,  lsize,  msize, asize;
    unsigned int cstart, lstart, mstart, astart;
    unsigned int endfree, startfree, sizefree;
    unsigned lastfree;
    int      i;
    SpaceMap *freespace;
    SpaceMapEntry testentry;
    Partition tstp;
    Partition *p, *ep;

    /* make sure there are any partitions to process */
    if (fdiskLastPartition(hd, &last ) == FDISK_ERR_BADNUM)
	return FDISK_ERR_BADNUM;
    
    /*  - look through freespace list                                    */
    /*  - if adjacent, movable partition exists, slide to close space    */
    /*    otherwise, find movable partitions to move to close   space    */
    /*  - if we made any changes, recompute freespace map                */
    /*  - keep going till we change nothing                              */
    /*                                                                   */
    /* First look at primary partitions, shift/relocate them if possible */
    done      = 0;
    lastfree  = hd->totalsectors;
    freespace = NULL;
    while (!done) {

	/* get a freespace map for the drive first */
	fdiskFreeMapGen( hd, &freespace );
	if (!freespace->num)
	    break;

	/* locate next free space to consider */
	for (i=0; i < freespace->num; i++)
	    if (freespace->entry[i].start < lastfree)
		break;

	/* couldnt find any more freespace to consider */
	if (i == freespace->num)
	    break;
	
	/* locate primary partition closest to freespace on the high side */
	startfree = freespace->entry[i].start;
	sizefree = freespace->entry[i].size;
	endfree   = startfree + sizefree - 1;

	mindiff = FDISK_SIZE_MAX;
	minnum  = 0;
	for (cur=1; cur <=hd->limits.maxPrimary; cur++) {
	    if (fdiskGetAttrPartition( hd, cur, &p))
		continue;

	    if (p->immutable) {
		free(p);
		continue;
	    }
	    
	    fdiskGetConstraint(&p->start,&cstart,&lstart,&mstart,&astart );
	    fdiskGetConstraint(&p->size, &csize, &lsize, &msize, &asize  );

	    if (cstart < startfree) {
		tmpdiff = startfree - cstart;
		if (tmpdiff < mindiff) {
		    mindiff = tmpdiff;
		    minnum  = cur;
		}
	    }
	    free(p);
	}

	/* see if we found anything */
	if ( minnum != 0 ) {
	    fdiskGetAttrPartition( hd, minnum, &p);

	    if (!fdiskIsExtended(p->type.current)) {
		/* its a simple primary partition               */
		/* see if we can move the partition and satisfy */
		/* any constraints if may have on its location  */
		/* if it works, move the partition and update   */
		/* last free considered                         */
		fdiskGetConstraint(&p->start,&cstart,
				   &lstart,&mstart,&astart );
		fdiskGetConstraint(&p->size, &csize,
				   &lsize, &msize, &asize  );
		startfree = freespace->entry[minnum].start;
		sizefree  = freespace->entry[minnum].size;
		endfree   = startfree + sizefree - 1;

		/* setup new location and round to a cyl boundary */
		/* if rounding makes the partition smaller, move  */
		/* start back another cylinder and try again      */
		/* primary partitions always start on a cylinder  */
		/* boundary (except for the one containing the MBR*/
		testentry.start = endfree - csize;
		fdiskRoundStartToCylinder( hd, &cstart );
		testentry.size  = endfree - cstart + 1;
		if (testentry.size < csize)
		    testentry.start -= hd->geom.sectors*hd->geom.heads;
		testentry.size  = csize;
		
		if (fdiskCheckContraints(hd, &testentry,
					 p, PRIMARY ) == FDISK_SUCCESS) {
		    fdiskMovePrimary(hd, minnum, p);
		    fdiskGetConstraint(&p->start,&cstart,
				       &lstart,&mstart,&astart );
		    fdiskGetConstraint(&p->size, &csize,
				       &lsize, &msize, &asize  );
		    lastfree = cstart + csize;
		}
	    } else {
		/* its a primary extended partition, lots more to do!  */
		/* have to also relocate all logical partitions inside */
		if (fdiskCheckContraints(hd, &freespace->entry[i],
					 p, EXTENDED ) == FDISK_SUCCESS) {
		    fdiskMovePrimaryExtended( hd, minnum, p );
		    fdiskGetConstraint(&p->start,&cstart,
				       &lstart,&mstart,&astart );
		    fdiskGetConstraint(&p->size, &csize,
				       &lsize, &msize, &asize  );
		    lastfree = cstart + csize;
		}

	    }
	} else {
	    done = 1;
	}
	
	/* anything left allocated? */
	if (freespace != NULL) {
	    fdiskSpaceMapFree( freespace );
	    freespace = NULL;
	}
    }

    /* anything left allocated? */
    if (freespace != NULL) {
	fdiskSpaceMapFree( freespace );
	freespace = NULL;
    }
    
}
#endif


/* given a list of requested partitions, take hdarr as starting point.  */
/* stick partitions into nhdarr                                         */
static int dofdiskAutoInsertPartitions( HardDrive **hdarr,    
			       unsigned int numhd,
			       HardDrive **newhdarr, 
			       PartitionSpec *spec, int wecare ) {
    unsigned int i;
    int          status;
    char         *bootpart;


    /* copy existing hard drive structure into the new hard drive struct */
    for (i=0; i < numhd; i++) 
	memcpy(newhdarr[i], hdarr[i], sizeof(HardDrive));

    /* get name of bootable partition */
    bootpart = fdiskBootablePartitionName(spec);
    if (!bootpart)
	bootpart = strdup ("NO BOOT PARTITION EXISTS");
    else
	bootpart = strdup (bootpart);

    /* start inserting the partitions */
    /* set the last alloc error to undef */
    LastAllocStat = ALLOC_UNDEF;
    for (i=0; i<spec->num; i++) {
	if (spec->entry[i].status != REQUEST_ORIGINAL) {
	    status = fdiskAutoInsertPartition( newhdarr, numhd,
					       &spec->entry[i].partition,
					       strcmp(bootpart, 
						       spec->entry[i].name));
	    if (status == FDISK_SUCCESS) {
		spec->entry[i].status = REQUEST_GRANTED;
		if (wecare) spec->entry[i].reason = ALLOC_OK;
	    } else {
		spec->entry[i].status = REQUEST_DENIED;
		if (wecare) spec->entry[i].reason = LastAllocStat;
	    }
	}
    }

    free (bootpart);
    return FDISK_SUCCESS;
}

int fdiskAutoInsertPartitions(HardDrive **hdarr,    unsigned int numhd,
			      HardDrive **newhdarr, 
			      PartitionSpec *spec) {
    return dofdiskAutoInsertPartitions(hdarr, numhd, newhdarr, spec, 1);
}


/*                                                                   */
/* growing routines                                                  */
/* these routines take an existing array for drives and a partspec   */
/* and grow the partitions marked for growth to fill available space */

/* this should work but is VERY messy!!!!! */
/* uses internals of partitions and partitions specs too much */
/* needs to be rewritten to use functions to get/set values   */
int fdiskGrowPartitions( HardDrive **hdarr, unsigned int numhd,
			 HardDrive **newhdarr, 
			 PartitionSpec *spec ) {

    PartitionSpec trialspec, startspec, origspec;
    Partition     *p;
    unsigned int  *freespace, *usedbygrow;
    unsigned int  min, max, cur, dif, ldif;
    int           i, j, k, l;
    int           statcur;
    float         f;
    
    /* copy existing partition spec in a safe place  */
    /* we make new copies of the 'names' field, since its malloc'd */
    memcpy(&startspec, spec, sizeof(PartitionSpec));
    for (j=0; j<spec->num; j++) {
	/* copy the name */
	if (spec->entry[j].name)
	    startspec.entry[j].name = strdup(spec->entry[j].name);

	/* if request was granted, lock partition to drive its on currently */
	if (spec->entry[j].status == REQUEST_GRANTED) {
	    startspec.entry[j].status = REQUEST_PENDING;
	    p = &startspec.entry[j].partition;
	    fdiskDeactivateAllDriveSet( &p->drive );
	    fdiskActivateDriveSet( &p->drive, p->drive.current );
	}	
    }

    /* go thru and weed out the denied partitions */
    for (j=0; j<spec->num; j++)
	if (spec->entry[j].status == REQUEST_DENIED)
	    fdiskDeletePartitionSpec(&startspec, spec->entry[j].name);
	    
    /* make array of freespace left on each drive */
    freespace  = (unsigned int *) alloca(numhd*sizeof(unsigned int));
    usedbygrow = (unsigned int *) alloca(numhd*sizeof(unsigned int));
    for (j=0; j<numhd; j++) {
	freespace[j] = hdarr[j]->totalsectors;
	usedbygrow[j] = 0;
    }
    
    for (j=0; j<startspec.num; j++) {
	p = &startspec.entry[j].partition;
	if (fdiskIsExtended(p->type.current))
	    continue;
	    
	for (k=0; k<numhd; k++)
	    if (hdarr[k]->num == p->drive.current)
		break;

	if (k==numhd) /* shouldnt happen */
	    continue;
	
	freespace[k] -= p->size.current;

	if (!p->size.active || p->size.min == p->size.max || p->immutable)
	    continue;
	usedbygrow[k] += p->size.current;
    }

    /* now grow them */
    memcpy(&trialspec, &startspec, sizeof(PartitionSpec));
    for (j=0; j<startspec.num; j++) {
	p = &startspec.entry[j].partition;
	if (!p->size.active || p->size.min == p->size.max || p->immutable)
	    continue;
	
	for (k=0; k<numhd; k++)
	    if (hdarr[k]->num == p->drive.current)
		break;

	if (k==numhd) /* shouldnt happen */
	    continue;

	/* OK, this is the ugliest binary search ever coded.   */
	/* see if max size is specified and not just us saying */
	/* upper limit for growing is unbounded.               */
	if (p->size.max != p->size.min && p->size.max != FDISK_SIZE_MAX) {
	    max = p->size.max;
	} else {
	  if (usedbygrow[k] != 0)
	    f = ((float)p->size.current/(float)usedbygrow[k]);
	  else
	    f = 0; /* shouldnt happen */
	  
	  /* see if we need to consider users maximum size request */
	  max = p->size.current + (unsigned int)((f*(float)freespace[k])+0.5);
	}

	min = startspec.entry[j].partition.size.min;
	cur = max - (max-min)/2; /* might help with rounding */
	dif = max - min;
	ldif = 0;

	while (min != max && ldif != dif) {
	    trialspec.entry[j].status   = REQUEST_PENDING;
	    trialspec.entry[j].partition.size.min = cur;
	    statcur = dofdiskAutoInsertPartitions(hdarr, numhd, newhdarr, 
					          &trialspec, 0);

	    /* check to see if any partitions got lost, if so thats bad */
	    for (l=0; l<trialspec.num; l++)
		if (trialspec.entry[l].status == REQUEST_DENIED) {
		    statcur = FDISK_ERR_NOFREE;
		    break;
		}
	
	    if (statcur != FDISK_SUCCESS)
		max = cur;
	    else
		min = cur;

	    cur = max - (max-min)/2;
	    ldif = dif;
	    dif  = max - min;

	}

	/* store the final working size in trialspec */
	/* max COULD have failed, so we have to look */
	/* at result of last try with it to see if   */
	/* we should use it instead of min. Min val  */
	/* SHOULD always be a successful size.       */
	trialspec.entry[j].status   = REQUEST_PENDING;
	trialspec.entry[j].partition.size.min =
	    ((statcur == FDISK_SUCCESS) ? cur : min);
    }

    /* we're done, copy into original partition spec and */
    /* insert one last time                              */
    memcpy(&origspec, spec, sizeof(PartitionSpec));
    for (j=0; j<startspec.num; j++) {
	p = &startspec.entry[j].partition;
	if (!p->size.active || p->size.min == p->size.max || p->immutable)
	    continue;

	for (k=0; k<spec->num; k++)
	    if (!strcmp(trialspec.entry[j].name,spec->entry[k].name))
		break;

	if (k == spec->num) /* shouldnt happen */
	    continue;

	spec->entry[k].partition.size.min=trialspec.entry[j].partition.size.min;
	spec->entry[k].status = REQUEST_PENDING;
    }

    dofdiskAutoInsertPartitions(hdarr, numhd, newhdarr, spec, 0);

    /* now put the 'min' size back to what the user wanted */
    for (j=0; j<spec->num; j++) {
	p = &spec->entry[j].partition;
	if (!p->size.active || p->size.min == p->size.max || p->immutable)
	    continue;

	spec->entry[j].partition.size.min=origspec.entry[j].partition.size.min;
    }
	
	
    /* now clean up and leave */
    for (j=0; j<startspec.num; j++)
	free(startspec.entry[j].name);
    
    return FDISK_SUCCESS;
}
    
    
