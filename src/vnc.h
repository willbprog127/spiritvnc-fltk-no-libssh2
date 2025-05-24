/*
 * vnc.h - part of SpiritVNC - FLTK
 * 2016-2025 Will Brokenbourgh https://www.willbrokenbourgh.com/brainout/
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


#ifndef VNC_H
#define VNC_H

#include <FL/Fl_Box.H>
#include <FL/Fl_Pixmap.H>
#include <rfb/rfbclient.h>
#include <fstream>
#include "hostitem.h"


/* forward declaration of HostItem class */
class HostItem;

/* vnc viewer class */
class VncObject
{
public:
  VncObject () :
    vncClient(rfbGetClient(8, 3, 4)),
    itm(NULL),
    allowDrawing(false),
    waitTime(0),
    nLastClientWidth(0),
    nLastClientHeight(0),
    imgCursor(NULL),
    nCursorXHot(0),
    nCursorYHot(0),
    //inactiveSeconds(0),
    nLastScrollX(0),
    nLastScrollY(0),
    centeredX(0),
    centeredY(0)
  {
    // client and general rfb options
    vncClient->canHandleNewFBSize = true;
    vncClient->appData.forceTrueColour = false;
    vncClient->appData.useRemoteCursor = false;
    vncClient->listenPort = 5500;

    // callbacks
    vncClient->GetCredential = VncObject::handleCredential;
    vncClient->GetPassword = VncObject::handlePassword;
    vncClient->GotCursorShape = VncObject::handleCursorShapeChange;
    vncClient->GotXCutText = VncObject::handleRemoteClipboardProc;
    vncClient->FinishedFrameBufferUpdate = VncObject::handleFrameBufferUpdate;

    vncClient->connectTimeout = SV_CONNECTION_TIMEOUT_SECS;

    rfbClientLog = VncObject::libVncLogging;
    rfbClientErr = VncObject::libVncLogging;
  }

  // public variables
  rfbClient * vncClient;
  HostItem * itm;
  bool allowDrawing;
  uint16_t waitTime;
  int nLastClientWidth;
  int nLastClientHeight;
  Fl_RGB_Image * imgCursor;
  int nCursorXHot;
  int nCursorYHot;
  //uint16_t inactiveSeconds;
  int nLastScrollX;
  int nLastScrollY;
  int centeredX;
  int centeredY;

  // public methods
  //  instance
  void setObjectVisible ();
  bool fitsScroller ();
  void endViewer ();
  //void libVncLogging (const char *, ...);

  //  static
  static void checkVNCMessages (VncObject *);
  static void cleanupVNCObject (HostItem *);
  static void createVNCObject (HostItem *);
  static void createVNCListener ();
  static void endAndDeleteViewer (VncObject **);
  static void endAllViewers ();
  static rfbCredential * handleCredential (rfbClient *, int);
  static void handleCursorShapeChange (rfbClient *, int, int, int, int, int);
  static void handleFrameBufferUpdate (rfbClient *);
  static char * handlePassword (rfbClient *);
  static void handleRemoteClipboardProc (rfbClient *, const char *, int);
  static void hideMainViewer ();
  static void * initVNCConnection (void *);
  static void libVncLogging (const char *, ...);
  static void masterMessageLoop ();
  static void parseErrorMessages(HostItem *, const char *);
};

/* vnc viewer widget class */
class VncViewer : public Fl_Box
{
public:
  VncViewer (int x, int y, int w, int h, const char * label = 0) :
  Fl_Box(x, y, w, h, label),
  vnc(NULL),
  fullscreen(false)
  {}

  VncObject * vnc;
  bool fullscreen;

  // public
  void setFullScreen ();
  void unsetFullScreen ();

private:
  int handle (int);
  void draw ();
  void sendCorrectedKeyEvent (const char *, const int, HostItem *, rfbClient *, bool);
  void drawMessageBar ();
};

#endif
