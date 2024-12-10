# Process w/ automake.  Or, autoreconf; make #
AUTOMAKE_OPTIONS = foreign
SUBDIRS = . doc sitelist examples
INCLUDES = -I. -I$(srcdir) -I$(prefix)/include -I$(prefix)/toolkit/include

includedir = $(prefix)/include 

lib_LTLIBRARIES = libgsl.la

libgsl_la_LDFLAGS = -version-info 1:4
libgsl_la_SOURCES = gsl.c gsl_to_hdf.c hdf_to_gsl.c get_GV_gauge_info.c

libgsl_la_DEPENDENCIES = $(build_headers)
$(libgsl_la_SOURCES): $(build_headers)

build_headers = gsl.h

gsl.h: Makefile
	@for p in $(build_headers); do \
	   echo -n "Checking file $$p ... ";\
	   cp $$p $$p.in;\
	   sed -e 's/GSL_VERSION_STR.*/GSL_VERSION_STR \"$(PACKAGE)-$(VERSION)\"/' \
	   < $$p.in > $$p.new;\
	   if cmp -s $$p $$p.new; then \
          rm $$p.new; \
          echo "$$p remains untouched."; \
       else \
          mv $$p.new $$p; \
          echo "substitutions made in $$p."; \
       fi;\
	   rm -f $$p.in;\
    done

install-exec-hook:
	$(INSTALL) -m 444 gsl.h $(includedir)

EXTRA_DIST = CHANGES $(build_headers)
