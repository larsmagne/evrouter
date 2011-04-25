/* 

action_xmms.c -- XMMS actions

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

$Id: action_xmms.c 6 2004-09-07 22:57:39Z alexios $

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


const static char rcsinfo [] = "$Id: action_xmms.c 6 2004-09-07 22:57:39Z alexios $";



#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <asm/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h> 

#include "../config.h"

#ifdef HAVE_LIBDL
#include <dlfcn.h>
#endif /* HAVE_LIBDL */

#include <linux/input.h>

#include "cmdline.h"
#include "parser.h"



typedef enum {
	xmms_play = 1,
	xmms_pause,
	xmms_stop,
	xmms_playpause,
	xmms_volume,
	xmms_balance,
	xmms_eject,
	xmms_prev,
	xmms_next,
	xmms_repeat,
	xmms_shuffle,
} xmms_op_t;


typedef struct {
	xmms_op_t  op;
	char *     name;
	int        arg;
} xmms_command_t;

static xmms_command_t commands [] = {
	{ xmms_play,      "play",      0 },
	{ xmms_pause,     "pause",     0 },
	{ xmms_stop,      "stop",      0 },
	{ xmms_playpause, "playpause", 0 },
	{ xmms_volume,    "volume",    1 },
	{ xmms_balance,   "balance",   1 },
	{ xmms_eject,     "eject",     0 },
	{ xmms_prev,      "prev",      0 },
	{ xmms_next,      "next",      0 },
	{ xmms_repeat,    "repeat",    0 },
	{ xmms_shuffle,   "shuffle",   0 },
	{ 0,              NULL,        0 },
};


int
parse_xmms (int line_num, event_rule_t * r, char * s)
{
	char *sorg = s;
	int i, n;

#ifndef ENABLE_XMMS
	fprintf (stderr, "%s: %d: warning: XMMS actions have been disabled in the source.\n",
		 progname, line_num);
#endif /* ENABLE_XMMS */

	for (i = 0; s [i]; i++) s [i] = tolower (s [i]);
	n = strcspn (s, "/");

	for (i = 0; commands [i].name != NULL; i++) {
		if (!strncmp (commands [i].name, s, n)) {
			r->action.xmms.op = commands [i].op;

			if (commands [i].arg) {
				s += n + 1;
				if (sscanf (s, "%d", &r->action.xmms.arg) != 1) {
					print_parse_error (error_action, sorg);
					return -1;
				}
			} else r->action.xmms.arg = 0;

			return 0;
		}
	}

	print_parse_error (error_action, sorg);
	return -1;
}


static void * xmms_handle = NULL;
static int (*xmms_remote_get_version)(int) = NULL;
static int (*xmms_remote_is_running)(int) = NULL;
static int (*xmms_remote_is_playing)(int) = NULL;
static void (*xmms_remote_play)(int) = NULL;
static void (*xmms_remote_stop)(int) = NULL;
static void (*xmms_remote_pause)(int) = NULL;
static void (*xmms_remote_play_pause)(int) = NULL;
static void (*xmms_remote_eject) (int) = NULL;
static void (*xmms_remote_playlist_prev) (int) = NULL;
static void (*xmms_remote_playlist_next) (int) = NULL;
static void (*xmms_remote_toggle_repeat) (int) = NULL;
static void (*xmms_remote_toggle_shuffle) (int) = NULL;
static int (*xmms_remote_get_main_volume) (int) = NULL;
static void (*xmms_remote_set_main_volume) (int, int) = NULL;
static int (*xmms_remote_get_balance) (int) = NULL;
static void (*xmms_remote_set_balance) (int, int) = NULL;

static void *
try_dlsym (char *symbol)
{
	char * error;
	void * fn = dlsym (xmms_handle, symbol);
	if ((error = dlerror()) != NULL)  {
		fprintf (stderr, "%s: XMMS: %s\n", progname, error);
		return NULL;
	}

	return fn;
}

#define LOAD(s) \
(fprintf (stderr, "%s: accessing symbol %s...\n", progname, #s), \
s = try_dlsym (#s), s)

static void *
try_dlopen ()
{
	/*char *s = strdup (XMMS_LIBPATH);*/

	if ((xmms_handle = dlopen ("libxmms.so", RTLD_LAZY)) == NULL) {
		fprintf (stderr, "%s: warning: %s (will ignore XMMS actions)\n",
			 progname, dlerror ());
		return NULL;
	}

	if (LOAD (xmms_remote_get_version) == NULL) exit (1);
	fprintf (stderr, "XMMS: xmms_remote_get_version() = %x\n",
		 xmms_remote_get_version (0));

	if (LOAD (xmms_remote_is_running) == NULL) exit (1);
	fprintf (stderr, "XMMS: xmms_remote_is_running() = %d\n",
		 xmms_remote_is_running (0));

	/* Access the functions we need */
	
	if (LOAD (xmms_remote_play) == NULL) exit (1);
	if (LOAD (xmms_remote_stop) == NULL) exit (1);
	if (LOAD (xmms_remote_pause) == NULL) exit (1);
	if (LOAD (xmms_remote_play_pause) == NULL) {
		fprintf (stderr, "%s: no problem, we can simulate it.\n", progname);

		/* This one may be unavailable, but we can get the
		 * same effect if we have xmms_remote_is_playing(). */
		
		if (LOAD (xmms_remote_is_playing) == NULL) exit (1);
	}
	if (LOAD (xmms_remote_eject) == NULL) exit (1);
	if (LOAD (xmms_remote_playlist_prev) == NULL) exit (1);
	if (LOAD (xmms_remote_playlist_next) == NULL) exit (1);
	if (LOAD (xmms_remote_toggle_repeat) == NULL) exit (1);
	if (LOAD (xmms_remote_toggle_shuffle) == NULL) exit (1);
	if (LOAD (xmms_remote_get_main_volume) == NULL) exit (1);
	if (LOAD (xmms_remote_set_main_volume) == NULL) exit (1);
	if (LOAD (xmms_remote_get_balance) == NULL) exit (1);
	if (LOAD (xmms_remote_set_balance) == NULL) exit (1);
		
	
	return xmms_handle;
}

void
handle_xmms (struct input_event * ev, event_rule_t * r, unsigned int mods)
{
#ifdef ENABLE_XMMS
	int i;

	/* Try to link dynamically the xmms library */
	if (xmms_handle == NULL) try_dlopen();

	/* Still NULL? Bail out. */
	if (xmms_handle == NULL) {
		fprintf (stderr, "%s: XMMS actions temporarily disabled.\n", progname);
		return;
	}

	/* XMMS commands are executed on keypress only, never on
	 * repeat or key release. */

	if ((ev->type == EV_KEY) && (ev->value != 1)) return;

	switch (r->action.xmms.op) {

	case xmms_play: xmms_remote_play (0); break;

	case xmms_pause: xmms_remote_pause (0); break;

	case xmms_stop: xmms_remote_stop (0); break;

	case xmms_playpause: 
		if (xmms_remote_play_pause == NULL) {
			if (xmms_remote_is_playing (0)) xmms_remote_pause (0);
			else xmms_remote_play (0);
		} else xmms_remote_play_pause (0);
		break;

	case xmms_volume:
		i = xmms_remote_get_main_volume (0);
		xmms_remote_set_main_volume (0, i + r->action.xmms.arg);
		break;

	case xmms_balance:
		i = xmms_remote_get_balance (0);
		xmms_remote_set_balance (0, i + r->action.xmms.arg);
		break;

	case xmms_eject: xmms_remote_eject (0); break;

	case xmms_prev: xmms_remote_playlist_prev (0); break;

	case xmms_next: xmms_remote_playlist_next (0); break;

	case xmms_repeat: xmms_remote_toggle_repeat (0); break;

	case xmms_shuffle: xmms_remote_toggle_shuffle (0); break;

	default:
		fprintf (stderr, "%s: unhandled XMMS action, should never happen.\n",
			 progname);
		assert (0);
	}
	
#else
	fprintf (stderr, "%s: XMMS action ignored (XMMS support disabled in the source)\n",
		 progname);
#endif /* ENABLE_XMMS*/
}


/* End of File */
