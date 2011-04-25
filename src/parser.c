/* 

parser.c -- Parse config files

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

$Id: parser.c 15 2009-03-09 23:09:29Z alexios $

$Log$
Revision 2.0  2004/09/07 22:57:39  alexios
Stepped version to recover CVS repository after near-catastrophic disk
crash.

Revision 1.1.1.1  2004/09/07 22:52:34  alexios
Initial post-crash revision, re-initialised to recover CVS repository after
near-catastrophic disk crash.

Revision 1.2  2004/01/28 21:43:50  alexios
Added 'ctrl' alias for modifier 2, 'control'. Changed name of rc file
to "evrouterrc".

Revision 1.1.1.1  2004/01/28 16:21:41  alexios
Initial imported revision.


*/


const static char rcsinfo [] = "$Id: parser.c 15 2009-03-09 23:09:29Z alexios $";



#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <regex.h>
#include <errno.h>

#include <linux/input.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>

#include "evdev.h"
#include "cmdline.h"
#include "x11.h"
#include "parser.h"

#include "action_xkey.h" 
#include "action_xbutton.h"
#include "action_shell.h" 
#include "action_xmms.h" 


static int nl = 0;
static char *filename = "stdin";
static int error_count = 0;


int            num_rules;
event_rule_t * ruleset;


struct action action_types [] = {
	{"xkey",     act_xkey,     parse_xkey,     handle_xkey},
	{"xbutton",  act_xbutton,  parse_xbutton,  handle_xbutton},
	{"shell",    act_shell,    parse_shell,    handle_shell},
	{"xmms",     act_xmms,     parse_xmms,     handle_xmms},

	{NULL, 0, NULL, NULL}
};


void
print_parse_error (parse_error_t parse_error, char * arg)
{
	error_count++;
	switch (parse_error) {
	case error_none:
		return;
		
	case error_no_quote:
		fprintf (stderr, "%s:%d: Double Quote (\") expected\n", filename, nl);
		break;

	case error_line_syntax:
		fprintf (stderr, "%s:%d: Syntax error (Expected 'Window' or a string)\n", filename, nl);
		break;

	case error_regex:
		fprintf (stderr, "%s:%d: regex: %s\n", filename, nl, arg);
		break;

	case error_modspec:
		fprintf (stderr, "%s:%d: invalid modifier specification near \"%s\"\n", filename, nl, arg);
		break;

	case error_eventspec:
		fprintf (stderr, "%s:%d: invalid event type \"%s\"\n", filename, nl, arg);
		break;

	case error_eventspec_key:
		fprintf (stderr, "%s:%d: expected integer after \"key/\", but got \"%s\".\n", filename, nl, arg);
		break;

	case error_eventspec_rel:
		fprintf (stderr, "%s:%d: expected integer after \"rel/\", but got \"%s\".\n", filename, nl, arg);
		break;

	case error_eventspec_rel2:
		fprintf (stderr, "%s:%d: two arguments expected after \"rel/\", got only one.\n", filename, nl);
		break;

	case error_actiontype:
		fprintf (stderr, "%s:%d: action type \"%s\" is unrecognised.\n", filename, nl, arg);
		break;

	case error_action:
		fprintf (stderr, "%s:%d: action string (X keysym list) expected.\n", filename, nl);
		break;

	case error_keysym:
		fprintf (stderr, "%s:%d: invalid or unknown keysym name \"%s\".\n",
			 filename, nl, arg);
		break;

#if 0
	case error_keycode:
		fprintf (stderr, "%s:%d: \"%s\" is a valid keysym but isn't mapped to a "
			 "keycode (try xmodmap -pk for a list).\n",
			 filename, nl, arg);
		break;
#endif

	default:
		assert (0);
	}

	exit (1);
}

static char *
parse_string (char *s, char * buf)
{
	int escape = 0;

	/* Skip leading space */

	for (; *s && isspace (*s); s++);

	/* Except (but skip) the leading quote */
	
	if (*s++ != '"') {
		print_parse_error (error_no_quote, NULL);
		return NULL;
	}

	/* Process the sting */

	for (; *s; s++) {

		/* Escape double quotes using the \" sequence */

		if (*s == '\\') {
			escape = 1;
			continue;
		}
		
		if (*s == '"') break;
		escape = 0;
		
		*buf++ = *s;
	}

	/* Skip (but expect) the closing quote */

	if (*s++ != '"') {
		print_parse_error (error_no_quote, NULL);
		return NULL;
	}

	/* Skip trailing space */

	while (*s && isspace (*s)) s++;

	*buf = '\0';
	return s;
}


