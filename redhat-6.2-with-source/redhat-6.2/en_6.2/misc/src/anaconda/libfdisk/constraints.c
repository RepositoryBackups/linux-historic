#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "libfdisk.h"

/* Constraints used to give allocation routines hints about what */
/* to do with a given partition                                  */

/* store current setttings of constraint in vars if active, otherwise */
/* use the specified defaults                                         */
int fdiskQueryConstraint( Constraint *c, unsigned int *cur,
			unsigned int *l,  unsigned int *m,
			unsigned int dl,  unsigned int dm ) {
    
    if (c->active) {
	*l = c->min;
	*m = c->max;
    } else {
	*l = dl;
	*m = dm;
    }

    *cur = c->current;
    return FDISK_SUCCESS;
}

/* store current setttings of constraint in vars if active, otherwise */
/* use the specified defaults                                         */
int fdiskGetConstraint( Constraint *c, unsigned int *cur,
			unsigned int *l,  unsigned int *m,
			unsigned int *active ) {
    
    *l      = c->min;
    *m      = c->max;
    *cur    = c->current;
    *active = c->active;
    return FDISK_SUCCESS;
}

/* get min/max of constraint */
int fdiskGetMinMaxConstraint(Constraint *c,unsigned int *l,unsigned int *m){
    *l      = c->min;
    *m      = c->max;
    return FDISK_SUCCESS;
}

/* get current value of constraint */
int fdiskGetCurrentConstraint( Constraint *c, unsigned int *cur ){
    *cur    = c->current;
    return FDISK_SUCCESS;
}

/* set current value constraint in cur */
int fdiskSetCurrentConstraint( Constraint *c, unsigned int cur ){
    c->current = cur;
    return FDISK_SUCCESS;
}

/* set a constraint that is fixed in value and has no range */
int fdiskSetFixedConstraint( Constraint *c, unsigned int value ) {
    c->current = c->min = c->max = value;
    c->active = 1;
    return FDISK_SUCCESS;
}

int fdiskEnableConstraint( Constraint *c ) {
    c->active  = 1;
    return FDISK_SUCCESS;
}

int fdiskDisableConstraint( Constraint *c ) {
    c->active = 0;
    return FDISK_SUCCESS;
}

int fdiskClearConstraint( Constraint *c ) {
    memset( c, 0, sizeof(Constraint) );
    return FDISK_SUCCESS;
}

int fdiskSetConstraint( Constraint *c, unsigned int cur,
			unsigned int min, unsigned int max,
			unsigned int active) {
    c->current = cur;
    c->min     = min;
    c->max     = max;
    c->active  = active;
    return FDISK_SUCCESS;
}

int fdiskResetConstraint( Constraint *c ) {
    memset( c, 0, sizeof(Constraint) );
    return FDISK_SUCCESS;
}

/* return true/false if constraint is active */
int fdiskConstraintIsActive( Constraint *c ) {
    return c->active;
}

/* another type of constraint is a DriveSet. It is a set of numbers from */
/* which is normally used to specify which drives a partition can go on  */
/* if the entry corresponding to drive num is active (equal to 0) we can */
/* consider the drive. If it is equal to 1, the drive is masked off      */
int fdiskCopyDriveSet (DriveSet *src, DriveSet *dest) {
    unsigned int i;

    for (i=0; i<=MAX_DRIVESET_NUM; i++)
	dest->active[i] = src->active[i];

    return FDISK_SUCCESS;
}

int fdiskActivateAllDriveSet( DriveSet *ds ) {
    unsigned int i;

    for (i=0; i<=MAX_DRIVESET_NUM; i++)
	ds->active[i] = 0;
    return FDISK_SUCCESS;
}

int fdiskDeactivateAllDriveSet( DriveSet *ds ) {
    unsigned int i;

    for (i=0; i<=MAX_DRIVESET_NUM; i++)
	ds->active[i] = 1;
    return FDISK_SUCCESS;
}

int fdiskActivateDriveSet( DriveSet *ds, unsigned int i ) {
    ds->active[i] = 0;
    return FDISK_SUCCESS;
}

int fdiskDeactivateDriveSet( DriveSet *ds, unsigned int i ) {
    ds->active[i] = 1;
    return FDISK_SUCCESS;
}

int fdiskThisDriveSetIsActive( DriveSet *ds, unsigned int i ) {
    return !ds->active[i];
}

/* tells us if the driveset is active (acting as a constraint)            */
/* if all bits are unset then there is no constraint, as all drives are   */
/* valid places to put the partition                                      */
/* NOTE that just because a drive is 'active' doesn't mean partition      */
/* can go there! Just means it can't (that drive number may not exist)    */
int fdiskDriveSetIsActive( DriveSet *ds ) {
    unsigned int k;

    for (k=0; k <= MAX_DRIVESET_NUM; k++)
	if (ds->active[k])
	    return 1;

    return 0;
}

/* get first entry in DriveSet, use fdiskWalkDriveSet to get other entries */
int fdiskInitWalkDriveSet( DriveSet *ds, unsigned int *i ) {
    unsigned int k;

    for (k=0; k <=MAX_DRIVESET_NUM; k++)
	if (!ds->active[k])
	    break;

    if (k <= MAX_DRIVESET_NUM) {
	*i = k;
	return FDISK_SUCCESS;
    } else {
	return FDISK_ERR_BADNUM;
    }
}

/* start at first set entry starting AFTER that passed in i */
int fdiskWalkDriveSet( DriveSet *ds, unsigned int *i ) {
    unsigned int k;

    if (*i == MAX_DRIVESET_NUM)
	return FDISK_ERR_BADNUM;
    
    for (k=*i+1; k <=MAX_DRIVESET_NUM; k++)
	if (!ds->active[k])
	    break;

    if (k <= MAX_DRIVESET_NUM) {
	*i = k;
	return FDISK_SUCCESS;
    } else {
	return FDISK_ERR_BADNUM;
    }
}

int fdiskGetCurrentDriveSet( DriveSet *ds, unsigned int *i ) {
    *i = ds->current;
    return FDISK_SUCCESS;
}

int fdiskSetCurrentDriveSet( DriveSet *ds, unsigned int i ) {
    ds->current = i;
    return FDISK_SUCCESS;
}
    
