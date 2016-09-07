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

#ident "$Id: main.c,v 1.1 1999/09/22 06:04:08 msw Exp $"

#include <string.h>
#include <sys/utsname.h>


extern int depmod_main(int argc, char **argv);
extern int depmod64_main(int argc, char **argv);

int
main(int argc, char **argv)
{
#ifdef ARCH_sparc
  struct utsname u;

  if (!uname (&u) && !strcmp (u.machine, "sparc64"))
    return depmod64_main(argc, argv);
#endif
  return depmod_main(argc, argv);
}
