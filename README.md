__SpiritVNC - FLTK__

SpiritVNC - FLTK is an FLTK-based multi-view VNC client for Unix-like systems, including Linux, macOS and FreeBSD.
SpiritVNC features VNC-through-SSH, reverse (listening) VNC connections and timed scanning of
connected viewers.

_(OpenIndiana was previously supported, but changes to their packages has caused some problems.  Please contact me if you need SpiritVNC - FLTK working on OI)_

2016-2021 Will Brokenbourgh
https://www.pismotek.com/brainout/

To God be the glory! :-D :heart:

SpiritVNC - FLTK has a 3-Clause BSD License

![SpiritVNC - FLTK screenshot](https://www.pismotek.com/media/spiritvnc-fltk-2020-11.png)



- - - -

__Dependencies__

You will need both the libraries and development packages of the following:
- fltk 1.3.4, 1.3.5 or newer
- libvncserver / libvncclient (if separate, you only need libvncclient)
- libssh2 (NOT libssh)

The 'pkg-config' program must be installed for building, unless you want to specify locations for includes and libs manually in the Makefile.

[MacPorts](https://www.macports.org) is highly recommended for installing dependencies on macOS.


- - -

__Building__

SpiritVNC - FLTK now requires a compiler that supports C++11.

'cd' into the directory that has the Makefile, then...

On most Linux distros and macOS:
```sh
$ make [debug]
$ sudo make install   # (don't use this command when building on macOS)
```

On FreeBSD, OpenBSD and others:
```sh
$ gmake [debug]
$ sudo gmake install
```
- - -

__Hey!__

If you have an issue, need something tweaked or fixed, let me know.  I have a busy schedule, but I want my software to work for you.  Please file an Issue and I'll address it, one way or another. :smile:
