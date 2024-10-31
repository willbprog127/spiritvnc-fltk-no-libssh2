/*
 * app.h - part of SpiritVNC - FLTK
 * 2016-2024 Will Brokenbourgh https://www.willbrokenbourgh.com/brainout/
 */

/*
 * (C) Will Brokenbourgh
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef APP_H
#define APP_H

/* === windows only === */
#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif

#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_draw.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/filename.H>
#include <FL/Fl_Help_View.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Image_Surface.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Window.H>

#include <fstream>

/* === *nix-like only == */
#ifndef _WIN32
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#endif

#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <signal.h>

#include "base64.h"
#include "consts_enums.h"
#include "hostitem.h"
#include "pixmaps.h"
#include "vnc.h"
#include "ssh.h"

class SVInput;
class SVQuickNoteInput;
class SVQuickNoteBox;
class SVQuickNotePack;
class SVItmOpt;

/* global app class */
class AppVars
{
public:
  AppVars() :
    mainWin(NULL),
    hostList(NULL),
    scroller(NULL),
    vncViewer(NULL),
    iconApp(NULL),
    iconDisconnected(NULL),
    iconDisconnectedError(NULL),
    iconDisconnectedBigError(NULL),
    iconConnected(NULL),
    iconNoConnect(NULL),
    iconConnecting(NULL),
    libVncVncPointer(strdup("VncObject")),
    configPath(""),
    configPathAndFile(""),
    requestedListWidth(170),
    nViewersWaiting(0),
    colorBlindIcons(false),
    shuttingDown(false),
    childWindowVisible(false),
    btnListAdd(NULL),
    btnListDelete(NULL),
    btnListHelp(NULL),
    btnListOptions(NULL),
    btnListUp(NULL),
    btnListDown(NULL),
    btnListListen(NULL),
    btnListScan(NULL),
    childWindowBeingDisplayed(NULL),
    itmBeingEdited(NULL),
    scanIsRunning(false),
    nCurrentScanItem(0),
    nScanTimeout(2),
    nStartingLocalPort(15000),
    showTooltips(true),
    enableLogToFile(false),
    debugMode(false),
    #ifdef _WIN32
    nAppFontSize(12),
    #else
    nAppFontSize(10),
    #endif
    strListFont("Sans"),
    #ifdef _WIN32
    nListFontSize(12),
    #else
    nListFontSize(10),
    #endif
    nMenuFontSize(11),
    blockLocalClipboardHandling(false),
    showReverseConnect(true),
    savedX(64),
    savedY(64),
    savedW(800),
    savedH(600),
    //maximized(false),
    createdObjects(0),
    strF12ClipVar(""),
    sshCommand("ssh"),
    quickInfoPack(NULL),
    quickInfoLabel(NULL),
    lastConnectedLabel(),
    lastConnected(NULL),
    lastErrorBox(NULL),
    quickNoteBox(NULL),
    quickNotePack(NULL),
    quickNoteInput(NULL),
    packButtons(NULL)
  {
    std::string userName = "";

    // get user's login name for reading/writing config file

    // linux / bsd
    if (getenv("USER") != NULL)
      userName = getenv("USER");

    // solaris / openindiana / ?
    if (userName == "" && getenv("LOGNAME") != NULL)
      userName = getenv("LOGNAME");

    // Windows 10+
    if (userName == "" && getenv("USERNAME") != NULL)
      userName = getenv("USERNAME");

    // uh-oh, can't figure out user's name
    if (userName == "")
    {
      std::cout << "SpiritVNC - FLTK: CRITICAL - Could not get user's login name"
          " from environment\n\nExiting\n";

      fl_message_hotspot(0);
      fl_message_title("SpiritVNC - FLTK");
      fl_message("%s", "CRITICAL ERROR - Could not get user's login name"
          " from environment\n\nExiting\n");

      exit(1);
    }

    // set up config file path and file

    // === macOS / OS X ===
    #if defined __APPLE__
    configPath = "/Users/" + userName + "/.spiritvnc/";
    // === Windows 10+ ===
    #elif defined _WIN32
    configPath = std::string(getenv("APPDATA")) + "\\SpiritVNC\\";
    // === solaris or openindiana ===
    #elif defined __sun__
    configPath = "/export/home/" + userName + "/.spiritvnc/";
    // === default linux, freebsd, others ===
    #else
    configPath = "/home/" + userName + "/.spiritvnc/";
    #endif

    // build full path
    configPathAndFile = configPath + "spiritvnc-fltk.conf";
  }

