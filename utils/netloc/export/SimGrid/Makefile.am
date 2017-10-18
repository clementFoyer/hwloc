# Copyright © 2017      Inria.  All rights reserved.
#
# See COPYING in top-level directory.
#
# $HEADER$
#

AM_CPPFLAGS = \
        -I$(top_builddir)/include \
        -I$(top_srcdir)/include

bin_PROGRAMS = \
        netloc_topo_to_simgrid

netloc_topo_to_simgrid_SOURCES = \
        netloc_topo_to_simgrid.c

netloc_topo_to_simgrid_LDADD = \
	$(top_builddir)/netloc/libnetloc.la \
	$(top_builddir)/utils/netloc/xml/libutils_xml_write.la

if NETLOC_HAVE_LIBXML2
AM_CPPFLAGS += \
	$(HWLOC_LIBXML2_CFLAGS)

netloc_topo_to_simgrid_LDADD += \
	$(HWLOC_LIBXML2_LIBS)
endif
