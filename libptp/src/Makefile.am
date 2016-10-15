EXTRA_DIST = ptp-pack.c

lib_LTLIBRARIES = libptp2.la

libptp2_la_SOURCES = ptp.c ptp.h properties.c
libptp2_la_LDFLAGS = -version-info @LIBPTP2_VERSION_INFO@

libptp2includedir = $(includedir)/libptp2
libptp2include_HEADERS = ptp.h libptp-endian.h libptp-stdint.h

DISTCLEANFILES = libptp-stdint.h libptp-endian.h

if PTPCAM
bin_PROGRAMS = ptpcam
if LINUX_OS
ptpcam_SOURCES = ptpcam.c ptpcam.h myusb.c
else 
ptpcam_SOURCES = ptpcam.c ptpcam.h
endif
ptpcam_LDADD = -lptp2 @PTPCAM_LDFLAGS@
ptpcam_DEPENDENCIES = libptp2.la
ptpcam_CFLAGS = @PTPCAM_CFLAGS@
endif
