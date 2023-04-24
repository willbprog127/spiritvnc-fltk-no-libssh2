/*
 * vnc.cxx - part of SpiritVNC - FLTK
 * 2016-2023 Will Brokenbourgh https://www.pismotek.com/brainout/
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


#include "app.h"
#include "consts_enums.h"
#include "vnc.h"


/* create a listening vnc obect */
void VncObject::createVNCListener ()
{
  HostItem * itm = new HostItem();

  if (itm == NULL)
  {
    fl_beep(FL_BEEP_DEFAULT);
    svMessageWindow("Error: Could not create listening VNC connection", "SpiritVNC - FLTK");
    return;
  }

  itm->name = "Listening";
  itm->scaling = 'f';
  itm->showRemoteCursor = true;
  itm->isListener = true;
  itm->ignoreInactive = true;

  // set host list status icon
  itm->icon = app->iconDisconnected;

  app->hostList->add("Listening", itm);
  app->hostList->icon(app->hostList->size(), itm->icon);
  app->hostList->bottomline(app->hostList->size());

  app->hostList->redraw();

  VncObject::createVNCObject(itm);
}


/* initializes and attempts connection with
 * libvnc client / VncObject object
 * (static method)
 */
void VncObject::createVNCObject (HostItem * itm)
{
  // if itm is null or our viewer is already created, return
  if (itm == NULL ||
      itm->isConnected == true ||
      itm->isConnecting == true)
  {
    fl_beep(FL_BEEP_DEFAULT);
    svMessageWindow("Error: Could not create VNC connection", "SpiritVNC - FLTK");
    return;
  }

  // if the host type is 'v' or 's', create vnc viewer
  if (itm->hostType == 'v' || itm->hostType == 's')
  {
    // create new vnc viewer
    itm->vnc = new VncObject();

    if (itm->vnc == NULL || itm->vnc->vncClient == NULL)
    {
      fl_beep(FL_BEEP_DEFAULT);
      return;
    }

    VncObject * vnc = itm->vnc;
    vnc->itm = itm;

    // address is missing on non-listening itm's
    if (itm->isListener == false && itm->hostAddress == "")
    {
      fl_beep(FL_BEEP_DEFAULT);
      svMessageWindow("Error: Host address is missing", "SpiritVNC - FLTK");
      return;
    }

    // reset itm state flags
    itm->isConnecting = true;
    itm->isConnected = false;
    itm->isWaitingForShow = false;
    itm->hasCouldntConnect = false;
    itm->hasError = false;
    itm->hasDisconnectRequest = false;
    itm->hasEnded = false;
    itm->lastErrorMessage = "";
    app->lastError->value("");

    // store this viewer pointer in libvnc client data
    rfbClientSetClientData(vnc->vncClient, app->libVncVncPointer, vnc);

    // set up remote / local cursor
    if (itm->showRemoteCursor == false)
    {
      vnc->vncClient->appData.useRemoteCursor = false;
      vnc->vncClient->GotCursorShape = NULL;
    }
    else
    {
      vnc->vncClient->appData.useRemoteCursor = true;
      vnc->vncClient->GotCursorShape = VncObject::handleCursorShapeChange;
    }

    // set up vnc compression and quality levels
    vnc->vncClient->appData.compressLevel = itm->compressLevel;
    vnc->vncClient->appData.qualityLevel = itm->qualityLevel;
    vnc->vncClient->appData.encodingsString = strdup("tight copyrect hextile");

    itm->vncAddressAndPort = itm->hostAddress + ":" + itm->vncPort;

    // add to viewers waiting ref count
    app->nViewersWaiting ++;

    svLogToFile("Attempting to connect to '" + itm->name + "' - " +
      itm->hostAddress);

    // set host list item status icon
    itm->icon = app->iconConnecting;
    Fl::awake(svHandleListItemIconChange);

    // ############  SSH CONNECTION ###############################################

    // we connect to this host with vnc through ssh
    if (itm->hostType == 's')
    {
      svDebugLog("svCreateVNCObject - Host is 'SVNC'");

      struct stat structStat;

      // oops, can't open ssh key file
      if (stat(itm->sshKeyPrivate.c_str(), &structStat) != 0)
      {
        itm->isConnecting = false;
        itm->hasCouldntConnect = true;
        itm->hasError = true;

        svLogToFile("ERROR - Could not open the private SSH key file");
        svMessageWindow("Error: Could not open the private SSH key "
          "file for '" + itm->name + "' - " + itm->hostAddress);

        //if (vnc->vncClient != NULL)
        VncObject::endAndDeleteViewer(&vnc);

        svHandleThreadConnection(itm);

        return;
      }

      itm->sshLocalPort = svFindFreeTcpPort();

      itm->vncAddressAndPort = "127.0.0.1:" + std::to_string(itm->sshLocalPort);

      svDebugLog("svCreateVNCObject - Creating and running threadSSH");

      // create, launch and detach call to create our ssh connection
      svCreateSSHConnection(itm);

      time_t sshDelay = time(NULL) + itm->sshWaitTime;

      svDebugLog("svCreateVNCObject - About to enter itm->sshReady timer loop");

      // loop until the ssh connection is ready
      // or exit if ssh times out
      while (1)
      {
        if (time(NULL) >= sshDelay || itm->hasError == true)
          break;

        Fl::check();
      }

      // exit if sshReady is false
      if (itm->sshReady == false)
      {
        itm->isConnecting = false;
        itm->hasCouldntConnect = true;

        svHandleThreadConnection(itm);

        return;
      }
    }
    // ############  SSH CONNECTION END ###########################################

    svDebugLog("svCreateVNCObject - Creating and running itm->threadRFB");

    // create, launch and detach call to create our vnc connection
    if (pthread_create(&itm->threadRFB, NULL, VncObject::initVNCConnection, itm) != 0)
    {
      svLogToFile("ERROR - Couldn't create RFB thread for '" + itm->name +
            "' - " + itm->hostAddress);
      itm->isConnecting = false;
      itm->hasCouldntConnect = true;
      itm->hasError = true;

      //if (vnc != NULL && vnc->vncClient != NULL)
      VncObject::endAndDeleteViewer(&vnc);

      svHandleThreadConnection(itm);

      return;
    }
  }

  // add to our count of created vncObjects so we
  // can stop 'expensive' stuff in masterMessageLoop
  app->createdObjects ++;

  return;
}


/*
 * ends all vnc objects (usually called right before program quits)
 * (static method)
 */
void VncObject::endAllViewers ()
{
  HostItem * itm = NULL;
  VncObject * vnc = NULL;

  for (uint16_t i = 0; i <= app->hostList->size(); i ++)
  {
    itm = static_cast<HostItem *>(app->hostList->data(i));

    if (itm != NULL)
    {
      vnc = itm->vnc;

      if (vnc != NULL && (itm->isConnected == true ||
                         itm->isConnecting == true ||
                         itm->isWaitingForShow == true))
      {
        itm->hasDisconnectRequest = true;

        VncObject::endAndDeleteViewer(&vnc);
      }

      vnc = NULL;
    }
  }
}


/* set vnc object to disconnect and stop working */
/* (instance method) */
void VncObject::endViewer ()
{
  //this->GONK!

  if (itm != NULL && itm->vnc != NULL)
  {
    // only hide main viewer if this is the currently-displayed itm
    if (app->vncViewer->vnc != NULL && itm == app->vncViewer->vnc->itm)
      hideMainViewer();

    // host disconnected unexpectedly / interrupted connection
    if (itm->isConnected == true && itm->hasDisconnectRequest == false)
    {
      itm->icon = app->iconDisconnectedError;
      Fl::awake(svHandleListItemIconChange);

      svLogToFile("Unexpectedly disconnected from '" + itm->name +
        "' - " + itm->hostAddress);
    }

    // we disconnected purposely from host
    if ((itm->isConnected == true || itm->isConnecting == true) &&
        itm->hasDisconnectRequest == true)
    {
      // set host list item status icon
      itm->icon = app->iconDisconnected;
      Fl::awake(svHandleListItemIconChange);

      if (app->shuttingDown)
      {
        svLogToFile("Automatically disconnecting.  Program is shutting down '" +
            itm->name + "' - " + itm->hostAddress);
      }
      else
      {
        svLogToFile("Manually disconnected from '" + itm->name + "' - " +
          itm->hostAddress);
      }
    }

    // decrement our count of created vncObjects so we
    // can check and avoid 'expensive' stuff in masterMessageLoop
    app->createdObjects --;

    // stop this viewer's connection worker thread
    if (itm->threadRFBRunning == true)
    {
      // make sure there's a valid thread before canceling
      if (itm->threadRFB != 0)
          pthread_cancel(itm->threadRFB);

      itm->threadRFBRunning = false;
    }

    itm->isConnected = false;
    itm->isConnecting = false;
    itm->hasDisconnectRequest = false;
    itm->hasEnded = true;

    // clean up the client
    rfbClientCleanup(vncClient);

    // tell ssh to clean up if a ssh/vnc connection
    if (itm->hostType == 's')
    {
      itm->sshReady = false;

      svCloseSSHConnection(itm);
    }
  }
}