static int
mask_devices (int nl, event_rule_t * rule, char * pattern, int use_devname)
{
	regex_t r;
	int res, i;

	if ((res = regcomp (&r,
			    pattern,
			    REG_EXTENDED | REG_ICASE | REG_NOSUB)) != 0) {

		char buf [8192];
		regerror (res, &r, buf, sizeof (buf));
		print_parse_error (error_regex, buf);
		return -1;
	}

	/* See what matches */

#if 0
	printf ("regex=/%s/, devs=%d rule->ifaces=%08x\n", 
		pattern, num_devs, rule->ifaces);
#endif

	for (i = 0; i < num_devs; i++) {
		char *s = use_devname? devs [i].devname: devs [i].filename;
		int j = regexec (&r, s, 0, NULL, 0) != 0;
		rule->ifaces &= ~(j << i);
#if 0
		printf ("regex=/%s/, string=\"%s\", dev=%d/%d, j=%d, rule->ifaces=%08x\n", 
			pattern, s, i, num_devs, j, rule->ifaces);
#endif
	}

	return 0;
}


#define SPECDELIMS "|+-,"

static int
parse_mods (int nl, event_rule_t * rule, char * modspec)
{
	char *cp;

	for (cp = modspec; *cp; cp++) *cp = tolower (*cp);

	rule->anymods = 0;
	if (!strcmp (modspec, "none")) {
		rule->mods = 0;
		return 0;
	} else if (!strcmp (modspec, "any")) {
		rule->mods = 0;
		rule->anymods = 1;
		return 0;
	}

	cp = strtok (modspec, SPECDELIMS);
	rule->mods = 0;
	while (cp) {
		if (!strcmp (cp, "shift")) rule->mods |= 1 << mod_shift;
		else if (!strcmp (cp, "control")) rule->mods |= 1 << mod_control;
		else if (!strcmp (cp, "ctrl")) rule->mods |= 1 << mod_control;
		else if (!strcmp (cp, "mod1")) rule->mods |= 1 << mod_1;
		else if (!strcmp (cp, "alt")) rule->mods |= 1 << mod_1;
		else if (!strcmp (cp, "meta")) rule->mods |= 1 << mod_1;
		else if (!strcmp (cp, "mod2")) rule->mods |= 1 << mod_2;
		else if (!strcmp (cp, "mod3")) rule->mods |= 1 << mod_3;
		else if (!strcmp (cp, "mod4")) rule->mods |= 1 << mod_4;
		else if (!strcmp (cp, "super")) rule->mods |= 1 << mod_4;
		else if (!strcmp (cp, "win")) rule->mods |= 1 << mod_4;
		else if (!strcmp (cp, "mod5")) rule->mods |= 1 << mod_5;
		else {
			print_parse_error (error_modspec, cp);
			return -1;
		}
		
		cp = strtok (NULL, SPECDELIMS);
	}
	
	return 0;
}


static int
parse_event (int nl, event_rule_t * rule, char * event)
{
	char *cp, *evtype, *arg1, *arg2;

	for (cp = event; *cp; cp++) *cp = tolower (*cp);

	/* Parse the event type */

	if ((evtype = strtok (event, "/")) == NULL) {
		print_parse_error (error_eventspec, event);
		return -1;
	}

	/* Parse the first argument */

	arg1 = strtok (NULL, "/");
	if (arg1 == NULL) {
		print_parse_error (error_eventspec, event);
		return -1;
	}

	/* Parse the second argument, don't worry yet if it's not there. */

	arg2 = strtok (NULL, "/");

	/* A key/button event */

	if (!strcmp (evtype, "key")) {
		rule->type = evt_key;
		if (sscanf (arg1, "%d", &rule->arg1) != 1) {
			print_parse_error (error_eventspec_key, arg1);
			return -1;
		}
	}

	/* A relative motion (mouse wheel, or mouse/joystick axis) */

	else if (!strcmp (evtype, "rel")) {
		rule->type = evt_rel;

		/* Parse the first argument */

		if (sscanf (arg1, "%d", &rule->arg1) != 1) {
			print_parse_error (error_eventspec_rel, arg1);
			return -1;
		}

		/* Parse the second argument */

		if (arg2 == NULL) {
			print_parse_error (error_eventspec_rel2, NULL);
			return -1;
		} else if (sscanf (arg2, "%d", &rule->arg2) != 1) {
			print_parse_error (error_eventspec_rel, arg2);
			return -1;
		}
	}

	/* A SW event */

	else if (!strcmp (evtype, "sw")) {
		rule->type = evt_sw;
		if (sscanf (arg1, "%d", &rule->arg1) != 1) {
			print_parse_error (error_eventspec_rel, arg1);
			return -1;
		}
		if (arg2 == NULL) {
			print_parse_error (error_eventspec_rel2, NULL);
			return -1;
		} else if (sscanf (arg2, "%d", &rule->arg2) != 1) {
			print_parse_error (error_eventspec_rel, arg2);
			return -1;
		}
	}

	return 0;
}


static int
parse_action (int nl, event_rule_t * rule, char * action)
{
	char *cp;
	int i = 0;
	
	if (action == NULL) {
		print_parse_error (error_action, NULL);
		return -1;
	}

	/* Parse the action type */

	cp = strtok (action, "/");
	for (i=0; cp [i]; i++) cp [i] = tolower (cp [i]);
	
	for (i=0; action_types [i].name != NULL; i++) {
		if (!strcmp (cp, action_types [i].name)) {
			rule->action_type = action_types [i].type;
			cp += strlen (cp) + 1;
			return (*action_types [i].parser) (nl, rule, cp);
		}
	}

	/* Fallback */

	print_parse_error (error_actiontype, cp);
	return -1;
}



int
parse (char *fname)
{
	char buf [1024];
	FILE *fp;
	event_rule_t rule;
	regex_t * window = (regex_t *) malloc (sizeof (regex_t));

	if (fname == NULL) {
		snprintf (buf, sizeof (buf), "%s/%s", getenv ("HOME"), ".evrouterrc");
		fname = buf;
	}

	/* Open the file (or try) */

	if ((fp = fopen (fname, "r")) == NULL) {
		fprintf (stderr, "%s: unable to open %s: %s\n",
			 progname, fname, strerror (errno));
		exit (1);
	}

	assert (window != NULL);
	if (regcomp (window, "",  REG_EXTENDED | REG_ICASE | REG_NOSUB)) {
		fprintf (stderr, "%s: unable to compile regular expression: %s\n",
			 progname, strerror (errno));
		exit (1);
	}

	num_rules = 0;
	ruleset = NULL;

	while (!feof (fp)) {
		char line [8192], arg [8192];
		char * cp;

		if (fgets (line, sizeof (line), fp) == NULL) break;
		nl++;
		
		/* Strip trailing and leading white space and comment lines */
		
		for (cp = line + strlen (line) - 1; cp >= line; cp--)
			if (isspace (*cp)) *cp = '\0';
			else break;
		for (cp = line; *cp; cp++)
			if (!isspace (*cp)) break;

		/* A blank line? */

		if (*cp == '\0') continue;

		/* A comment line? */

		else if (*cp == '#') continue;

		/* Is it an event rule? */

		else if (*cp == '"') {
			char *mods = NULL;
			char *event = NULL;

			memset (&rule, 0, sizeof (rule));
			rule.ifaces = (1 << num_devs) - 1;
			rule.window = window;

			/* The device name regex */
			cp = parse_string (cp, arg);
			if (mask_devices (nl, &rule, arg, 1)) break;

			/* The device node regex */
			cp = parse_string (cp, arg);
			if (mask_devices (nl, &rule, arg, 0)) break;

			/* Read the modifiers */
			mods = strtok (cp, " \t");

			/* The event pattern */
			event = strtok (NULL, " \t");

			/* The action */
			cp = parse_string (event + strlen (event) + 1, arg);
			if (parse_action (nl, &rule, arg)) break;

			/* Parse the modifiers and event type */
			if (parse_mods (nl, &rule, mods)) break;
			if (parse_event (nl, &rule, event)) break;

			/* Now add the rule to the ruleset */

			num_rules++;
			ruleset = realloc (ruleset, num_rules * sizeof (event_rule_t));
			memcpy (&ruleset [num_rules - 1], &rule, sizeof (event_rule_t));
		}

		/* Is it the Window keyword? */

		else if ((strncmp (cp, "Window ", 7) == 0) ||
			 (strncmp (cp, "window ", 7) == 0)) {
			int res;
			cp += 7;
			
			cp = parse_string (cp, arg);
			window = (regex_t *) malloc (sizeof (regex_t));
			assert (window != NULL);
			res = regcomp (window, arg, REG_EXTENDED | REG_ICASE | REG_NOSUB);
			if (res != 0) {
				char buf [8192];
 				regerror (res, window, buf, sizeof (buf));
				print_parse_error (error_regex, buf);
				exit (1);
			}
			/*printf ("WINDOW = /%s/\n", arg);*/
			
		}

		/* Issue a syntax error. */

		else {
			print_parse_error (error_line_syntax, NULL);
			break;
		}
	}

	printf ("Parsed %d rules, %lu bytes\n", num_rules, sizeof (event_rule_t) * num_rules);

	return error_count;
}


