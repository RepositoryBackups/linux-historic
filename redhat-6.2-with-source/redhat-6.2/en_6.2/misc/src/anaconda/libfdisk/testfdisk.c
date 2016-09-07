/* test of the libfdisk library */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "libfdisk.h"



/* clean up that string */
void TrimWhitespace( char *s ) {
    char *f, *l, *p, *q;

    if (!(*s))
        return;

    for (f=s; *f && isspace(*f); f++) ;

    if (!*f) {
        *s = '\0';
        return;
    }

    for (l=f+strlen(f)-1; isspace(*l) ; l--)
    *l = '\0';

    q = s, p = f;
    while (*p)
        *q++ = *p++;

    *q = '\0';
} 

int ReadUnsigned( unsigned int *i ) {
    char num[12];
    char *eptr;
    
    fgets(num,12,stdin);
    num[strlen(num)-1] = 0; /* blow the newline */
    *i=strtol(num,&eptr,10);

    if (eptr != num && *eptr == 0)
	return 0;
    else
	return FDISK_ERR_USERABORT;
}

void ReadString( char **p ) {
    char num[100];
    
    fgets(num,100,stdin);
    num[strlen(num)-1] = 0; /* blow the newline */

    /* cleanup front, end whitespace */
    TrimWhitespace( num );
    
    *p = (char *) malloc(strlen(num)+1);
    strcpy(*p, num);
}

int ReadBoolean( void ) {
    char *str;
    int  ret;
    
    ReadString(&str);
    ret = (*str == 'Y' || *str == 'y');
    free(str);
    return ret;
}
    
int ReadConstraint( char *name, Constraint *c, int required ) {
    unsigned int v;

    printf("Requesting constraint for the parameter \"%s\":\n", name);
    if (required)
	printf("%s -> min value [required     ] :", name);
    else
	printf("%s -> min value [Enter to skip] :", name);
    
    if (ReadUnsigned(&v)) {
	c->active  = 0;
	c->min     = 0;
	c->max     = 0;
	c->current = 0;
	return 1;
    }

    c->min     = v;
    c->active  = 1;

#if 0
    printf("%s -> max value [Enter sets to min] :", name);
    if (ReadUnsigned(&v))
	c->max     = c->min;
    else
	c->max     = v;
#endif

    return 0;
}

void print_raw_partition_table_head( void ) {
    printf("Nr AF  Hd Sec  Cyl  Hd Sec  Cyl   Start      Size    ID\n");
}

void print_raw_partition_table(int num, RawPartitionTable *pt) {
    int s_cyl, s_head, s_sector;
    int e_cyl, e_head, e_sector;
    unsigned int i;

    RawPartition *p;
    
    print_raw_partition_table_head();

    for (i=0; i<4; i++) {
	p = &pt->entry[i];
	
	s_head    = p->start_head;
	s_sector  = p->start_sec & 0x3f;
	s_cyl     = ((unsigned int)(p->start_sec & 0xc0) << 2) + p->start_cyl;
	
	e_head    = p->end_head;
	e_sector  = p->end_sec & 0x3f;
	e_cyl     = ((unsigned int)(p->end_sec & 0xc0) << 2) + p->end_cyl;
	
	printf("%2d %2x %3d %3d %4d %3d %3d %4d %7d   %7d    %2x\n",
	       i+1, p->active, s_head, s_sector, s_cyl,
	       e_head, e_sector, e_cyl, p->start, p->size,
	       p->type);
    }
}

void print_user_partition_table(HardDrive *hd) {
    int i;
    Partition *up;
    unsigned int first, last, end;
    unsigned int s_cyl, s_hd, s_sec, e_cyl, e_hd, e_sec;

    
    printf("Nr AF     Start C/H/S     End C/H/S      Start      Size    ID\n");

    /* print primary partitions first */
    fdiskFirstPartition( hd, &first );
    fdiskLastPartition( hd, &last );
    for (i=first; i<=last; i++) {
	up = &hd->table[i];
	if (!(up->size.current) || !(up->type.current))
	    continue;

	fdiskSectorToCHS( hd, up->start.current, &s_cyl, &s_hd, &s_sec );
	end = up->start.current + up->size.current - 1;
	fdiskSectorToCHS( hd, end, &e_cyl, &e_hd, &e_sec );
	printf("%2d %2x   [%4d/%3d/%3d]  [%4d/%3d/%3d] %7d   %7d    %2x\n",
	       up->num.current,
	       up->active.current,
	       s_cyl, s_hd, s_sec,
	       e_cyl, e_hd, e_sec,
	       up->start.current,
	       up->size.current,
	       up->type.current);
    }

}
void print_user_extended_partition_table(HardDrive *hd) {
    int i;
    Partition *up;
    unsigned int last, end;
    unsigned int s_cyl, s_hd, s_sec, e_cyl, e_hd, e_sec;

    printf("Nr AF     Start C/H/S     End C/H/S      Start      Size    ID\n");
    if (!hd->pep) {
	printf(" No  Extended Partitions Present\n");
	return;
    }
	
    /* print extended partitions only */
    fdiskLastPartition( hd, &last );
    for (i=hd->pep; i<=last; i++) {
	if (i!=hd->pep)
	    up = &hd->eptable[i];
	else
	    up = &hd->table[i];
	
	if (!(up->size.current) || !(up->type.current))
	    continue;

	fdiskSectorToCHS( hd, up->start.current, &s_cyl, &s_hd, &s_sec );
	end = up->start.current + up->size.current - 1;
	fdiskSectorToCHS( hd, end, &e_cyl, &e_hd, &e_sec );
	printf("%2d %2x   [%4d/%3d/%3d]  [%4d/%3d/%3d] %7d   %7d    %2x\n",
	       up->num.current,
	       up->active.current,
	       s_cyl, s_hd, s_sec,
	       e_cyl, e_hd, e_sec,
	       up->start.current,
	       up->size.current,
	       up->type.current);

	/* make sure we move to first extended/logical pair */
	if (i==hd->pep)
	    i = 4;
    }
}


int print_used_spacemap( HardDrive *hd ) {
    unsigned int i;
    SpaceMap *map;
    
    fdiskUsedMapGen( hd, &map );

    printf("Number           Start        End         Size\n");
    for (i=0; i < map->num; i++)
	printf("   %d        %7d       %7d       %7d\n",
	       i,
	       map->entry[i].start,
	       map->entry[i].start+map->entry[i].size,
	       map->entry[i].size);

    fdiskSpaceMapFree( map );

    return 0;
}

int print_free_spacemap( HardDrive *hd ) {
    unsigned int i;
    SpaceMap *map;
    
    fdiskFreeMapGen( hd, &map );

    printf("Number           Start        End         Size\n");
    for (i=0; i < map->num; i++)
	printf("   %d        %7d       %7d       %7d\n",
	       i,
	       map->entry[i].start,
	       map->entry[i].start+map->entry[i].size,
	       map->entry[i].size);

    fdiskSpaceMapFree( map );

    return 0;
}

void print_partitionspec( PartitionSpec *spec ) {
    unsigned int j;
    unsigned int drive;
    Partition   *p;
    
    printf("Requested partitions: \n\n");
    printf("Mount Point  Num    Drive            Size          Endcyl     Weight   Status\n");
    printf("-----------  ---  ----------    --------------   ----------   ------  ---------\n");

    for (j=0; j<spec->num; j++) {
	p = &spec->entry[j].partition;
	printf("%11s  ", spec->entry[j].name);

	printf("%2d   ",p->num.current);
	
	if (fdiskDriveSetIsActive(&p->drive)) {
	    fdiskInitWalkDriveSet(&p->drive,&drive);
	    printf("%2d", drive);
	    do {
		printf(",%2d", drive);
	    } while (fdiskWalkDriveSet(&p->drive,&drive)==FDISK_SUCCESS);
	} else
	    printf("Any Drive  ");

	if (p->size.active) {
	    printf("%8d", p->size.min);
	    if (p->size.max != p->size.min)
		printf("+");
	    else
		printf(" ");
	    printf("/%8d", p->size.current);
	}else
	    printf("       Not Active      ");

	if (p->endcyl.active)
	    printf("  <= %5d  ", p->endcyl.max);
	else
	    printf("  Not Active");

	printf("   %5d    ", fdiskGetConstraintPriority( p ));

	if (spec->entry[j].status == REQUEST_PENDING)
	    printf("PENDING");
	else if (spec->entry[j].status == REQUEST_DENIED)
	    printf("DENIED");
	else if (spec->entry[j].status == REQUEST_GRANTED)
	    printf("GRANTED");
	else if (spec->entry[j].status == REQUEST_ORIGINAL)
	    printf("ORIGINAL");
	else
	    printf("UNKNOWN");

	printf("\n");
    }
}

/* get user params and add a partition to the HardDrive hd     */
/* new partition table stored on HardDrive nhd, and partition  */
/* is added to the PartitionSpec spec                          */
int TestAddPartition( HardDrive **hdarr, unsigned int numhd,
		      HardDrive **nhdarr, PartitionSpec *spec ) {
    int status;
    int num;
    unsigned int temp;
    char *name;
    Constraint tempc;
    Partition p;

    /* we read in a set of constaints for the desired partition */
    /* from the user, then use the auto-allocation routines to  */
    /* insert it into the current partition table.              */
    /*                                                          */
    num = spec->num;
    if (num >= MAX_PARTITION_SPEC)
	return FDISK_ERR_NOFREE;

    /* start from scratch */
    memset(&p, 0, sizeof(Partition));

    /* get the desired name */
    printf("Enter the mount point of the partition -> ");
    ReadString( &name );
    if (!*name)
	return FDISK_ERR_NOFREE;

    /* We REQUIRE a size */
    if (ReadConstraint("Size [in kilobytes]", &tempc, 1 )) {
	printf("User aborted add.\n");
	free(name);
	return FDISK_ERR_USERABORT; /* they aborted */
    }
    temp = tempc.min*2;

    printf("Should partition be allowed to grow? ");
    if (!ReadBoolean())
	fdiskSetConstraint(&p.size, 0, temp, temp, 1);
    else
	fdiskSetConstraint(&p.size, 0, temp, FDISK_SIZE_MAX, 1);
    
    /* We REQUIRE a type */
    if (ReadConstraint("Type", &tempc, 1 )) {
	printf("User aborted add.\n");
	free(name);
	return FDISK_ERR_USERABORT; /* they aborted */
    }
    temp = tempc.min;
    fdiskSetConstraint(&p.type, temp, temp, temp, 1);

    status = fdiskInsertPartitionSpec( spec, name, &p, REQUEST_PENDING );

    if (status == FDISK_SUCCESS) {
	status = fdiskAutoInsertPartitions( hdarr, numhd, nhdarr, spec );
	printf("Status of add was %d\n",status);
    }

    free(name);
    return status;
}

/* edit partition specs, refresh table          */
int TestEditPartition( HardDrive **hdarr, unsigned int numhd,
		       HardDrive **nhdarr, PartitionSpec *spec ) {
    int status;
    int num;
    unsigned int temp;
    char *name;
    Constraint tempc;
    Partition *p;

    if ((num=spec->num) == 0)
	return FDISK_ERR_BADNUM;

    /* which should we change? */
    printf("Enter mount point to edit [q to exit] ->");
    ReadString( &name );
    
    if (!*name)
	return FDISK_ERR_USERABORT;

    status = fdiskReturnPartitionSpec( spec, name, &p );
    if (status != FDISK_SUCCESS)
	return FDISK_ERR_BADNUM;

    if (p->immutable){
	free(p);
	return FDISK_ERR_BADNUM;
    }
    
    /* let them them change size or type or growability */
    /* We REQUIRE a size */
    if (!ReadConstraint("Size [in kilobytes]", &tempc, 1 )) {
	temp = tempc.min*2;
    }

    printf("Should partition be allowed to grow? ");
    if (!ReadBoolean())
	fdiskSetConstraint(&p->size, 0, temp, temp, 1);
    else
	fdiskSetConstraint(&p->size, 0, temp, FDISK_SIZE_MAX, 1);
    
    /* We REQUIRE a type */
    if (!ReadConstraint("Type", &tempc, 1 )) {
	temp = tempc.min;
	fdiskSetConstraint(&p->type, temp, temp, temp, 1);
    }

    status = fdiskModifyPartitionSpec( spec, name, p, REQUEST_PENDING );
    
    if (status == FDISK_SUCCESS) {
	status = fdiskAutoInsertPartitions( hdarr, numhd, nhdarr, spec );
	printf("Status of add was %d\n",status);
    }

    free(name);
    free(p);
    return status;
}


/* delete partition specs, refresh table          */
int TestDeletePartition( HardDrive **hdarr, unsigned int numhd,
			 HardDrive **nhdarr, PartitionSpec *spec ) {
    int status;
    int num;
    unsigned int c, l, m;
    char *name;
    Partition *p;

    if ((num=spec->num) == 0)
	return FDISK_ERR_BADNUM;

    /* which should we change? */
    printf("Enter mount point to delete [q to exit] -> ");
    ReadString( &name );
    
    if (!*name)
	return FDISK_ERR_USERABORT;

    status = fdiskReturnPartitionSpec( spec, name, &p );
    if (status != FDISK_SUCCESS)
	return FDISK_ERR_BADNUM;

    if (p->immutable) {
	printf("Deleting a immutable partition (ok it sounds funny haha).\n");
	fdiskGetCurrentConstraint(&p->num, &c);
	fdiskGetCurrentDriveSet(&p->drive, &l);
	for (m=0; m<numhd; m++)
	    if (hdarr[m]->num == l)
		break;
	
	fdiskRemovePartition(hdarr[m], c);

	/* make it so we can delete this partition now */
	p->immutable = 0;
	fdiskModifyPartitionSpec( spec, name, p, REQUEST_PENDING );
    }
    
    status = fdiskDeletePartitionSpec( spec, name );
    
    if (status == FDISK_SUCCESS) {
	status = fdiskAutoInsertPartitions( hdarr, numhd, nhdarr, spec );
	printf("Status of add was %d\n",status);
    }

    free(name);
    free(p);

    fdiskCleanOriginalSpecs( hdarr, numhd, spec );
    return status;
}