/* calls endViewer and cleans up associated VncObject * memory */
/* (static method) */
void VncObject::endAndDeleteViewer (VncObject ** vnc)
{
  if (vnc == NULL)
      return;

  (*vnc)->endViewer();

  delete *vnc;
  *vnc = NULL;
}


/* checks to see if vnc client will fit within scroller */
/* (instance method) */
bool VncObject::fitsScroller ()
{
  const HostItem * itm = static_cast<HostItem *>(this->itm);

  if (itm == NULL || itm->vnc == NULL)
    return false;

  const rfbClient * cl = itm->vnc->vncClient;

  if (cl == NULL)
    return false;

  if (cl->width <= app->scroller->w() && cl->height <= app->scroller->h())
    return true;
  else
    return false;
}


/* handle cursor change */
/* (static method / callback) */
void VncObject::handleCursorShapeChange (rfbClient * cl, int xHot, int yHot, int nWidth,
    int nHeight, int nBytesPerPixel)
{
  VncObject * vnc = static_cast<VncObject *>(rfbClientGetClientData(cl, app->libVncVncPointer));

  if (vnc == NULL ||
      cl == NULL ||
      Fl::belowmouse() != app->vncViewer ||
      vnc->allowDrawing == false)
    return;

  const int nSSize = nWidth * nHeight * nBytesPerPixel;

  // if image has alpha, apply mask
  if (nBytesPerPixel == 2 || nBytesPerPixel == 4)
  {
    int nM = 0;
    int nMV;

    for (int i = (nBytesPerPixel - 1); i < nSSize; i += nBytesPerPixel)
    {
      nMV = cl->rcMask[nM];

      if (nMV > 0)
          nMV = 255;

      cl->rcSource[i] = nMV;

      nM ++;
    }
  }

  // create rgb image from raw data
  Fl_RGB_Image * img = new Fl_RGB_Image(cl->rcSource, nWidth, nHeight, nBytesPerPixel);

  if (img == NULL)
    return;

  // delete previous copy, if any
  if (vnc->imgCursor != NULL)
    delete vnc->imgCursor;

  // copy rgb image to vncViewer and set x+y hotspots
  vnc->imgCursor = static_cast<Fl_RGB_Image *>(img->copy());

  if (vnc->imgCursor == NULL)
    return;

  vnc->nCursorXHot = xHot;
  vnc->nCursorYHot = yHot;

  Fl::awake(svHandleThreadCursorChange, reinterpret_cast<void *>(false));

  delete img;
}


/* (static method) */
void VncObject::handleFrameBufferUpdate (rfbClient * cl) //, int x, int y, int w, int h)
{
  VncObject * vnc = static_cast<VncObject *>(rfbClientGetClientData(cl, app->libVncVncPointer));

  if (vnc == NULL || vnc->allowDrawing == false)
    return;

  app->vncViewer->redraw();
}


/* handle copy/cut FROM vnc host */
/* (static method) */
void VncObject::handleRemoteClipboardProc (rfbClient * cl, const char * text, int textlen)
{
  VncObject * vnc = static_cast<VncObject *>(rfbClientGetClientData(cl, app->libVncVncPointer));

  if (vnc == NULL)
    return;

  VncObject * vnc2 = app->vncViewer->vnc;

  if (vnc2 == NULL)
    return;

  if (vnc2 == vnc && text != NULL && textlen > 0)
  {
    app->blockLocalClipboardHandling = true;

    Fl::copy(text, textlen, 1);

    SendFramebufferUpdateRequest(cl, 0, 0, cl->width, cl->height, false);

    app->blockLocalClipboardHandling = false;

    app->vncViewer->redraw();
    Fl::awake();
  }
}


