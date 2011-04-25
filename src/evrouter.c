/* 

evrouter.c -- Main file

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

$Id: evrouter.c 15 2009-03-09 23:09:29Z alexios $

$Log$
Revision 2.0  2004/09/07 22:57:39  alexios
Stepped version to recover CVS repository after near-catastrophic disk
crash.

Revision 1.1.1.1  2004/09/07 22:52:34  alexios
Initial post-crash revision, re-initialised to recover CVS repository after
near-catastrophic disk crash.

Revision 1.4  2004/02/12 00:32:48  alexios
Made bad_window_handler only issue its warning if verbose is set, and
added a newline to its annoying, frequent message.

Revision 1.3  2004/02/12 00:15:56  alexios
Added foreground behaviour (I forgot the if in the previous version,
d'oh!). Rewrote the daemon functionality to support killing off the
currently running daemon and to write the daemon's PID in the lock
file.

Revision 1.2  2004/01/28 21:43:10  alexios
Slight beautification. Fixed Lock modifier bug (Caps Lock caused
modifier matching to fail -- sorry, but my keyboards don't have Caps
Lock). Fixed rule dumping so that the format is up to date.

Revision 1.1.1.1  2004/01/28 16:21:41  alexios
Initial imported revision.


*/


const static char rcsinfo [] = "$Id: evrouter.c 15 2009-03-09 23:09:29Z alexios $";



#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <asm/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>
#include <signal.h>

#include <regex.h>

#include <linux/input.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>

#include "evdev.h"
#include "x11.h"
#include "parser.h"
#include "cmdline.h"

char * progname;


#define NEV 64


#if 0
#define MAX_TYPE 6
static char * event_types [MAX_TYPE] = {
	"syn",			/* 0 */
	"key",			/* 1 */
	"rel",			/* 2 */
	"abs",			/* 3 */
	"msc",			/* 4 */
	"sw",			/* 5 */
};
#endif


static char *
get_focus ()
{
	Window     focus;
	XClassHint xch = {NULL, NULL};
	int        i;
	char       * wname = NULL;
	static char * oldwname = "(null)";

	XGetInputFocus (d, &focus, &i);
	if (focus != None) {
		XGetClassHint (d, focus, &xch);
		XFetchName (d, focus, &wname);
	}

	if (train_mode) {
		if (strcmp (oldwname, wname? wname: "")) {
			printf ("\nWindow \"%s\": # Window title\n", wname? wname: "(null)");
			printf ("# Window \"%s\": # Resource name\n", xch.res_name);
			printf ("# Window \"%s\": # Class name\n", xch.res_class);
		}
	}

	if (wname) oldwname = wname;

	return wname;
}


void printXModifierKeymap(XModifierKeymap *xmk)
{
	int i,j;

	printf ("XModifierKeymap:\n");
	for(j=0; j<8; j++)
		for(i=0; i<xmk->max_keypermod && xmk->modifiermap[(j*xmk->max_keypermod)+i]; i++)
			printf("\t[%d,%02d] %d 0x%x \"%s\"\n",
				j,i,
				xmk->modifiermap[(j*xmk->max_keypermod)+i],
				xmk->modifiermap[(j*xmk->max_keypermod)+i],
				XKeysymToString(XKeycodeToKeysym(d,xmk->modifiermap[(j*xmk->max_keypermod)+i],0)));
}

#define getbit(buf,n) (buf [(n) >> 3] & (1 << ((n) & 7)))

static unsigned int
get_modifier_state ()
{
	int i, j;
	char km [32];
	unsigned char mods = 0;

	XQueryKeymap (d, km);
	xmk = XGetModifierMapping (d);

	for(j = 0; j < 8; j++) {
		for(i = 0; (i < xmk->max_keypermod) &&
			    xmk->modifiermap [(j * xmk->max_keypermod) + i]; i++) {
			if (getbit (km, xmk->modifiermap [(j * xmk->max_keypermod) + i])) {
				mods |= (1 << j);
			}
		}
	}

	mods &= ~2; /* Ignore the 'Lock' mod */

	return mods;
}


static char *
string_modifiers (unsigned mods)
{
	int i, j;
	static char modstring [256], tmp [8];

	modstring [0] = 0;
	for (i = 1, j = 0; j < 8; i <<= 1, j++) {
		if (mods & i) {
			switch (j) {
			case 0: strcat (modstring, "shift+");   break;
			case 1: /*strcat (modstring, "lock|");*/ break;
			case 2: strcat (modstring, "control+"); break;
			case 3: strcat (modstring, "alt+"); break;
			default: 
				sprintf (tmp, "mod%d+", j - 2);
				strcat (modstring, tmp);
			}
		}
	}
	
	if (!strlen (modstring)) strcpy (modstring, "none");
	else modstring [strlen (modstring) - 1] = '\0';

	return modstring;
}


static void
perform_action (struct input_event * ev, event_rule_t * r, unsigned int mods)
{
	int i;

	//fprintf (stderr, "PERFORMING ACTION (type=%d, value=%d)\n", ev->type, ev->value);

	for (i=0; action_types [i].name != NULL; i++) {
		if (r->action_type == action_types [i].type) {
			(*action_types [i].handler) (ev, r, mods);
			return;
		}
	}

	fprintf (stderr, "%s: unhandled action type %d, should never happen.\n",
		 progname, r->action_type);
	assert (0);
}


static int
bad_window_handler (Display * d, XErrorEvent * e)
{
	if (verbose) {
		fprintf (stderr, "%s: no such window with id %ld, treating window title as blank.\n",
			 progname, e->resourceid);
	}
	return 0;
}


#define NOTNULL(s) ((s != NULL)? s: "")


static void
handle_event (evdev_t * dev, int devindex, struct input_event * ev)
{
	Window          focus;
	XClassHint      xch = {NULL, NULL};
	int             i, k;
	char          * wname = NULL;
	unsigned int    mods;
	XErrorHandler   old_handler;
		
	if ((ev->type != EV_KEY) && 
	    ((ev->type != EV_REL)) &&
	    (ev->type != EV_SW)) return;

	assert (devindex >= 0);
	assert (devindex < 32);
	devindex = 1 << devindex;

	//XTestGrabControl (d, True);

	/* Get the Window title, class and resource name */

	old_handler = XSetErrorHandler (bad_window_handler);
	XGetInputFocus (d, &focus, &i);
	if (focus != None) {
		XGetClassHint (d, focus, &xch);
		XFetchName (d, focus, &wname);
	}
	XSetErrorHandler(old_handler);

	//XUngrabKey (d, 162, AnyModifier, DefaultRootWindow (d));

#if 0
	printf("\"%s\" \"%s\" \"%s\"\n\"%s\" \"%s\" %s key/%d/%d \"fill this in!\" # Change me!\n",
	       wname, xch.res_name, xch.res_class,
	       dev->devname,
	       dev->filename,
	       string_modifiers (get_modifier_state (d)),
	       ev->code, ev->value);
#endif
	
	for (k = 0; k < num_rules; k++) {
		event_rule_t * r = &ruleset [k];

		/* Match the event itself -- easiest test, weeds most rules out
		 * so the more expensive tests aren't executed very often. */
		
		if ((ev->type == EV_KEY) && (r->type == evt_key) && (r->arg1 == ev->code)) {
			/*printf ("MATCHED KEY type=%d, value=%d, arg1=%d code=%d k=%d\n",
			  ev->type, ev->value, r->arg1, ev->code, k);*/
		}

		else if ((ev->type == EV_REL) && (r->type == evt_rel) &&
			 (r->arg1 == ev->code) && (r->arg2 == ev->value)) {
			/*printf ("MATCHED RELATIVE AXIS\n");*/
		}

		else if ((ev->type == EV_SW) && (r->type == evt_sw) &&
			(r->arg1 == ev->code) && (r->arg2 == ev->value)) {
			printf ("MATCHED SW type=%d, value=%d\n", ev->type, ev->value, r->arg1);
		}

		else continue;

		/* Match the device -- easy test */
		
		if ((r->ifaces & devindex) == 0) continue;
		
		/* Match the window name */
		
		if ((r->window != NULL) &&
		    regexec (r->window, NOTNULL (wname), 0, NULL, 0) &&
		    regexec (r->window, NOTNULL (xch.res_name), 0, NULL, 0) &&
		    regexec (r->window, NOTNULL (xch.res_class), 0, NULL, 0)) {
			//printf ("UNMATCHED WINDOW, SMART SKIP at k=%d\n",k);
			while ((k < num_rules) && (ruleset [k].window == r->window)) k++;
			k--;
			continue;
		}

		/* Match the modifiers, IF this is a keypress. We don't care
		 * about modifier matching for key releases because the
		 * modifier map may have changed while the key/button is
		 * pressed, and we run the risk of never 'releasing' the
		 * simulated keys. */
		
		if ((ev->type == EV_KEY) &&
		    (ev->value > 0) &&
		    (r->anymods == 0) &&
		    (r->mods != (mods = get_modifier_state (d)))) continue;

		/* Perform the action */

		perform_action (ev, r, mods);
		break;
	}

	//XTestGrabControl (d, False);
	/*
		XGrabKey (d,
			  162,
			  AnyModifier,
			  DefaultRootWindow (d),
			  False,
			  GrabModeAsync,
			  GrabModeAsync);*/
}


