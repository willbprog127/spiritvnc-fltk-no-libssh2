# SpiritVNC - FLTK
SpiritVNC - FLTK is an FLTK-based multi-view VNC client for most Linux distros, macOS, FreeBSD, OpenIndiana and *experimentally* on Windows 10 and 11.  SpiritVNC features VNC-through-SSH, reverse (listening) VNC connections and timed scanning of connected viewers.  SpiritVNC - FLTK may run on other Unix-like systems using X11, as well.

#### A note about NOT using libssh2
Because of changes the last year or two with libssh2, I was forced to develop this non-libssh2 version of SpiritVNC - FLTK.  This version uses the system's built-in or readily-installed SSH client.  Most Linux distros, recent macOS releases and BSDs have the `ssh` command ready for use.  If not, it usually can be installed easily enough.  You can even install a SSH client on Windows 10 and higher.

Using the system's SSH client with SpiritVNC is NOT an elegant solution, and you may encounter some freezes while connections are closed in the background.  On some OSs the child `ssh` processes won't even close properly.  I keep working and experimenting, and eventually I'll find some better way to do this.  Any suggestions are welcome! 👍

#### Does it work on Windows?
While testing Windows 11 recently, I *have* been able to get SpiritVNC-FLTK working on it, although easy inclusion and static compiling of dependencies has been a challenge.  Windows-specific code is upcoming, although the user experience is far from great; mostly due to terminal windows opening when connecting to VNC-through-SSH servers and [Windows-specific libvncclient bugs](https://github.com/LibVNC/libvncserver/issues?q=is%3Aissue+is%3Aopen+windows).  Check back often to see what's happening with this.

2016-2023 Will Brokenbourgh
https://www.pismotek.com/brainout/

To God be the glory! :-D :heart:

SpiritVNC - FLTK has a 3-Clause BSD License

![SpiritVNC - FLTK screenshot](https://www.pismotek.com/media/spiritvnc-fltk-screenshot-2023-04-21--15-02.png?)



- - - -

__Dependencies__

You will need both the libraries and development packages of the following:
- fltk 1.3.4, 1.3.5 or newer
- libvncserver / libvncclient (if separate, you only need libvncclient)
- An installed SSH client runable with the `ssh` command or similar (adjustable within program settings)

The 'pkg-config' program must be installed for building, unless you want to specify locations for includes and libs manually in the Makefile.

**macOS - Intel** [Homebrew](https://brew.sh/) is now the recommended way to install dependencies on macOS, at least on Intel.  I stopped using MacPorts because too many bugs affected packages I needed.  Also, I don't have access to an Apple Silicon Mac (M1, M2, etc) so patches from people who *do* have these machines would be helpful.

**Windows** [MSYS2](https://www.msys2.org/) is the recommended way to install dependencies on x86_64 versions of Windows 10 and 11.  No Windows ARM machines are available to me, so testing is non-existent.  libvncclient has some [Windows-specific bugs](https://github.com/LibVNC/libvncserver/issues?q=is%3Aissue+is%3Aopen+windows) that may not get fixed right away.


- - -

__Building__

SpiritVNC - FLTK requires a compiler that supports C++11.

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
* Click a server entry, then click in the Quick Note box near the bottom left of SpiritVNC's window to enter a brief message
* When viewing a remote VNC server, press the F8 key to display some common key combinations to send or to send F8 or F12 keys

##### Server entry list buttons
* Click the [+] button to add a new server entry, click the [-] button to delete the selected server entry
* Click the [up arrow] button or [down arrow] button to move the selected server entry up or down the list
* Click the [timer icon] button to begin timed scanning of all connected servers
* Click the [ear] button to start listening for reverse VNC connections on port 5500
* Click the [?] button for version and help information
* Click the [three control sliders icon] button to adjust program settings

##### Application settings / options
* **Scan wait time (seconds)**: The amount of time in seconds the program will wait before switching to the next connected server entry in the list during timed scanning (the wait time is approximate; the program may switch to another server entry sooner than this number)
* **Starting local SSH port number**: If your operating system is stubborn about which port numbers to use, adjust this number higher
* **Inactive connection timeout (seconds)**: The amount of time the program will wait before auto-disconnecting a connected server entry due to no activity on the remote VNC server's screen.  Auto-disconnect can be enabled or disabled per-server entry
* **SSH command**: The full path and command name for your system's installed SSH client program (ie: /usr/bin/ssh)

*Appearance Options*
* **Application font size**: The font size used for most labels and text-entry boxes
* **List font name**: The desired font name (not file name) used to display the server list entries
* **List font size**: The size of the font used to display the server list entries.  This could be points or pixels, depending on your OS
* **List width**: The desired width of the server list.  This could be pixels or device units, depending on your OS
* **Use icons for color-blind users**: Will use uncolored uniquely-shaped connection status icons for those with color-blindness
* **Show tooltips**: Shows tooltips on most widgets
* **Show reverse connection notification**: When enabled, pops up a dialog box notifying you of an incoming reverse VNC connection

*Be sure to click [Save], then quit and restart SpiritVNC for font or icon changes, because that's how I roll*

##### Server entry edit help
* **Connection name**: The unique name you enter to recognize this connection in the list
* **Connection group**: Use the same group name for all computers in one location (Home, Office, Customer1, etc)
* **Remote address**: The IPv4 address of the remote VNC or VNC-over-SSH server (IPv6 not supported by libvncclient yet https://github.com/LibVNC/libvncserver/issues/436)
* **F12 macro**: Use F12 when viewing a remote server to send this text, such as frequently-used phrases, passwords, etc
* **VNC**: This connection connects directly to a VNC server
* **VNC through SSH**: This connection connects to a VNC server through SSH port forwarding
* **VNC port**: The port that the remote VNC server is listening on
* **VNC password**: The password used to access the remote VNC server (not used with login authentication)
* **VNC login user name**: If the remote VNC server requires a login username, this is used (ie: macOS, etc)
* **VNC login password**: If the remote VNC server requires a login password, this is used (ie: macOS, etc)
* **Compression level**: The amount of desired compression from the remote VNC server, 0 (none) to 9 (full)
* **Quality level**: The desired image quality from the remote VNC server, 0 (very poor) to 9 (best)
* **Auto-disconnect when inactive**: This connection will automatically disconnect if there's no activity from the remote VNC server after the time set in program settings (This used to be labeled "Don't auto-disconnect when inactive" which was unclear)
* **Scale off (scroll)**: The image from the remote VNC server will not be resized to SpiritVNC's viewer but scrolled
* **Scale up and down**: The image from the remote VNC server will be scaled to fit SpiritVNC's viewer
* **Scale down only**: The image from the remote VNC server will only be scaled down.  Remote screens slightly equal or smaller than SpiritVNC's viewer will not be scaled up

*VNC through SSH options*
* **SSH user name**: The name used when authenticating to the remote SSH server
* ~~**SSH password**: The password, if any, used when authentication to the remote SSH server~~ Currently deprecated and disabled
* **SSH remote port**: The port used by the remote SSH server (usually 22)
* **SSH private key**: Private key identity file to use if your account on the remote SSH server requires it.  Use the [...] button to display a file chooser for the desired private key or simply type the full path to it

*Buttons at bottom of window*
* **[Delete]**: Displays a confirmation if you'd like to delete this server entry
* **[Cancel]**: Abandons any changes made to this server entry and closes the edit window
* **[Save]**: Saves any changes made to this server entry and closes the edit window

##### Other information

* You can only have 65,000 connection entries in the host-list (should be more than enough!)
- - -

__Hey!__

If you have an issue, need something tweaked or fixed, let me know.  I have a busy schedule, but I want my software to work for you.  Please file an Issue and I'll address it, one way or another. :smile:
