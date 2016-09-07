#ifndef FDISK_PRIMARY_H
#define FDISK_PRIMARY_H

int fdiskSetAttrPrimary( HardDrive *hd, unsigned int n, Partition *p );

int fdiskGetAttrPrimary( HardDrive *hd, unsigned int n, Partition **p );

int fdiskRemovePrimary( HardDrive *hd, unsigned int n );

int fdiskCreatePrimary( HardDrive *hd, unsigned int n );

int fdiskFindFreePrimary( HardDrive *hd, unsigned int *n );

int fdiskFindPrimary( HardDrive *hd, unsigned int n, Partition **p );

int fdiskFirstPrimary( HardDrive *hd, unsigned int *first );

int fdiskLastPrimary( HardDrive *hd, unsigned int *last );

#endif
