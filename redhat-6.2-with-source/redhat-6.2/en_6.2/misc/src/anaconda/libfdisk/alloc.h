#ifndef FDISK_ALLOC_H
#define FDISK_ALLOC_H

/* space map structs */

struct spacemapentry {
    unsigned int start;
    unsigned int size;
};

struct spacemap {
    unsigned int               len;
    unsigned int               num;
    struct spacemapentry       *entry;
};

typedef struct spacemapentry SpaceMapEntry;
typedef struct spacemap      SpaceMap;

/* number of entries to allocate at a time */
#define SpaceMapChunk 8


int fdiskSpaceMapInit( SpaceMap **map );

int fdiskSpaceMapKrunch( SpaceMap *map, unsigned int fuzz );

int fdiskSpaceMapFree( SpaceMap *map );

int fdiskSpaceMapAdd( SpaceMap *map, SpaceMapEntry *entry, unsigned int fuzz );

int fdiskSpaceMapDel( SpaceMap *map, unsigned int n );

int fdiskUsedMapGen( HardDrive *hd, SpaceMap **map );

int fdiskFreeMapGen( HardDrive *hd, SpaceMap **map );

int fdiskAutoInsertPartition(HardDrive **hdar, unsigned int nhd, Partition *p,
			     int useBiggestChunk);

int fdiskAutoInsertPartitions(HardDrive **hdar, unsigned int nhd, 
			      HardDrive **newhdar, PartitionSpec *spec);

int fdiskGrowPartitions(HardDrive **hdar, unsigned int nhd,
			     HardDrive **newhdar, PartitionSpec *spec);
unsigned long long fdiskMaxSwap( void );

#ifdef __sparc__
/* Fixme: Put this into some better header */
int fdiskIsSparc64;
#endif

#endif

	
