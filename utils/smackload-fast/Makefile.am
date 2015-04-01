instdir = ${bindir}
bin_PROGRAMS = smackload-fast
noinst_PROGRAMS = genmodes

genmodes_SOURCES = genmodes.c
smackload_fast_SOURCES = main.c input.c output.c common.h
smackload_fast_LDFLAGS = -pie
smackload_fast_CPPFLAGS = -fPIE
nodist_smackload_fast_SOURCES = modes.h

BUILD_SOURCES: modes.h
output.c: modes.h
CLEANFILES = modes.h

modes.h: genmodes
	./genmodes > $@
