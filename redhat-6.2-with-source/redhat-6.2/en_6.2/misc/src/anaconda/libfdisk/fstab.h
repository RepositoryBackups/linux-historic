#ifndef H_FSTAB
#define H_FSTAB

#define PART_EXT2	(1 << 0)
#define PART_SWAP	(1 << 1)
#define PART_DOS	(1 << 2)
#define PART_HPFS	(1 << 3)
#define PART_NFS	(1 << 4)
#define PART_FAT32	(1 << 5)
#define PART_UFS        (1 << 6)
#define PART_OTHER	(1 << 7)
#define PART_IGNORE	(1 << 8)

struct fstabEntry {
    char * device;		/* malloced! */
    char * netHost;		/* malloced! */
    char * netPath;		/* malloced! */
    char * mntpoint;		/* malloced! */
    char * tagName;
    int size;
    unsigned int type;
    int isMounted;
    int doFormat;
}; 

struct fstab {
    struct fstabEntry * entries;
    int numEntries;
};


void fstabSort(struct fstab * fstab);
void initFstabEntry(struct fstabEntry * e);
int addFstabEntry(struct fstab * fstab, struct fstabEntry entry);
void freeFstab(struct fstab fstab);

#endif