static void
print_event (evdev_t * dev, int devindex, struct input_event * ev)
{
	assert (devindex >= 0);
	assert (devindex < 32);
	devindex = 1 << devindex;

	if ((ev->type == EV_KEY) && (ev->value == 1)) {
		get_focus ();
		
		printf("\"%s\" \"%s\" %s key/%d \"fill this in!\"\n",
		       dev->devname,
		       dev->filename,
		       string_modifiers (get_modifier_state (d)),
		       ev->code);
	} else if ((ev->type == EV_REL)) {
		get_focus ();
		
		printf("\"%s\" \"%s\" %s rel/%d/%d \"fill this in!\"\n",
		       dev->devname,
		       dev->filename,
		       string_modifiers (get_modifier_state (d)),
		       ev->code, ev->value);
	} else if (ev->type == EV_SW) {
		get_focus ();

		printf("\"%s\" \"%s\" %s sw/%d/%d \"fill this in!\"\n",
			dev->devname,
			dev->filename,
			string_modifiers (get_modifier_state (d)),
			ev->code, ev->value);
	}
}


static void
device_read ()
{
	int      i, j;
	int      maxfd = 0;
	int      sel;
	fd_set   readset, errset;

	/* Get the maximum file descriptor */

	for (i = 0; i < num_devs; i++) {
		if (devs [i].fd > maxfd) maxfd = devs [i].fd;
	}

	/* Read from the devices. */

	while (1) {
		FD_ZERO (&readset);
		FD_ZERO (&errset);
		for (i = 0; i < num_devs; i++) {
			if (devs [i].fd < 0) continue;
			if (devs [i].filename) FD_SET (devs [i].fd, &readset);
			if (devs [i].filename) FD_SET (devs [i].fd, &errset);
		}

		sel = select (maxfd + 1, &readset, NULL, &errset, NULL);

		/* Read X Events */

		while (XPending (d)) {
			XEvent e;
			XNextEvent (d, &e);
			/*printf ("*** EVENT type=%d, kc=%d\n", e.type, e.xkey.keycode);*/
		}

		/* Check where the data came from. */

		for (i = 0; i < num_devs; i++) {
			int nb = 0;
			struct input_event ev [NEV];

			if (devs [i].filename == NULL) continue;
			if (!FD_ISSET (devs [i].fd, &readset)) continue;

			nb = read (devs [i].fd, ev, sizeof (struct input_event) * NEV);

			/* If an error occurs, remove the device. */

			if (nb < 0) {
				close (devs[i].fd);
				devs[i].fd = -1;
				continue;
			}

			for (j = 0; j < nb / sizeof (struct input_event); j++) {
				if (train_mode) print_event (&devs [i], i, &ev [j]);
				else handle_event (&devs [i], i, &ev [j]);
			}
		}

	}

	/* Exit (closing the devices implicitly). */
}


static char fname [1024];

static void rmlock ()
{
	unlink (fname);
}

static void forced_exit ()
{
	rmlock ();
	exit (1);
}

static void
become_daemon ()
{
	int fd;
	int pid;
	char s [40]; /* Overkill */

	snprintf (fname, sizeof (fname), "/tmp/.evrouter%s", getenv ("DISPLAY"));
	if (verbose) printf ("Lock file: %s\n", fname);

	/* Kill the daemon? */

	if (sepuku) {
		fd = open (fname, O_RDONLY, 0600);

		if (fd < 0) {
			fprintf (stderr, "%s: file %s could not be opened (no daemon killed): %s\n",
				 progname, fname, strerror (errno));
			exit (1);
		}

		if (read (fd, s, sizeof (s))) {
			if (sscanf (s, "%d", &pid)) {
				fprintf (stderr, "Killing off process %d and exiting.\n", pid);
				kill (pid, SIGKILL);
			}

			exit (0);
		}

	} else {

	/* Or start the daemon? */
		
		fd = open (fname, O_EXCL | O_CREAT | O_RDWR, 0600);
		if (fd < 0) {
			fprintf (stderr, "%s: error creating %s: %s\n\n"
				 "Please make sure this file can be created, no other instance of\n"
				 "this program is running on X display %s. Remove the file\n"
				 "%s if no other instance is running.\n",
				 progname, fname, strerror (errno), getenv ("DISPLAY"), fname);
			exit (1);
		}
	}

	/* Fork? */

	if (!foreground) {
		pid = fork();
		if (pid == 0) {
			atexit (rmlock);
			signal (SIGINT, forced_exit);
			signal (SIGTERM, forced_exit);
			signal (SIGKILL, forced_exit);
		} else if (pid < 0) {
			fprintf (stderr, "%s: failed to fork off daemon: %s\n",
				 progname, strerror (errno));
			exit (1);
		}
	} else pid = getpid();

	/* Write our PID to a run file */

	if (pid) {
		snprintf (s, sizeof (s), "%d\n", pid);
		write (fd, s, strlen (s));

		/* The parent process exits here (if running as a daemon) */

		if (!foreground) exit (0);
	}
}


int main (int argc, char **argv)
{
	progname = argv [0];
	cmdline_parse (argc, argv);

	if (sepuku == 0) {
		device_init (numdevs, devnames);
		if (device_dump_mode) exit (0);

		x11_init (argc, argv);
	}

	if (!train_mode) {
		if (sepuku == 0) parse (configfile);
		become_daemon ();
	}

	if (sepuku == 0) device_read ();

	return 0;
}
