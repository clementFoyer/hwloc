# Copyright © 2017      Inria.  All rights reserved.
#
# See COPYING in top-level directory.
#
# $HEADER$
#

noinst_LTLIBRARIES = \
	libnetloc_xml_export.la

AM_CPPFLAGS = \
        -I$(top_builddir)/include \
        -I$(top_srcdir)/include

if NETLOC_HAVE_LIBXML2
AM_CPPFLAGS += \
	$(HWLOC_LIBXML2_CFLAGS)
endif

libnetloc_xml_export_la_SOURCES = \
    netloc_xml_export.c \
	netloc_xml_write.c

libnetloc_xml_export_la_LIBADD = \
	$(top_builddir)/utils/netloc/utils/xml/libutils_xml_write.la \
	$(top_builddir)/netloc/libnetloc.la

if NETLOC_HAVE_LIBXML2
libnetloc_xml_export_la_LIBADD += \
	$(HWLOC_LIBXML2_LIBS)
endif
