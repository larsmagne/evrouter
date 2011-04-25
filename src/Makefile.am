# $Id: Makefile.am 9 2009-02-18 09:09:02Z alexios $
#
# $Log$
# Revision 2.0  2004/09/07 22:57:39  alexios
# Stepped version to recover CVS repository after near-catastrophic disk
# crash.
#
# Revision 1.1.1.1  2004/09/07 22:52:34  alexios
# Initial post-crash revision, re-initialised to recover CVS repository after
# near-catastrophic disk crash.
#
# Revision 1.2  2004/01/28 21:40:09  alexios
# Added example.
#
# Revision 1.1.1.1  2004/01/28 16:21:41  alexios
# Initial imported revision.
#

evrouter_CFLAGS=$(CFLAGS) -Wall -g -D__USE_GNU $(XMMS_CFLAGS)
#LDADD=$(XMMS_LDFLAGS)
bin_PROGRAMS=evrouter
evrouter_SOURCES=\
		evrouter.c \
		cmdline.c cmdline.h \
		evdev.c evdev.h \
		parser.c parser.h \
		x11.c x11.h \
		action_shell.c action_shell.h \
		action_xkey.c action_xkey.h \
		action_xbutton.c action_xbutton.h \
		action_xmms.c action_xmms.h

#  install the man pages
man_MANS=evrouter.1

EXTRA_DIST=example
