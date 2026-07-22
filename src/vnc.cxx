/*
 * vnc.cxx - part of SpiritVNC - FLTK
 * 2016-2026 Will Brokenbourgh https://www.willbrokenbourgh.com/brainout/
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

/* pointer for libvncclient's setclientdata and getclientdata */
void * m_vncObjPtr = reinterpret_cast<void *>(0x777);


/* create a listening vnc obect */
/* (static method) */
void VncObject::createVNCListener ()
{
  HostItem * itm = new HostItem();
  if (!itm)
  {
    fl_beep(FL_BEEP_DEFAULT);
    svMessageWindow("Error: Could not create listening VNC connection", "SpiritVNC - FLTK");
    return;
  }

  itm->name = "Listening";
  itm->scaling = 'f';
  itm->showRemoteCursor = true;
  itm->isListener = true;

  // set host list status icon
  itm->icon = app->iconDisconnected;

  app->hostList->add("Listening", itm);
  app->hostList->icon(app->hostList->size(), itm->icon);
  app->hostList->bottomline(app->hostList->size());

  app->hostList->redraw();

  VncObject::createVNCObject(itm);
}


/* do rfbClientCleanup, delete and null VncObject */
/* (static method) */
void VncObject::cleanupVNCObject (HostItem * itm)
{
  if (!itm)
    return;

  itm->vncNeedsCleanup = false;

  // clean up client structure
  if (itm->vnc)
  {
    // do client cleanup first
    if (itm->vnc->vncClient && itm->initOkay)
      rfbClientCleanup(itm->vnc->vncClient);

    // delete and null VncObject
    delete itm->vnc;
    itm->vnc = NULL;
  }
}


/*
  initializes and attempts connection with
  libvnc client / VncObject object
  (static method)
*/
void VncObject::createVNCObject (HostItem * itm)
{
  // if itm is null or our viewer is already created, return
  if (!itm || itm->isConnected || itm->isConnecting)
  {
    fl_beep(FL_BEEP_DEFAULT);
    svMessageWindow("Error: Could not create VNC connection", "SpiritVNC - FLTK");
    return;
  }

  // if the host type is 'v' or 's', create vnc viewer
  if (itm->hostType == 'v' || itm->hostType == 's')
  {
    // just in case it wasn't done already
    if (itm->vncNeedsCleanup)
      VncObject::cleanupVNCObject(itm);

    // create new vnc object
    itm->vnc = new VncObject();
    if (!itm->vnc || !itm->vnc->vncClient)
    {
      fl_beep(FL_BEEP_DEFAULT);
      return;
    }

    // no, this isn't confusing at all! :-P
    VncObject * vnc = itm->vnc;
    vnc->itm = itm;

    // address is missing on non-listening itm
    if (!itm->isListener && itm->hostAddress.empty())
    {
      fl_beep(FL_BEEP_DEFAULT);
      std::string strAddErr = itm->name + " - Error: Host address is missing";
      svMessageWindow(strAddErr, "SpiritVNC - FLTK");
      return;
    }

    // reset itm state flags
    itm->isConnecting = true;
    itm->isConnected = false;
    itm->isWaitingForShow = false;
    itm->hasCouldntConnect = false;
    itm->hasError = false;
    itm->hasDisconnectRequest = false;
    itm->initOkay = false;
    itm->lastErrorMessage = "";

    // store this viewer pointer in libvnc client data
    rfbClientSetClientData(vnc->vncClient, m_vncObjPtr, vnc);

    // set up remote / local cursor
    if (!itm->showRemoteCursor)
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
    vnc->vncClient->appData.encodingsString = "tight copyrect hextile";

    itm->vncAddressAndPort = itm->hostAddress + ":" + itm->vncPort;

    // add to viewers waiting ref count
    app->nViewersWaiting ++;

    svLogToFile("Attempting to connect to '" + itm->name + "' - " + itm->hostAddress);

    // set host list item status icon
    itm->icon = app->iconConnecting;
    Fl::awake(svHandleListItemIconChange);

    // ############  SSH CONNECTION ###############################################

    // we connect to this host with vnc through ssh
    if (itm->hostType == 's')
    {
      svDebugLog("svCreateVNCObject - Host is 'SVNC'");

      // check if we can access ssh key file
      std::ifstream keyStream(itm->sshKeyPrivate);
      if (!keyStream.is_open())
      {
        itm->isConnecting = false;
        itm->hasCouldntConnect = true;
        itm->hasError = true;

        svLogToFile("ERROR - Could not open the private SSH key file");
        svMessageWindow("Error: Could not open the private SSH key "
          "file for '" + itm->name + "' - " + itm->hostAddress);

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
      while (!app->shuttingDown)
      {
        if (time(NULL) >= sshDelay || itm->hasError)
          break;

        Fl::check();
      }

      // exit if sshReady is false
      if (!itm->sshReady)
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
      itm->threadRFBRunning = false;

      svLogToFile("ERROR - Couldn't create RFB thread for '" + itm->name + "' - " + itm->hostAddress);
      itm->isConnecting = false;
      itm->hasCouldntConnect = true;
      itm->hasError = true;

      svHandleThreadConnection(itm);

      return;
    }
  }

  // threadRFBRunning is set in actual thread

  // add to our count of created vncObjects so we
  // can stop 'expensive' stuff in masterMessageLoop
  app->createdObjects ++;

  return;
}


/*
  ends all vnc objects (usually called right before program quits)
  (static method)
*/
void VncObject::endAllViewers ()
{
  for (uint16_t i = 0; i <= app->hostList->size(); i ++)
  {
    HostItem * itm = static_cast<HostItem *>(app->hostList->data(i));
    if (itm)
    {
      VncObject * vnc = itm->vnc;

      if (vnc && (itm->isConnected || itm->isConnecting || itm->isWaitingForShow))
      {
        itm->hasDisconnectRequest = true;

        vnc->endViewer();
      }
    }
  }
}


/*
  set vnc object to disconnect and stop working
  (instance method)
*/
void VncObject::endViewer ()
{
  //this->GONK!

  if (this->itm && this->itm->vnc)
  {
    // only hide main viewer if this is the currently-displayed itm
    if (app->vncViewer->vnc && this->itm == app->vncViewer->vnc->itm)
    {
      app->vncViewer->unsetFullScreen();
      hideMainViewer();
    }

    // host disconnected unexpectedly / interrupted connection
    if (this->itm->isConnected && !this->itm->hasDisconnectRequest)
    {
      this->itm->icon = app->iconDisconnectedError;
      Fl::awake(svHandleListItemIconChange);

      svLogToFile("Unexpectedly disconnected from '" + this->itm->name + "' - " + this->itm->hostAddress);
    }

    // we disconnected purposely from host
    if ((this->itm->isConnected || this->itm->isConnecting) && this->itm->hasDisconnectRequest)
    {
      // set host list item status icon
      this->itm->icon = app->iconDisconnected;
      Fl::awake(svHandleListItemIconChange);

      if (app->shuttingDown)
        svLogToFile("Automatically disconnecting.  Program is shutting down '" + this->itm->name +
          "' - " + itm->hostAddress);
      else
        svLogToFile("Manually disconnected from '" + this->itm->name + "' - " + this->itm->hostAddress);
    }

    // decrement our count of created vncObjects so we
    // can check and avoid 'expensive' stuff in masterMessageLoop
    app->createdObjects --;

    this->itm->isConnected = false;
    this->itm->hasDisconnectRequest = false;

    // tell ssh to clean up if a ssh/vnc connection
    if (this->itm->hostType == 's')
      svCloseSSHConnection(itm);

    // set this for cleanup later
    if (this->vncClient && !this->itm->isConnecting)
      this->itm->vncNeedsCleanup = true;

    this->itm->isConnecting = false;

    this->itm->clipboard.clear();
  }
}


/*
  checks to see if vnc client will fit within scroller
  (instance method)
*/
bool VncObject::fitsScroller ()
{
  if (app->vncViewer->fullscreen)
    return true;

  if (!this->itm || !this->itm->vnc)
    return false;

  const rfbClient * cl = this->itm->vnc->vncClient;
  if (!cl)
    return false;

  if (cl->width <= app->scroller->w() && cl->height <= app->scroller->h())
    return true;

  return false;
}


/*
  handle cursor change
  (static method / callback)
*/
void VncObject::handleCursorShapeChange (rfbClient * cl, int xHot, int yHot, int nWidth, int nHeight,
  int nBytesPerPixel)
{
  VncObject * vnc = static_cast<VncObject *>(rfbClientGetClientData(cl, m_vncObjPtr));

  if (!vnc || !cl || !vnc->allowDrawing || Fl::belowmouse() != app->vncViewer)
    return;

  const int nSSize = nWidth * nHeight * nBytesPerPixel;

  // if image has alpha, apply mask
  if (nBytesPerPixel == 2 || nBytesPerPixel == 4)
  {
    int nM = 0;

    for (int i = (nBytesPerPixel - 1); i < nSSize; i += nBytesPerPixel)
    {
      int nMV = cl->rcMask[nM];

      if (nMV > 0)
        nMV = 255;

      cl->rcSource[i] = nMV;

      nM ++;
    }
  }

  // create rgb image from raw data
  Fl_RGB_Image * img = new Fl_RGB_Image(cl->rcSource, nWidth, nHeight, nBytesPerPixel);

  if (!img)
    return;

  // delete previous copy, if any
  if (vnc->imgCursor)
    delete vnc->imgCursor;

  // copy rgb image to vncViewer and set x+y hotspots
  vnc->imgCursor = static_cast<Fl_RGB_Image *>(img->copy());

  if (!vnc->imgCursor)
    return;

  vnc->nCursorXHot = xHot;
  vnc->nCursorYHot = yHot;

  Fl::awake(svHandleThreadCursorChange, reinterpret_cast<void *>(false));

  delete img;
}


/*
  redraw VncObject when remote host updates framebuffer
  if the object is the active one
  (static method)
*/
void VncObject::handleFrameBufferUpdate (rfbClient * cl)
{
  if (!cl)
    return;

  const VncObject * vnc = static_cast<VncObject *>(rfbClientGetClientData(cl, m_vncObjPtr));
  if (!vnc)
    return;

  if (vnc->allowDrawing)
    app->vncViewer->redraw();
}


/*
  handle copy/cut FROM vnc host
  (static method)
*/
void VncObject::handleRemoteClipboardProc (rfbClient * cl, const char * text, int textlen)
{
  // The selection buffer (source is 0) is used for middle-mouse pastes and
  // for drag-and-drop selections. The clipboard (source is 1) is used for
  // copy/cut/paste operations.

  const VncObject * vnc = static_cast<VncObject *>(rfbClientGetClientData(cl, m_vncObjPtr));
  if (!vnc)
    return;

  HostItem * itm = vnc->itm;
  if (!itm)
    return;

  if (textlen > 0)
    itm->clipboard = text;
}


/*
  set VncViewer object to hide itself
  (static method)
*/
void VncObject::hideMainViewer ()
{
  VncObject * vnc = app->vncViewer->vnc;
  if (!vnc)
    return;

  vnc->allowDrawing = false;

  app->mainWin->cursor(FL_CURSOR_DEFAULT);

  Fl::lock();
  app->vncViewer->vnc = NULL;
  app->scroller->scroll_to(0, 0);
  app->scroller->type(0);
  app->scroller->redraw();
  Fl::unlock();
}


/*
  initialize and connect to a vnc host/server
  (this is called as a thread because it blocks)
  (static method)
*/
void * VncObject::initVNCConnection (void * data)
{
  // detach this thread
  pthread_detach(pthread_self());

  char * strParams[2] = {NULL};
  int nNumOfParams = 2;

  Fl::lock();
  HostItem * itm = static_cast<HostItem *>(data);
  Fl::unlock();

  if (!itm)
    return SV_RET_VOID;

  itm->threadRFBRunning = true;

  VncObject * vnc = itm->vnc;
  if (!vnc)
  {
    itm->isConnected = false;
    itm->isConnecting = false;
    itm->hasError = true;
    itm->threadRFBRunning = false;

    Fl::awake(svHandleThreadConnection, itm);

    return SV_RET_VOID;
  }

  // set parameter 0 - 'program name'
  strParams[0] = strdup("SpiritVNCFLTK");

  // set parameter 1
  if (!itm->isListener)
    // remote host address and port
    strParams[1] = strdup(itm->vncAddressAndPort.c_str());
  else
  {
    // local listening address
    strParams[1] = strdup("-listennofork");
    vnc->vncClient->listenAddress = app->listenAddressStr;
  }

  // if the second parameter is invalid, get out
  if (!strParams[1] || strlen(strParams[1]) < 7)
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
  if (!rfbInitClient(vnc->vncClient, &nNumOfParams, strParams))
  {
    // * connection failed *

    // (rfbClientCleanup should have happened inside rfbInitClient on failure)

    int errNum = errno;

    VncObject::parseErrorMessages(itm, strerror(errNum));

    itm->isConnected = false;
    itm->hasCouldntConnect = true;
  }
  else
  {
    // * connection succeeded *
    itm->isConnected = true;
    itm->isWaitingForShow = true;
    itm->initOkay = true;
  }

  // set flags for either outcome
  itm->isConnecting = false;
  itm->threadRFBRunning = false;

  // send message to main thread
  Fl::awake(svHandleThreadConnection, itm);

  // free strdups
  free(strParams[0]);
  free(strParams[1]);

  return SV_RET_VOID;
}


/*
  check connection errors and inform user, if necessary
  (static method)
*/
void VncObject::parseErrorMessages (HostItem * itm, const char * strMessageIn)
{
  char strMsg[FL_PATH_MAX] = {0};

  if (!itm || !strMessageIn)
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
    if (itm->hostType == 's' && !itm->sshReady)
      itm->lastErrorMessage = "Unable to connect to host's SSH server";
    else
      itm->lastErrorMessage = "Unable to connect to VNC server";

    return;
  }

  itm->lastErrorMessage = strMessage;
}


