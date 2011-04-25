/* 

action_xkey.c -- X11 keys

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

$Id: action_xkey.c 6 2004-09-07 22:57:39Z alexios $

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


const static char rcsinfo [] = "$Id: action_xkey.c 6 2004-09-07 22:57:39Z alexios $";



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
parse_xkey (int line_num, event_rule_t * rule, char * s)
{
	int     i;
	char  * cp;
	KeySym keysyms [MAX_KEYCODES + 1];

	/* Huuuge-For-Loops-R-Us */

	memset (keysyms, 0, sizeof (keysyms));
	for (i = 0, cp = strtok (NULL, ACTION_DELIMS);
	     (i < MAX_KEYCODES) && (cp != NULL);
	     i++, cp = strtok (NULL, ACTION_DELIMS)) {
		
		keysyms [i] = XStringToKeysym (cp);

		/*printf ("*** i=%d keysym=(%s) ks=%08lx\n", i, cp, keysyms [i]);*/
		if (keysyms [i] == 0) {
			print_parse_error (error_keysym, cp);
			return -1;
		} 
	}

	rule->action.xkey.num_keysyms = i;

	/* Copy the list into the rule */

	rule->action.xkey.keysyms = (KeySym *) malloc (i * sizeof (KeySym));
	if (rule->action.xkey.keysyms == NULL) {
		perror ("allocating memory");
		exit (-1);
	}
	memcpy (rule->action.xkey.keysyms, keysyms, i * sizeof (KeySym));

	return 0;
}


static void
fake_key (event_rule_t * r, int i, unsigned int mods, int is_keypress)
{
	int kc;
	
	kc = XKeysymToKeycode (d, r->action.xkey.keysyms [i]);
	/*printf ("*** %d %s %04x\n", i, is_keypress? "KEY PRESS": "KEY RELEASE", kc);*/
	if (kc != 0) XTestFakeKeyEvent (d, kc, is_keypress, 0);
	else {
		fprintf (stderr,
			 "%s: keysym %s (0x%lx) is not currently mapped to a keycode, ignoring it.\n",
			 progname, 
			 XKeysymToString (r->action.xkey.keysyms [i]),
			 r->action.xkey.keysyms [i]);
	}
}


void
handle_xkey (struct input_event * ev, event_rule_t * r, unsigned int mods)
{
	int i;

	//XTestGrabControl(d,True);
	switch (ev->type) {

	/* Convert KEY events. We simulate the way a human would press
	 * multi-key combinations. For example, the emacs-y combination
	 * Ctrl-Shift-5 should be emulated as Press Ctrl, Press Shift, Press 5,
	 * Release 5, Release Shift, Release Ctrl.
	 *
         * To do this, we simulate key presses when the event button is
         * pressed, then simulate key releases when it is released. Key release
         * events are sent in the reverse order of key presses. If 'repeat'
         * events are received, we re-issue the *last* keypress, which
         * simulates what keyboards do. */

	case EV_KEY:

		switch (ev->value) {
		case 1:		/* Key pressed */
			for (i = 0; i < r->action.xkey.num_keysyms; i++) {
				fake_key (r, i, mods, True);
			}
			break;

		case 2:		/* Key repeat */
			fake_key (r, r->action.xkey.num_keysyms - 1, mods, True);
			break;

		case 0:		/* Key released */
			for (i = r->action.xkey.num_keysyms - 1; i >= 0; i--) {
				fake_key (r, i, mods, False);
			}
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
		for (i = 0; i < r->action.xkey.num_keysyms; i++) {
			fake_key (r, i, mods, True);
		}
		for (i = r->action.xkey.num_keysyms - 1; i >= 0; i--) {
			fake_key (r, i, mods, False);
		}
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
