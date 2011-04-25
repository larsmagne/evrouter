/* 

evdev.c -- Event Device handling

Copyright (c) 2004 Alexios Chouchoulas

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  

$Id: evdev.c 6 2004-09-07 22:57:39Z alexios $

$Log$
Revision 2.0  2004/09/07 22:57:39  alexios
Stepped version to recover CVS repository after near-catastrophic disk
crash.

Revision 1.1.1.1  2004/09/07 22:52:34  alexios
Initial post-crash revision, re-initialised to recover CVS repository after
near-catastrophic disk crash.

Revision 1.2  2004/01/28 21:41:39  alexios
Fixed off-by-one error in the argp-handled argument list.

Revision 1.1.1.1  2004/01/28 16:21:41  alexios
Initial imported revision.


*/


const static char rcsinfo [] = "$Id: evdev.c 6 2004-09-07 22:57:39Z alexios $";


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <asm/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#include <linux/input.h>

#include "evdev.h"
#include "cmdline.h"

evdev_t * devs;
int       num_devs = 0;


void
device_init (int argc, char ** argv)
{
	int  i, j;
	char name [256];
	
	devs = calloc (MAX_EVDEV, sizeof (evdev_t));

	for (i = 0, j = 0; (i < argc) && (j < MAX_EVDEV); i++) {
		devs [j].filename = NULL;

		/* Open the device */

		if ((devs [j].fd = open(argv [i], O_RDONLY)) < 0) {
			fprintf (stderr, "%s: error opening device %s: %s\n",
				 progname, argv [i], strerror (errno));
			continue;
		}
		
		/* Initialise the device filename */

		devs [j].filename = strdup (argv [i]);

		/* Get the device name */

		if (ioctl (devs [j].fd, EVIOCGNAME (sizeof (name)), name) < 0) {
			fprintf (stderr, "%s: error querying device %s: %s\n",
				 progname, argv [i], strerror (errno));
			close (devs [j].fd);
			continue;
		}

		devs [j].devname = strdup (name);
		printf ("device %2d: %s: %s\n", j, argv [i], name);
		j++;
	}

	num_devs = j;

	if (num_devs == 0) {
		fprintf (stderr, "%s: no input devices were opened. Exiting.\n",
			 progname);
		exit (1);
	}
}
