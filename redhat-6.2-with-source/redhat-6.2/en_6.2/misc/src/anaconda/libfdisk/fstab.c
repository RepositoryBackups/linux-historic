#include <stdlib.h>

#include "fstab.h"

void freeFstabEntry( struct fstabEntry *e ) {
    	if (e->mntpoint) free(e->mntpoint);
	if (e->device)   free(e->device);
	if (e->netPath)  free(e->netPath);
	if (e->netHost)  free(e->netHost);
}

void freeFstab(struct fstab fstab) {
    int i;

    for (i = 0; i < fstab.numEntries; i++) {
	freeFstabEntry( &fstab.entries[i] );
    }

    if (fstab.numEntries) free(fstab.entries);
}

int addFstabEntry(struct fstab * fstab, struct fstabEntry entry) {
    int i;

    for (i = 0; i < fstab->numEntries; i++) 
	if (!strcmp(entry.device, fstab->entries[i].device))
	    break;

    if (i == fstab->numEntries) {
	fstab->numEntries++;
	if (fstab->numEntries > 1)
	    fstab->entries = realloc(fstab->entries, 
				sizeof(entry) * fstab->numEntries);
	else
	    fstab->entries = malloc(sizeof(entry));
    }

    fstab->entries[i] = entry;

    return i;
}

void initFstabEntry(struct fstabEntry * e) {
    e->device = NULL;
    e->netHost = NULL;
    e->netPath = NULL;
    e->mntpoint = NULL;
    e->tagName = NULL;
    e->size = 0;
    e->type = PART_OTHER;
    e->isMounted = 0;
    e->doFormat = 0;
}

static int fstabCmp(const void * a, const void * b) {
    const struct fstabEntry * first = a;
    const struct fstabEntry * second = b;

    if (first->type != second->type) {
	if (first->type == PART_NFS)
	    return 1;
	else if (second->type == PART_NFS)
	    return -1;
    }

    return strcmp(first->mntpoint, second->mntpoint);
}

void fstabSort(struct fstab * fstab) {
    qsort(fstab->entries, fstab->numEntries, 
	  sizeof(*fstab->entries), fstabCmp);
}