/* set VncViewer object to hide itself */
/* (static method) */
void VncObject::hideMainViewer ()
{
  VncObject * vnc = app->vncViewer->vnc;

  if (vnc == NULL)
    return;

  vnc->allowDrawing = false;

  app->mainWin->cursor(FL_CURSOR_DEFAULT);

  Fl::lock();
  app->vncViewer->vnc = NULL;
  app->vncViewer->size(0, 0);
  app->scroller->scroll_to(0, 0);
  app->scroller->type(0);
  app->scroller->redraw();
  Fl::unlock();
}


/* initialize and connect to a vnc host/server */
/* (this is called as a thread because it blocks) */
void * VncObject::initVNCConnection (void * data)
{
  // detach this thread
  pthread_detach(pthread_self());

  char * strParams[2] = {NULL, NULL};

  Fl::lock();
  HostItem * itm = static_cast<HostItem *>(data);
  Fl::unlock();

  if (itm == NULL)
  {
    Fl::awake(svHandleThreadConnection, itm);

    return SV_RET_VOID;
  }

  itm->threadRFBRunning = true;

  int nNumOfParams = 2;
  VncObject * vnc = itm->vnc;

  if (vnc == NULL)
  {
    itm->isConnected = false;
    itm->isConnecting = false;
    itm->hasError = true;
    itm->threadRFBRunning = false;

    Fl::awake(svHandleThreadConnection, itm);

    return SV_RET_VOID;
  }

  std::string strP;

  // 'program name' parameter
  strParams[0] = strdup("SpiritVNCFLTK");

  if (itm->isListener == false)
  {
    strP = itm->vncAddressAndPort;
    strParams[1] = strdup(strP.c_str());
  }
  else
  {
    strP = "-listennofork";
    vnc->vncClient->listenAddress = strdup("0.0.0.0");
    strParams[1] = strdup(strP.c_str());
  }

  // if the second parameter is invalid, get out
  if (strParams[1] == NULL || strlen(strParams[1]) < 7)
  {
    itm->isConnected = false;
    itm->isConnecting = false;
    itm->hasError = true;
    itm->threadRFBRunning = false;

    Fl::awake(svHandleThreadConnection, itm);

    return SV_RET_VOID;
  }

  // libvnc - attempt to connect to host
  // this function blocks, that's why this function runs as a thread
  if (rfbInitClient(vnc->vncClient, &nNumOfParams, strParams) == false)
  {
    int errNum = errno;

    VncObject::parseErrorMessages(itm, strerror(errNum));

    itm->isConnected = false;
    itm->isConnecting = false;
    itm->hasCouldntConnect = true;
    itm->threadRFBRunning = false;

    Fl::awake(svHandleThreadConnection, itm);

    return SV_RET_VOID;
  }

  itm->isConnected = true;
  itm->isConnecting = false;
  itm->isWaitingForShow = true;
  itm->threadRFBRunning = false;

  // send message to main thread
  Fl::awake(svHandleThreadConnection, itm);

  return SV_RET_VOID;
}


/* check connection errors and inform user, if necessary */
void VncObject::parseErrorMessages (HostItem * itm, const char * strMessageIn)
{
  char strMsg[FL_PATH_MAX] = {0};

  if (itm == NULL || strMessageIn == NULL)
    return;

  // format for log file
  snprintf(strMsg, FL_PATH_MAX, "[Error] '%s' - %s: %s", itm->name.c_str(),
    itm->hostAddress.c_str(), strMessageIn);

  // write to log file
  svLogToFile(strMsg);

  // search and 'fix up' any special cases

  // load into string
  std::string strMessage = strMessageIn;

  // 'fix up' for possible bad / missing password
  if (strMessage.find("Resource temporarily unavailable") != std::string::npos)
  {
    itm->lastErrorMessage = "Bad / missing password or Resource temporarily unavailable";
    return;
  }

  // fix dumb Operation now in progress error
  if (strMessage.find("Operation now in progress") != std::string::npos)
  {
    if (itm->hostType == 's' && itm->sshReady == false)
        itm->lastErrorMessage = "Unable to connect to host's SSH server";
    else
        itm->lastErrorMessage = "Unable to connect to VNC server";

    return;
  }

  itm->lastErrorMessage = strMessage;
}


/* libvnc logging callback */
/* (static function) */
void VncObject::libVncLogging (const char * format, ...)
{

  if (app->debugMode == true)
  {
    va_list args;
    char msgBuf[FL_PATH_MAX] = {0};

    // combine format string and args into nowBuff
    va_start(args, format);
    vsnprintf(msgBuf, FL_PATH_MAX, format, args);
    va_end(args);

    // log to file
    svLogToFile(msgBuf);
  }
}


