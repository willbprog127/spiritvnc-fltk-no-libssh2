__SpiritVNC - FLTK__

SpiritVNC - FLTK is an FLTK-based multi-view VNC client for most Linux distros, macOS and FreeBSD.  SpiritVNC features VNC-through-SSH, reverse (listening) VNC connections and timed scanning of connected viewers.  SpiritVNC - FLTK may run on other Unix-like systems using X11, as well.

_(OpenIndiana was previously supported, but changes to their packages has caused some problems.  Please contact me if you need SpiritVNC - FLTK working on OI)_

__Now working on macOS again!__ :smile:

__A note about NOT using libssh2__ - Because of recent issues with libssh2, I was forced to develop this non-libssh2 version of SpiritVNC - FLTK.  This version uses the system's built-in or readily-installed SSH client.  Most Linux distros, recent macOS releases and BSDs have the `ssh` command ready for use.  If not, it usually can be installed easily enough.  You can even install a SSH client on Windows 10 and higher.

Using the system's SSH client with SpiritVNC is NOT an elegant solution, and you may encounter some freezes while connections are closed in the background.  I keep working and experimenting, and eventually I'll find some better way to do this.  Any suggestions are welcome! 👍

2016-2022 Will Brokenbourgh
https://www.pismotek.com/brainout/

To God be the glory! :-D :heart:

SpiritVNC - FLTK has a 3-Clause BSD License

![SpiritVNC - FLTK screenshot](https://www.pismotek.com/media/spiritvnc-fltk-2020-11.png)



- - - -

__Dependencies__

You will need both the libraries and development packages of the following:
- fltk 1.3.4, 1.3.5 or newer
- libvncserver / libvncclient (if separate, you only need libvncclient)
- An installed SSH client runable with the `ssh` command

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
__Usage__

(this section is 'under construction')

`$ ./spiritvnc-fltk`

* You can only have 65,500 connection entries in the host-list (should be more than enough!)
- - -

__Hey!__

If you have an issue, need something tweaked or fixed, let me know.  I have a busy schedule, but I want my software to work for you.  Please file an Issue and I'll address it, one way or another. :smile:
