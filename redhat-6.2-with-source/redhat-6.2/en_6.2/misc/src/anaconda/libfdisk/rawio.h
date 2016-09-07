/* rawio.c */
#ifndef FDISK_RAWIO_H
#define FDISK_RAWIO_H

#include <sys/types.h>

int fdiskRoundStartToCylinder( HardDrive *hd, unsigned int *start );

int fdiskRoundEndToCylinder( HardDrive *hd, unsigned int *end );

int fdiskSectorToCHS( HardDrive *hd, unsigned int start,
		      unsigned int *c, unsigned int *h, unsigned int *s );

int fdiskOpenDevice(char *dev, unsigned int num, HardDrive **hddev);
int fdiskCloseDevice(HardDrive *hddev);
int fdiskCloseDeviceFd(HardDrive * hddev);
int fdiskReopenDeviceFd(HardDrive * hddev, char * dev);

int fdiskReadPartitionTable(HardDrive *hddev, unsigned int loc,
			     RawPartitionTable **pt );
int fdiskWritePartitionTable(HardDrive *hddev, unsigned int loc,
			     RawPartitionTable *pt );

int fdiskReReadPartitions(HardDrive *hddev);

int fdiskZeroMBR( HardDrive *hddev );
#endif
