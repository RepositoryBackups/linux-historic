#ifndef FDISK_PARTSPEC_H
#define FDISK_PARTSPEC_H

/* reasons partition wasnt allocated                  */
/*  ALLOC_UNDEF   - reason is currently undefined     */
/*  ALLOC_OK      - it was allocated just fine        */
/*  ALLOC_SIZE    - no room on allowed drives         */
/*  ALLOC_START   - couldnt start at requested sector */
/*  ALLOC_CYL     - couldnt put in requested range    */
/*  ALLOC_DRIVE   - not really used yet               */
/*  ALLOC_FREEPRI - no free primary partitions        */
/*  ALLOC_EXTCRE  - couldnt create extended partition */
/*  ALLOC_FREEPART- no free slots in HardDrive struct */
enum allocReason {ALLOC_UNDEF, ALLOC_OK, ALLOC_SIZE, ALLOC_START, ALLOC_CYL, 
	ALLOC_DRIVE, ALLOC_FREEPRI, ALLOC_EXTCRE, ALLOC_FREEPART };

/* holds the reason latest alloc attempt failed (or didnt) */
extern enum allocReason LastAllocStat;

/* some structures used to abstact partitions even more */
struct partition_spec {
    char                 *name;
    unsigned int         status;
    enum allocReason     reason;
    Partition            partition;
};

#define MAX_PARTITION_SPEC 100
struct all_partition_spec {
    unsigned int              num;
    struct partition_spec     entry[MAX_PARTITION_SPEC];
};

typedef struct partition_spec       PartitionSpecEntry;
typedef struct all_partition_spec   PartitionSpec; 

#define REQUEST_GRANTED   1
#define REQUEST_PENDING   2
#define REQUEST_DENIED    4
#define REQUEST_ORIGINAL  8

char *GetReasonString( enum allocReason reason );
void fdiskSetPartitionLimitsBootable (char *mntpt,
				      HardDrive **hdarr, unsigned int numhd,
				      PartitionSpec *spec );
void fdiskUnsetPartitionLimitsBootable (char *mntpt, PartitionSpec *spec );

char *fdiskBootablePartitionName ( PartitionSpec *spec );
int fdiskHandleSpecialPartitions( HardDrive **, unsigned int numhd,
				  PartitionSpec *spec );
int fdiskGetConstraintPriority( Partition *p );
int fdiskReturnPartitionSpec( PartitionSpec *spec, char *name, Partition **p );
int fdiskIndexPartitionSpec( PartitionSpec *spec, char *name, unsigned int *i);
int fdiskInsertPartitionSpec( PartitionSpec *spec, char *name, Partition *p,
			      unsigned int status);
int fdiskRenamePartitionSpec( PartitionSpec *spec, char *name, char *newname);
int fdiskModifyPartitionSpec( PartitionSpec *spec, char *name, Partition *p,
			      unsigned int status);
int fdiskDeletePartitionSpec( PartitionSpec *spec, char *name );
int fdiskWipePartitionSpec( PartitionSpec *spec );
int fdiskSetupPartitionSpec( HardDrive **hdarr, unsigned int numhd,
			     PartitionSpec *spec );
int fdiskCleanOriginalSpecs( HardDrive **hdarr, unsigned int numhd,
			     PartitionSpec *spec );
int fdiskMakeUniqSpecName( PartitionSpec *spec, char * base, char **s );
int fdiskMakeSwapSpecName( PartitionSpec *spec, char **s );
int fdiskMakeRaidSpecName( PartitionSpec *spec, char **s );
int fdiskSortPartitionSpec( PartitionSpec *spec );
#endif


