/* 

cmdline.h -- Parse command line options

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

$Id: cmdline.h 6 2004-09-07 22:57:39Z alexios $

$Log$
Revision 2.0  2004/09/07 22:57:39  alexios
Stepped version to recover CVS repository after near-catastrophic disk
crash.

Revision 1.1.1.1  2004/09/07 22:52:34  alexios
Initial post-crash revision, re-initialised to recover CVS repository after
near-catastrophic disk crash.

Revision 1.3  2004/02/12 00:14:35  alexios
Added command line option for killing the running daemon.

Revision 1.2  2004/01/28 21:41:20  alexios
Added mode to list event devices.

Revision 1.1.1.1  2004/01/28 16:21:41  alexios
Initial imported revision.


*/


#ifndef CMDLINE_H
#define CMDLINE_H

#include "evdev.h"

extern char * progname;

extern int train_mode;
extern int device_dump_mode;
extern int verbose;
extern int foreground;
extern int sepuku;

extern int numdevs;

extern char * devnames [MAX_EVDEV];

extern char * configfile;

void cmdline_parse (int argc, char ** argv);


#endif /* CMDLINE_H */