/*
  libvnc logging callback
  (static method)
*/
void VncObject::libVncLogging (const char * format, ...)
{
  if (app->debugMode)
  {
    va_list args;
    char msgBuf[SV_MAX_BUF_LEN] = {0};

    // combine format string and args into nowBuff
    va_start(args, format);
    vsnprintf(msgBuf, SV_MAX_BUF_LEN, format, args);
    va_end(args);

    // log to file
    svLogToFile(msgBuf);
  }
}


/*
  master loop to handle all vnc objects' message checking
  (static method)
*/
void VncObject::masterMessageLoop ()
{
  // run until it's time to shut down
  while (!app->shuttingDown)
  {
    // only loop if there are objects alive
    if (app->createdObjects != 0)
    {
      VncObject * vnc = app->vncViewer->vnc;

      if (vnc && vnc->itm)
        VncObject::checkVNCMessages(vnc);

      // keep from making too tight a loop

      // message loop waiting time - set by user now
      // the lower the number, faster drawing, but also more CPU usage
      // higher the number, slower drawing, less CPU usage
      Fl::wait(app->messageLoopWaitTime);
    }
    else
      // slower looping since nothing is connected right now
      Fl::wait(0.7);
  }
}


/*
  libvnc send credential to host callback
  credentials are mostly used by macOS VNC server but probably others too
  (static function)
*/
rfbCredential * VncObject::handleCredential (rfbClient * cl, int credentialType)
{
  // client is null
  if (!cl)
  {
    svLogToFile("ERROR - handlePassword: vnc->vncClient is null");
    return NULL;
  }

  // unsupported credential type
  if (credentialType != rfbCredentialTypeUser)
  {
    svLogToFile("ERROR - handleCredential: Non-username / password required for authentication");

    return NULL;
  }

  VncObject * vnc = static_cast<VncObject *>(rfbClientGetClientData(cl, m_vncObjPtr));
  // vnc object is null
  if (!vnc)
  {
    svLogToFile("ERROR - handlePassword: vnc is null");
    return NULL;
  }

  HostItem * itm = vnc->itm;
  // itm is null
  if (!itm)
  {
    svLogToFile("ERROR - handlePassword: itm is null");
    return NULL;
  }

  // build and populate credential
  rfbCredential * cred = static_cast<rfbCredential *>(malloc(sizeof(rfbCredential)));
  // get out if we can't allocate memory
  if (!cred)
    return NULL;

  // allocate memory for username / password
  cred->userCredential.username = static_cast<char *>(malloc(RFB_BUF_SIZE));
  cred->userCredential.password = static_cast<char *>(malloc(RFB_BUF_SIZE));

  // get out if we can't allocate memory
  if (!cred->userCredential.username || !cred->userCredential.password)
  {
    free(cred->userCredential.username);
    free(cred->userCredential.password);
    free(cred);
    return NULL;
  }

  // set username / password
  strncpy(cred->userCredential.username, itm->vncLoginUser.c_str(), (RFB_BUF_SIZE - 1));
  strncpy(cred->userCredential.password, base64Decode(itm->vncLoginPassword).c_str(), (RFB_BUF_SIZE - 1));

  return cred;
}


/*
  libvnc send password to host callback
  (static function)
*/
char * VncObject::handlePassword (rfbClient * cl)
{
  // check if client is non-null
  if (!cl)
  {
    svLogToFile("ERROR - handlePassword: vnc->vncClient is null");
    return NULL;
  }

  // check if vncObj is non-null
  const VncObject * vnc = static_cast<VncObject *>(rfbClientGetClientData(cl, m_vncObjPtr));
  if (!vnc)
  {
    svLogToFile("ERROR - handlePassword: vnc is null");
    return NULL;
  }

  // check if itm is non-null
  const HostItem * itm = vnc->itm;
  if (!itm)
  {
    svLogToFile("ERROR - handlePassword: itm is null");
    return NULL;
  }

  // malloc a char * that will be freed by libvncclient after use
  char * strPass = static_cast<char *>(malloc(10));
  if (!strPass)
    return NULL;

  // make a char * from the output of the base64Decode
  char * decodedPassword = strdup(base64Decode(itm->vncPassword).c_str());
  if (!decodedPassword)
  {
    free(strPass);
    return NULL;
  }

  // copy the vnc password to the malloc'd char *
  strncpy(strPass, decodedPassword, 9);

  // free our decoded char *
  free(decodedPassword);

  return strPass;
}


/*
  set vnc object to show itself
  (instance method)
*/
void VncObject::setObjectVisible ()
{
  if (!this->itm || !this->vncClient)
      return;

  app->vncViewer->vnc = this;

  SendFramebufferUpdateRequest(this->vncClient, 0, 0, this->vncClient->width, this->vncClient->height, false);

  //int leftMargin = app->flexLeftSide->w(); // + 3; //(app->hostList->x() + app->hostList->w() + 3);

  // scale off / scroll if host screen is too big
  if (this->itm->scaling == 's' || (this->itm->scaling == 'f' && this->fitsScroller()))
  {
    app->scroller->type(Fl_Scroll::BOTH);
    app->vncViewer->size(this->vncClient->width, this->vncClient->height);
  }

  // scale 'zoom' or 'fit'
  if (this->itm->scaling == 'z' || (this->itm->scaling == 'f' && !this->fitsScroller()))
  {
    // maximize vnc viewer size
    //app->vncViewer->size(app->mainWin->w() - leftMargin, app->mainWin->h());

    app->vncViewer->size(app->scroller->w(), app->scroller->h());

    // calculate host screen ratio
    float dRatio = static_cast<float>(this->vncClient->width) / static_cast<float>(this->vncClient->height);

    // set height of image to max height and check if resulting width is less than max width,
    // otherwise set width of image to max width and calculate new height
    if (static_cast<float>(app->vncViewer->h()) * dRatio <= app->vncViewer->w())
      app->vncViewer->size(static_cast<int>(static_cast<float>(app->vncViewer->h()) * dRatio),
        app->vncViewer->h());
    else
      app->vncViewer->size(app->vncViewer->w(),
        static_cast<int>(static_cast<float>(app->vncViewer->w()) / dRatio));
  }

  // only use when not in fullscreen
  if (!app->vncViewer->fullscreen)
    svResizeScroller();

  app->scroller->scroll_to(this->nLastScrollX, this->nLastScrollY);

  this->allowDrawing = true;

  app->scroller->redraw();
}


