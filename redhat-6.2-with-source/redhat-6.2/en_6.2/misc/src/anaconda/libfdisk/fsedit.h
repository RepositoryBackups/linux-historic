#ifndef H_FSEDIT
#define H_FSEDIT

#include "libfdisk.h"
#include <libintl.h>
#include <locale.h>
#ifndef _
# define _(a) gettext(a)
#endif

#define MAX_HARDDRIVES  16

/* need to move somewhere else eventually! */
/* mostly gleamed from fdisk.[ch]          */
struct parttypes {
      unsigned int index;
      char *name;
};

extern struct parttypes allparttypes[];
extern int nparttypes;

struct attemptedPartition {
    char * mount;		/* NULL for the last in the list */
    int size;			/* size in megs */
    int maxSize;
    int type;
    int grow;
    int start;
    char * device;
    int partNum;
};

typedef struct partitionList_t {
    char *device;
    char * name;
    int type;
    int start;
    int size;
} partitionList;

typedef enum {
     RAID0 = 0, RAID1 = 1, RAID5 = 5,
} RaidType;

struct raidInstance {
    char            *mntpt;
    char            *device;
    RaidType        type;
    uint	    parttype;
    char **	    devices;
};

#include "fstab.h"

typedef struct _FseditContext {
    HardDrive     *prestinehdarr[MAX_HARDDRIVES]; /* untouched original HD */
    HardDrive     *hdarr[MAX_HARDDRIVES]; /* modified ORIGINAL HD */
    HardDrive     *newhdarr[MAX_HARDDRIVES]; /* mod orig + new HD */

    PartitionSpec spec;
    struct repartitionInfo * repartInfo;
    struct fstab fstab;
    int numhd;
    int flags;
    struct {
	void * function;
	void * data;
    } cbi;

    /* This is filled in by exitFsedit *ONLY*. It DOES NOT keep state
       information for the GUI. Updating this field only will do
       ABSOLUTELY no good. */
    struct raidInstance * raid;
} FseditContext;

int validLoopbackSetup(PartitionSpec * spec);
int FSFreeState(void * partList);
int FSWritePartitions(char ** drives, void * partList);
int FSEditPartitions(FseditContext *context);
int newFseditContext (char ** drives,
		      struct fstab * fstab,
		      int flags,
		      FseditContext ** new);
void delFseditContext (FseditContext * context);
void exitFsedit (FseditContext *context, struct raidInstance * raidInfo);
partitionList * FSPartitionList(FseditContext *context);
void FSEditInitialize(FseditContext * context);

/*  int guessAtPartitioning(HardDrive ** hdarr, int numhd, */
/*  			       PartitionSpec * spec, int * runDruid, */
/*  			       int dir, int flags, */
/*  			       struct attemptedPartition * goals); */
int guessAtPartitioning(FseditContext *context, int flags, char *name,
			struct attemptedPartition * goals,
			int interactive);

int tryGoal(HardDrive ** hdarr, HardDrive ** tmphdarr, int numhd, 
		   PartitionSpec * spec, struct attemptedPartition * goals);


#define SUNPARTTYPE    0x100
#define NONSUNPARTTYPE 0x200

#define SECPERMEG 2048

#define INST_CANCEL -1
#define INST_OKAY 0
#define INST_ERROR 2

#define FSEDIT_READONLY			(1 << 0)
#define FSEDIT_CLEARLINUX		(1 << 1)
#define FSEDIT_CLEARALL			(1 << 2)
#define FSEDIT_USEEXISTING		(1 << 3)
#define FSEDIT_TESTING			(1 << 4)
#define FSEDIT_AUTOCLEARMBR		(1 << 5)
#define FSEDIT_IGNOREFAILEDDRIVES 	(1 << 6)


#endif
