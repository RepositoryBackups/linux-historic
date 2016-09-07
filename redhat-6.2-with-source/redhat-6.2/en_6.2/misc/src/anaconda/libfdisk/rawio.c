/* Raw disk i/o handling the partition tables on a hard drive       */
/*                                                                  */
/* Michael Fulbright <msf@redhat.com>  June 1997                    */
/* Recoded to use new HardDrive structure                           */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <malloc.h>
#include <linux/fs.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include "libfdisk.h"
#ifdef __sparc__
#include "sunlabel.h"
#endif

long long llseek(int fd, long long offset,  int whence);


int fdiskSectorToCHS( HardDrive *hd, unsigned int start,
		      unsigned int *c, unsigned int *h, unsigned int *s ) {

    *s = start % hd->geom.sectors + 1;
    start /= hd->geom.sectors;
    *h = start % hd->geom.heads;
    start /= hd->geom.heads;
    *c = start;
    return FDISK_SUCCESS;
}

/* move absolute sector position to the next highest cylinder boundary */
/* HOWEVER, if we are within 2 heads of the previous cylinder we dont  */
/* round up. This is how partitions with partition tables at the front */
/* work (like LP).                                                     */
int fdiskRoundStartToCylinder( HardDrive *hd, unsigned int *start ) {
    unsigned int m;

    m = *start % (hd->geom.heads*hd->geom.sectors);
    if (m > 2*hd->geom.sectors)
	*start += (hd->geom.heads*hd->geom.sectors) - m;
    
    return FDISK_SUCCESS;
}

/* move absolute sector position to the next lowest  cylinder boundary */
int fdiskRoundEndToCylinder( HardDrive *hd, unsigned int *end ) {
    unsigned int m;

    if (*end > hd->geom.heads*hd->geom.sectors) {
	m = (*end % (hd->geom.heads*hd->geom.sectors));
	if (m != hd->geom.heads*hd->geom.sectors-1)
	    *end -= m+1;
    }
    return FDISK_SUCCESS;
}


    
/* these routes return < 0 on system error, errno has error */
/*                       0 on success                       */
/*                     > 0 on other errors                  */
/* open device dev, returns allocated HardDrive structure */
/* num is the booting numbers, ie, IDE drives come first, then SCSI, etc */
/* up to caller to set values                                            */
int fdiskOpenDevice(char *dev, unsigned int num, HardDrive **hddev) {
    int fd;
    struct hd_geometry g;
    HardDrive *hd;
#ifdef __sparc__
    struct sun_disklabel label;
    unsigned short *ush;
    int csum;
#endif

    *hddev = NULL;
    
    /* try to determine everything, if it works then we return */
    /* filled in hd_device struct                              */
#ifdef MSF_PARANOID
    fd = open(dev, O_RDONLY);
#else
    fd = open(dev, O_RDWR);
#endif
    if (fd < 0)
	return -1;

    if (ioctl(fd, HDIO_GETGEO, &g)) {
        close(fd);
	return -1;
    }

    hd = (HardDrive *) malloc(sizeof(HardDrive));
    hd->fd = fd;
    strncpy(hd->name, dev, MAX_HD_NAMELEN);
    hd->geom.heads = g.heads;
    hd->geom.sectors = g.sectors;
    hd->geom.cylinders = g.cylinders;
    hd->geom.start = g.start;
    
#ifdef __sparc__

    /* Sun disklabels use the geometry stored in the valid     */
    /* boot label.					       */

    if (lseek(hd->fd, 0, SEEK_SET)) return -1;
    if (read(hd->fd, &label, sizeof(label)) != sizeof(label)) return -1;
    if (label.magic == SUN_LABEL_MAGIC) {
        ush = ((unsigned short *) (&label + 1)) - 1;
        for (csum = 0; ush >= (unsigned short *) &label;) csum ^= *ush--;
        /*  Checksum != 0, checksum fails */
        if (!csum) {
	    hd->geom.heads = label.ntrks;
	    hd->geom.cylinders = label.ncyl;
   	    hd->geom.sectors = label.nsect;
        }
    }

#endif

    hd->totalsectors = hd->geom.sectors*hd->geom.heads*hd->geom.cylinders;
    
    *hddev = hd;
    
    /* zero out the partition table */
    memset(&hd->table, 0, (MAX_PARTITIONS+1)*sizeof(Partition));
    memset(&hd->eptable, 0, (MAX_PARTITIONS+1)*sizeof(Partition));
    
    /* we initialize the available/unavailable stuff when we read the
       table */
    
    /* no PEP yet! */
    hd->pep = 0;
    
    hd->num = num;
    return FDISK_SUCCESS;
}

