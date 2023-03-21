# SpiritVNC - FLTK
SpiritVNC - FLTK is an FLTK-based multi-view VNC client for most Linux distros, macOS, FreeBSD and OpenIndiana.  SpiritVNC features VNC-through-SSH, reverse (listening) VNC connections and timed scanning of connected viewers.  SpiritVNC - FLTK may run on other Unix-like systems using X11, as well.

#### A note about NOT using libssh2
Because of changes the last year or two with libssh2, I was forced to develop this non-libssh2 version of SpiritVNC - FLTK.  This version uses the system's built-in or readily-installed SSH client.  Most Linux distros, recent macOS releases and BSDs have the `ssh` command ready for use.  If not, it usually can be installed easily enough.  You can even install a SSH client on Windows 10 and higher.

Using the system's SSH client with SpiritVNC is NOT an elegant solution, and you may encounter some freezes while connections are closed in the background.  On some OSs the child `ssh` processes won't even close properly.  I keep working and experimenting, and eventually I'll find some better way to do this.  Any suggestions are welcome! 👍

#### Windows version?
While testing Windows 11 recently, I *have* been able to get SpiritVNC-FLTK working on it, although easy inclusion and static compiling of dependencies has been a challenge.  Unfortunately I have not had the time to work on this lately due to higher-priority projects.  If anyone needs this, please file an issue.

2016-2023 Will Brokenbourgh
https://www.pismotek.com/brainout/

To God be the glory! :-D :heart:

SpiritVNC - FLTK has a 3-Clause BSD License

![SpiritVNC - FLTK screenshot](https://www.pismotek.com/media/spiritvnc-fltk-screenshot-2023-03-21--15-20.png?)



- - - -

__Dependencies__

You will need both the libraries and development packages of the following:
- fltk 1.3.4, 1.3.5 or newer
- libvncserver / libvncclient (if separate, you only need libvncclient)
- An installed SSH client runable with the `ssh` command

The 'pkg-config' program must be installed for building, unless you want to specify locations for includes and libs manually in the Makefile.

[Homebrew](https://brew.sh/) is now the recommended way to install dependencies on macOS, at least on Intel.  I stopped using MacPorts because too many bugs affected packages I needed.  Also, I don't have access to an Apple Silicon Mac (M1, M2, etc) so patches from people who *do* have these machines would be helpful.


- - -

__Building__

SpiritVNC - FLTK now requires a compiler that supports C++11.

'cd' into the directory that has the Makefile, then...

On most Linux distros and macOS:
```sh
$ make [debug]
$ sudo make install   # (don't use this command when building on macOS)
```

On FreeBSD, OpenBSD and others (OpenIndiana?):
```sh
$ gmake [debug]
$ sudo gmake install
```
- - -
__Usage__

`cd` to the directory where `spiritvnc-fltk` was built, then...
```sh
$ ./spiritvnc-fltk
```

##### Basic usage
* Double-click a disconnected server entry to try to connect to it
* Single-click a connected server entry to switch to it from another
* Right-click a connected server entry to close the connection *(except `Listening` entries)*
* Right-click a disconnected server entry to display a pop-up menu with various actions you can perform
* Click a server entry, then click in the Quick Note box near the bottom left to enter a brief message

##### Server entry buttons
* Click the [+] button to add a new server entry, click the [-] button to delete a server entry
* Click the [up arrow] button or [down arrow] button to move the selected server entry up or down the list
* Click the [timer icon] button to begin timed scanning of all connected servers
* Click the [ear] button to start listening for reverse VNC connections on port 5500
* Click the [?] button for version and help information
* Click the [three control sliders icon] button to adjust program settings

*(You can only have 65,000 connection entries in the host-list (should be more than enough!))*
- - -

__Hey!__

If you have an issue, need something tweaked or fixed, let me know.  I have a busy schedule, but I want my software to work for you.  Please file an Issue and I'll address it, one way or another. :smile:
