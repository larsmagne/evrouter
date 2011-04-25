/* 

action_shell.c -- Shell command actions

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

$Id: action_shell.c 6 2004-09-07 22:57:39Z alexios $

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


const static char rcsinfo [] = "$Id: action_shell.c 6 2004-09-07 22:57:39Z alexios $";



#include <stdlib.h>
#include <stdio.h>
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

#include <linux/input.h>

#include "../config.h"
#include "cmdline.h"
#include "parser.h"


int
parse_shell (int line_num, event_rule_t * r, char * s)
{
	r->action.shell.command = strdup (s);

#ifndef ENABLE_SHELL
	fprintf (stderr, "%s: %d: warning: shell actions have been disabled in the source.\n",
		 progname, line_num);
#endif /* ENABLE_SHELL */

	return 0;
}


void
handle_shell (struct input_event * ev, event_rule_t * r, unsigned int mods)
{
#ifdef ENABLE_SHELL
	int res;

	/* Shell commands are executed on keypress only, never on
	 * repeat or key release. */

	if ((ev->type == EV_KEY) && (ev->value != 1)) return;
	
	res = system (r->action.shell.command);
	printf ("Executed shell command \"%s\", exit code %d\n",
		r->action.shell.command,
		WEXITSTATUS (res));
#else
	fprintf (stderr, "%s: action \"shell/%s\" ignored (shell actions disabled in the source)\n",
		 progname, r->action.shell.command);
#endif /* ENABLE_SHELL*/
}


/* End of File */