/* master loop to handle all vnc objects' message checking */
/* (static function) */
void VncObject::masterMessageLoop ()
{
  HostItem * itm = NULL;
  VncObject * vnc = NULL;

  uint16_t nSize = app->hostList->size();

  // run until it's time to shut down
  while (app->shuttingDown == false)
  {
    // only loop if there are objects alive
    if (app->createdObjects != 0)
    {
      // spend the most time processing the active vnc connection
      for (uint8_t i = 0; i < 250; i++)  //100; i++)
      {
        // keep from making too tight a loop
        //Fl::check();
        Fl::wait(0.015);

        vnc = app->vncViewer->vnc;

        if (vnc != NULL && vnc->itm != NULL)
          VncObject::checkVNCMessages(vnc);
        else
          break;
      }

      // after current connection looping 250 times, (orig. 100 times)
      // iterate through the host list one time and
      // check each to see if the connection is alive
      nSize = app->hostList->size();

      for (uint16_t i = 0; i <= nSize; i ++)
      {
        itm = static_cast<HostItem *>(app->hostList->data(i));

        if (itm == NULL)
          continue;

        vnc = itm->vnc;

        if (vnc == NULL)
          continue;

        // if this vnc object is connected and is active, process vnc messages
        if (itm->isConnected == true && itm->hasEnded == false && app->shuttingDown == false)
          VncObject::checkVNCMessages(vnc);

        itm = NULL;
        vnc = NULL;
      }

      vnc = app->vncViewer->vnc;

      if (vnc != NULL && vnc->itm != NULL)
        Fl::wait(0.015);
      else
        Fl::wait(1.0);
    }
    else
      Fl::wait(1.0);
    //Fl::wait(0.250);
  }
}


/* libvnc send password to host callback */
/* (static function) */
rfbCredential * VncObject::handleCredential (rfbClient * cl, int credentialType)
{
  if (credentialType != rfbCredentialTypeUser)
  {
    svLogToFile("ERROR - handleCredential: Non-username / password required for authentication");

    return NULL;
  }

  // client is null
  if (cl == NULL)
  {
    svLogToFile("ERROR - handlePassword: vnc->vncClient is null");
    return NULL;
  }

  VncObject * vnc = static_cast<VncObject *>(rfbClientGetClientData(cl, app->libVncVncPointer));

  // vnc object is null
  if (vnc == NULL)
  {
    svLogToFile("ERROR - handlePassword: vnc is null");
    return NULL;
  }

  HostItem * itm = vnc->itm;

  // itm is null
  if (itm == NULL)
  {
    svLogToFile("ERROR - handlePassword: itm is null");
    return NULL;
  }


  // build and populate credential
  rfbCredential * cred = static_cast<rfbCredential *>(malloc(sizeof(rfbCredential)));

  // allocate memory for username / password
  cred->userCredential.username = static_cast<char *>(malloc(RFB_BUF_SIZE));
  cred->userCredential.password = static_cast<char *>(malloc(RFB_BUF_SIZE));

  // set username / password
  strncpy(cred->userCredential.username, itm->vncLoginUser.c_str(), (RFB_BUF_SIZE - 1));
  strncpy(cred->userCredential.password, itm->vncLoginPassword.c_str(), (RFB_BUF_SIZE - 1));

  return cred;
}


 /* libvnc send password to host callback */
 /* (static function) */
char * VncObject::handlePassword (rfbClient * cl)
{
  if (cl == NULL)
  {
    svLogToFile("ERROR - handlePassword: vnc->vncClient is null");
    return NULL;
  }

  VncObject * vnc = static_cast<VncObject *>(rfbClientGetClientData(cl, app->libVncVncPointer));

  if (vnc == NULL)
  {
    svLogToFile("ERROR - handlePassword: vnc is null");
    return NULL;
  }

  HostItem * itm = vnc->itm;

  if (itm == NULL)
  {
    svLogToFile("ERROR - handlePassword: itm is null");
    return NULL;
  }

  // ------ trying this out due to 'bad password' errors sometimes -------
  char * strPass = static_cast<char *>(malloc(10));

  if (strPass != NULL)
    strncpy(strPass, itm->vncPassword.c_str(), 9);

  return strPass;
  // ------ trying this out due to 'bad password' errors sometimes -------

  //return strdup(itm->vncPassword.c_str());  // <<<--- temporarily disabled / testing above
}


