/* these routines work at a higher, abstract level than those in rawio.c */
#ifndef FDISK_USEROPS_H
#define FDISK_USEROPS_H

int fdiskReadPartitions( HardDrive *hd );

int fdiskCreatePartition( HardDrive *hd, unsigned int n );

int fdiskRemovePartition( HardDrive *hd, unsigned int n );

int fdiskSetAttrPartition( HardDrive *hd, unsigned int n, Partition *p );

int fdiskGetAttrPartition( HardDrive *hd, unsigned int n, Partition **p );

int fdiskFirstPartition( HardDrive *hd, unsigned int *first );

int fdiskLastPartition( HardDrive *hd, unsigned int *last );

int fdiskInitSunLabel( HardDrive * hd );

int fdiskInitLabel( HardDrive * hd );

#endif