  Fl_Window * mainWin;
  Fl_Hold_Browser * hostList;
  Fl_Scroll * scroller;
  VncViewer * vncViewer;
  Fl_RGB_Image * iconApp;
  Fl_Image * iconDisconnected;
  Fl_Image * iconDisconnectedError;
  Fl_Image * iconDisconnectedBigError;
  Fl_Image * iconConnected;
  Fl_Image * iconNoConnect;
  Fl_Image * iconConnecting;
  void * libVncVncPointer;
  std::string configPath;
  std::string configPathAndFile;
  int requestedListWidth;
  int nViewersWaiting;
  bool colorBlindIcons;
  bool shuttingDown;
  bool childWindowVisible;
  Fl_Button * btnListAdd;
  Fl_Button * btnListDelete;
  Fl_Button * btnListHelp;
  Fl_Button * btnListOptions;
  Fl_Button * btnListUp;
  Fl_Button * btnListDown;
  Fl_Button * btnListListen;
  Fl_Button * btnListScan;
  Fl_Window * childWindowBeingDisplayed;
  HostItem * itmBeingEdited;
  bool scanIsRunning;
  int nCurrentScanItem;
  uint16_t nScanTimeout;
  int nStartingLocalPort;
  bool showTooltips;
  bool enableLogToFile;
  bool debugMode;
  int nAppFontSize;
  std::string strListFont;
  int nListFontSize;
  int nMenuFontSize;
  bool blockLocalClipboardHandling;
  bool showReverseConnect;
  int savedX;
  int savedY;
  int savedW;
  int savedH;
  //bool maximized;
  int createdObjects;
  std::string strF12ClipVar;
  std::string sshCommand;
  Fl_Pack * quickInfoPack;
  Fl_Box * quickInfoLabel;
  Fl_Box * lastConnectedLabel;
  Fl_Box * lastConnected;
  Fl_Multiline_Output * lastErrorBox;
  SVQuickNoteBox * quickNoteBox;
  SVQuickNotePack * quickNotePack;
  SVQuickNoteInput * quickNoteInput;
  Fl_Pack * packButtons;
} extern * app;


/* subclassed input box */
class SVInput : public Fl_Input
{
public:
  SVInput (int x, int y, int w, int h, const char * label = 0) :
    Fl_Input(x, y, w, h, label) {}
  int handle (int event);
};

/* subclassed password input box */
class SVSecretInput : public Fl_Secret_Input
{
public:
  SVSecretInput (int x, int y, int w, int h, const char * label = 0) :
    Fl_Secret_Input(x, y, w, h, label) {}
private:
  int handle (int evt);
};

/* subclassed box */
class SVQuickNoteBox : public Fl_Multiline_Output
{
public:
  SVQuickNoteBox (int x, int y, int w, int h, const char * label = 0) :
    Fl_Multiline_Output(x, y, w, h, label) {}
private:
  int handle (int event);
};

/* subclassed group */
class SVQuickNotePack : public Fl_Pack
{
public:
  SVQuickNotePack (int x, int y, int w, int h, const char * label = 0) :
    Fl_Pack(x, y, w, h, label) {}
private:
  int handle (int event);
};

/* subclassed input box */
class SVQuickNoteInput : public SVInput
{
public:
  SVQuickNoteInput (int x, int y, int w, int h, const char * label = 0) :
    SVInput(x, y, w, h, label) {}
private:
  int handle (int event);
};


/* forward function declarations */
void svBlinkCursor (void *);
void svCloseChildWindow (Fl_Widget *, void *);
void svCloseSSHConnection (void *);
void svConfigCreateNewDir ();
void svConfigReadCreateHostList ();
void svConfigWrite ();
void svConnectionWatcher (void *);
void svCreateAppIcons (const bool fromAppOptions = false);
std::string svConvertBooleanToString (bool);
bool svConvertStringToBoolean (const std::string&);
void svCreateGUI ();
void svCreateQuickNoteEditWidgets ();
void svDebugLog (const std::string&);
void svDeleteItem (const int);
void svDeselectAllItems ();
void svEnableDisableTooltips ();
int svFindFreeTcpPort ();
std::string svGetConfigProperty (const char *);
std::string svGetConfigValue (const char *);
void svHandleAppOptionsButtons ();
void svHandleItmOptionsButtons (Fl_Widget *, void *);
void svHandleLocalClipboard (const int, void *);
void svHandleHostListButtons (Fl_Widget *, void *);
void svHandleHostListEvents (Fl_Widget *, void *);
void svHandleMainWindowEvents (Fl_Widget *, void *);
void svPositionWidgets ();
void svHandleListItemIconChange (void *);
void svHandleThreadConnection (void *);
void svHandleThreadCursorChange (void *);
void svHideQuickNoteEditWidgets ();
void svInsertEmptyItem ();
int svItemNumFromItm (const HostItem *);
void svItmOptionsChoosePrvKeyBtnCallback (Fl_Widget *, void *);
void svItmOptionsRadioButtonsCallback (Fl_Widget *, void *);
void svListeningModeBegin ();
void svListeningModeEnd ();
void svLogToFile (const std::string&);
std::string svMakeTimeStamp (bool dashSeps = true);
void svMessageWindow (const std::string&, const std::string& = "SpiritVNC");
bool svThereAreConnectedItems ();
void svPopUpEditMenu (Fl_Input_ *);
void svQuickInfoSetLabelAndText (HostItem *);
void svQuickInfoSetToEmpty ();
void svResizeScroller ();
void svRestoreWindowSizePosition (void *);
void svRunCommand(const std::string&, const std::string&);
void * svRunCommandHelper(void *);
void svScanTimer (void *);
void svSendKeyStrokesToHost (const std::string&, const VncObject *);
void svSetAppTooltips ();
void svShowAboutHelp ();
void svShowAppOptions ();
void svShowF8Window ();
void svShowItemOptions (HostItem *);
void svUpdateHostListItemText ();

#endif
