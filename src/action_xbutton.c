/* 

action_xbutton.c -- X11 buttons

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

$Id: action_xbutton.c 6 2004-09-07 22:57:39Z alexios $

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


const static char rcsinfo [] = "$Id: action_xbutton.c 6 2004-09-07 22:57:39Z alexios $";



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
#include <assert.h>

#include <linux/input.h>

#include "x11.h"
#include "cmdline.h"
#include "parser.h"


int
parse_xbutton (int line_num, event_rule_t * rule, char * s)
{
	if (sscanf (s, "%d", &(rule->action.xbutton.button)) != 1) {
		print_parse_error (error_action, s);
		return -1;
	}

	return 0;
}


void
handle_xbutton (struct input_event * ev, event_rule_t * r, unsigned int mods)
{
	switch (ev->type) {
		
	case EV_KEY:

		switch (ev->value) {
		case 1:		/* Key pressed */
			XTestFakeButtonEvent (d, r->action.xbutton.button, True, 0);
			break;
			
		case 2:		/* Ignore key repeats */
			break;

		case 0:		/* Key released */
			XTestFakeButtonEvent (d, r->action.xbutton.button, False, 0);
			break;

		default:
			fprintf (stderr, "Unhandled keypress value, should never happen.\n");
			assert (NULL);
		}
		break;

	/* Wheel (and other relative) events are converted to momentary key
	 * presses (keypress events immediately followed by key release
	 * events. */

	case EV_REL:
		XTestFakeButtonEvent (d, r->action.xbutton.button, True, 0);
		XTestFakeButtonEvent (d, r->action.xbutton.button, False, 0);
		break;
		
	default:
		fprintf (stderr, "Unhandled event type, should never happen.\n");
		assert (NULL);
	}

	/* Flush the display */

	//XTestGrabControl(d,False);
	XFlush (d);
	//XSync (d, False);
	//printf ("*** %d PENDING\n", XPending (d));
}


/* End of File */
