/* 

x11.c -- X11 functionality

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

$Id: x11.c 6 2004-09-07 22:57:39Z alexios $

$Log$
Revision 2.0  2004/09/07 22:57:39  alexios
Stepped version to recover CVS repository after near-catastrophic disk
crash.

Revision 1.1.1.1  2004/09/07 22:52:34  alexios
Initial post-crash revision, re-initialised to recover CVS repository after
near-catastrophic disk crash.

Revision 1.1.1.1  2004/01/28 16:21:41  alexios
Initial imported revision.


*/


const static char rcsinfo [] = "$Id: x11.c 6 2004-09-07 22:57:39Z alexios $";



#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <asm/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#include "x11.h"
#include "cmdline.h"


Display         * d;
char            * display_name;
XModifierKeymap * xmk;


void
x11_done ()
{
	/*XUngrabButton (d, 7, AnyModifier, DefaultRootWindow (d));*/
	//XUngrabKey (d, 162, AnyModifier, DefaultRootWindow (d));
}

void
x11_init (int argc, char ** argv)
{
	int i;

	if (display_name == NULL) display_name = XDisplayName (NULL);
	printf ("Display name: %s\n", display_name);
	d = XOpenDisplay (display_name);
	if(d == NULL) {
		fprintf (stderr,"%s: could not open display \"%s\".\n", progname, display_name);
		exit(1);
	}
	XAllowEvents (d, AsyncBoth, CurrentTime);

#if 1
	if (0) {
		for (i=1; i<9; i++) {
			XUngrabButton (d, 8, AnyModifier, DefaultRootWindow (d));
		}
	} else {

/*		
		XGrabKey (d,
			  162,
			  AnyModifier,
			  DefaultRootWindow (d),
			  False,
			  GrabModeAsync,
			  GrabModeAsync);
*/

/*		XGrabButton (d,
			     7,
			     AnyModifier,
			     DefaultRootWindow (d),
			     False, ButtonReleaseMask | ButtonPressMask,
			     GrabModeAsync, GrabModeAsync,
			     None,
			     None);*/

	}
#endif
}
