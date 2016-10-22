AUTOMAKE_OPTIONS = 1.4

INCLUDES=-I..

EXTRA_DIST=doxygen_mainpage.h

lib_LTLIBRARIES = libgbdd.la
libgbdd_la_SOURCES = \
	space.cc gspace.cc cudd-space.cc bdd.cc \
	buddy-space.cc domain.cc mutex-space.cc \
	structure-relation.cc bdd-relation.cc structure-constraint.cc \
	bdd-equivalence-relation.cc bool-constraint.cc

libgbdd_la_LDFLAGS = -version-info 3:0:0
libgbdd_la_LIBADD = -lpthread 


libgbddincludedir = $(includedir)/gbdd
libgbddinclude_HEADERS = \
	sgi_ext.h \
	config.h \
	domain.h \
	gspace.h \
	space.h	\
	bdd.h \
	gbdd.h \
	buddy-space.h \
	mutex-space.h \
	relation-compat.h \
	structure-constraint.h \
	structure-relation.h \
	structure-binary-relation.h \
	bdd-relation.h \
	bdd-equivalence-relation.h \
	cudd-space.h \
	bool-constraint.h 

test_programs = test-bdd test-relation

noinst_PROGRAMS = $(test_programs)

test_bdd_SOURCES = test-bdd.cc
test_bdd_LDADD = libgbdd.la

test_relation_SOURCES = test-relation.cc
test_relation_LDADD = libgbdd.la
