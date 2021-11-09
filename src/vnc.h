/*
 * vnc.h - part of SpiritVNC - FLTK
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


#ifndef VNC_H
#define VNC_H

#include <FL/Fl_Box.H>
#include <FL/Fl_Pixmap.H>
#include <rfb/rfbclient.h>
#include <fstream>
#include "hostitem.h"

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
        inactiveSeconds(0),
        centeredX(0),
        centeredY(0)
    {
        // client and general rfb options
        vncClient->canHandleNewFBSize = true;
        vncClient->appData.forceTrueColour = false;
        vncClient->appData.useRemoteCursor = false;
        vncClient->listenPort = 5500;

        // callbacks
        vncClient->GetPassword = VncObject::handlePassword;
        vncClient->GotCursorShape = VncObject::handleCursorShapeChange;
        vncClient->GotXCutText = VncObject::handleRemoteClipboardProc;
        vncClient->FinishedFrameBufferUpdate = VncObject::handleFrameBufferUpdate;

        rfbClientLog = VncObject::libVncLogging;
        rfbClientErr = VncObject::libVncLogging;
    }

    // public variables
    rfbClient * vncClient;
    HostItem * itm;
    bool allowDrawing;
    int waitTime;
    int nLastClientWidth;
    int nLastClientHeight;
    Fl_RGB_Image * imgCursor;
    int nCursorXHot;
    int nCursorYHot;
    int inactiveSeconds;
    int centeredX;
    int centeredY;

    // public methods
    //  instance
    void setObjectVisible ();
    bool fitsScroller ();
    void endViewer ();

    //  static
    static void hideMainViewer ();
    static void endAndDeleteViewer (VncObject **);
    static void endAllViewers ();
    static char * handlePassword (rfbClient *);
    static void handleCursorShapeChange (rfbClient *, int, int, int, int, int);
    static void libVncLogging (const char *, ...);
    static void parseErrorMessages(HostItem *, const char *);
    static void checkVNCMessages (VncObject *);
    static void handleRemoteClipboardProc (rfbClient *, const char *, int);
    static void handleFrameBufferUpdate (rfbClient *);
    static void createVNCObject (HostItem *);
    static void createVNCListener ();
    static void * initVNCConnection (void *);
    static void masterMessageLoop ();
};

/* vnc viewer widget class */
class VncViewer : public Fl_Box
{
public:
    VncViewer (int x, int y, int w, int h, const char * label = 0) :
    Fl_Box(x, y, w, h, label),
    vnc(NULL)
    {
        box(FL_FLAT_BOX);
    }

    VncObject * vnc;
private:
    int handle (int);
    void draw ();
    void sendCorrectedKeyEvent (const char *, const int, HostItem *, rfbClient *, bool);
};

#endif