/* Note - no error checking (yet) */
int fdiskCloseDevice(HardDrive *hddev) {
    if (hddev->fd >= 0) close(hddev->fd);
    free(hddev);
    return FDISK_SUCCESS;
}

int fdiskCloseDeviceFd(HardDrive * hddev) {
    close(hddev->fd);
    hddev->fd = -1;
    return FDISK_SUCCESS;
}

int fdiskReopenDeviceFd(HardDrive * hddev, char * dev) {
#ifdef MSF_PARANOID
    hddev->fd = open(dev, O_RDONLY);
#else
    hddev->fd = open(dev, O_RDWR);
#endif

    if (hddev->fd < 0) return -1;
    return 0;
}

/* cause kernel to re-read partition table */
int fdiskReReadPartitions( HardDrive *hd ) {
    return ioctl(hd->fd, BLKRRPART);
}

/* these two routines read/write a Partition Table at the location */
/* specified by the caller                                         */
/* returns a value of FDISK_ERR_BADNUM if no table found at loc    */
int fdiskReadPartitionTable(HardDrive *hddev, unsigned int loc,
			     RawPartitionTable **pt ) {
    int i;
    long long offset;
    unsigned int magic;
    unsigned char b[SECTORSIZE];
    RawPartitionTable *t;

    offset = (long long) loc * (long long) SECTORSIZE;
    if (llseek(hddev->fd, offset, SEEK_SET) < 0)
	return -1;

    if (read(hddev->fd, b, SECTORSIZE) != SECTORSIZE)
	return -1;

    /* see if we really got something */
    magic = b[PARTMAGOFF]*256 + b[PARTMAGOFF+1];
    if (magic != PARTMAGIC)
	return FDISK_ERR_BADMAGIC;

    t = (RawPartitionTable *) malloc(sizeof(RawPartitionTable));
    
    for (i=0; i<4; i++)
	memcpy(  &t->entry[i],
		 b+PARTTBLOFF+i*sizeof(RawPartition),
		 sizeof(RawPartition));
    *pt = t;
    return FDISK_SUCCESS;
}

/* we read the current sector and modify it, in case there is some */
/* boot code there we need to preserve                             */
/* not sure this is what we want to ALWAYS do                      */
/* accepts location as the SECTOR OFFSET!!!                        */
int fdiskWritePartitionTable(HardDrive *hddev, unsigned int loc,
			     RawPartitionTable *pt ) {
    int i;
    long long offset;
    unsigned char b[SECTORSIZE];

#ifdef REALLY_DEBUG
    printf("seeking to %d and writing partition table\n",loc);
    print_raw_partition_table(0, pt);
#endif
    offset = (long long) loc * (long long) SECTORSIZE;
    if (llseek(hddev->fd, offset, SEEK_SET) < 0)
	return -1;

    if (read(hddev->fd, b, SECTORSIZE) != SECTORSIZE)
	return -1;

    /* move partition data into the sector */
    for (i=0; i<4; i++)
	memcpy(  b+PARTTBLOFF+i*sizeof(RawPartition),
		 &pt->entry[i],
		 sizeof(RawPartition));

    /* stick the magic on there */
    b[PARTMAGOFF   ] = PARTMAGIC >> 8;
    b[PARTMAGOFF+1 ] = PARTMAGIC & 0xff;
    
    if (llseek(hddev->fd, offset, SEEK_SET) < 0)
	return -1;

    if (write(hddev->fd, b, SECTORSIZE) != SECTORSIZE)
	return -1;

    return FDISK_SUCCESS;
}


int fdiskZeroMBR( HardDrive *hddev ) {
    RawPartitionTable pt;

    memset(&pt, 0, sizeof(RawPartitionTable));

    return fdiskWritePartitionTable( hddev, 0, &pt );
}
	    

