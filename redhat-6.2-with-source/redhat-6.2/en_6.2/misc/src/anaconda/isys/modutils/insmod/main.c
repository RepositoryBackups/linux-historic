/* Main dispatch switch for running a combined executable.
   Copyright 1996, 1997 Linux International.

   Contributed by Richard Henderson <rth@tamu.edu>

   This file is part of the Linux modutils.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ident "$Id: main.c,v 1.2 1999/09/11 15:43:57 msw Exp $"

#include <string.h>
#include <sys/utsname.h>
#include "logger.h"


extern int insmod_main(int argc, char **argv);
extern int rmmod_main(int argc, char **argv);

int
main(int argc, char **argv)
{
  char *p = strrchr(argv[0], '/');
  p = p ? p+1 : argv[0];

  if (strcmp(p, "insmod") == 0)
    {
#ifdef ARCH_sparc
      extern int insmod64_main(int argc, char **argv);
      struct utsname u;

      if (!uname (&u) && !strcmp (u.machine, "sparc64"))
	return insmod64_main(argc, argv);
#endif
      return insmod_main(argc, argv);
    }
  else if (strcmp(p, "rmmod") == 0)
    return rmmod_main(argc, argv);
  else
    {
      error("The insmod/rmmod combined binary is having an identity crisis.\n"
	    "Please give it a proper name.");
      return 1;
    }
}