/*
  check and act on libvnc host messages
  (static method)
*/
void VncObject::checkVNCMessages (VncObject * vnc)
{
  int nMsg = 0;

  if (!vnc || !vnc->vncClient)
      return;

  // below modified to '0' based on select() docs suggestion
  nMsg = WaitForMessage(vnc->vncClient, 0);

  if (nMsg < 0)
  {
    vnc->endViewer();
    return;
  }

  if (nMsg)
  {
    if (!HandleRFBServerMessage(vnc->vncClient))
    {
      vnc->endViewer();
      return;
    }
    else
      Fl::check();
  }
}


/*
  ########################################################################################
  ########################## VNC VIEWER WIDGET ###########################################
  ########################################################################################
*/

/*
  draw event for vnc view widget
  (instance method)
*/
void VncViewer::draw ()
{
  VncObject * v = this->vnc;

  if (!v || !v->allowDrawing || !v->vncClient)
    return;

  rfbClient * cl = v->vncClient;
  if (!cl->frameBuffer)
    return;

  const HostItem * itm = v->itm;
  if (!itm)
    return;

  int nBytesPerPixel = cl->format.bitsPerPixel / 8;

  // get out if client or scroller size is wrong
  if (cl->width < 1 || cl->height < 1 || app->scroller->w() < 1 || app->scroller->h() < 1)
    return;

  // 's'croll or 'f'it + real size scale mode geometry
  if (itm->scaling == 's' || (itm->scaling == 'f' && v->fitsScroller()))
  {
    v->nLastScrollX = app->scroller->xposition();
    v->nLastScrollY = app->scroller->yposition();

    // draw that v host!
    fl_draw_image(
      cl->frameBuffer,
      app->scroller->x() - v->nLastScrollX,
      app->scroller->y() - v->nLastScrollY,
      cl->width,
      cl->height,
      nBytesPerPixel,
      0);

    return;
  }

  // 'z'oom or 'f'it + oversized scale mode geometry
  if (itm->scaling == 'z' || (itm->scaling == 'f' && !v->fitsScroller()))
  {
    int isize = cl->width * cl->height * nBytesPerPixel;

    // if there's an alpha byte, set it to 255
    if (nBytesPerPixel == 2 || nBytesPerPixel == 4)
      for (int i = (nBytesPerPixel - 1); i < isize; i+= nBytesPerPixel)
          cl->frameBuffer[i] = 255;

    // create an RGB image from libvncclient's framebuffer
    Fl_RGB_Image * imgRGB = new Fl_RGB_Image(cl->frameBuffer, cl->width, cl->height, nBytesPerPixel);
    if (imgRGB)
    {
      // set appropriate scale quality
      if (itm->scalingFast)
        imgRGB->RGB_scaling(FL_RGB_SCALING_NEAREST);
      else
        imgRGB->RGB_scaling(FL_RGB_SCALING_BILINEAR);

      //// scale down imgRGB image to imgFinal
      imgRGB->scale(app->vncViewer->w(), app->vncViewer->h());
      imgRGB->draw(this->x(), this->y());

      delete imgRGB;
    }
  }
}


/* handle events for vnc view widget */
/* (instance method) */
int VncViewer::handle (int event)
{
  if (!app->vncViewer)
    return 0;

  VncObject * v = this->vnc;
  if (!v)
    return 0;

  // bail out if this is not the active vnc object
  if (!v->allowDrawing)
    return 0;

  float nMouseX = 0;
  float nMouseY = 0;

  static int nButtonMask;

  const HostItem * itm = v->itm;
  if (!itm)
    return 0;

  // don't handle events if view only
  if (itm->viewOnly)
    return 0;

  const rfbClient * cl = v->vncClient;
  if (!cl)
    return 0;

  // scrolled / non-scaled sizing
  if (itm->scaling == 's' || (itm->scaling == 'f' && v->fitsScroller()))
  {
    nMouseX = Fl::event_x() - app->scroller->x() + app->scroller->xposition();
    nMouseY = Fl::event_y() - app->scroller->y() + app->scroller->yposition();
  }

  // scaled sizing
  if (itm->scaling == 'z' || (itm->scaling == 'f' && !v->fitsScroller()))
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

      SendPointerEvent(v->vncClient, nMouseX, nMouseY, nButtonMask);
      SendIncrementalFramebufferUpdateRequest(v->vncClient);

      app->scanIsRunning = false;
      return 1;
      break;

    case FL_PUSH:
        // left mouse button
        if (Fl::event_button() == FL_LEFT_MOUSE)
        {
          nButtonMask |= rfbButton1Mask;
          SendPointerEvent(v->vncClient, nMouseX, nMouseY, nButtonMask);
          app->scanIsRunning = false;
          return 1;
        }
        // right mouse button
        if (Fl::event_button() == FL_RIGHT_MOUSE)
        {
          nButtonMask |= rfbButton3Mask;
          SendPointerEvent(v->vncClient, nMouseX, nMouseY, nButtonMask);
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
          SendPointerEvent(v->vncClient, nMouseX, nMouseY, nButtonMask);
          SendIncrementalFramebufferUpdateRequest(v->vncClient);
          app->scanIsRunning = false;
          return 1;
        }

        // right mouse button
        if (Fl::event_button() == FL_RIGHT_MOUSE)
        {
          nButtonMask &= ~rfbButton3Mask;
          SendPointerEvent(v->vncClient, nMouseX, nMouseY, nButtonMask);
          SendIncrementalFramebufferUpdateRequest(v->vncClient);
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
          SendPointerEvent(v->vncClient, nMouseX, nMouseY, nButtonMask);

          nButtonMask &= ~nYDirection;
          SendPointerEvent(v->vncClient, nMouseX, nMouseY, nButtonMask);

          SendIncrementalFramebufferUpdateRequest(v->vncClient);
          return 1;
        }
        break;
    }

    case FL_MOVE:
      SendPointerEvent(v->vncClient, nMouseX, nMouseY, nButtonMask);
      return 1;
      break;

    // ** keyboard events **
    case FL_KEYDOWN:
      sendCorrectedKeyEvent(Fl::event_text(), Fl::event_key(), true);
      app->scanIsRunning = false;
      return 1;
      break;

    case FL_SHORTCUT:
      sendCorrectedKeyEvent(Fl::event_text(), Fl::event_key(), true);
      app->scanIsRunning = false;
      return 1;
      break;

    case FL_KEYUP:
      sendCorrectedKeyEvent(Fl::event_text(), Fl::event_key(), false);
      app->scanIsRunning = false;
      return 1;
      break;

    // ** misc events **
    case FL_ENTER:
      if (v->imgCursor && itm->showRemoteCursor)
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
          std::string strClipText = Fl::event_text();

          // send clipboard text to remote server
          SendClientCutText(app->vncViewer->vnc->vncClient,
            const_cast<char *>(strClipText.c_str()), intClipLen);
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
  (const int not used so parameter name removed)
