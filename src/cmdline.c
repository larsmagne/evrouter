/* 

cmdline.c -- Parse command line options

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

$Id: cmdline.c 7 2004-09-07 23:00:59Z alexios $

$Log$
Revision 2.1  2004/09/07 23:00:59  alexios
Changed email address and URL.

Revision 2.0  2004/09/07 22:57:39  alexios
Stepped version to recover CVS repository after near-catastrophic disk
crash.

Revision 1.1.1.1  2004/09/07 22:52:34  alexios
Initial post-crash revision, re-initialised to recover CVS repository after
near-catastrophic disk crash.

Revision 1.3  2004/02/12 00:14:31  alexios
Added command line option for killing the running daemon.

Revision 1.2  2004/01/28 21:41:08  alexios
Added mode to list event devices.

Revision 1.1.1.1  2004/01/28 16:21:41  alexios
Initial imported revision.


*/


const static char rcsinfo [] = "$Id: cmdline.c 7 2004-09-07 23:00:59Z alexios $";


#include <stdlib.h>

#include "../config.h"
#include "evdev.h"
#include "argp.h"

char * progname = NULL;

int train_mode = 0;
int device_dump_mode = 0;
int verbose = 0;
int foreground = 0;
int sepuku = 0;

int numdevs = 0;
char * devnames [MAX_EVDEV];
char * configfile = NULL;


#define _(x) x


static char const rcsid[] = "$Id";


/* Option flags and variables.  These are initialized in parse_opt.  */

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static void show_version (FILE *stream, struct argp_state *state);


/* The argp functions examine these global variables.  */
const char *argp_program_bug_address = "<alexios@bedroomlan.org>";
void (*argp_program_version_hook) (FILE *, struct argp_state *) = show_version;


/* A description of the arguments we accept. */
static char args_doc[] = "EVENT-DEVICE ...";


struct argp_option options[] =
{
	{ "dump",         'd',     NULL, 0,
	  _("Dump input events in a format suitable for creating a configuration file.")},
	{ "devices",      'D',     NULL, 0,
	  _("Show the names of all devices specified on the command line.")},
	{ "foreground",   'f',     NULL, 0,
	  _("Do not become a daemon, run in the foreground instead (default if -d specified).")},
	{ "config",       'c',    "CONFIG-FILE", 0,
	  _("Set the configuration file to open.")},
	{ "quit",         'q',    NULL, 0,
	  _("Kill the currently running evrouter daemon, then exit.")},
	{ "verbose",      'v',     NULL,            0,
	  _("Print more information (mostly for debugging)"), 0 },

	{ NULL, 0, NULL, 0, NULL, 0 }
};



struct argp argp =
{
	options, parse_opt, args_doc,
	"An Event Router for Linux.",
	NULL, NULL, NULL
};


#define ACTIONFMT "\t%-15s %s\n"
/* Show the version number and copyright information.  */
static void
show_version (FILE *stream, struct argp_state *state)
{
	(void) state;
	/* Print in small parts whose localizations can hopefully be copied
	   from other programs.  */
	fputs(PACKAGE" "VERSION"\n\n", stream);
	fprintf(stream, 
		"Written by Alexios Chouchoulas <alexios@bedroomlan.org>.\n"
		"Copyright (C) %s %s\n"
		"This program is free software; you may redistribute it under the terms of\n"
		"the GNU General Public License.  This program has absolutely no warranty.\n",
		"2004", "Alexios Chouchoulas");

	fprintf (stream, "\nSupported actions:\n");
	fprintf (stream, ACTIONFMT, "XKey",    "X11 Keys");
	fprintf (stream, ACTIONFMT, "XButton", "X11 Buttons");

#ifdef ENABLE_SHELL
	fprintf (stream, ACTIONFMT, "Shell",   "Shell commands");
#endif /* ENABLE_SHELL */

#ifdef ENABLE_XMMS
	fprintf (stream, ACTIONFMT, "XMMS",    "XMMS 'remote control' commands");
#endif /* ENABLE_XMMS */
	
	fprintf (stream, "\n");
}


/* Parse a single option.  */

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key) {
	case 'd':
		train_mode = 1;
		break;
		
	case 'D':
		device_dump_mode = 1;
		break;
		
	case 'c':
		configfile = arg;
		break;
		
	case 'f':
		foreground = 1;
		break;
		
	case 'q':
		sepuku = 1;
		break;
		
	case 'v':
		verbose++;
		break;

	case ARGP_KEY_ARG:
		if ((numdevs + 1) >= MAX_EVDEV) {
			fprintf (stderr, "%s: too many devices listed (maximum %d)\n",
				 progname, MAX_EVDEV);
			argp_usage (state);
		} else devnames [numdevs++] = arg;

		break;

	case ARGP_KEY_END:
		if ((numdevs < 1) && (sepuku == 0)) {
			fprintf (stderr, "%s: no devices specified.\n", progname);
			argp_usage (state);
		}
		break;

	default:
		return ARGP_ERR_UNKNOWN;
	}
	
	return 0;
}

void
cmdline_parse (int argc, char ** argv)
{
	/* Parse the arguments */
	
	argp_parse (&argp, argc, argv, ARGP_IN_ORDER, NULL, NULL);
}
