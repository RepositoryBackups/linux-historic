/* simple program to print out fdisk_errlist */
#include <stdio.h>

#include "libfdisk.h"

main () {

  int i;

  printf("Max error is %d\n",fdisk_nerr);
  for (i=0; i<=fdisk_nerr; i++) {
      printf("%d -> %s\n", i, fdisk_errlist[i]);
  }

}