/* set vnc object to show itself */
/* (instance method) */
void VncObject::setObjectVisible ()
{
  if (itm == NULL || vncClient == NULL)
      return;

  app->vncViewer->vnc = this;

  SendFramebufferUpdateRequest(vncClient, 0, 0, vncClient->width, vncClient->height, false);

  int leftMargin = (app->hostList->x() + app->hostList->w() + 3);

  // scale off / scroll if host screen is too big
  if (itm->scaling == 's' || (itm->scaling == 'f' && fitsScroller() == true))
  {
    app->scroller->type(Fl_Scroll::BOTH);
    app->vncViewer->size(vncClient->width, vncClient->height);
  }

  // scale 'zoom' or 'fit'
  if (itm->scaling == 'z' || (itm->scaling == 'f' && fitsScroller() == false))
  {
    // maximize vnc viewer size
    app->vncViewer->size(app->mainWin->w() - leftMargin, app->mainWin->h());

    // calculate host screen ratio
    float dRatio = static_cast<float>(vncClient->width) / static_cast<float>(vncClient->height);

    // set height of image to max height and check if resulting width is less than max width,
    // otherwise set width of image to max width and calculate new height
    if (static_cast<float>(app->vncViewer->h()) * dRatio <= app->vncViewer->w())
      app->vncViewer->size(static_cast<int>(static_cast<float>(app->vncViewer->h()) * dRatio),
        app->vncViewer->h());
    else
      app->vncViewer->size(app->vncViewer->w(),
        static_cast<int>(static_cast<float>(app->vncViewer->w()) / dRatio));
  }

  svResizeScroller();

  app->scroller->scroll_to(nLastScrollX, nLastScrollY);

  // viewer centering (only if not zooming)
  if (itm->scaling == 's' || (itm->scaling == 'f' && fitsScroller() == true))
  {
    // figure out x position for viewer
    if (itm->centerX == true)
    {
      app->scroller->position((app->scroller->w() - vncClient->width) / 2 + leftMargin,
        app->scroller->y());

      if (app->scroller->x() < 0)
        app->scroller->position(leftMargin, app->scroller->y());
    }
    else
        app->scroller->position(leftMargin, app->scroller->y());

    // figure out y position for viewer
    if (itm->centerY == true)
    {
        app->scroller->position(app->scroller->x(), (app->scroller->h() - vncClient->height) / 2);

        if (app->scroller->y() < 0)
          app->scroller->position(app->scroller->x(), 0);
    }
    else
      app->scroller->position(app->scroller->x(), 0);
  }

  allowDrawing = true;

  app->scroller->redraw();
}


/* check and act on libvnc host messages */
/* (static method) */
void VncObject::checkVNCMessages (VncObject * vnc)
{
  int nMsg = 0;

  if (vnc == NULL || vnc->vncClient == NULL)
      return;

  // below modified to '0' based on select() docs suggestion
  nMsg = WaitForMessage(vnc->vncClient, 0);

  if (nMsg < 0)
  {
    VncObject::endAndDeleteViewer(&vnc);
    return;
  }

  if (nMsg)
  {
    // reset inactive seconds so we don't automatically disconnect
    vnc->inactiveSeconds = 0;

    if (HandleRFBServerMessage(vnc->vncClient) == FALSE)
    {
      VncObject::endAndDeleteViewer(&vnc);
      return;
    }
    else
      //Fl::awake();
      Fl::check();
  }
}


/*
 * ########################################################################################
 * ########################## VNC VIEWER WIDGET ###########################################
 * ########################################################################################
 */

/* draw event for vnc view widget */
/* (instance method) */
void VncViewer::draw ()
{
  VncObject * vnc = app->vncViewer->vnc;

  if (vnc == NULL ||
      vnc->allowDrawing == false ||
      vnc->vncClient == NULL)
    return;

  rfbClient * cl = vnc->vncClient;
  HostItem * itm = static_cast<HostItem *>(vnc->itm);

  if (itm == NULL || cl->frameBuffer == NULL)
    return;

  int nBytesPerPixel = cl->format.bitsPerPixel / 8;

  // get out if client or scroller size is wrong
  if (cl->width < 1 || cl->height < 1 || app->scroller->w() < 1 || app->scroller->h() < 1)
    return;

  // 's'croll or 'f'it + real size scale mode geometry
  if (itm->scaling == 's' || (itm->scaling == 'f' && vnc->fitsScroller() == true))
  {
    vnc->nLastScrollX = app->scroller->xposition();
    vnc->nLastScrollY = app->scroller->yposition();

    // draw that vnc host!
    fl_draw_image(
      cl->frameBuffer,
      app->scroller->x() - vnc->nLastScrollX,
      app->scroller->y() - vnc->nLastScrollY,
      cl->width,
      cl->height,
      nBytesPerPixel,
      0);

    return;
  }

  // 'z'oom or 'f'it + oversized scale mode geometry
  if (itm->scaling == 'z' || (itm->scaling == 'f' && vnc->fitsScroller() == false))
  {
    Fl_Image * imgC = NULL;
    Fl_RGB_Image * imgZ = NULL;

    int isize = cl->width * cl->height * nBytesPerPixel;

    // if there's an alpha byte, set it to 255
    if (nBytesPerPixel == 2 || nBytesPerPixel == 4)
    {
      for (int i = (nBytesPerPixel - 1); i < isize; i+= nBytesPerPixel)
          cl->frameBuffer[i] = 255;
    }

    imgZ = new Fl_RGB_Image(cl->frameBuffer, cl->width, cl->height, nBytesPerPixel);

    if (imgZ != NULL)
    {
      // set appropriate scale quality
      if (itm->scalingFast == true)
        imgZ->RGB_scaling(FL_RGB_SCALING_NEAREST);
      else
        imgZ->RGB_scaling(FL_RGB_SCALING_BILINEAR);

      // scale down imgZ image to imgC
      imgC = imgZ->copy(app->vncViewer->w(), app->vncViewer->h());

      // draw scaled image onto screen
      if (imgC != NULL)
        imgC->draw(app->vncViewer->x(), app->vncViewer->y());
    }

    if (imgC != NULL)
      delete imgC;

    if (imgZ != NULL)
      delete imgZ;
  }
}


