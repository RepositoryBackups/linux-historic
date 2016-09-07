/* included file for extended.c */
#ifndef FDISK_EXTENDED_H
#define FDISK_EXTENDED_H

int fdiskIsExtended(unsigned int type);

int fdiskRenumberLogical( HardDrive *hd );

int fdiskAppendLogical( HardDrive *hd, unsigned int *num);

int fdiskRemoveLogical( HardDrive *hd, unsigned int num);

int fdiskSetAttrLogical( HardDrive *hd, unsigned int num, Partition *p );

int fdiskGetAttrLogical( HardDrive *hd, unsigned int num, Partition **p );

int fdiskSetAttrExtended( HardDrive *hd, unsigned int num, Partition *p );

int fdiskGetAttrExtended( HardDrive *hd, unsigned int num, Partition **p );

int fdiskQueryPEP( HardDrive *hd, unsigned int *start, unsigned int *size );
int fdiskFindLogical( HardDrive *hd, unsigned int l, Partition **p );

int fdiskFirstLogical( HardDrive *hd, unsigned int *first );

int fdiskLastLogical( HardDrive *hd, unsigned int *last );
#endif

			 
