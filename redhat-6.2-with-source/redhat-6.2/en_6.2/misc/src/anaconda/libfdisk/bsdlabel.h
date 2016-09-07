
#define BSD_PARTITION		0xa5	/* Partition ID */

#define BSD_DISKMAGIC	(0x82564557UL)	/* The disk magic number */
#define BSD_MAXPARTITIONS	8
#define BSD_FS_UNUSED		0	/* disklabel unused partition entry ID */
#define BSD_LABEL_OFFSET	64
#define	BSD_DTYPE_SMD		1		/* SMD, XSMD; VAX hp/up */
#define	BSD_DTYPE_MSCP		2		/* MSCP */
#define	BSD_DTYPE_DEC		3		/* other DEC (rk, rl) */
#define	BSD_DTYPE_SCSI		4		/* SCSI */
#define	BSD_DTYPE_ESDI		5		/* ESDI interface */
#define	BSD_DTYPE_ST506		6		/* ST506 etc. */
#define	BSD_DTYPE_HPIB		7		/* CS/80 on HP-IB */
#define BSD_DTYPE_HPFL		8		/* HP Fiber-link */
#define	BSD_DTYPE_FLOPPY	10		/* floppy */

#define	BSD_BBSIZE        8192		/* size of boot area, with label */
#define	BSD_SBSIZE        8192		/* max size of fs superblock */


struct bsd_disklabel {
	unsigned int	d_magic;		/* the magic number */
	signed short	d_type;			/* drive type */
	signed short	d_subtype;		/* controller/d_type specific */
	char	d_typename[16];		/* type name, e.g. "eagle" */
	char	d_packname[16];			/* pack identifier */ 
	unsigned int	d_secsize;		/* # of bytes per sector */
	unsigned int	d_nsectors;		/* # of data sectors per track */
	unsigned int	d_ntracks;		/* # of tracks per cylinder */
	unsigned int	d_ncylinders;		/* # of data cylinders per unit */
	unsigned int	d_secpercyl;		/* # of data sectors per cylinder */
	unsigned int	d_secperunit;		/* # of data sectors per unit */
	unsigned short	d_sparespertrack;	/* # of spare sectors per track */
	unsigned short	d_sparespercyl;		/* # of spare sectors per cylinder */
	unsigned int	d_acylinders;		/* # of alt. cylinders per unit */
	unsigned short	d_rpm;			/* rotational speed */
	unsigned short	d_interleave;		/* hardware sector interleave */
	unsigned short	d_trackskew;		/* sector 0 skew, per track */
	unsigned short	d_cylskew;		/* sector 0 skew, per cylinder */
	unsigned int	d_headswitch;		/* head switch time, usec */
	unsigned int	d_trkseek;		/* track-to-track seek, usec */
	unsigned int	d_flags;		/* generic flags */
#define NDDATA 5
	unsigned int	d_drivedata[NDDATA];	/* drive-type specific information */
#define NSPARE 5
	unsigned int	d_spare[NSPARE];	/* reserved for future use */
	unsigned int	d_magic2;		/* the magic number (again) */
	unsigned short	d_checksum;		/* xor of data incl. partitions */

			/* filesystem and partition information: */
	unsigned short	d_npartitions;		/* number of partitions in following */
	unsigned int	d_bbsize;		/* size of boot area at sn0, bytes */
	unsigned int	d_sbsize;		/* max size of fs superblock, bytes */
	struct	bsd_partition {		/* the partition table */
		unsigned int	p_size;		/* number of sectors in partition */
		unsigned int	p_offset;	/* starting sector */
		unsigned int	p_fsize;	/* filesystem basic fragment size */
		unsigned char	p_fstype;	/* filesystem type, see below */
		unsigned char	p_frag;		/* filesystem fragments per block */
		unsigned short	p_cpg;		/* filesystem cylinders per group */
	} d_partitions[BSD_MAXPARTITIONS];	/* actually may be more */
};

