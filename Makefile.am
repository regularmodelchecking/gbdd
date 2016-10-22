AUTOMAKE_OPTIONS = 1.4

SUBDIRS = gbdd

bin_SCRIPTS = gbdd-config

EXTRA_DIST = gbdd.m4 gbdd.spec

m4datadir = $(datadir)/aclocal
m4data_DATA = gbdd.m4

if HAS_DOXYGEN

doxygen::
	doxygen Doxyfile

docdir = $(prefix)/share/doc/@PACKAGE@-@VERSION@

install-data-local: doxygen
	$(mkinstalldirs) $(DESTDIR)$(docdir)
	$(INSTALL_DATA) $(srcdir)/docs/html/* $(DESTDIR)$(docdir)
endif
