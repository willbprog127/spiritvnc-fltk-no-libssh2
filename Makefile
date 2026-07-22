cc_cmd   = c++
cflags   = -O2 -Wall -Wunused -lpthread $(shell fltk-config --use-images --cxxflags --ldflags) --std=c++11 \
  -finline-functions -Wunused-variable -Wunused-function -Wpedantic
dbg_flgs = -g -O0
bindir   = /usr/local/bin
target   = spiritvnc-fltk
src      = $(wildcard src/*.cxx)
pkgconf  = $(shell command -v pkg-config)
libvnc   = $(shell pkg-config --cflags --libs libvncclient libvncserver)
osname   = $(shell uname -s)

# make teh thing
spiritvnc-fltk:
	@echo "Building on '$(osname)'"
	@echo

ifeq ($(pkgconf),)
	@echo
	@echo "**** ERROR: Unable to run 'pkg-config' ****"
	@echo "**** Be sure pkg-config is installed on your system ****"
	@exit 1
endif

	$(cc_cmd) $(src) -o $(target) $(cflags) $(libvnc)
	@echo

debug:
	@echo "Building debug on '$(osname)'"
	@echo

ifeq ($(pkgconf),)
	@echo
	@echo "**** ERROR: Unable to run 'pkg-config' ****"
	@echo "**** Be sure pkg-config is installed on your system ****"
	@exit 1
endif

	$(cc_cmd) $(src) -o $(target) $(cflags) $(libvnc) $(dbg_flgs)
	@echo

.PHONY: clean
clean::
	rm -f $(target)

install:
	install -c -s -o root -m 555 $(target) $(bindir)
	@echo

uninstall:
	if [ -f $(bindir)"/"$(target) ] ; then \
		rm -fv $(bindir)"/"$(target) ; \
	fi
	@echo
