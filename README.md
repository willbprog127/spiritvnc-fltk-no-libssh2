# SpiritVNC - FLTK
SpiritVNC - FLTK is an FLTK-based multi-view VNC client for most Linux distros, macOS, FreeBSD, OpenIndiana and *experimentally* on Windows 10 and 11 (using [MSYS2](https://www.msys2.org/)).  SpiritVNC features VNC-through-SSH, reverse (listening) VNC connections and timed scanning of connected viewers.  SpiritVNC - FLTK may run on other Unix-like systems using X11, as well.

#### A note about NOT using libssh2
Because of unfavorable changes with libssh2, I was forced to develop this non-libssh2 version of SpiritVNC - FLTK.  This version uses a system's built-in or readily-installed SSH client.  Most Linux distros, recent macOS and Windows releases and BSDs have the `ssh` command ready for use.  If not, it usually can be installed easily enough.

Using the system's SSH client with SpiritVNC is NOT an elegant solution, and you may encounter some freezes while connections are closed in the background.  On some OSs the child `ssh` processes won't even close properly.  I keep working and experimenting, and eventually I'll find some better way to do this.  Any suggestions are welcome! 👍

Ultimately libssh2 isn't used anymore because it cannot reliably use *only* a local private key.  Even when a public *and* private key are provided, libssh2 still doesn't work properly.  I'm unsure how anyone else is successfully using local private keys with libssh2.

#### Why not libssh?
Based on testing, libssh (not 'libssh2') does not work properly in the threads that are used in this project. 🤷‍♂️

#### Why FLTK?
FLTK seems to have the best balance of being lightweight, capable and cross-platform.  While FLTK *does* look fairly dated, that doesn't bother me right now.  Maybe FLTK 1.4 will have some more modern-looking themes...?  The FLTK community and contributors are very helpful, understanding and accommodating while the teams for other toolkits have often been abrasive and unhelpful.

#### Why not GTK or Qt?
I'm not a big fan of how the GNOME and GTK projects are being run, so I'm no longer working on a GTK version.  Qt licensing has always been perplexing to me, so I'd rather not invest a whole bunch of work into a Qt version only to be sued by them for some weird reason.

#### How about a native macOS Cocoa version?
Just -- no.  Build on macOS as shown below.  I worked tirelessly (quite literally) on a native Cocoa version and, while it was getting there, it didn't meet my standards.  Developing for macOS is like working for a merciless and unrelenting taskmaster.  When macOS development becomes more sane, I'll take a look, but I'm not planning on purchasing any more Apple hardware unless it's donated -- I no longer have a modern Mac.

#### Why C++ and not *(Rust, Go, Lua, Free Pascal, Java, Xojo, Bash, Python, assembly, etc)*
I only have *half* a brain -- come on! 😉  I did make an earlier private version of SpiritVNC using Python and GObject introspection, but each Linux distro packaged things differently and I grew tired of the GTK project's silliness.

#### Does it work on Windows?
Yes.  I use SpiritVNC-FLTK in a production environment on Windows 10 and 11.  Please note, however, that the user experience is far from great; mostly due to terminal windows opening when connecting to VNC-through-SSH servers and [Windows-specific libvncclient bugs](https://github.com/LibVNC/libvncserver/issues?q=is%3Aissue+is%3Aopen+windows).  On Windows, SpiritVNC-FLTK will *only* compile and run *under the MSYS2 system* for now.  Double-clicking the compiled exe from Explorer will yield an error because *all* of SpiritVNC-FLTK's dependencies need to be copied to the same folder where `spiritvnc-fltk.exe` is located.  A pre-compiled Windows installer is planned but not yet available.

#### Is it localized for my language?
Unless your language is English, no.  It is NOT out of disrespect for your particular language, I just don't have the time or resources to maintain different localizations and I feel adding a localization mechanism to SpiritVNC-FLTK would add complexity and increase executable size.  If anyone has any feedback or ideas for _lightweight_ localization, just create an issue.

2016-2025 Will Brokenbourgh
https://www.willbrokenbourgh.com/brainout/

To God be the glory! 😀 :heart:

SpiritVNC - FLTK has a 3-Clause BSD License

![SpiritVNC - FLTK screenshot](https://www.willbrokenbourgh.com/media/spiritvnc-fltk-screenshot-2023-04-07--10-55.png)

![SpiritVNC - FLTK screenshot on Windows 11](https://www.willbrokenbourgh.com/media/spiritvnc-fltk-screenshot-windows-2023-08-22.png)

- - - -

__Dependencies__
> [!NOTE]
> The current release of FLTK is 1.4.x.  SpiritVNC-FLTK hasn't been optimized for FLTK 1.4.x yet, so you may see deprecation warnings when building.  Please file an issue if you get errors due to this.

You will need both the libraries and development packages of the following:
- fltk 1.3.4 or newer
- libvncserver / libvncclient (if separate, you only need libvncclient)
- pkg-config program must be installed unless you want to specify locations for includes and libs manually in the Makefile
- An installed SSH client runable with the `ssh` command or similar (adjustable within program settings)
- GNU make must be installed on OpenIndiana and FreeBSD

**macOS**
> [!NOTE]
> I no longer have access to a modern Mac so _you_ will have to test any fixes I make.
* Intel - [Homebrew](https://brew.sh/) is the recommended way to install dependencies on macOS Intel. 
* Apple Silicon - I have no access to an Apple Silicon Mac, so your guess is as good as mine _(maybe Homebrew?)_

**Windows**
* Intel - [MSYS2](https://www.msys2.org/) is the recommended way to install dependencies on x86_64 versions of Windows 10 and 11.  libvncclient has some [Windows-specific bugs](https://github.com/LibVNC/libvncserver/issues?q=is%3Aissue+is%3Aopen+windows) that may not get fixed right away.
* ARM - No Windows ARM machines are available to me, so use what's best for you.

- - -

__Building__

SpiritVNC - FLTK requires a compiler that supports C++11.

`cd` into the directory that has the Makefile, then...

On most Linux distros, macOS and MSYS2 on Windows (where GNU make is default):
```sh
$ make [debug]
```

On FreeBSD, OpenBSD, OpenIndiana (GNU make must be installed first):
```sh
$ gmake [debug]
```
> [!IMPORTANT]
> Using `make install` or `gmake install` is not recommended on any OS right now.
- - -
__Usage__

`cd` to the directory where `spiritvnc-fltk` was built, then...
```sh
$ ./spiritvnc-fltk
```

##### Basic usage
* Double-click a disconnected server entry to try to connect to it
* Single-click a connected server entry to switch to it from another
* Right-click a connected server entry to close the connection *(except 'Listening' entries)*
* Right-click a disconnected server entry to display a pop-up menu with various actions and custom commands you can perform
* Click a server entry, then click in the Quick Note box near the bottom left of SpiritVNC's window to enter a brief message.  Press Enter to save or Esc to cancel.  Any notes entered for 'Listening' connections are temporary and will not be saved

When viewing a remote VNC server:
* Pressing the F8 key will display F8, F11, F12 and common key combinations that can be sent to the remote host
* Pressing the F11 key will toggle semi-fullscreen mode.  Fullscreen will not work when disconnected from a remote host
* If the current connection has an F12 macro, pressing the F12 key will send it, otherwise the F12 key itself will be sent

##### Server entry list buttons
* Click the [+] button to add a new server entry, click the [-] button to delete the selected server entry
* Click the [up arrow] button or [down arrow] button to move the selected server entry up or down the list
* Click the [timer icon] button to begin or stop timed scanning of all connected servers. Click in the viewer or press a key to stop scanning
* Click the [ear] button to start listening for reverse VNC connections on port 5500
* Click the [?] button for version and help information
* Click the [three control sliders icon] button to adjust program settings

##### Application settings / options
|Option|Description|
|------|-----------|
|**Scan wait time (seconds)**| The amount of time in seconds the program will wait before switching to the next connected server entry in the list during timed scanning (the wait time is approximate; the program may switch to another server entry sooner than this number)|
|**Starting local SSH port number**| If your operating system is stubborn about which port numbers to use, adjust this number higher|
|**Inactive connection timeout (seconds)**| The amount of time the program will wait before auto-disconnecting a connected server entry due to no activity on the remote VNC server's screen.  Auto-disconnect can be enabled or disabled per-server entry|
|**SSH command**| The full path and command name for your system's installed SSH client program (ie: /usr/bin/ssh)|
| | |
|*Appearance Options*|
|**Application font size**| The font size used for most labels and text-entry boxes|
|**List font name**| The desired font name (not file name) used to display the server list entries|
|**List font size**| The size of the font used to display the server list entries.  This could be points or pixels, depending on your OS|
|**List width**| The desired width of the server list.  This could be pixels or device units, depending on your OS|
|**Use icons for color-blind users**| Will use uncolored uniquely-shaped connection status icons for those with color-blindness|
|**Show tooltips**| Shows tooltips on most widgets|
|**Show reverse connection notification**| When enabled, pops up a dialog box notifying you of an incoming reverse VNC connection|

> [!NOTE]
> Be sure to click [Save], then quit and restart SpiritVNC for font or icon changes, because that's how I roll

##### Server entry edit help
|Option|Description|
|------|-----------|
|[VNC options tab]| |
|**Connection name**| The unique name you enter to recognize this connection in the list|
|**Connection group**| Use the same group name for all computers in one location (Home, Office, Customer1, etc)|
|**Remote address**| The IPv4 address of the remote VNC or VNC-over-SSH server (IPv6 not supported by libvncclient yet https://github.com/LibVNC/libvncserver/issues/436)|
|**F12 macro**| Press F12 when viewing a remote server to send this text, such as frequently-used phrases, passwords, etc|
|**VNC**| This connection connects directly to a VNC server|
|**VNC through SSH**| This connection connects to a VNC server through SSH port forwarding|
|**VNC port**| The port that the remote VNC server is listening on|
|**VNC password**| The password used to access the remote VNC server (not used with login authentication)|
|**VNC login user name**| If the remote VNC server requires a login username, this is used (ie: macOS, etc)|
|**VNC login password**| If the remote VNC server requires a login password, this is used (ie: macOS, etc)|
|**Compression level**| The amount of desired compression from the remote VNC server, 0 (none) to 9 (full)|
|**Quality level**| The desired image quality from the remote VNC server, 0 (very poor) to 9 (best)|
|**Scale off (scroll)**| The image from the remote VNC server will not be resized to SpiritVNC's viewer but scrolled|
|**Scale up and down**| The image from the remote VNC server will be scaled to fit SpiritVNC's viewer|
|**Scale down only**| The image from the remote VNC server will only be scaled down.  Remote screens slightly equal or smaller than SpiritVNC's viewer will not be scaled up|
| | |
|[SSH options tab]| |
|**SSH user name**| The name used when authenticating to the remote SSH server|
|**SSH remote port**| The port used by the remote SSH server (usually 22)|
|**SSH private key**| Private key identity file to use if your account on the remote SSH server requires it.  Use the [...] button to display a file chooser for the desired private key or simply type the full path to it|
| | |
|[Custom commands tab]| |
|**Command _n_ enabled**| Put a checkmark in this box to enable and show this command in this item's disconnected right-click menu|
|_(first text-box)_| This is the command's label which displays in this item's disconnected right-click menu when enabled|
|_(second text-box)_| This is the command that is run when you click its label in this items's disconnected right-click menu|
| | Note: Custom commands are run by right-clicking a _disconnected_ connection item, then clicking the custom command(s) toward the bottom of the right-click menu (if it/they are enabled in this item's settings).  If the command entered in this item's settings runs successfully, you will _not_ receive a message about it.  If the command returns a non-zero result (usually an error), a dialog box will display showing which command you attempted to run and the exit code of that failing command|
| | |
|[Buttons at bottom of window]| |
|**[Delete]**| Displays a confirmation if you'd like to delete this server entry|
|**[Cancel]**| Abandons any changes made to this server entry and closes the edit window|
|**[Save]**| Saves any changes made to this server entry and closes the edit window|

##### Other information

> [!NOTE]
> You can only have 65,000 connection entries in the host-list (should be more than enough!)
- - -

__Hey!__

If you have an issue, need something tweaked or fixed, let me know.  I have a busy schedule, but I want my software to work for you.  Please file an Issue and I'll address it, one way or another. :smile:
