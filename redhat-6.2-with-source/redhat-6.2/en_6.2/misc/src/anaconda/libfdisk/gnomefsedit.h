
void ddruidShowMessage(char * title, char * button, char * text, ...);
/* returns 0 on No, 1 on Yes */
int ddruidYesNo(char * title, char * yes, char * no, char * text, int def);
/*  int StartMaster( HardDrive **hdarr, unsigned int numhd, */
/*  			PartitionSpec *spec, */
/*  			struct fstab *remotefs, */
/*  			int dontPartition, */
/*  			int *writeChanges); */

int
StartMaster( FseditContext *state,
	     struct fstab *remotefs,
	     int dontPartition,
	     int *writeChanges);
