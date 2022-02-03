/*
 * app.h - part of SpiritVNC - FLTK
 * 2016-2021 Will Brokenbourgh https://www.pismotek.com/brainout/
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
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Window.H>

#include <fstream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <stdlib.h>
#include <signal.h>

#include "base64.h"
#include "consts_enums.h"
#include "hostitem.h"
#include "pixmaps.h"
#include "vnc.h"
#include "ssh.h"


/* global app class */
class AppVars
{
public:
    AppVars() :
        mainWin(NULL),
        hostList(NULL),
        scroller(NULL),
        vncViewer(NULL),
        iconDisconnected(NULL),
        iconDisconnectedError(NULL),
        iconDisconnectedBigError(NULL),
        iconConnected(NULL),
        iconNoConnect(NULL),
        iconConnecting(NULL),
        libVncVncPointer((void *)"VncObject"),
        userName(""),
        configPath(""),
        configPathAndFile(""),
        nConnectionTimeout(SV_CONNECTION_TIMEOUT_SECS),
        nViewersWaiting(0),
        verboseLogging(false),
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
        nMainWinPreviousW(0),
        nMainWinPreviousH(0),
        nScanTimeout(2),
        nDeadTimeout(100),
        nStartingLocalPort(15000),
        showTooltips(true),
        debugMode(false),
        nAppFontSize(10),
        strListFont("Sans"),
        nListFontSize(10),
        nMenuFontSize(11),
        blockLocalClipboardHandling(false),
        packButtons(NULL),
        showReverseConnect(true),
        savedX(0),
        savedY(0),
        savedW(800),
        savedH(600),
        createdObjects(0),
        msgThread(0),
        strF12ClipVar(""),
        windowIcon(NULL),
        sshCommand("ssh")
    {
        // get user's login name for reading/writing config file

        // linux / bsd
        if (getenv("USER") != NULL)
            userName = getenv("USER");

        // solaris / openindiana / ?
        if (userName.empty() == true && getenv("LOGNAME") != NULL)
            userName = getenv("LOGNAME");

        // uh-oh, can't figure out user's name
        if (userName.empty() == true)
        {
            std::cout << "SpiritVNC - FLTK: CRITICAL - Could not get user's login name"
                " from environment\n\nExiting\n";

            Fl::lock();
            fl_message_hotspot(0);
            fl_message_title("SpiritVNC - FLTK");
            fl_message("%s", "CRITICAL ERROR - Could not get user's login name"
                " from environment\n\nExiting\n");
            Fl::unlock();
            exit(1);
        }

        // set up config file path and file


        // for macOS / OS X
		#if defined __APPLE__
        configPath = "/Users/" + userName + "/.spiritvnc/";
        #elif defined __sun__
        // for solaris or openindiana
        configPath = "/export/home/" + userName + "/.spiritvnc/";
        #else
        // default is typical linux, freebsd path
        configPath = "/home/" + userName + "/.spiritvnc/";
        #endif

        // build full path
        configPathAndFile = configPath + "spiritvnc-fltk.conf";
    }

    Fl_Window * mainWin;
    Fl_Hold_Browser * hostList;
    Fl_Scroll * scroller;
    VncViewer * vncViewer;
    Fl_Image * iconDisconnected;
    Fl_Image * iconDisconnectedError;
    Fl_Image * iconDisconnectedBigError;
    Fl_Image * iconConnected;
    Fl_Image * iconNoConnect;
    Fl_Image * iconConnecting;
    void * libVncVncPointer;
    std::string userName;
    std::string configPath;
    std::string configPathAndFile;
    int nConnectionTimeout;
    int nViewersWaiting;
    bool verboseLogging;
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
    int nMainWinPreviousW;
    int nMainWinPreviousH;
    int nScanTimeout;
    int nDeadTimeout;
    int nStartingLocalPort;
    bool showTooltips;
    bool debugMode;
    int nAppFontSize;
    std::string strListFont;
    int nListFontSize;
    int nMenuFontSize;
    bool blockLocalClipboardHandling;
    Fl_Pack * packButtons;
    bool showReverseConnect;
    int savedX;
    int savedY;
    int savedW;
    int savedH;
    int createdObjects;
    pthread_t msgThread;
    std::string strF12ClipVar;
    Fl_Image * windowIcon;
    std::string sshCommand;
} extern * app;


/* subclassed input box */
class SVInput : public Fl_Input
{
public:
    SVInput (int x, int y, int w, int h, const char * label = 0) :
        Fl_Input(x, y, w, h, label) {}
private:
    int handle (int event);
};


/* subclassed password input box */
class SVSecretInput : public Fl_Secret_Input
{
public:
    SVSecretInput (int x, int y, int w, int h, const char * label = 0) :
        Fl_Secret_Input(x, y, w, h, label) {}
private:
    int handle (int event);
};


/* forward function declarations */
void svCloseChildWindow (Fl_Widget *, void *);
void svConfigCreateNew ();
void svConfigReadCreateHostList ();
void svConfigWrite ();
void svConnectionWatcher (void *);
void svCreateAppIcons (bool fromAppOptions = false);
std::string svConvertBooleanToString (bool);
bool svConvertStringToBoolean (const std::string&);
void svCreateGUI ();
void * svCreateSSHConnection(void *);
void svDebugLog (const std::string&);
void svDeleteItem (int);
void svDeselectAllItems ();
int svFindFreeTcpPort ();
std::string svGetConfigProperty (char *);
std::string svGetConfigValue (char *);
void svHandleAppOptionsButtons ();
void svHandleItmOptionsButtons (Fl_Widget *, void *);
void svHandleLocalClipboard (int, void *);
void svHandleHostListButtons (Fl_Widget *, void *);
void svHandleHostListEvents (Fl_Widget *, void *);
void svHandleMainWindowEvents (Fl_Widget *, void *);
void svHandlePKill (void *);
void svPositionWidgets ();
void svHandleListItemIconChange (void * notUsed);
void svHandleThreadConnection (void *);
void svHandleThreadCursorChange (void * notUsed);
void svInsertEmptyItem ();
int svItemNumFromItm (HostItem *);
int svItemNumFromVnc (VncObject *);
HostItem * svItmFromVnc (VncObject *);
void svItmOptionsChoosePrvKeyBtnCallback (Fl_Widget *, void *);
void svItmOptionsChoosePubKeyBtnCallback (Fl_Widget *, void *);
void svItmOptionsRadioButtonsCallback (Fl_Widget *, void *);
void svListeningModeBegin ();
void svListeningModeEnd ();
void svLogToFile (const std::string&);
void svMessageWindow (const std::string&, const std::string& = "SpiritVNC");
bool svThereAreConnectedItems ();
void svResizeScroller ();
void svRestoreWindowSizePosition (void *);
void svScanTimer (void *);
void svSendKeyStrokesToHost (std::string&, VncObject *);
void svSetUnsetMainWindowTooltips ();
void svShowAboutHelp ();
void svShowAppOptions ();
void svShowF8Window ();
void svShowItemOptions (HostItem *);
void svUpdateHostListItemText ();

#endif
