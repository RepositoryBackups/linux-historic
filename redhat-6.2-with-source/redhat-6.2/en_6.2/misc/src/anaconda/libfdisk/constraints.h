#ifndef FDISK_CONSTRAINTS_H
#define FDISK_CONSTRAINTS_H

/* lets define a 'contraint' - use for automatic allocation of resources     */
/* contraints are able to handle unsigned values only, but thats all we need */
/* if active, contraint requires value to be within the range defined by     */
/* min and max, inclusively.                                                 */
struct constraint {
    unsigned int         active;     /* 0 is contraint disabled, 1 otherwise */
    unsigned int         current;    /* current value         */
    unsigned int         min;        /* minimum value allowed */
    unsigned int         max;        /* maximum value allowed */
};

typedef struct constraint     Constraint;

/* DriveSet describes a set of numbers (drive numbers usually) */
/* If active[n] is non-zero, then the number 'n' is in the set */
#define MAX_DRIVESET_NUM 32
struct driveset {
    unsigned int  current;
    unsigned char active[MAX_DRIVESET_NUM+1];
};

typedef struct driveset       DriveSet;

int fdiskSetFixedConstraint( Constraint *c, unsigned int value );
int fdiskGetConstraint( Constraint *c, unsigned int *cur,
			unsigned int *min, unsigned int *max,
			unsigned int *active);
int fdiskSetConstraint( Constraint *c, unsigned int cur,
			unsigned int min, unsigned int max,
			unsigned int active);
int fdiskGetMinMaxConstraint(Constraint *c,unsigned int *mn,unsigned int *mx);
int fdiskGetCurrentConstraint(Constraint *c, unsigned int *current);
int fdiskSetCurrentConstraint(Constraint *c, unsigned int current);
int fdiskQueryConstraint( Constraint *c, unsigned int *cur,
			unsigned int *min, unsigned int *max,
			unsigned int dmin, unsigned int dmax );
int fdiskConstraintIsActive( Constraint *c );
int fdiskEnableConstraint( Constraint *c );
int fdiskDisableConstraint( Constraint *c );
int fdiskResetConstraint( Constraint *c );

int fdiskCopyDriveSet (DriveSet *src, DriveSet *dest );
int fdiskActivateAllDriveSet( DriveSet *ds );
int fdiskDeactivateAllDriveSet( DriveSet *ds );
int fdiskActivateDriveSet( DriveSet *ds, unsigned int i);
int fdiskDeactiveDriveSet( DriveSet *ds, unsigned int i);
int fdiskThisDriveSetIsActive( DriveSet *ds, unsigned int i);
int fdiskDriveSetIsActive( DriveSet *ds );
int fdiskInitWalkDriveSet( DriveSet *ds, unsigned int *i );
int fdiskWalkDriveSet( DriveSet *ds, unsigned int *i );
int fdiskSetCurrentDriveSet( DriveSet *ds, unsigned int i );
int fdiskGetCurrentDriveSet( DriveSet *ds, unsigned int *i );
#endif
