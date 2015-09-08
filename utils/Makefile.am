instdir = ${bindir}
bin_PROGRAMS = smackctl smackaccess smackload smackcipso smackd chsmack
AM_CPPFLAGS = \
	-I$(top_srcdir)/libsmack

smackctl_SOURCES = smackctl.c common.c
smackctl_LDADD = ../libsmack/libsmack.la
smackctl_LDFLAGS = -pie
smackctl_CFLAGS = -fPIE

smackaccess_SOURCES = smackaccess.c
smackaccess_LDADD = ../libsmack/libsmack.la
smackaccess_LDFLAGS = -pie
smackaccess_CFLAGS = -fPIE

smackload_SOURCES = smackload.c common.c
smackload_LDADD = ../libsmack/libsmack.la
smackload_LDFLAGS = -pie
smackload_CFLAGS = -fPIE

smackcipso_SOURCES = smackcipso.c common.c
smackcipso_LDADD = ../libsmack/libsmack.la
smackcipso_LDFLAGS = -pie
smackcipso_CFLAGS = -fPIE

smackd_SOURCES = smackd.c common.c
smackd_LDADD = ../libsmack/libsmack.la
smackd_LDFLAGS = -pie
smackd_CFLAGS = -fPIE

chsmack_SOURCES = chsmack.c
chsmack_LDADD = ../libsmack/libsmack.la
chsmack_LDFLAGS = -pie
chsmack_CFLAGS = -fPIE

EXTRA_DIST = common.h

SUBDIRS = smackload-fast
