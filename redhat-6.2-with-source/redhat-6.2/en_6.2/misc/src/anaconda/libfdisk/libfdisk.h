#ifndef LIB_FDISK_H
#define LIB_FDISK_H

/* Function prototypes and data type definitions for the hard drive */
/* partitioning library.                                            */
/*                                                                  */
/* Michael Fulbright <msf@redhat.com>  June 1997                    */


/* some includes we'll need for our data types */
#include <linux/hdreg.h>
#include <limits.h>

/* stuff to handle errors */
extern int fdisk_nerr;
extern char *fdisk_errlist[];

/* internal defines */

#include "constraints.h"

/* some useful defines */
#define SECTORSIZE 512

/* offsets within the sector containing partition table of different values */
#define PARTTBLOFF 446    /* start of partition table entries */
#define PARTMAGOFF 510    /* offset of magic bytes */
#define PARTMAGIC  0x55aa /* magic */

/* partition types */
#define UNALLOCATED_PARTITION                   0x00
#define DOS_PRIMARY_lt32MEG_PARTITION           0x04
#define DOS_EXTENDED_PARTITION                  0x05
#define DOS_PRIMARY_gt32MEG_PARTITION           0x06
#define WIN_VFAT32_PARTITION                    0x0b
#define WIN_VFAT32_LBA_PARTITION                0x0c
#define WIN98_EXTENDED_PARTITION                0x0f
#define LINUX_LEGACY_PARTITION                  0x81
#define LINUX_SWAP_PARTITION                    0x82
#define LINUX_NATIVE_PARTITION                  0x83
#define LINUX_EXTENDED_PARTITION                0x85
#define LINUX_RAID_PARTITION                    0xfd

/* use > 255 values for non-physical partitions (remote) */
#define NFS_REMOTE_PARTITION                    0x1000

/* some libfdisk errors */
#define FDISK_ERR_SYSTEM      -1       /* system error, use errno       */
#define FDISK_SUCCESS          0       /* Good job no problems          */
#define FDISK_ERR_USERABORT    1       /* user aborted                  */

#define FDISK_ERR_BADNUM      10       /* bad numeric value passed      */
#define FDISK_ERR_BADPTR      11       /* bad ptr value passed          */
#define FDISK_ERR_BADMAGIC    12       /* partition table has bad magic */

#define FDISK_ERR_TWOEXT      15       /* two extended partitions!      */
#define FDISK_ERR_TWOLOG      16       /* two LP per EPT!               */
#define FDISK_ERR_CORRUPT     17       /* something in PT is corrupt    */

#define FDISK_ERR_NOFREE      20       /* unable to allocate a resource */
#define FDISK_ERR_NOFREEPRIM  21       /* unable to allocate a primary  */
#define FDISK_ERR_NOFREEEXT   22       /* unable to allocate a extended */
#define FDISK_ERR_INUSE       23       /* resource currently in use     */
#define FDISK_ERR_NOPEP       24       /* No Primary Extended Partition */
#define FDISK_ERR_CNSTRFAIL   30       /* constraint(s) failed          */

/* Limits for each constraint type (some like 'type' doesnt make sense) */
#define FDISK_START_MIN            0
#define FDISK_START_MAX     UINT_MAX
#define FDISK_SIZE_MIN             1
#define FDISK_SIZE_MAX      UINT_MAX
#define FDISK_ENDCYL_MIN           0
#define FDISK_ENDCYL_MAX    UINT_MAX
#define FDISK_OFFSET_MIN           0
#define FDISK_OFFSET_MAX    UNIT_MAX
#define FDISK_DRIVE_MIN            0
#define FDISK_DRIVE_MAX           64

/* types of partitions, used by allocation routines internally */
#define PRIMARY           1
#define LOGICAL           2
#define PRIMARY_EXTENDED  4
   
/* some low-level data types */
/* these are used only to manipulate sectors read off the disk */
/* ASAP it is converted from this form into a higher level form*/
/* which is easier to manipulate                               */
struct raw_PartitionEntry {
    unsigned char active;
    unsigned char start_head;
    unsigned char start_sec;
    unsigned char start_cyl;
    unsigned char type;
    unsigned char end_head;
    unsigned char end_sec;
    unsigned char end_cyl;
    unsigned int  start;
    unsigned int  size;
};

struct raw_PartitionTable {
    struct raw_PartitionEntry entry[4];
};

/* make these easier to type! */
typedef struct raw_PartitionTable  RawPartitionTable;
typedef struct raw_PartitionEntry  RawPartition;

/* THESE ARE THE NEW HIGH LEVEL STRUCTURES */
/* IN PROCESS OF RECODING                  */

/*                                                                  */
/* A partition defines a portion of the disk                        */
/*                                                                  */
/* Partitions have various attributes:                              */
/*     size & start -> define region of space used                  */
/*     offset       -> defined start of data inside partition       */
/*                     used for logical partititons in extended     */
/*     type         -> defines which OS(s) are using partition      */
/*     active       -> can we boot from the partition. used by DOS  */
/*     num          -> Partition #, start at 1. Logical start at 5  */
/*     drive        -> Which drive can/is the partition on          */
/*     status       -> Various flags used when allocating partitions*/
/*     immutable    -> If non-zero, do nothing to disturb any of the*/
/*                     above values. Used to preserve existing part.*/
/*                                                                  */
/* Partitions DO NOT have to actually define a space on disk used by*/
/* an OS! Can be used (internally by this code) to represent a region*/
/* which is free for allocation, but cannot be moved. Usually this  */
/* code tries to rearrange partitions to keep free space in one big */
/* contiguous region. Extended/logical partitions, because of how   */
/* they work, can't always be moved around. So it is possible to    */
/* have a extended/logical partition pair which exists but is not   */
/* allocated to a partition type other than 0. This space is prime  */
/* for allocating to the next requested partition.                  */
/*                                                                  */

struct fdisk_partition {
    Constraint     num;        /* for /dev/hda1, num = 1 */
    Constraint     start;      /* start sector, from start of drive */
    Constraint     size;       /* size in sectors */
    Constraint     endcyl;     /* ending cylinder, exclusive w start*/
    Constraint     offset;
    Constraint     type;
    Constraint     active;
    DriveSet       drive;      /* current driveset constraint */
    DriveSet       userdrive;  /* user specified constraint */
                               /* can differ if we force constraints */
                               /* one a bootable partition. Store */
                               /* user preference in case change partition */
                               /* mount point and it is no longer bootable */
    unsigned int   userdriveflag; /* if non-zero userdrive is storing */
                                  /* user preference since we're overriding */
                                  /* for bootable partition */


    unsigned int   status;     /* used to store info for allocating */
    unsigned int   immutable;  /* if non-zero, do not touch anything*/

};

typedef struct fdisk_partition                Partition;


/* status values */
/* UNAVAILABLE means work has to be done to use it (like make an extended) */
/* UNUSABLE means there is no way to map this space                        */
#define AVAILABLE         0
#define UNAVAILABLE       1
#define ALLOCATED         2
#define UNUSABLE          4


/*                                                                          */
/* A hard drive describes a collection of partitions                        */
/* Each hard drive has several physical characteristics which can affect    */
/* the attributes that a partition can contain:                             */
/*                                                                          */
/*    geometry -> sectors/heads/cylinders of drive                          */
/*                partitions have to start and end on cylinder boundaries   */
/*                                                                          */
/*    name     -> something like "/dev/hda1"                                */
/*                                                                          */
/*    fd       -> the file descriptor being used to access this drive       */
/*                                                                          */
/*    pep      -> slot number of the Primary Extended Partion, 0 if none    */
/*                                                                          */
/*    table    -> array of partition descriptors, indexed by the number     */
/*                of the partition. Primary partitions range in number from */
/*                1 to 4, while logical partitions start at number 5.       */
/*                You can have any or all of the primaries defined, but the */
/*                logicals always start with 5 and increment upwards. If you*/
/*                remove a logical then all the other logicals get renumbred*/
/*                Note that sometimes you can remove a logical but the      */
/*                associated extended partition DOES NOT get removed. This  */
/*                can happen when a logical partition down the chain of     */
/*                extended partitions is immutable.                         */
/*                                                                          */
/*    eptable  -> array of extended partition descriptors, indexed by the   */
/*                number of the logical partition which it encapsulates.    */
/*                                                                          */
/* The partition table describes existing partitions. Entries 1 thru 4 are  */
/* for the primary partitions (PPs). At most 1 of these can be an           */
/* extended partition (EP). The primary extended partition (PEP) describes  */
/* the chunk of the disk in which ALL other logical partitions (LPs) can be.*/
/* Each LP has an associated EP. At the start of each EP is an extended     */
/* partition table (EPT). It is possible that an EPT could have up to 4 LPs */
/* defined within a given EP, but we (and all other partitioning software I */
/* can find) limit you to 1 LP per EP.  To make more LP within the space    */
/* described by the PEP, you make an entry in the EPT describing another EP.*/
/* These EPT make a chain, starting with the PEP and working down. The first*/
/* link on this chain gets a partition number of 5, and the number goes up  */
/* by one for each link down the chain.                                     */
/*                                                                          */
/* Deleting a PP involved just setting its size and type to 0. However,     */
/* because of the way the EPT chain works, deleting anything but the tail   */
/* link in the EPT chain can have numerous consequences.                    */
/* Say you have LPs 5 thru 7. You delete LP 5. The EPT for LP 6 now has to  */
/* move to the location of the EPT for LP 5, since LP 6 has now become LP 5.*/
/* The old EPT for LP 6 is not used anymore, but LP 6 (now LP 5) will occupy*/
/* at the exact same space on disk. The write routine which takes the       */
/* HardDrive structure and writes it to the disk must be aware of these     */
/* facts so that EPT and LP stay in the same places as read in.             */
/* This routine will write LP 5 first, then 6, etc etc, linking each with   */
/* the following. At some point it would be nice if the automatic allocation*/
/* routines could straighten the EPT chain out.                             */
/*                                                                          */
/* We mark a partition table entry as available by:                         */
/*     setting its size and type to 0                                       */
/*     settting its status to AVAILABLE <- use this for testing status      */
/* We mark a partition used by:                                             */
/*     setting its size and type to != 0                                    */
/*     setting its status to ALLOCATED  <- use this for testing status      */

#if 0
#define FDISK_MAX_PRIMARY 8
#define FDISK_MAX_PRIMARY 4
#endif

struct hdLimits {
    int maxPrimary;
    int maxPartitions;
    int skipPartition; /* -1 means don't skip anything */
};

typedef struct {
       unsigned int heads;
       unsigned int sectors;
       unsigned int cylinders;
       unsigned long start;
} HDGeometry;


#define MAX_HD_NAMELEN 128
#define MAX_PARTITIONS   16
struct hd_device {
    char                            name[MAX_HD_NAMELEN];
    char                            prefix[MAX_HD_NAMELEN];
    unsigned int                    num; /* 'BIOS' number of drive */
    HDGeometry                      geom;
    unsigned int                    totalsectors;
    int                             fd;
    unsigned int                    pep;
    Partition                       table[MAX_PARTITIONS+1];
    Partition                       eptable[MAX_PARTITIONS+1];
    struct hdLimits		    limits;
    int 			    (*write_f)
					(struct hd_device *hd);
#define FDISK_PART_TYPE_DOS            0
#define FDISK_PART_TYPE_SUN            1
#define FDISK_PART_TYPE_BSD            2
    int                                    part_type;
};

typedef struct hd_device           HardDrive;




/* functions everyone should see */
#include "rawio.h"
#include "primary.h"
#include "extended.h"
#include "partition.h"
#include "partspec.h"
#include "alloc.h"
#include "fstab.h"
#include "fsedit.h"

char * nstrdup(const char * foo);
void TrimWhitespace( char *s );
int badMountPoint(unsigned int type, char * item);
int DisksChanged( HardDrive **oldhd, HardDrive **newhd, unsigned int numhd );
int doDeletePartitionSpec( HardDrive **hdarr, unsigned int numhd,
			 PartitionSpec *spec, PartitionSpecEntry *entry);
int PartitionSpecToFstab( HardDrive **hdarr, int numhd,
			  PartitionSpec *spec, struct fstab *fstab );
int MergeFstabEntries( HardDrive **hdarr, int numhd,
			      PartitionSpec *spec, struct fstab *fstab );
struct fstab copyRemoteFSFstab(struct fstab * fstab);
void MergeRemoteFSFstab(struct fstab *oldfstab, struct fstab *newfstab);
int deletePartitionClass(HardDrive ** hd, int numhd, 
				PartitionSpec * spec, int justLinux);

#endif