/* handle events for vnc view widget */
/* (instance method) */
int VncViewer::handle (int event)
{
  if (app->vncViewer == NULL)
    return 0;

  VncObject * vnc = app->vncViewer->vnc;

  if (vnc == NULL)
    return 0;

  // bail out if this is not the active vnc object
  if (vnc->allowDrawing == false)
    return 0;

  float nMouseX = 0;
  float nMouseY = 0;

  static int nButtonMask;
  HostItem * itm = vnc->itm;

  // itm is null
  if (itm == NULL)
    return 0;

  rfbClient * cl = vnc->vncClient;

  // client is null
  if (cl == NULL)
    return 0;

  // scrolled / non-scaled sizing
  if (itm->scaling == 's' || (itm->scaling == 'f' && vnc->fitsScroller() == true))
  {
    nMouseX = Fl::event_x() - app->scroller->x() + app->scroller->xposition();
    nMouseY = Fl::event_y() - app->scroller->y() + app->scroller->yposition();
  }

  // scaled sizing
  if (itm->scaling == 'z' || (itm->scaling == 'f' && vnc->fitsScroller() == false))
  {
    float fXAdj = float(app->vncViewer->w()) / float(cl->width);
    float fYAdj = float(app->vncViewer->h()) / float(cl->height);

    nMouseX = float(Fl::event_x() - app->scroller->x()) / fXAdj;
    nMouseY = float(Fl::event_y() - app->scroller->y()) / fYAdj;
  }

  switch (event)
  {
    // ** mouse events **
    case FL_DRAG:
      if (Fl::event_button() == FL_LEFT_MOUSE)
        nButtonMask |= rfbButton1Mask;

      if (Fl::event_button() == FL_RIGHT_MOUSE)
        nButtonMask |= rfbButton3Mask;

      SendPointerEvent(vnc->vncClient, nMouseX, nMouseY, nButtonMask);
      SendIncrementalFramebufferUpdateRequest(vnc->vncClient);

      app->scanIsRunning = false;
      return 1;
      break;

    case FL_PUSH:
        // left mouse button
        if (Fl::event_button() == FL_LEFT_MOUSE)
        {
          nButtonMask |= rfbButton1Mask;
          SendPointerEvent(vnc->vncClient, nMouseX, nMouseY, nButtonMask);
          app->scanIsRunning = false;
          return 1;
        }
        // right mouse button
        if (Fl::event_button() == FL_RIGHT_MOUSE)
        {
          nButtonMask |= rfbButton3Mask;
          SendPointerEvent(vnc->vncClient, nMouseX, nMouseY, nButtonMask);
          app->scanIsRunning = false;
          return 1;
        }
        break;

    case FL_RELEASE:
        // left mouse button
        if (Fl::event_button() == FL_LEFT_MOUSE)
        {
          // left mouse click
          nButtonMask &= ~rfbButton1Mask;
          SendPointerEvent(vnc->vncClient, nMouseX, nMouseY, nButtonMask);
          SendIncrementalFramebufferUpdateRequest(vnc->vncClient);
          app->scanIsRunning = false;
          return 1;
        }

        // right mouse button
        if (Fl::event_button() == FL_RIGHT_MOUSE)
        {
          nButtonMask &= ~rfbButton3Mask;
          SendPointerEvent(vnc->vncClient, nMouseX, nMouseY, nButtonMask);
          SendIncrementalFramebufferUpdateRequest(vnc->vncClient);
          app->scanIsRunning = false;
          return 1;
        }
        break;

    case FL_MOUSEWHEEL:
      {
        int nYWheel = Fl::event_dy();

        // handle vertical scrolling
        // (vnc protocol doesn't seem to support horizontal scrolling)
        if (nYWheel != 0)
        {
          int nYDirection = 0;

          if (nYWheel > 0)
            nYDirection = rfbWheelDownMask;

          if (nYWheel < 0)
            nYDirection = rfbWheelUpMask;

          nButtonMask |= nYDirection;
          SendPointerEvent(vnc->vncClient, nMouseX, nMouseY, nButtonMask);

          nButtonMask &= ~nYDirection;
          SendPointerEvent(vnc->vncClient, nMouseX, nMouseY, nButtonMask);

          SendIncrementalFramebufferUpdateRequest(vnc->vncClient);
          return 1;
        }
        break;
    }

    case FL_MOVE:
      SendPointerEvent(vnc->vncClient, nMouseX, nMouseY, nButtonMask);
      return 1;
      break;

    // ** keyboard events **
    case FL_KEYDOWN:
      sendCorrectedKeyEvent(Fl::event_text(), Fl::event_key(), itm, cl, true);
      app->scanIsRunning = false;
      return 1;
      break;

    case FL_SHORTCUT:
      sendCorrectedKeyEvent(Fl::event_text(), Fl::event_key(), itm, cl, true);
      app->scanIsRunning = false;
      return 1;
      break;

    case FL_KEYUP:
      sendCorrectedKeyEvent(Fl::event_text(), Fl::event_key(), itm, cl, false);
      app->scanIsRunning = false;
      return 1;
      break;

    // ** misc events **
    case FL_ENTER:
      if (vnc->imgCursor != NULL && itm->showRemoteCursor == true)
        Fl::awake(svHandleThreadCursorChange, reinterpret_cast<void *>(false));
      return 1;
      break;

    case FL_LEAVE:
      Fl::awake(svHandleThreadCursorChange, reinterpret_cast<void *>(true));
      return 1;
      break;

    case FL_FOCUS:
      return 1;
      break;

    case FL_UNFOCUS:
      return 1;
      break;

    case FL_PASTE:
      {
        int intClipLen = Fl::event_length();

        if (intClipLen > 0)
        {
          char strClipText[intClipLen];

          strncpy(strClipText, Fl::event_text(), intClipLen);

          // send clipboard text to remote server
          SendClientCutText(app->vncViewer->vnc->vncClient,
            const_cast<char *>(strClipText), intClipLen);
      }
      return 1;

    }

    default:
      break;
  }

  return Fl_Box::handle(event);
}


/*
  fix / convert X11 key codes to rfb key codes
  (const in not used so parameter name removed)
*/
void VncViewer::sendCorrectedKeyEvent (const char * strIn, const int, HostItem * itm,
  rfbClient * cl, bool downState)
{
  int nK = Fl::event_key();

  if (cl == NULL || itm == NULL)
    return;

  // F8 window
  if (nK == XK_F8 && downState == false)
  {
    svShowF8Window();
    return;
  }

  // F12 macro
  if (nK == XK_F12 && downState == false)
  {
    svSendKeyStrokesToHost(itm->f12Macro, vnc);
    return;
  }

  // send key
  if ((nK >= 32 && nK <= 255) && Fl::event_ctrl() == 0)
    SendKeyEvent(cl, strIn[0], downState);
  else
    SendKeyEvent(cl, nK, downState);
}
