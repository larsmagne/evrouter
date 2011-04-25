/* 

parser.h -- Parse config files

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

$Id: parser.h 15 2009-03-09 23:09:29Z alexios $

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


#ifndef PARSER_H
#define PARSER_H


#include <sys/types.h>
#include <regex.h>

#include "x11.h"


/* An event device set is 32 bits wide because the kernel only
 * supports 32 event devices for now. */

typedef unsigned int set_t;

typedef enum {
	evt_key,
	evt_rel,
	evt_sw
} event_type_t;

typedef enum {
	mod_shift   = 0,
	mod_lock    = 1,
	mod_control,
	mod_1,
	mod_2,
	mod_3,
	mod_4,
	mod_5
} mod_t;

typedef enum {
	act_xkey,
	act_xbutton,
	act_shell,
	act_xmms,

	act_reserved
} action_type_t;

struct action_xkey {
	int            num_keysyms;
	KeySym       * keysyms;
	int            step;
};


struct action_xbutton {
	int            button;
};


struct action_shell {
	char * command;
};


struct action_xmms {
	int op;
	int arg;
};

union action_t {

	struct action_xkey     xkey;
	struct action_xbutton  xbutton;
	struct action_shell    shell;
	struct action_xmms     xmms;
};

typedef struct {
	set_t          ifaces;
	regex_t      * window;
	int            window_matched;
	mod_t          mods;
	int            anymods;
	event_type_t   type;
	int            arg1;
	int            arg2;

	action_type_t  action_type;
	union action_t action;
} event_rule_t;


typedef enum {
	error_none = 0,
	error_no_quote,
	error_line_syntax,
	error_regex,
	error_modspec,
	error_eventspec,
	error_eventspec_key,
	error_eventspec_rel,
	error_eventspec_rel2,
	error_actiontype,
	error_action,
	error_keysym,
#if 0
	error_keycode,
#endif
	
	error_reserved
} parse_error_t;


struct action {
	char           * name;
	action_type_t    type;
	int           (*parser)  (int line_num, event_rule_t *, char * string);
	void          (*handler) (struct input_event *, event_rule_t *, unsigned int mods);
};


extern struct action action_types [];


#define ACTION_DELIMS "+-:|"


/* Not even emacs could reach this limit of simultaneous keypresses. */

#define MAX_KEYCODES 256



extern int            num_rules;

extern event_rule_t * ruleset;

int parse (char * fname);

void print_parse_error (parse_error_t parse_error, char * arg);

#endif /* PARSER_H */
