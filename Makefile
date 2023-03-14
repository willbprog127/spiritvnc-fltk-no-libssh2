CC       =	c++
CFLAGS   =	-O2 -Wall -Wunused -lpthread `fltk-config --use-images --cxxflags --ldflags` \
			--std=c++11 -finline-functions -march=x86-64
DEBUGFLGS=	-g -O0
BINDIR   = /usr/local/bin
TARGET   =	spiritvnc-fltk
SRC 	 =	`ls src/*.cxx`
PKGCONF  =	`which pkg-config`
LIBVNC	 =
OSNAME   = $(shell uname -s)


# fix OI / Solaris stuff
ifeq ($(OSNAME), SunOS)
	LIBVNC = `libvncserver-config --cflags --libs` -m64
else
	LIBVNC = `pkg-config --cflags --libs libvncclient libvncserver`
endif

spiritvnc-fltk:
	@echo "Building on '$(OSNAME)'"
	@echo ""

	@if [ -z $(PKGCONF) ]; then \
		echo " " ; \
		echo "#### error: 'pkg-config' not found ####" ; \
		exit 1 ; \
	fi

	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LIBVNC)

debug:
	@echo "Building debug on '$(OSNAME)'"
	@echo ""

	@if [ -z $(PKGCONF) ]; then \
		echo " " ; \
		echo "#### error: 'pkg-config' not found ####" ; \
		echo "## Please install pkg-config and try again" ; \
		exit 1 ; \
	fi

	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LIBVNC) $(DEBUGFLGS)

.PHONY: clean
clean::
	rm -f $(TARGET)

install:
	install -c -s -o root -m 555 $(TARGET) $(BINDIR)

uninstall:
	@if [ -f $(BINDIR)"/"$(TARGET) ] ; then \
		rm -fv $(BINDIR)"/"$(TARGET) ; \
	fi