int main(int argc, char **argv) {
    int status;
    int refresh;
    int done;
    
    HardDrive       *ohd;          /* stores the original configuration    */
    HardDrive       *hd;           /* stores current immutable partitions  */
    HardDrive       *nhd;          /* stores current snapshot of everything*/
    HardDrive       *thd;          /* tempoary snapshot used in updates    */
    PartitionSpec   spec;          /* stores all the requested partitions  */

    if (argc < 2) {
	fprintf(stderr, "Need to give a device name as argument.\n");
	exit(1);
    }
    
    status=fdiskOpenDevice(argv[1], 1, &ohd);
    if (status < 0) {
	perror("fdiskOpenDevice");
	exit(1);
    } else if (status > 0) {
	fprintf(stderr,"fdiskOpendevice returns error %d\n",status);
	exit(1);
    } else
	fprintf(stderr,"fdiskOpenDevice returns successfully\n");
	

    status=fdiskReadPartitions(ohd);
    if (status < 0) {
	perror("fdiskReadPartitions");
	exit(1);
    } else if (status > 0) {
	fprintf(stderr,"fdiskReadPartitions returns error %d\n",status);
	exit(1);
    } else
	fprintf(stderr,"fdiskReadPartitions returns successfully.\n");

    /* copy harddrive data into working version */
    hd = (HardDrive *) alloca(sizeof(HardDrive));
    nhd = (HardDrive *) alloca(sizeof(HardDrive));
    thd = (HardDrive *) alloca(sizeof(HardDrive));
    memcpy(hd, ohd, sizeof(HardDrive));
    memcpy(nhd, ohd, sizeof(HardDrive));
    
    /* setup original partitions */
    fdiskSetupPartitionSpec( &hd, 1, &spec );
    
    /* ultra-simple fdisk program */
    done = 0;
    refresh = 1;

    while (!done) {
	char cmd[5];

	if (refresh) {
	    printf("\nDrive %s: Geom  %d heads, %d sectors, %d cylinders\n",
		   hd->name,
		   hd->geom.heads, hd->geom.sectors, hd->geom.cylinders);
	    printf("Total number of sectors is %d\n",hd->totalsectors);

	    printf("\nPartition Table is currently\n---------------\n");
	    print_user_partition_table(nhd);
	    printf("-----------------\nEnd of Table\n\n");
	    printf("\nExtended Partition Tables are currently\n");
	    printf("---------------------------------------\n\n");
	    print_user_extended_partition_table(nhd);
	    printf("-----------------\nEnd of Table\n\n");
	    print_partitionspec( &spec );
	    printf("\nUsed Space Map\n---------------\n\n");
	    print_used_spacemap(nhd);
	    printf("-----------------\nEnd of Table\n\n");
	    printf("\nFree Space Map\n---------------\n\n");
	    print_free_spacemap(nhd);
	    printf("-----------------\nEnd of Table\n\n");
	    refresh = 0;
	}
	
        printf("[a]dd [d]elete [e]dit [p]rint [v]erify [w]rite [q]uit: ");
	fgets(cmd, 5, stdin);

	switch (*cmd) {
	  case 'd':
	    memcpy(thd, hd, sizeof(HardDrive));
	    if (TestDeletePartition( &hd, 1, &thd, &spec ) == FDISK_SUCCESS) {
		memcpy(nhd, thd, sizeof(HardDrive));
		refresh = 1;
	    } else
		printf("error deleting partition\n");
	    break;

	  case 'p':
	    refresh = 1;
	    break;

	  case 'a':
	    memcpy(thd, hd, sizeof(HardDrive));
	    if (TestAddPartition(&hd, 1, &thd, &spec) == FDISK_SUCCESS) {
		memcpy(nhd, thd, sizeof(HardDrive));
		refresh = 1;
	    } else
		printf("error adding partition\n");
	    break;

	  case 'e':
	    memcpy(thd, hd, sizeof(HardDrive));
	    if (TestEditPartition( &hd, 1, &thd, &spec ) == FDISK_SUCCESS) {
		memcpy(nhd, thd, sizeof(HardDrive));
		refresh = 1;
	    } else
		printf("error editting partition\n");
	    break;

	  case 'w':
	    nhd->write_f( nhd );
	    break;

	  case 'q':
	    done = 1;
	    break;

	  default:
	    printf("Unknown command!\n");
	    break;
	}

    }
	    

    status=fdiskCloseDevice(ohd);
    if (status < 0) {
	perror("fdiskCloseDevice");
	exit(1);
    } else if (status > 0) {
	fprintf(stderr,"fdiskCloseDevice returns error %d\n",status);
	exit(1);
    } else
	fprintf(stderr,"fdiskCloseDevice returns successfully\n");

    exit(0);
}

