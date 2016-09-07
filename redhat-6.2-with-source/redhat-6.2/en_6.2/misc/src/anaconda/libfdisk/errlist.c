/* array of all errmsgs */
#define FDISK_NERR 30
const int  fdisk_nerr=FDISK_NERR;
char *fdisk_errlist[FDISK_NERR+1] = {
                        "Success",                          /* 0*/
			"User aborted",                     /* 1 */
			"",                                 /* 2 */
			"",                                 /* 3 */
			"",                                 /* 4 */
			"",                                 /* 5 */
			"",                                 /* 6 */
			"",                                 /* 7 */
			"",                                 /* 8 */
			"",                                 /* 9 */
			"Bad numeric value",                /* 10 */
			"Bad pointer",                      /* 11 */
			"Bad magic",                        /* 12 */
			"",                                 /* 13 */
			"",                                 /* 14 */
			"Two extended partitons",           /* 15 */
			"Two logical in an EPT",            /* 16 */
			"Partition table corrupt",          /* 17 */
			"",                                 /* 18 */
			"",                                 /* 19 */
			"No free resources",                /* 20 */
			"Could not allocate primary",       /* 21 */
			"Could not allocate extended",      /* 22 */
			"Resource currently in use",        /* 23 */
			"No primary extended partition",    /* 24 */
			"",                                 /* 25 */
			"",                                 /* 26 */
			"",                                 /* 27 */
			"",                                 /* 28 */
			"",                                 /* 29 */
			"Failed to satisfy constraint(s)"   /* 30 */
};
			

