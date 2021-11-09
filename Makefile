CC       =	c++
CFLAGS   =	-O2 -Wall -Wunused -lpthread `fltk-config --use-images --cxxflags --ldflags` \
			--std=c++11 \
			`pkg-config --cflags --libs libvncclient libvncserver libssh2`
DEBUGFLGS=	-g -O0
BINDIR   = /usr/local/bin
TARGET   =	spiritvnc-fltk
SRC 	 =	`ls src/*.cxx`
PKGCONF  =	`which pkg-config`
LIBXPM   =
OSNAME   = $(shell uname -s)

# don't include X11 stuff for mac
ifeq ($(OSNAME), Darwin)
	LIBXPM =
else
	LIBXPM = -lXpm
endif

spiritvnc-fltk:
	@echo "Building on '$(OSNAME)'"
	@echo ""

	@if [ -z $(PKGCONF) ]; then \
		echo " " ; \
		echo "#### error: 'pkg-config' not found ####" ; \
		exit 1 ; \
	fi

	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LIBXPM)

debug:
	@echo "Building debug on '$(OSNAME)'"
	@echo ""

	@if [ -z $(PKGCONF) ]; then \
		echo " " ; \
		echo "#### error: 'pkg-config' not found ####" ; \
		echo "## Please install pkg-config and try again" ; \
		exit 1 ; \
	fi

	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LIBXPM) $(DEBUGFLGS)

.PHONY: clean
clean::
	rm -f $(TARGET)

install:
	install -c -s -o root -m 555 $(TARGET) $(BINDIR)

uninstall:
	@if [ -f $(BINDIR)"/"$(TARGET) ] ; then \
		rm -fv $(BINDIR)"/"$(TARGET) ; \
	fi
