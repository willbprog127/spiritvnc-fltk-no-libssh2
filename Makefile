CC       =  c++
CFLAGS   =  -O2 -Wall -Wunused -lpthread $(shell fltk-config --use-images --cxxflags --ldflags) \
			--std=c++11 -finline-functions
DEBUGFLGS=  -g -O0
BINDIR   = /usr/local/bin
TARGET   =  spiritvnc-fltk
SRC      = $(wildcard src/*.cxx)
PKGCONF  = $(shell command -v pkg-config)
LIBVNC   = $(shell pkg-config --cflags --libs libvncclient libvncserver)
OSNAME   = $(shell uname -s)


spiritvnc-fltk:
	@echo "Building on '$(OSNAME)'"
	@echo

ifeq ($(PKGCONF),)
	@echo
	@echo "**** ERROR: Unable to run 'pkg-config' ****"
	@exit 1
endif

	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LIBVNC)
	@echo

debug:
	@echo "Building debug on '$(OSNAME)'"
	@echo

ifeq ($(PKGCONF),)
	@echo
	@echo "**** ERROR: Unable to run 'pkg-config' ****"
	@exit 1
endif

	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LIBVNC) $(DEBUGFLGS)
	@echo

.PHONY: clean
clean::
	rm -f $(TARGET)

install:
	install -c -s -o root -m 555 $(TARGET) $(BINDIR)
	@echo

uninstall:
	if [ -f $(BINDIR)"/"$(TARGET) ] ; then \
		rm -fv $(BINDIR)"/"$(TARGET) ; \
	fi
	@echo