*/
/* (instance method) */
void VncViewer::sendCorrectedKeyEvent (const char * strIn, const int nK, bool downState)
{
  VncObject * v = this->vnc;
  if (!v)
    return;

  HostItem * itm = v->itm;
  if (!itm)
    return;

  rfbClient * cl = v->vncClient;
  if (!cl)
    return;

  // F8 window
  if (nK == XK_F8)
  {
    if (!downState)
      svShowF8Window();

    return;
  }

  // F11 fullscreen
  if (nK == XK_F11)
  {
      // toggle vncViewer's fullscreen state
    if (!downState)
    {
      // we can disable fullscreen no matter what
      if (this->fullscreen)
      {
        this->unsetFullScreen();
        return;
      }

      // only enable fullscreen if we're displaying a connection
      if (!this->fullscreen && v)
      {
        this->setFullScreen();
        return;
      }
    }

    return;
  }

  // F12 macro
  if (nK == XK_F12)
  {
    if (!downState)
    {
      // send the F12 macro for this connection, if any,
      // if there isn't an F12 macro, send the F12 key itself
      if (!itm->f12Macro.empty())
        svSendKeyStrokesToHost(itm->f12Macro, v);
      else
      {
        SendKeyEvent(v->vncClient, XK_F12, true);
        SendKeyEvent(v->vncClient, XK_F12, false);
      }
    }

    return;
  }

  // send key
  if ((nK >= 32 && nK <= 255) && Fl::event_ctrl() == 0)
    SendKeyEvent(cl, strIn[0], downState);
  else
    SendKeyEvent(cl, nK, downState);
}


/* enable fullscreen */
/* (instance method) */
void VncViewer::setFullScreen ()
{
  this->fullscreen = true;

  // hide the left flex container
  app->flexLeftSide->hide();

  // set scroller stuff
  app->scroller->position(0, 0);
  app->scroller->size(app->mainWin->w(), app->mainWin->h());
  app->scroller->redraw();

  this->size(app->scroller->w(), app->scroller->h());

  if (this->vnc)
    this->vnc->setObjectVisible();

  this->set_visible_focus();

  Fl::redraw();
  Fl::check();
}


/* disable fullscreen */
/* (instance method) */
void VncViewer::unsetFullScreen ()
{
  this->fullscreen = false;

  // show the left flex container
  app->flexLeftSide->show();
  app->flexLeftSide->redraw();

  // set scroller stuff
  app->scroller->position(app->hostList->x() + app->hostList->w() + 3, app->scroller->y());
  app->scroller->size(app->mainWin->w(), app->mainWin->h());
  app->scroller->redraw();
  svResizeScroller();

  if (this->vnc)
    this->vnc->setObjectVisible();

  this->set_visible_focus();

  Fl::redraw();
  Fl::check();
}
