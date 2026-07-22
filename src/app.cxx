/*
 * app.cxx - part of SpiritVNC - FLTK
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

/* unordered maps for child windows and their children */
std::unordered_map<std::string, void *> m_appOptions;
std::unordered_map<std::string, void *> m_f8Actions;
std::unordered_map<std::string, void *> m_itmSettings;
std::unordered_map<std::string, void *> m_quickNoteEdit;


/*
  resize override method for SVMainWindow
  (instance method)
*/
void SVMainWindow::resize (int x, int y, int w, int h)
{
  // call original handler
  Fl_Double_Window::resize(x, y, w, h);

  if (!app->vncViewer)
    return;

  VncObject * vnc = app->vncViewer->vnc;

  // only check if there's a valid vnc object being displayed
  if (vnc)
  {
    // check if the main window's geometry has changed
    if (app->savedX != x ||
        app->savedY != y ||
        app->savedW != w ||
        app->savedH != h)
    {
      // store new geometry
      app->savedX = x;
      app->savedY = y;
      app->savedW = w;
      app->savedH = h;

      VncObject::hideMainViewer();

      // re-display active vnc object
      vnc->setObjectVisible();
    }
  }
}

/*
  handle method for SVInput
  (instance method)
*/
int SVInput::handle (int evt)
{
  // handle child window input controls right-click
  if (evt == FL_PUSH && Fl::event_button3() != 0)
  {
    svPopUpEditMenu(this);

    return 1;
  }

  return Fl_Input::handle(evt);
}


/*
  handle method for SVIntInput
  (instance method)
*/
int SVIntInput::handle (int evt)
{
  // handle child window input controls right-click
  if (evt == FL_PUSH && Fl::event_button3() != 0)
  {
    svPopUpEditMenu(this);

    return 1;
  }

  /* (apparently Fl_Int_Input isn't supported yet?? it's in the FLTK docs though...) */
  return Fl_Int_Input::handle(evt);
}


/*
  handle method for SVSecretInput
  (instance method)
*/
int SVSecretInput::handle (int evt)
{
  // handle child window input controls right-click
  if (evt == FL_PUSH && Fl::event_button3() != 0)
  {
    svPopUpEditMenu(this);

    return 1;
  }

  return Fl_Secret_Input::handle(evt);
}


/*
  handle method for SVQuickNoteGroup
  (instance method)
*/
int SVQuickNotePack::handle (int evt)
{
  // we will accept focus by returning non-zero
  if (evt == FL_FOCUS || evt == FL_UNFOCUS)
    return 1;

  //return Fl_Pack::handle(evt);
  return Fl_Flex::handle(evt);
}


/*
  handle method for SVBox
  (instance method)
*/
int SVQuickNoteBox::handle (int evt)
{
  // get currently selected list item
  int curLine = app->hostList->value();

  // handle quicknote click if an item is selected
  if (evt == FL_PUSH && curLine > 0 && !app->scanIsRunning)
  {
    // show the editor if there's a selected item
    HostItem * itm = static_cast<HostItem *>(app->hostList->data(curLine));
    if (itm && !itm->name.empty())
      svShowQuickNoteEditorWindow(itm);
  }

  return Fl_Multiline_Output::handle(evt);
}

/*
  fl_text_editor subclass handler for quick note editing
  (instance method)
*/
int SVQuickNoteTextEditor::handle (int evt)
{
  // handle enter key - save edits
  if (evt == FL_KEYDOWN && Fl::event_key() == FL_Enter && !Fl::event_state(FL_SHIFT))
  {
    if (m_quickNoteEdit.empty())
      return 1;

    Fl_Button * button = static_cast<Fl_Button *>(m_quickNoteEdit["btnSave"]);

    svHandleQuickNoteEditorButtons(button, NULL);

    return 1;
  }

  return Fl_Text_Editor::handle(evt);
}


void svDoStartupTasks ()
{
  // create program UI
  svCreateGUI();

  // load host list
  svLoadHostList();

  // add default empty host list item if no items added from config file
  if (app->hostList->size() == 0)
  {
    svInsertEmptyItem();
    app->hostList->size(170, app->hostList->h());
  }

  // set app tooltips
  svSetAppTooltips();

  // set app icons
  svCreateAppIcons();

  // manually trigger misc events callback
  svPositionWidgets();

  // show app window
  app->mainWin->show(app->argc, app->argv);

  Fl::focus(app->hostList);
  app->hostList->take_focus();
}


/* actually does the hiding and deletion of child windows */
void svCloseDeleteFinalizeChildWindow (Fl_Window * win)
{
  // close the child window, if valid
  if (win)
  {
    win->hide();
    delete win;
    win = NULL;
  }

  // set app child window visible property
  app->childWindowVisible = false;
  app->childWindowBeingDisplayed = NULL;

  if (app->mainWin)
    app->mainWin->redraw();
}


/*
  child window 'OK' button callback - closes child windows (settings, options, etc
  (Fl_Widget * is unused, so parameter name removed according to 'best practices')
*/
void svCloseChildWindow (Fl_Widget *, void * data)
{
  Fl_Window * childWindow = static_cast<Fl_Window *>(data);

  svCloseDeleteFinalizeChildWindow(childWindow);
}


/*  creates new configuration directory if none is found  */
void svConfigCreateNewDir ()
{
  struct stat st;

  if (stat(app->configPath.c_str(), &st) == -1)
  {
    // windows
    #ifdef _WIN32
    if (mkdir(app->configPath.c_str()) == -1)
    // *nix-like
    #else
    if (mkdir(app->configPath.c_str(), 0700) == -1)
    #endif
    {
      std::cout << "SpiritVNC - ERROR - Cannot create config file directory" << std::endl;
      exit(-1);
    }
  }
}

void svLoadHostList ()
{
  std::ifstream ifs;
  char strP[SV_MAX_PROP_LINE_LEN] = {0};
  std::string strLastGroup;
  std::string strProp;
  std::string strVal;
  HostItem * itm = NULL;
  bool addSep = false;
  uint16_t numOfEntries = 0;

  app->hostList->clear();

  // try to open config file
  ifs.open(app->configPathAndFile.c_str(), std::ifstream::in);

  // oops, can't open config file
  if (ifs.fail())
    return;

  // process hosts part of config file
  while (!app->shuttingDown)
  {
    ifs.getline(strP, SV_MAX_PROP_LINE_LEN);

    if (ifs.good())
    {
      strProp = svGetConfigProperty(strP);
      strVal = svGetConfigValue(strP);

      if (strP[0] != '\0' && strP[0] != '#')
      {
        // #############################################################################
        // ######## per-connection options #############################################
        // #############################################################################

        // new host entry
        if (strProp == "host")
        {
          // add last host entry to host list
          if (itm && numOfEntries < SV_MAX_HOSTLIST_ENTRIES)
          {
            numOfEntries ++;
            app->hostList->add(itm->name.c_str(), static_cast<void *>(itm));
          }

          itm = new HostItem();

          itm->name = strVal;
        }

        // host address
        if (strProp == "hostaddress")
          itm->hostAddress = strVal;

        // group
        if (strProp == "group")
        {
          itm->group = strVal;

          if (strLastGroup != itm->group)
          {
            if (addSep)
              // add a separator
              // color 16 (@C16) is supposed to be gray colour
              app->hostList->add("@C16@.· · ·");
            else
            {
              // add empty row at top of list
              app->hostList->add(" ");
              addSep = true;
            }
          }

          strLastGroup = itm->group;
        }

        // vnc port
        if (strProp == "vncport")
          itm->vncPort = strVal;

        // ssh port
        if (strProp == "sshport")
          itm->sshPort = strVal;

        // ssh key private file path
        if (strProp == "sshkeyprivate")
          itm->sshKeyPrivate = strVal;

        // ssh user
        if (strProp == "sshuser")
          itm->sshUser = strVal;

        //// ssh password
        //if (strProp == "sshpass")
          //itm->sshPass = strVal;

        // vnc password (password authentication)
        if (strProp == "vncpass")
          itm->vncPassword = strVal;

        // vnc login user (credential authentication)
        if (strProp == "vncloginuser")
          itm->vncLoginUser = strVal;

        // vnc login password (credential authentication)
        if (strProp == "vncloginpass")
          itm->vncLoginPassword = strVal;

        // host type
        if (strProp == "type")
        {
          if (strVal == "s")
            itm->hostType = 's';
        }

        // F12 macro
        if (strProp == "f12macro")
          itm->f12Macro = strVal;

        // scaling
        if (strProp == "scale")
        {
          if (strVal == "f")
            itm->scaling = 'f';
          else if (strVal == "z")
            itm->scaling = 'z';
          else if (strVal == "s")
            itm->scaling = 's';
        }

        // fast scaling?
        if (strProp == "scalefast")
          itm->scalingFast = svConvertStringToBoolean(strVal);

        // show remote cursor?
        if (strProp == "showremotecursor")
          itm->showRemoteCursor = svConvertStringToBoolean(strVal);

        // compression level
        if (strProp == "compression")
        {
          itm->compressLevel = atoi(strVal.c_str());

          if (itm->compressLevel > 9)
            itm->compressLevel = 9;
        }

        // quality level
        if (strProp == "quality")
        {
          itm->qualityLevel = atoi(strVal.c_str());

          if (itm->qualityLevel > 9)
            itm->qualityLevel = 9;
        }

        //// center x?
        //if (strProp == "centerx")
          //itm->centerX = svConvertStringToBoolean(strVal);

        //// center y?
        //if (strProp == "centery")
          //itm->centerY = svConvertStringToBoolean(strVal);

        // quicknote
        if (strProp == "quicknote")
          itm->quickNote = base64Decode(strVal);

        // last connected time
        if (strProp == "lastconnecttime")
          itm->lastConnectedTime = strVal;

        // view only
        if (strProp == "viewonly")
          itm->viewOnly= svConvertStringToBoolean(strVal);

        // custom command 1 enabled?
        if (strProp == "customcommand1enabled")
          itm->customCommand1Enabled = svConvertStringToBoolean(strVal);

        // custom command 1 label
        if (strProp == "customcommand1label")
          itm->customCommand1Label = strVal;

        // custom command 1
        if (strProp == "customcommand1")
          itm->customCommand1 = strVal;

        // custom command 2 enabled?
        if (strProp == "customcommand2enabled")
          itm->customCommand2Enabled = svConvertStringToBoolean(strVal);

        // custom command 2 label
        if (strProp == "customcommand2label")
          itm->customCommand2Label = strVal;

        // custom command 2
        if (strProp == "customcommand2")
          itm->customCommand2 = strVal;

        // custom command 3 enabled?
        if (strProp == "customcommand3enabled")
          itm->customCommand3Enabled = svConvertStringToBoolean(strVal);

        // custom command 3 label
        if (strProp == "customcommand3label")
          itm->customCommand3Label = strVal;

        // custom command 3
        if (strProp == "customcommand3")
          itm->customCommand3 = strVal;
      }
    }
    else
    {
      // add last host entry to host list
      if (itm && numOfEntries < SV_MAX_HOSTLIST_ENTRIES)
        app->hostList->add(itm->name.c_str(), itm);

      // add a separator
      if (addSep)
        // color 16 (@C16.) is supposed to be gray
        app->hostList->add("@C16@.· · ·");

      break;
    }
  }

  ifs.close();
}


/*
  read from the config file, set app
  options and populate host list
*/
void svConfigRead ()
{
  std::ifstream ifs;
  char strP[SV_MAX_PROP_LINE_LEN] = {0};
  std::string strProp;
  std::string strVal;

  // try to open config file
  ifs.open(app->configPathAndFile.c_str(), std::ifstream::in);

  // oops, can't open config file
  if (ifs.fail())
  {
    std::cout << "SpiritVNC - Could not open config file.  Using defaults" << std::endl;
    svConfigCreateNewDir();
    return;
  }

  svLogToFile("--- Program started up ---");

  while (!app->shuttingDown)
  {
    ifs.getline(strP, SV_MAX_PROP_LINE_LEN);

    if (ifs.good())
    {
      strProp = svGetConfigProperty(strP);
      strVal = svGetConfigValue(strP);

      if (strP[0] != '\0' && strP[0] != '#')
      {
        // ########## app options ###################################

        // hostlist width
        // NOTE: setting this too low may hide
        // one or more of the buttons at the bottom of the hostlist
        // such as 'options', 'help', 'listen'...
        if (strProp == "hostlistwidth")
        {
          app->requestedListWidth = atoi(strVal.c_str());

          if (app->requestedListWidth < 100)
            app->requestedListWidth = 100;
        }

        // use colorblind icons?
        if (strProp == "colorblindicons")
          app->colorBlindIcons = svConvertStringToBoolean(strVal);

        // scan timeout in seconds
        if (strProp == "scantimeout")
        {
          int w = atoi(strVal.c_str());

          if (w < 1)
            w = 1;

          app->nScanTimeout = w;
        }

        // ssh command
        if (strProp == "sshcommand")
        {
          app->sshCommand = strVal;

          if (app->sshCommand.empty())
            app->sshCommand = "ssh";
        }

        // starting local port number for ssh
        if (strProp == "startinglocalport")
        {
          int w = atoi(strVal.c_str());

          if (w < 1)
            w = 15000;

          app->nStartingLocalPort = w;
        }

        // display tooltips?
        if (strProp == "showtooltips")
          app->showTooltips = svConvertStringToBoolean(strVal);

        // log app events to file?
        if (strProp == "logtofile")
          app->enableLogToFile = svConvertStringToBoolean(strVal);

        // right-click immediately closes connection
        if (strProp == "rightclicktoclose")
          app->rightClickToClose = svConvertStringToBoolean(strVal);

        // display debug messages?
        if (strProp == "debugmode")
          app->debugMode = svConvertStringToBoolean(strVal);

        // app font size
        if (strProp == "appfontsize")
        {
          app->nAppFontSize = atoi(strVal.c_str());

          // fix minimum app font size
          if (app->nAppFontSize < SV_APP_FONT_SIZE_MIN)
            app->nAppFontSize = SV_APP_FONT_SIZE_MIN;

          // fix maximum app font size
          if (app->nAppFontSize > SV_APP_FONT_SIZE_MAX)
            app->nAppFontSize = SV_APP_FONT_SIZE_MAX;
        }

        // list font
        if (strProp == "listfont")
        {
          if (!strVal.empty())
            app->strListFont = strVal;
        }

        // list font size
        if (strProp == "listfontsize")
        {
          app->nListFontSize = atoi(strVal.c_str());

          // fix minimum list font size
          if (app->nListFontSize < SV_LIST_FONT_SIZE_MIN)
            app->nListFontSize = SV_LIST_FONT_SIZE_MIN;

          // fix maximum list font size
          if (app->nListFontSize > SV_LIST_FONT_SIZE_MAX)
            app->nListFontSize = SV_LIST_FONT_SIZE_MAX;
        }

        // saved x position
        if (strProp == "savedx")
        {
          app->savedX = atoi(strVal.c_str());

          if (app->savedX < 1)
            app->savedX = 0;
        }

        // saved y position
        if (strProp == "savedy")
        {
          app->savedY = atoi(strVal.c_str());

          if (app->savedY < 1)
            app->savedY = 0;
        }

        // saved width
        if (strProp == "savedw")
        {
          app->savedW = atoi(strVal.c_str());

          if (app->savedW < 1)
            app->savedW = 800;
        }

        // saved height
        if (strProp == "savedh")
        {
          app->savedH = atoi(strVal.c_str());

          if (app->savedH < 1)
            app->savedH = 600;
        }

        // display message when reverse connections connect?
        if (strProp == "showreverseconnect")
          app->showReverseConnect = svConvertStringToBoolean(strVal);

        // maximize if last window state was maximized
        if (strProp == "maximized")
          app->maximized = svConvertStringToBoolean(strVal);

        // host list button position - top or bottom
        if (strProp == "buttonsontop")
          app->buttonsOnTop = svConvertStringToBoolean(strVal);

        // message loop wait time
        if (strProp == "msgloopspeed")
        {
          app->messageLoopSpeed = atoi(strVal.c_str());

          svValidateAndSetMessageLoopSpeed();
        }
      }
    }
    else
      break;
  }

  ifs.close();
}


/*  write config file  */
void svConfigWrite ()
{
  static bool inConfigWrite = false;
  if (inConfigWrite)
    return;

  inConfigWrite = true;

  // output stream
  std::ofstream ofs;

  // open our config file
  ofs.open(app->configPathAndFile.c_str());

  // oops, can't open config file
  if (ofs.fail())
  {
    std::cout << "SpiritVNC ERROR - Could not open config file for writing" << std::endl;
    inConfigWrite = false;
    return;
  }

  // write header
  ofs << "# SpiritVNC-FLTK config file" <<  std::endl;
  ofs << "# Generated by program version " SV_APP_VERSION << std::endl;
  ofs << "#" << std::endl;
  ofs << "# option names / properties should always be lower-case without spaces" << std::endl;
  ofs << "# host type can be 'v' for vnc and 's' for vnc through ssh" << std::endl;
  ofs << "# scale can be 's' for scrolled, 'z' for scale up/down and 'f' for scale"
      " down only" << std::endl;
  ofs << std::endl;

  // app options
  ofs << "# program options" << std::endl;

  // hostlist width
  ofs << "hostlistwidth=" << app->requestedListWidth << std::endl;

  // colorblind icons
  ofs << "colorblindicons=" << svConvertBooleanToString(app->colorBlindIcons) << std::endl;

  // scan timeout in seconds
  ofs << "scantimeout=" << app->nScanTimeout << std::endl;

  // starting local port number (+99) for ssh connections
  ofs << "startinglocalport=" << app->nStartingLocalPort << std::endl;

  // ssh command
  ofs << "sshcommand=" << app->sshCommand << std::endl;

  // show tool tips
  ofs << "showtooltips=" << svConvertBooleanToString(app->showTooltips) << std::endl;

  // log app events to file
  ofs << "logtofile=" << svConvertBooleanToString(app->enableLogToFile) << std::endl;

  // right-click immediately closes connection
  ofs << "rightclicktoclose=" << svConvertBooleanToString(app->rightClickToClose) << std::endl;

  // show debugging messages
  ofs << "debugmode=" << svConvertBooleanToString(app->debugMode) << std::endl;

  // show reverse-connect message
  ofs << "showreverseconnect=" << svConvertBooleanToString(app->showReverseConnect) << std::endl;

  // app font size
  ofs << "appfontsize=" << app->nAppFontSize << std::endl;

  // list font
  ofs << "listfont=" << app->strListFont << std::endl;
  ofs << "listfontsize=" << app->nListFontSize << std::endl;

  // saved position and size
  ofs << "savedx=" << app->savedX << std::endl;
  ofs << "savedy=" << app->savedY << std::endl;
  ofs << "savedw=" << app->savedW << std::endl;
  ofs << "savedh=" << app->savedH << std::endl;

  ofs << "maximized=" << app->maximized << std::endl;

  // buttons on top or bottom
  ofs << "buttonsontop=" << svConvertBooleanToString(app->buttonsOnTop) << std::endl;

  // vnc message loop wait time
  ofs << "msgloopspeed=" << app->messageLoopSpeed << std::endl;

  // blank line
  ofs << std::endl;

  // host list entries
  ofs << "# host-list entries" << std::endl;

  uint16_t nSize = app->hostList->size();

  for (uint16_t i = 0; i <= nSize; i ++)
  {
    HostItem * itm = static_cast<HostItem *>(app->hostList->data(i));

    if (!itm || itm->isListener)
      continue;

    ofs << "host=" << itm->name << std::endl;
    ofs << "group=" << itm->group << std::endl;
    ofs << "hostaddress=" << itm->hostAddress << std::endl;
    ofs << "vncport=" << itm->vncPort << std::endl;
    ofs << "sshport=" << itm->sshPort << std::endl;
    ofs << "vncpass=" << itm->vncPassword << std::endl;
    ofs << "vncloginuser=" << itm->vncLoginUser << std::endl;
    ofs << "vncloginpass=" << itm->vncLoginPassword << std::endl;
    ofs << "type=" << itm->hostType << std::endl;
    ofs << "sshkeyprivate=" << itm->sshKeyPrivate << std::endl;
    ofs << "sshuser=" << itm->sshUser << std::endl;
    //ofs << "sshpass=" << itm->sshPass << std::endl;
    ofs << "scale=" << itm->scaling << std::endl;
    ofs << "scalefast=" << svConvertBooleanToString(itm->scalingFast) << std::endl;
    ofs << "f12macro=" << itm->f12Macro << std::endl;
    ofs << "showremotecursor=" << svConvertBooleanToString(itm->showRemoteCursor) << std::endl;
    ofs << "compression=" << std::to_string(itm->compressLevel) << std::endl;
    ofs << "quality=" << std::to_string(itm->qualityLevel) << std::endl;
    //ofs << "ignoreinactive=" << svConvertBooleanToString(itm->ignoreInactive) << std::endl;
    //ofs << "centerx=" << svConvertBooleanToString(itm->centerX) << std::endl;
    //ofs << "centery=" << svConvertBooleanToString(itm->centerY) << std::endl;
    ofs << "quicknote=" << base64Encode(reinterpret_cast<const unsigned char *>
      (itm->quickNote.c_str()), itm->quickNote.size()) << std::endl;
    ofs << "lastconnecttime=" << itm->lastConnectedTime << std::endl;
    ofs << "viewonly=" << svConvertBooleanToString(itm->viewOnly) << std::endl;
    ofs << "customcommand1enabled=" << svConvertBooleanToString(itm->customCommand1Enabled) << std::endl;
    ofs << "customcommand1label=" << itm->customCommand1Label << std::endl;
    ofs << "customcommand1=" << itm->customCommand1 << std::endl;
    ofs << "customcommand2enabled=" << svConvertBooleanToString(itm->customCommand2Enabled) << std::endl;
    ofs << "customcommand2label=" << itm->customCommand2Label << std::endl;
    ofs << "customcommand2=" << itm->customCommand2 << std::endl;
    ofs << "customcommand3enabled=" << svConvertBooleanToString(itm->customCommand3Enabled) << std::endl;
    ofs << "customcommand3label=" << itm->customCommand3Label << std::endl;
    ofs << "customcommand3=" << itm->customCommand3 << std::endl;

    ofs << std::endl;
  }

  ofs << std::endl;

  // close file
  ofs.close();

  inConfigWrite = false;
}


/*
  a connection 'supervisor' that is called approx. every second by a timer
  (timer callback)
  (void * not used so parameter name removed)
*/
void svConnectionWatcher (void *)
{
  // only check if there are waiting viewers
  if (app->nViewersWaiting > 0)
  {
    svDebugLog("svConnectionWatcher - At least one itm ready for processing");

    uint16_t nSize = app->hostList->size();

    // iterate through hostlist items
    for (uint16_t i = 0; i <= nSize; i ++)
    {
      HostItem * itm = static_cast<HostItem *>(app->hostList->data(i));
      if (!itm)
        continue;

      const VncObject * vnc = itm->vnc;
      if (!vnc)
        continue;

      // if ssh connection faltered, shut down the vnc viewer
      if (itm->isConnected && (itm->hostType == 's' && !itm->sshReady))
      {
        app->nViewersWaiting --;
        svDebugLog("svConnectionWatcher - SSH problem during connection, ending");

        itm->vnc->endViewer();
      }

      // cleanup vnc client structure and delete vnc object
      if (itm->vncNeedsCleanup)
        VncObject::cleanupVNCObject(itm);
    }
  }

  // set timer to call this function again in 1 second
  // (do NOT change this interval as connection timeout
  // values rely on this being at or near 1 second)
  Fl::repeat_timeout(SV_ONE_SECOND, svConnectionWatcher);
}


/*  convert boolean to string  */
std::string svConvertBooleanToString (bool boolIn)
{
  if (boolIn)
    return "true";

  return "false";
}


/*  convert string to boolean  */
bool svConvertStringToBoolean (const std::string& strIn)
{
  std::string strOut;

  // go character by character to build lowercase string
  for (const char & c : strIn)
    strOut += std::tolower(c);

  if (strOut == "true" || strOut == "yes" || strOut == "on" || strOut == "1")
    return true;

  return false;
}


/*  create icons for app  */
void svCreateAppIcons (const bool fromAppOptions)
{
  // default or colorblind icons
  if (!app->colorBlindIcons)
  {
    // use default icons
    app->iconDisconnected = new Fl_Pixmap(pmStatusDisconnected);
    app->iconDisconnectedError = new Fl_Pixmap(pmStatusDisconnectedError);
    app->iconDisconnectedBigError = new Fl_Pixmap(pmStatusDisconnectedBigError);
    app->iconConnected = new Fl_Pixmap(pmStatusConnected);
    app->iconNoConnect = new Fl_Pixmap(pmStatusNoConnect);
    app->iconConnecting = new Fl_Pixmap(pmStatusConnecting);
  }
  else
  {
    // use colorblind icons
    app->iconDisconnected = new Fl_Pixmap(pmStatusDisconnectedCB);
    app->iconDisconnectedError = new Fl_Pixmap(pmStatusDisconnectedErrorCB);
    app->iconDisconnectedBigError = new Fl_Pixmap(pmStatusDisconnectedBigErrorCB);
    app->iconConnected = new Fl_Pixmap(pmStatusConnectedCB);
    app->iconNoConnect = new Fl_Pixmap(pmStatusNoConnectCB);
    app->iconConnecting = new Fl_Pixmap(pmStatusConnectingCB);
  }

  // list button icons
  app->btnListAdd->image(new Fl_Pixmap(pmListAddNew));
  app->btnListDelete->image(new Fl_Pixmap(pmListDelete));
  app->btnListDown->image(new Fl_Pixmap(pmListDown));
  app->btnListListen->image(new Fl_Pixmap(pmListListen));
  app->btnListScan->image(new Fl_Pixmap(pmListScan));
  app->btnListUp->image(new Fl_Pixmap(pmListUp));
  app->btnListOptions->image(new Fl_Pixmap(pmListOptions));
  app->btnListHelp->image(new Fl_Pixmap(pmListHelp));

  // set appropriate list icons only when starting up
  if (!fromAppOptions)
  {
    // set initial icons on hostlist items
    for (uint16_t i = 0; i <= app->hostList->size(); i ++)
    {
      if (app->hostList->data(i))
        app->hostList->icon(i, app->iconDisconnected);
    }

    app->hostList->redraw();
  }
}


/* create the task list button section */
/* (this could be positioned at the top or bottom of the left side) */
void createHostListButtons ()
{
  // =============== host list buttons start ====================
  // button size constant
  const u_int nBtnSize = 20;

  app->packButtons = new Fl_Flex(Fl_Flex::HORIZONTAL);

  app->btnListAdd = new Fl_Button(0, 0, nBtnSize, nBtnSize);
  app->btnListAdd->clear_visible_focus();
  app->btnListAdd->callback(svHandleHostListButtons);

  app->btnListDelete = new Fl_Button(0, 0, nBtnSize, nBtnSize);
  app->btnListDelete->clear_visible_focus();
  app->btnListDelete->callback(svHandleHostListButtons);

  app->btnListUp = new Fl_Button(0, 0, nBtnSize, nBtnSize);
  app->btnListUp->clear_visible_focus();
  app->btnListUp->callback(svHandleHostListButtons);

  app->btnListDown = new Fl_Button(0, 0, nBtnSize, nBtnSize);
  app->btnListDown->clear_visible_focus();
  app->btnListDown->callback(svHandleHostListButtons);

  app->btnListScan = new Fl_Button(0, 0, nBtnSize, nBtnSize);
  app->btnListScan->clear_visible_focus();
  app->btnListScan->callback(svHandleHostListButtons);

  app->btnListListen = new Fl_Button(0, 0, nBtnSize, nBtnSize);
  app->btnListListen->clear_visible_focus();
  app->btnListListen->callback(svHandleHostListButtons);

  app->btnListHelp = new Fl_Button(0, 0, nBtnSize, nBtnSize);
  app->btnListHelp->clear_visible_focus();
  app->btnListHelp->user_data(app->btnListHelp);
  app->btnListHelp->callback(svHandleHostListButtons);

  app->btnListOptions = new Fl_Button(0, 0, nBtnSize, nBtnSize);
  app->btnListOptions->clear_visible_focus();
  app->btnListOptions->callback(svHandleHostListButtons);

  app->packButtons->end();

  app->flexLeftSide->fixed(app->packButtons, nBtnSize + 4);
}


/*  create program GUI  */
void svCreateGUI ()
{
  // widget tooltips are set in svSetTooltips
  // some additional positioning and sizing is done in svPositionWidgets

  // create main window
  app->mainWin = new SVMainWindow(1024, 768);
  app->mainWin->size_range(800, 600, 32767, 32767);
  app->mainWin->label("SpiritVNC");
  app->mainWin->xclass("spiritvncfltk");
  app->mainWin->callback(svHandleMainWindowEvents);

  // do app icon first
  app->iconApp = new Fl_RGB_Image(new Fl_Pixmap(pmSpiritvnc_xpm));
  app->mainWin->default_icon(app->iconApp);

  // explicitly start adding widgets to the app window
  app->mainWin->begin();

  // *** create flexParent ***
  app->flexParent = new Fl_Flex(0, 0, 1024, 768, Fl_Flex::HORIZONTAL);

  // *** create app->flexLeftSide ***
  app->flexLeftSide = new Fl_Flex(Fl_Flex::VERTICAL);
  app->flexLeftSide->margin(2); //2, 2, 0, 2);
  app->flexLeftSide->gap(2);

  // ======= create host list buttons (if user wants them on top) =======
  if (app->buttonsOnTop)
    createHostListButtons();

  // create host list
  app->hostList = new Fl_Hold_Browser(0, 0, 0, 0); //0, 0, 163, 548);
  app->hostList->clear_visible_focus();
  app->hostList->callback(svHandleHostListEvents, NULL);
  app->hostList->box(FL_THIN_DOWN_BOX);

  // set host list font face and size
  Fl::set_font(SV_LIST_FONT_ID, app->strListFont.c_str());
  app->hostList->textfont(SV_LIST_FONT_ID);
  app->hostList->textsize(app->nListFontSize);

  // set app menu font size based on hostlist font size
  app->nMenuFontSize = app->nListFontSize;

  // =============== quick info start =======================

  // quick info pack
  app->quickInfoPack = new Fl_Flex(Fl_Flex::VERTICAL);

  // item name label
  app->quickInfoLabel = new Fl_Box(0, 0, 0, 0);
  app->quickInfoLabel->labelsize(app->nAppFontSize - 1);
  app->quickInfoLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  app->quickInfoLabel->labelfont(FL_HELVETICA);
  app->quickInfoLabel->labelcolor(fl_rgb_color(SV_QUICK_INFO_FG_COLOR));

  app->quickInfoPack->fixed(app->quickInfoLabel, 20);

  // last connected label
  app->lastConnectedLabel = new Fl_Box(0, 0, 0, 0);
  app->lastConnectedLabel->labelsize(app->nAppFontSize - 2);
  app->lastConnectedLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_BOTTOM);
  app->lastConnectedLabel->labelcolor(fl_rgb_color(SV_QUICK_INFO_FG_COLOR));

  app->quickInfoPack->fixed(app->lastConnectedLabel, 18);

  // last connected
  app->lastConnected = new Fl_Box(0, 0, 0, 0);
  app->lastConnected->labelsize(app->nAppFontSize - 2);
  app->lastConnected->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_TOP);
  app->lastConnected->labelcolor(fl_rgb_color(SV_QUICK_INFO_FG_COLOR));

  app->quickInfoPack->fixed(app->lastConnected, 18);

  // last error message
  app->lastErrorBox = new Fl_Multiline_Output(0, 0, 0, 0);
  app->lastErrorBox->textsize(app->nAppFontSize - 2);
  app->lastErrorBox->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE);
  app->lastErrorBox->box(FL_THIN_DOWN_BOX);
  app->lastErrorBox->wrap(1);
  app->lastErrorBox->textcolor(FL_DARK_RED);
  app->lastErrorBox->readonly(1);
  app->lastErrorBox->clear_visible_focus();

  // note - very brief item info
  app->quickNoteBox = new SVQuickNoteBox(0, 0, 0, 0);
  app->quickNoteBox->textsize(app->nAppFontSize - 1);
  app->quickNoteBox->textfont(FL_HELVETICA_ITALIC);
  app->quickNoteBox->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE);
  app->quickNoteBox->box(FL_THIN_DOWN_BOX);
  app->quickNoteBox->textcolor(fl_rgb_color(SV_QUICK_INFO_FG_COLOR));
  app->quickNoteBox->wrap(1);
  app->quickNoteBox->readonly(1);
  app->quickNoteBox->clear_visible_focus();

  app->quickInfoPack->end();

  app->flexLeftSide->fixed(app->quickInfoPack, 250);

  // set quick info text to empty defaults
  svQuickInfoSetToEmpty();
  // =============== quick info end =======================

  // ============= create host list buttons (if user wants them at bottom) =========
  if (!app->buttonsOnTop)
    createHostListButtons();

  app->flexLeftSide->end();

  app->flexParent->fixed(app->flexLeftSide, app->requestedListWidth);

  // create scrolling window we will add viewers to
  app->scroller = new Fl_Scroll(0, 0, 0, 0);
  app->vncViewer = new VncViewer(0, 0, 0, 0);
  app->scroller->type(0);
  app->scroller->end();

  // done adding child widgets
  app->flexParent->end();

  // done adding stuff to mainWin
  app->mainWin->end();

  // set resizable widget for whole app
  app->mainWin->resizable(app->flexParent);
}


/* log debug messages, if enabled in config file */
void svDebugLog (const std::string& strDebugMessage)
{
  if (app->debugMode)
    svLogToFile("DEBUG - " + strDebugMessage);
}


/* show confirmation and delete item from hostList */
void svDeleteItem (const int nItem)
{
  static bool inDeleteItem = false;

  bool okayToDelete = false;

  // prevent re-entry (FLTK menu bug)
  if (inDeleteItem)
    return;
  else
    inDeleteItem = true;

  const HostItem * itm = static_cast<HostItem *>(app->hostList->data(nItem));
  if (!itm)
  {
    fl_beep(FL_BEEP_DEFAULT);
    inDeleteItem = false;
    return;
  }

  // listening connection doesn't need delete confirmation
  if (itm->isListener)
    okayToDelete = true;
  else
  {
    // standard (non-listening) connection
    std::string strConfirm = "Are you sure you want to delete '" + itm->name + "'?";

    // set window position
    int nX = (app->mainWin->w() / 2);
    int nY = (app->mainWin->h() / 2);

    fl_message_hotspot(0);
    fl_message_position(nX, nY, 1);
    fl_message_title("SpiritVNC - Delete Item");

    if (fl_choice("%s", "Cancel", "No", "Yes", strConfirm.c_str()) == SV_CHOICE_BTN_3)
      okayToDelete = true;
  }

  // delete itm if everything is okay
  if (okayToDelete)
  {
    delete itm;
    itm = NULL;
    app->hostList->remove(nItem);
    app->hostList->redraw();
    svQuickInfoSetToEmpty();
  }

  inDeleteItem = false;
}


/*  sets all host list items to deselected  */
void svDeselectAllItems ()
{
  if (app->hostList)
    app->hostList->deselect();
}


/*  enable or disable tooltips  */
void svEnableDisableTooltips ()
{
  // enable or disable ALL app tooltips
  if (app->showTooltips)
    Fl_Tooltip::enable();
  else
    Fl_Tooltip::disable();
}


/*  find unused TCP port in a range  */
int svFindFreeTcpPort ()
{
  struct sockaddr_in structSockAddress;

  structSockAddress.sin_family = AF_INET;
  structSockAddress.sin_addr.s_addr = htonl(INADDR_ANY);

  int nSock = socket(AF_INET, SOCK_STREAM, 0);
  if (nSock < 0)
  {
    svLogToFile("ERROR - Cannot create socket for svFindFreeTCPPort");
    return 0;
  }

  // go through a range of startingLocalPort to + 99 to see if we can use for ssh forwarding
  for (uint16_t nPort = app->nStartingLocalPort; nPort < (app->nStartingLocalPort + 99); nPort ++)
  {
    structSockAddress.sin_port = htons((unsigned short)nPort);

    // don't clobber the listening port
    if (nPort == 5500)
      continue;

    // if nothing is on this port and it's not the reverse vnc port, return the port number
    if (bind(nSock, reinterpret_cast<sockaddr *>(&structSockAddress),
      sizeof(structSockAddress)) == 0)
    {
      close(nSock);
      return nPort;
    }
  }

  close(nSock);

  return 0;
}


/*  return config property from input  */
std::string svGetConfigProperty (const char * strIn)
{
  if (!strIn)
    return "";

  // assign to temp std::string
  std::string strTemp = strIn;

  // find the equals sign
  std::string::size_type pos = strTemp.find("=");

  // if found, return the part before the equals sign
  if (pos != std::string::npos)
    return strTemp.substr(0, pos);

  // not found, return empty string
  return "";
}


/*  return config value from input  */
std::string svGetConfigValue (const char * strIn)
{
  if (!strIn)
    return "";

  // assign to temp std::string
  std::string strTemp = strIn;

  // find the equals sign
  std::string::size_type pos = strTemp.find("=");

  // if found, return the part after the equals sign
  if (pos != std::string::npos)
    return strTemp.substr(pos + 1);

  // not found, return empty string
  return "";
}


/* validate vnc message loop speed */
void svValidateAndSetMessageLoopSpeed ()
{
  // correct negative value
  if (app->messageLoopSpeed < 0)
    app->messageLoopSpeed = 0;

  // correct too high a value
  if (app->messageLoopSpeed > 9)
    app->messageLoopSpeed = 9;

  // set based on speed setting
  switch (app->messageLoopSpeed)
  {
    case 0:
      app->messageLoopWaitTime = 0.100;
      break;
    case 1:
      app->messageLoopWaitTime = 0.070;
      break;
    case 2:
      app->messageLoopWaitTime = 0.055;
      break;
    case 3:
      app->messageLoopWaitTime = 0.045;
      break;
    case 4:
      app->messageLoopWaitTime = 0.040;
      break;
    case 5:
      app->messageLoopWaitTime = 0.030;
      break;
    case 6:
      app->messageLoopWaitTime = 0.025;
      break;
    case 7:
      app->messageLoopWaitTime = 0.020;
      break;
    case 8:
      app->messageLoopWaitTime = 0.010;
      break;
    case 9:
      app->messageLoopWaitTime = 0.005;
      break;
    default:
      app->messageLoopWaitTime = 0.030;
  }
}


/*
  handle app options buttons
  (void * not used so parameter name removed)
*/
void svHandleAppOptionsButtons (Fl_Widget * widget, void *)
{
  Fl_Window * childWindow = static_cast<Fl_Window *>(m_appOptions["win"]);
  const Fl_Button * btn = static_cast<Fl_Button *>(widget);

  if (!childWindow || !btn)
  {
    svCloseDeleteFinalizeChildWindow(childWindow);
    return;
  }

  // cancel button clicked
  if (btn == m_appOptions["btnCancel"])
    svCloseDeleteFinalizeChildWindow(childWindow);

  // save button clicked
  if (btn == m_appOptions["btnSave"])
  {
    bool needsRefresh = false;
    bool needsRestart = false;

    // vnc message loop speed / wait time
    app->messageLoopSpeed = static_cast<Fl_Spinner *>(m_appOptions["spinMsgLoopSpeed"])->value();

    svValidateAndSetMessageLoopSpeed();

    // scan timeout spinner
    app->nScanTimeout = static_cast<Fl_Spinner *>(m_appOptions["spinScanTimeout"])->value();

    // local ssh start port number spinner
    app->nStartingLocalPort = static_cast<Fl_Spinner *>(m_appOptions["spinLocalSSHPort"])->value();

    // ssh command input
    app->sshCommand = static_cast<SVInput *>(m_appOptions["inSSHCommand"])->value();

    // app font size input
    int appFontSize = static_cast<SVIntInput *>(m_appOptions["inAppFontSize"])->ivalue();
    if (app->nAppFontSize != appFontSize)
      needsRefresh = true;
    app->nAppFontSize = appFontSize;
    if (app->nAppFontSize < SV_APP_FONT_SIZE_MIN)
      app->nAppFontSize = SV_APP_FONT_SIZE_MIN;
    if (app->nAppFontSize > SV_APP_FONT_SIZE_MAX)
      app->nAppFontSize = SV_APP_FONT_SIZE_MAX;

    // hostlist font name input
    const char * listFontName = static_cast<SVInput *>(m_appOptions["inListFont"])->value();
    if (strcmp(app->strListFont.c_str(), listFontName) != 0)
      needsRestart = true;
    app->strListFont = listFontName;

    // hostlist font size input
    int listFontSize = static_cast<SVIntInput *>(m_appOptions["inListFontSize"])->ivalue();
    if (app->nListFontSize != listFontSize)
      needsRefresh = true;
    app->nListFontSize = listFontSize;
    if (app->nListFontSize < SV_LIST_FONT_SIZE_MIN)
      app->nListFontSize = SV_LIST_FONT_SIZE_MIN;
    if (app->nListFontSize > SV_LIST_FONT_SIZE_MAX)
      app->nListFontSize = SV_LIST_FONT_SIZE_MAX;

    // hostlist requested width input
    int listWidth = static_cast<SVIntInput *>(m_appOptions["inListWidth"])->ivalue();
    if (app->requestedListWidth != listWidth)
      needsRefresh = true;
    app->requestedListWidth = listWidth;
    if (app->requestedListWidth < 100)
      app->requestedListWidth = 100;
    svPositionWidgets();

    // user color-blind icons checkbutton
    char cbIcons = static_cast<Fl_Check_Button *>(m_appOptions["chkCBIcons"])->value();
    if (app->colorBlindIcons != cbIcons)
      needsRefresh = true;
    if (cbIcons == 1)
      app->colorBlindIcons = true;
    else
      app->colorBlindIcons = false;

    // show tooltips checkbutton
    if (static_cast<Fl_Check_Button *>(m_appOptions["chkShowTooltips"])->value() == 1)
      app->showTooltips = true;
    else
      app->showTooltips = false;

    svEnableDisableTooltips();

    // show reverse notification checkbutton
    if (static_cast<Fl_Check_Button *>(m_appOptions["chkShowReverseConnect"])->value() == 1)
      app->showReverseConnect = true;
    else
      app->showReverseConnect = false;

    // position host list buttons on top
    char buttonsOnTop = static_cast<Fl_Check_Button *>(m_appOptions["chkButtonsOnTop"])->value();
    if (app->buttonsOnTop != buttonsOnTop)
      needsRefresh = true;
    if (buttonsOnTop == 1)
      app->buttonsOnTop = true;
    else
      app->buttonsOnTop = false;

    // log app events to file
    if (static_cast<Fl_Check_Button *>(m_appOptions["chkLogToFile"])->value() == 1)
      app->enableLogToFile = true;
    else
      app->enableLogToFile = false;

    // right-click immediately closes connection
    if (static_cast<Fl_Check_Button *>(m_appOptions["chkRightClickToClose"])->value() == 1)
      app->rightClickToClose = true;
    else
      app->rightClickToClose = false;

    svCloseDeleteFinalizeChildWindow(childWindow);

    svConfigWrite();

    if (needsRestart)
      svMessageWindow("When able, please restart the app to apply font changes");

    // one or more options changed triggering a 'needs refresh'
    if (needsRefresh)
    {
      // set window position
      int nX = (app->mainWin->w() / 2);
      int nY = (app->mainWin->h() / 2);

      // refresh recommended
      std::string strConfirm = "One or more options have been changed that recommends a refresh.\n\n"
        "A refresh will close all connections and reset the app. Refresh now?";

      // confirmation box settings
      fl_message_hotspot(0);
      fl_message_position(nX, nY, 1);
      fl_message_title("SpiritVNC - Delete Item");

      // show refresh confirmation box
      if (fl_choice("%s", "Yes", "No", NULL, strConfirm.c_str()) == SV_CHOICE_BTN_2)
      {
        svMessageWindow("When able, please restart the app to apply changes");
        return;
      }

      // ***######### RESTART SEQUENCE ################***
      VncObject::endAllViewers();

      // destroy all connection itms
      for (uint16_t i = 0; i <= app->hostList->size(); i ++)
      {
        HostItem * itm = static_cast<HostItem *>(app->hostList->data(i));
        if (itm)
          delete itm;
      }

      // delete various widgets
      delete app->hostList;
      app->hostList = NULL;
      if (app->vncViewer)
        delete app->vncViewer;
      delete app->scroller;
      app->scroller = NULL;
      // list buttons
      app->btnListAdd->image(NULL);
      app->btnListDelete->image(NULL);
      app->btnListDown->image(NULL);
      app->btnListListen->image(NULL);
      app->btnListScan->image(NULL);
      app->btnListUp->image(NULL);
      app->btnListOptions->image(NULL);
      app->btnListHelp->image(NULL);
      delete app->btnListAdd;
      delete app->btnListDelete;
      delete app->btnListDown;
      delete app->btnListListen;
      delete app->btnListScan;
      delete app->btnListUp;
      delete app->btnListOptions;
      delete app->btnListHelp;
      // --
      delete app->packButtons;
      app->packButtons = NULL;
      delete app->quickNoteBox;
      app->quickNoteBox = NULL;
      delete app->quickNotePack;
      app->quickNotePack = NULL;
      delete app->quickInfoPack;
      app->quickInfoPack = NULL;

      // destroy main parent flex container along with children
      delete app->flexParent;
      app->flexParent = NULL;

      // icons
      delete app->iconDisconnected;
      delete app->iconDisconnectedError;
      delete app->iconDisconnectedBigError;
      delete app->iconConnected;
      delete app->iconNoConnect;
      delete app->iconConnecting;

      // finally, destroy the main window
      delete app->mainWin;
      app->mainWin = NULL;

      // start all over
      svDoStartupTasks();

      app->mainWin->redraw();
    }
  }
}


/*
  handle F8 buttons
  (void * not used so parameter name removed)
*/
void svHandleF8Buttons (Fl_Widget * widget, void *)
{
  Fl_Window * childWindow = static_cast<Fl_Window *>(m_f8Actions["win"]);
  const Fl_Button * btn = static_cast<Fl_Button *>(widget);

  if (!childWindow || !widget || !btn)
  {
    svCloseDeleteFinalizeChildWindow(childWindow);
    return;
  }

  const VncObject * vnc = app->vncViewer->vnc;
  if (vnc)
  {
    // ctrl + alt + delete button clicked
    if (btn == m_f8Actions["btnCAD"])
    {
      SendKeyEvent(vnc->vncClient, XK_Control_L, true);
      SendKeyEvent(vnc->vncClient, XK_Alt_L, true);
      SendKeyEvent(vnc->vncClient, XK_Delete, true);

      SendKeyEvent(vnc->vncClient, XK_Control_L, false);
      SendKeyEvent(vnc->vncClient, XK_Alt_L, false);
      SendKeyEvent(vnc->vncClient, XK_Delete, false);
    }

    // ctrl + shift + esc button clicked
    if (btn == m_f8Actions["btnCSE"])
    {
      SendKeyEvent(vnc->vncClient, XK_Control_L, true);
      SendKeyEvent(vnc->vncClient, XK_Shift_L, true);
      SendKeyEvent(vnc->vncClient, XK_Escape, true);

      SendKeyEvent(vnc->vncClient, XK_Control_L, false);
      SendKeyEvent(vnc->vncClient, XK_Shift_L, false);
      SendKeyEvent(vnc->vncClient, XK_Escape, false);
    }

    // ask server for a screen refresh
    if (btn == m_f8Actions["btnRefresh"])
      SendFramebufferUpdateRequest(vnc->vncClient, 0, 0,
        vnc->vncClient->width, vnc->vncClient->height, false);

    // send F8 key
    if (btn == m_f8Actions["btnSendF8"])
    {
      SendKeyEvent(vnc->vncClient, XK_F8, true);
      SendKeyEvent(vnc->vncClient, XK_F8, false);
    }

    // send F11 key
    if (btn == m_f8Actions["btnSendF11"])
    {
      SendKeyEvent(vnc->vncClient, XK_F11, true);
      SendKeyEvent(vnc->vncClient, XK_F11, false);
    }

    // send F12 key
    if (btn == m_f8Actions["btnSendF12"])
    {
      SendKeyEvent(vnc->vncClient, XK_F12, true);
      SendKeyEvent(vnc->vncClient, XK_F12, false);
    }
  }

  svCloseDeleteFinalizeChildWindow(childWindow);
}


/*
  handle host list button events
  (void * not used so parameter name removed)
*/
void svHandleHostListButtons (Fl_Widget * button, void *)
{
  const Fl_Button * btn = static_cast<Fl_Button *>(button);
  if (!btn)
    return;

  bool isSeparator;

  int nListVal = app->hostList->value();

  if (nListVal > 0 && strcmp(app->hostList->text(nListVal), "@C16@.· · ·") == 0)
    isSeparator = true;
  else
    isSeparator = false;

  // add new item button
  if (btn == app->btnListAdd)
    svShowConnectionEditor(NULL);

  // delete current item button
  if (btn == app->btnListDelete && !isSeparator)
  {
    if (nListVal > 0)
      svDeleteItem(nListVal);
    else
      svMessageWindow("Nothing was selected so nothing was deleted", "SpiritVNC - Delete Item");
  }

  // show app options
  if (btn == app->btnListOptions)
    svShowAppOptions();

  // show About / Help
  if (btn == app->btnListHelp)
    svShowAboutHelp();

  // move item up button
  if (btn == app->btnListUp && !isSeparator)
  {
    if (nListVal > 1)
    {
      app->hostList->swap(nListVal, nListVal - 1);
      app->hostList->select(nListVal - 1);
      app->hostList->redraw();
    }
  }

  // move item down button
  if (btn == app->btnListDown && !isSeparator)
  {
    if (nListVal < app->hostList->size())
    {
      app->hostList->swap(nListVal, nListVal + 1);
      app->hostList->select(nListVal + 1);
      app->hostList->redraw();
    }
  }

  // list item scan button
  if (btn == app->btnListScan)
  {
    // stop scan if already running
    if (app->scanIsRunning)
    {
      app->scanIsRunning = false;
      app->mainWin->label("SpiritVNC");
      app->btnListScan->image(new Fl_Pixmap(pmListScan));

      return;
    }

    // start scanning
    app->mainWin->label("SpiritVNC [Scanning]");
    app->btnListScan->image(new Fl_Pixmap(pmListScanScanning));
    app->nCurrentScanItem = app->hostList->value();
    app->scanIsRunning = true;
    svDeselectAllItems();
    svScanTimer(NULL);
  }

  // create a listening vnc object
  if (btn == app->btnListListen)
  {
    uint16_t nSize = app->hostList->size();

    // check the host list for other listening viewers
    for (uint16_t i = 0; i <= nSize; i ++)
    {
      const HostItem * itm = static_cast<HostItem *>(app->hostList->data(i));
      if (itm)
      {
        const VncObject * vnc = itm->vnc;

        if (vnc && itm->isListener)
        {
          svMessageWindow("Only one active listening viewer is allowed");
          return;
        }
      }
    }

    VncObject::createVNCListener();
  }
}


/*
  handle mouse click and other events from the host list
  (no parameters used so all parameter names removed)
*/
void svHandleHostListEvents (Fl_Widget *, void *)
{
  int event = app->hostList->when();
  int nHostItemNum = app->hostList->value();

  HostItem * itm = static_cast<HostItem *>(app->hostList->data(nHostItemNum));
  if (!itm)
  {
    // erase quick note stuff and return if there's no valid itm here
    svQuickInfoSetToEmpty();

    return;
  }

  VncObject * vnc = itm->vnc;

  static bool menuUp = false;

  // set quick note label and note text
  svQuickInfoSetLabelAndText(itm);

  // *** DO *NOT* CHECK vnc FOR NULL HERE!!! ***
  // *** IT'S OKAY IF vnc IS NULL AT THIS POINT!!! ***

  // left mouse button
  if (Fl::event_button() == FL_LEFT_MOUSE)
  {
    // double-click
    if (Fl::event_clicks() != 0 && nHostItemNum > 0)
    {
      if (app->childWindowVisible)
        return;

      // start new connection
      if (!itm->isConnected && !itm->isConnecting && !itm->isListener)
      {
        VncObject::hideMainViewer();
        app->lastErrorBox->value("");
        VncObject::createVNCObject(itm);
      }

      return;
    }

    // single-click
    if (event == FL_WHEN_RELEASE_ALWAYS || event == FL_WHEN_RELEASE)
    {
      if (app->childWindowVisible)
        return;

      VncObject::hideMainViewer();

      // show single-clicked viewer (if connected)
      if (itm->isConnected)
        vnc->setObjectVisible();

      return;
    }
  }

  // right-click
  if (Fl::event_button() == FL_RIGHT_MOUSE)
  {
    int nF12Flags = 0;
    int nViewOnlyFlag = 0;
    int nConnectDisconnectFlag = FL_MENU_INACTIVE;
    int nCustCommand1Flags = FL_MENU_INACTIVE;
    int nCustCommand2Flags = FL_MENU_INACTIVE;
    int nCustCommand3Flags = FL_MENU_INACTIVE;
    int nClipboardFlag = FL_MENU_INACTIVE;

    if (app->childWindowVisible)
      return;

    // disconnect connection if close-on-right-click is enabled
    if (
      (itm->isConnected || itm->isConnecting) &&
      !itm->hasDisconnectRequest &&
      !itm->isListener &&
      app->rightClickToClose
    )
    {
      itm->hasDisconnectRequest = true;

      vnc->endViewer();

      return;
    }

    // enable / disable 'Copy F12 macro' item in menu
    if (itm->f12Macro.empty())
      nF12Flags = FL_MENU_INACTIVE;

    // set viewonly checkbox
    if (itm->viewOnly)
      nViewOnlyFlag = FL_MENU_VALUE;

    // set custom commands sensitivity
    if (itm->customCommand1Enabled)
      nCustCommand1Flags = 0;

    if (itm->customCommand2Enabled)
      nCustCommand2Flags = 0;

    if (itm->customCommand3Enabled)
      nCustCommand3Flags = 0;

    if (!itm->clipboard.empty())
      nClipboardFlag = 0;

    // *** the menu below only displays if 'rightClickToClose' is false ***

    // show pop-up menu if not a listening connection
    if (!itm->hasDisconnectRequest
        && !itm->isListener
        && !menuUp)
    {
      // set default text as 'connecting'
      char strConnectDisconnect[20] = "Connecting...";

      // enable/disable connect/disconnect as needed
      if (itm->isConnected)
      {
        strncpy(strConnectDisconnect, "Disconnect", 19);
        nConnectDisconnectFlag = 0;
      }
      else
      {
        if (!itm->isConnecting)
        {
          strncpy(strConnectDisconnect, "Connect", 19);
          nConnectDisconnectFlag = 0;
        }
      }

      // create context menu
      // text,shortcut,callback,user_data,flags,labeltype,labelfont,labelsize
      const Fl_Menu_Item miMain[] = {
        {strConnectDisconnect,        0, 0, 0, nConnectDisconnectFlag, 0, FL_HELVETICA, app->nMenuFontSize},
        {"Edit",           0, 0, 0, 0,         0, FL_HELVETICA, app->nMenuFontSize},
        {"Get F12 macro",  0, 0, 0, nF12Flags, 0, FL_HELVETICA, app->nMenuFontSize},
        {"Delete...",      0, 0, 0, FL_MENU_DIVIDER, 0, FL_HELVETICA, app->nMenuFontSize},
        {"View only",      0, 0, 0, FL_MENU_TOGGLE | nViewOnlyFlag | FL_MENU_DIVIDER, 0, FL_HELVETICA, app->nMenuFontSize},
        {"Copy this connection's clipboard",      0, 0, 0, nClipboardFlag | FL_MENU_DIVIDER, 0, FL_HELVETICA, app->nMenuFontSize},
        {itm->customCommand1Label.c_str(), 0, 0, 0, nCustCommand1Flags, 0, FL_HELVETICA, app->nMenuFontSize},
        {itm->customCommand2Label.c_str(), 0, 0, 0, nCustCommand2Flags, 0, FL_HELVETICA, app->nMenuFontSize},
        {itm->customCommand3Label.c_str(), 0, 0, 0, nCustCommand3Flags, 0, FL_HELVETICA, app->nMenuFontSize},
        {0}
      };

      // prevent re-entry (FLTK menu bug)
      menuUp = true;

      // show context menu and return selected item, if any
      const Fl_Menu_Item * miRes = miMain->popup(Fl::event_x() + 14, Fl::event_y() - 10);
      if (miRes)
      {
        const char * strRes = miRes->text;
        if (strRes)
        {
          // connect
          if (strcmp(strRes, "Connect") == 0)
          {
            app->lastErrorBox->value("");

            VncObject::createVNCObject(itm);
          }

          // disconnect
          if (strcmp(strRes, "Disconnect") == 0)
          {
            itm->hasDisconnectRequest = true;

            vnc->endViewer();
          }

          // edit itm
          if (strcmp(strRes, "Edit") == 0)
            svShowConnectionEditor(itm);

          // store this item's F12 macro to app f12 macro variable
          if (strcmp(strRes, "Get F12 macro") == 0)
          {
            app->strF12ClipVar = itm->f12Macro;
          }

          // delete item (and itm)
          if (strcmp(strRes, "Delete...") == 0)
            svDeleteItem(nHostItemNum);

          // view only
          if (strcmp(strRes, "View only") == 0)
            itm->viewOnly = !itm->viewOnly;

          // copy itm's clipboard to ours
          if (strcmp(strRes, "Copy this connection's clipboard") == 0)
          {
            Fl::copy(itm->clipboard.c_str(), itm->clipboard.size(), 1);
            itm->clipboard.clear();
          }

          // custom commands
          // command 1
          if (strcmp(strRes, itm->customCommand1Label.c_str()) == 0)
            svRunCommand(itm->customCommand1Label, itm->customCommand1);

          // command 2
          if (strcmp(strRes, itm->customCommand2Label.c_str()) == 0)
            svRunCommand(itm->customCommand2Label, itm->customCommand2);

          // command 1
          if (strcmp(strRes, itm->customCommand3Label.c_str()) == 0)
            svRunCommand(itm->customCommand3Label, itm->customCommand3);
        }
      }

      menuUp = false;
    }

    // show pop-up menu for reverse / listening connections
    if (itm->isListener && !menuUp)
    {
      // prevent re-entry (FLTK menu bug)
      menuUp = true;

      // listener is not connected
      if (!itm->isConnected && !itm->isWaitingForShow)
      {
        // create context menu
        // text,shortcut,callback,user_data,flags,labeltype,labelfont,labelsize
        const Fl_Menu_Item miM[] = {
          {"Delete", 0, 0, 0, 0, 0, FL_HELVETICA, app->nMenuFontSize},
          {0}
        };

        // show context menu and return selected item, if any
        const Fl_Menu_Item * miRes = miM->popup(Fl::event_x() + 14, Fl::event_y() - 10);
        if (miRes)
        {
          const char * strRes = miRes->text;

          // delete item (and itm)
          if (strRes && strcmp(strRes, "Delete") == 0)
            svDeleteItem(nHostItemNum);
        }

        menuUp = false;
        return;
      }
      else
      {
        // listener is connected

        // enable / disable 'Paste F12 macro' item in menu
        if (app->strF12ClipVar.empty())
          nF12Flags = FL_MENU_INACTIVE;
        else
          nF12Flags = 0;

        int nF12Flags2 = 0;

        // enable / disable 'Clear F12 macro' item in menu
        if (itm->f12Macro.empty())
          nF12Flags2 = FL_MENU_INACTIVE;

        // create context menu
        // text,shortcut,callback,user_data,flags,labeltype,labelfont,labelsize
        const Fl_Menu_Item miMain[] = {
          {"Disconnect",      0, 0, 0, 0,          0, FL_HELVETICA, app->nMenuFontSize},
          {"Edit",            0, 0, 0, 0,          0, FL_HELVETICA, app->nMenuFontSize},
          {"Put F12 macro", 0, 0, 0, nF12Flags,  0, FL_HELVETICA, app->nMenuFontSize},
          {"Clear F12 macro", 0, 0, 0, nF12Flags2, 0, FL_HELVETICA, app->nMenuFontSize},
          {0}
        };

        menuUp = true;

        // show context menu and return selected item, if any
        const Fl_Menu_Item * miRes = miMain->popup(Fl::event_x() + 14, Fl::event_y() - 10);
        if (miRes)
        {
          const char * strRes = miRes->text;
          if (strRes)
          {
            // disconnect
            if (strcmp(strRes, "Disconnect") == 0)
            {
              int iIndx = svItemNumFromItm(itm);

              vnc->endViewer();
              svDeleteItem(iIndx);

              menuUp = false;

              return;
            }

            // edit itm
            if (strcmp(strRes, "Edit") == 0)
              svShowConnectionEditor(itm);

            // put app-private F12 macro variable into listening item
            if (strcmp(strRes, "Put F12 macro") == 0)
            {
              itm->f12Macro = app->strF12ClipVar;
              app->strF12ClipVar = "";
            }

            // clear F12 macro variable of listening item
            if (strcmp(strRes, "Clear F12 macro") == 0)
              itm->f12Macro = "";
          }
        }

        menuUp = false;
        return;
      }
    }
  }
}


/*
  handle itm option window button presses
  (void * not used so parameter name removed)
*/
void svHandleConnEditButtons (Fl_Widget * widget, void *)
{
  Fl_Window * childWindow = static_cast<Fl_Window *>(m_itmSettings["win"]);
  HostItem * itm = static_cast<HostItem *>(m_itmSettings["itm"]);
  const Fl_Button * btn = static_cast<Fl_Button *>(widget);

  if (!itm || !childWindow || !btn)
  {
    svCloseDeleteFinalizeChildWindow(NULL);
    return;
  }

  // cancel button clicked
  if (btn == static_cast<Fl_Button *>(m_itmSettings["btnCancel"]))
    svCloseDeleteFinalizeChildWindow(childWindow);

  // delete button clicked
  if (btn == static_cast<Fl_Button *>(m_itmSettings["btnDel"]))
  {
    int nItem = svItemNumFromItm(itm);

    if (nItem > 0)
    {
      svDeleteItem(nItem);
      svCloseDeleteFinalizeChildWindow(childWindow);
    }

    return;
  }

  // save button clicked
  if (btn == static_cast<Fl_Button *>(m_itmSettings["btnSave"]))
  {
    // #### vnc tab ########################################

    // connection name text input
    itm->name = static_cast<SVInput *>(m_itmSettings["inName"])->value();

    // connection group text input
    itm->group = static_cast<SVInput *>(m_itmSettings["inGroup"])->value();

    // connection address text input
    itm->hostAddress = static_cast<SVInput *>(m_itmSettings["inAddress"])->value();

    // f12 macro text input
    itm->f12Macro = static_cast<SVInput *>(m_itmSettings["inF12Macro"])->value();

    // vnc connection radio button
    if (static_cast<Fl_Radio_Round_Button *>(m_itmSettings["rbVNC"])->value() == 1)
      itm->hostType = 'v';

    // vnc-through-ssh radio button
    if (static_cast<Fl_Radio_Round_Button *>(m_itmSettings["rbSVNC"])->value() == 1)
      itm->hostType = 's';

    // vnc port text input
    itm->vncPort = static_cast<SVIntInput *>(m_itmSettings["inVNCPort"])->value();

    // vnc password secret input
    const char * vncPasswordIn = static_cast<SVSecretInput *>(m_itmSettings["inVNCPassword"])->value();
    itm->vncPassword = base64Encode(reinterpret_cast<const unsigned char *>(vncPasswordIn), strlen(vncPasswordIn));

    // vnc login name input
    itm->vncLoginUser = static_cast<SVInput *>(m_itmSettings["inVNCLoginUser"])->value();

    // vnc login password secret input
    const char * vncLoginPassword = static_cast<SVSecretInput *>(m_itmSettings["inVNCLoginPassword"])->value();
    itm->vncLoginPassword = base64Encode(reinterpret_cast<const unsigned char *>(vncLoginPassword),
      strlen(vncLoginPassword));

    // vnc compression level text input
    itm->compressLevel = atoi(static_cast<SVIntInput *>(m_itmSettings["inVNCCompressLevel"])->value());

    if (itm->compressLevel > 9)
      itm->compressLevel = 9;

    // vnc quality level text input
    itm->qualityLevel = atoi(static_cast<SVIntInput *>(m_itmSettings["inVNCQualityLevel"])->value());

    if (itm->qualityLevel > 9)
      itm->qualityLevel = 9;

    // scroll only / no scaling radio button
    if (static_cast<Fl_Radio_Round_Button *>(m_itmSettings["rbScaleOff"])->value() == 1)
      itm->scaling = 's';

    // zoom radio button
    if (static_cast<Fl_Radio_Round_Button *>(m_itmSettings["rbScaleZoom"])->value() == 1)
      itm->scaling = 'z';

    // fit radio button
    if (static_cast<Fl_Radio_Round_Button *>(m_itmSettings["rbScaleFit"])->value() == 1)
      itm->scaling = 'f';

    // fast (jaggy) scaling checkbutton
    if (static_cast<Fl_Check_Button *>(m_itmSettings["chkScalingFast"])->value() == 1)
      itm->scalingFast = true;
    else
      itm->scalingFast = false;

    // show remote cursor checkbutton
    if (static_cast<Fl_Check_Button *>(m_itmSettings["chkShowRemoteCursor"])->value() == 1)
      itm->showRemoteCursor = true;
    else
      itm->showRemoteCursor = false;

    // #### ssh tab ###########################################

    // ssh username
    itm->sshUser = static_cast<SVInput *>(m_itmSettings["inSSHName"])->value();

    // ssh port
    itm->sshPort = static_cast<SVIntInput *>(m_itmSettings["inSSHPort"])->value();

    // ssh private key
    itm->sshKeyPrivate = static_cast<SVInput *>(m_itmSettings["inSSHPrvKey"])->value();

    // #### custom commands ####################################
    // custom command 1
    if (static_cast<Fl_Check_Button *>(m_itmSettings["chkCommand1Enabled"])->value() == 1)
      itm->customCommand1Enabled = true;
    else
      itm->customCommand1Enabled = false;

    itm->customCommand1Label = static_cast<SVInput *>(m_itmSettings["inCommand1Label"])->value();
    itm->customCommand1 = static_cast<SVInput *>(m_itmSettings["inCommand1"])->value();

    // custom command 2
    if (static_cast<Fl_Check_Button *>(m_itmSettings["chkCommand2Enabled"])->value() == 1)
      itm->customCommand2Enabled = true;
    else
      itm->customCommand2Enabled = false;

    itm->customCommand2Label = static_cast<SVInput *>(m_itmSettings["inCommand2Label"])->value();
    itm->customCommand2 = static_cast<SVInput *>(m_itmSettings["inCommand2"])->value();

    // custom command 3
    if (static_cast<Fl_Check_Button *>(m_itmSettings["chkCommand3Enabled"])->value() == 1)
      itm->customCommand3Enabled = true;
    else
      itm->customCommand3Enabled = false;

    itm->customCommand3Label = static_cast<SVInput *>(m_itmSettings["inCommand3Label"])->value();
    itm->customCommand3 = static_cast<SVInput *>(m_itmSettings["inCommand3"])->value();

    // add item to host list if new
    if (svItemNumFromItm(itm) == 0 && app->hostList->size() < SV_MAX_HOSTLIST_ENTRIES)
    {
      // insert near selected item, otherwise at bottom/end
      if (app->hostList->size() > 0 && app->hostList->value() > 0)
      {
        app->hostList->insert(app->hostList->value() + 1, itm->name.c_str(), itm);
        app->hostList->icon(app->hostList->value() + 1, app->iconDisconnected);
        app->hostList->make_visible(app->hostList->value() + 1);
      }
      else
      {
        app->hostList->add(itm->name.c_str(), itm);
        app->hostList->icon(app->hostList->size(), app->iconDisconnected);
        app->hostList->make_visible(app->hostList->size());
      }

      Fl::check();
    }

    // clean up
    svUpdateHostListItemText();

    svCloseDeleteFinalizeChildWindow(childWindow);

    // refresh any visual changes if connected
    if (itm->isConnected && itm->vnc)
    {
      SetFormatAndEncodings(itm->vnc->vncClient);
      itm->vnc->setObjectVisible();
    }

    svConfigWrite();
  }
}


/*
  sends new clipboard text to the currently displayed vnc host
  (void * not used so parameter name removed)
*/
void svHandleLocalClipboard (const int source, void *)
{
  // The selection buffer (source is 0) is used for middle-mouse pastes and
  // for drag-and-drop selections. The clipboard (source is 1) is used for
  // copy/cut/paste operations.

  // don't process clipboard if there's no remote server being displayed
  // of it's the selection buffer
  if (!app->vncViewer->vnc || source != 1)
    return;

  Fl::paste(*app->vncViewer, 1);
}


/*
  handle main window close event
  (void * not used so parameter name removed)
*/
void svHandleMainWindowEvents (Fl_Widget * window, void *)
{
  int event = Fl::event();

  // don't close window with Esc key
  if (event == FL_SHORTCUT && Fl::event_key() == FL_Escape)
    return;

  // window closing
  if (event == FL_CLOSE)
  {
    app->shuttingDown = true;

    VncObject::endAllViewers();

    svLogToFile("--- Program shutting down ---");

    // save main window position and size
    app->savedX = app->mainWin->x();
    app->savedY = app->mainWin->y();
    app->savedW = app->mainWin->w();
    app->savedH = app->mainWin->h();

    // check if we're maximized
    app->maximized = app->mainWin->maximize_active();

    svConfigWrite();

    // finish up any queued events
    Fl::check();

    // we gone
    exit(0);
  }
}


/*  popup edit menu in input widgets and handle choice  */
void svPopUpEditMenu (Fl_Input_ * input)
{
  static bool inMenu = false;

  // prevent re-entry
  if (inMenu)
    return;

  inMenu = true;

  // create context menu
  // text,shortcut,callback,user_data,flags,labeltype,labelfont,labelsize
  const Fl_Menu_Item miMain[] = {
    {"Undo",  0, 0, 0, FL_MENU_DIVIDER, 0, FL_HELVETICA, app->nMenuFontSize},
    {"Cut",   0, 0, 0, 0,               0, FL_HELVETICA, app->nMenuFontSize},
    {"Copy",  0, 0, 0, 0,               0, FL_HELVETICA, app->nMenuFontSize},
    {"Paste", 0, 0, 0, 0,               0, FL_HELVETICA, app->nMenuFontSize},
    {0}
  };

  // show context menu and return selected item, if any
  const Fl_Menu_Item * miRes = miMain->popup(Fl::event_x() + 14, Fl::event_y() - 10);
  // process response, if any
  if (miRes)
  {
    const char * strRes = miRes->text;
    // do edit action based on menu item label text, if any
    if (strRes)
    {
      std::string strR = strRes;

      if (strR == "Undo")
        input->undo();
      else if (strR == "Cut")
      {
        input->copy(1);
        input->cut();
      }
      else if (strR == "Copy")
        input->copy(1);
      else if (strR == "Paste")
        Fl::paste(*input, 1);
    }
  }

  inMenu = false;
}


/*  set quick note text to current item  */
void svQuickInfoSetLabelAndText (HostItem * itm)
{
  // set itm's quick info label text, if any
  app->quickInfoLabel->copy_label(itm->name.c_str());

  // set last connected text, if any
  if (!itm->lastConnectedTime.empty())
  {
    if (!itm->isListener)
      app->lastConnectedLabel->copy_label("Last connected");
    else
      app->lastConnectedLabel->copy_label("Connected");

    app->lastConnected->copy_label(itm->lastConnectedTime.c_str());
  }
  else
  {
    app->lastConnectedLabel->copy_label("");
    app->lastConnected->copy_label("");
  }

  // set last error text, if any
  app->lastErrorBox->value(itm->lastErrorMessage.c_str());

  // set appropriate text style and quick note text
  if (itm->quickNote.empty())
  {
    app->quickNoteBox->textfont(FL_HELVETICA_ITALIC);

    // listening connections don't save any data, so
    // best to call the item 'Temporary Note'
    if (itm->isListener)
      app->quickNoteBox->value("(no Temporary Note)");
    else
      app->quickNoteBox->value("(no Quick Note)");
  }
  else
  {
    app->quickNoteBox->textfont(FL_HELVETICA);
    app->quickNoteBox->value(itm->quickNote.c_str());
  }
}


/*  set quick note to empty / no item  */
void svQuickInfoSetToEmpty ()
{
  // blank itm's quick info text
  app->quickInfoLabel->copy_label("-");
  app->lastConnectedLabel->copy_label("");
  app->lastConnected->copy_label("");
  app->lastErrorBox->value("");
  app->quickNoteBox->textfont(FL_HELVETICA_ITALIC);
  app->quickNoteBox->value("-");
}


/*
  handle app and main window events, such as resize, move, etc
  and resize gui elements
*/
void svPositionWidgets ()
{
  svDebugLog("svPositionWidgets - Resizing GUI elements");

  if (app->vncViewer)
  {
    VncObject * vnc = app->vncViewer->vnc;
    // reset scroller position
    if (vnc)
    {
      svResizeScroller();
      vnc->setObjectVisible();
    }
  }

  Fl::redraw();
  Fl::check();
}


/*
  handle host item icon change
  (void * not used so parameter name removed)
*/
void svHandleListItemIconChange (void *)
{
  uint16_t nSize = app->hostList->size();

  // iterate through host list and set status icons for items
  for (uint16_t i = 0; i < nSize; i++)
  {
    const HostItem * itm = static_cast<HostItem *>(app->hostList->data(i));

    if (itm && itm->icon)
    {
      app->hostList->icon(i, itm->icon);

      Fl::check();
    }
  }

  app->hostList->redraw();
}


/*  handle connection changes from child threads  */
void svHandleThreadConnection (void * data)
{
  HostItem * itm = static_cast<HostItem *>(data);
  if (!itm)
    return;

  VncObject * vnc = itm->vnc;
  if (!vnc)
    return;

  int nItem = svItemNumFromItm(itm);

  // set viewer as connected
  if (itm->isWaitingForShow)
  {
    svDebugLog("svConnectionWatcher - itm changing from 'isWaitingToShow' to 'isConnected'");

    itm->isWaitingForShow = false;

    app->nViewersWaiting --;

    // set host list item status icon
    itm->icon = app->iconConnected;
    svHandleListItemIconChange(NULL);

    // store connection time
    itm->lastConnectedTime = svMakeTimeStamp(false);

    // only update the quick info if we're on this host item
    if (app->hostList->value() == svItemNumFromItm(itm))
      svQuickInfoSetLabelAndText(itm);

    svLogToFile("Connected to '" + itm->name + "' - " + itm->hostAddress);

    // show viewer if it matches the selected host list item
    uint16_t nSelectedHost = app->hostList->value();

    if (nItem == nSelectedHost && !itm->isListener)
    {
      svDebugLog("svConnectionWatcher - Showing viewer because it's selected");
      vnc->setObjectVisible();
    }

    // append desktop name to this listener and create another listening viewer
    if (itm->isListener)
    {
      // add remote desktop's name to this Listening item
      int nListeningItem = svItemNumFromItm(itm);

      std::string strHostViewName = "Listening - ";
      strHostViewName.append(vnc->vncClient->desktopName);
      itm->name = strHostViewName;

      app->hostList->text(nListeningItem, itm->name.c_str());

      // (try to) create another listener
      svDebugLog("svConnectionWatcher - Creating Listener object");

      VncObject::createVNCListener();

      if (app->showReverseConnect)
        svMessageWindow("A remote VNC host has just reverse-connected"
          "\n\nClick the 'Listen' item(s) in the host list to view");
    }
  }

  // set no connect icon
  if (itm->hasCouldntConnect)
  {
    svDebugLog("svConnectionWatcher - itm changing from 'hasCouldntConnect' to"
      " 'isConnected = false'");

    itm->isConnected = false;
    app->nViewersWaiting --;

    // set host list item status icon
    if (!itm->lastErrorMessage.empty())
    {
      itm->icon = app->iconDisconnectedBigError;

      // only update the lastError quick info if we're on this host item
      if (app->hostList->value() == svItemNumFromItm(itm))
        app->lastErrorBox->value(itm->lastErrorMessage.c_str());
    }
    else
      itm->icon = app->iconNoConnect;

    svHandleListItemIconChange(NULL);

    // deal with listening items
    if (itm->isListener)
    {
      // output to console and attempt to log error
      std::string strLErr = svMakeTimeStamp() + " - SpiritVNC-FLTK - Error: Incoming reverse VNC connection failed.  "
        "Attempting to recover";
      svLogToFile(strLErr);

      // log to stdout if we're not using LogToFile
      if (!app->enableLogToFile)
        std::cout << strLErr << std::endl;

      // remove item from host list
      app->hostList->remove(svItemNumFromItm(itm));

      // try to create another listener
      VncObject::createVNCListener();
    }

    // set cleanup flag so svConnectionWatcher will do the thing
    itm->vncNeedsCleanup = true;
  }
}


/*
  handle thread cursor change
  (void * not used so parameter name removed)
*/
void svHandleThreadCursorChange (void * data)
{
  bool setDefault = static_cast<bool>(data);

  // if we're just resetting the cursor and nothing else
  if (setDefault)
  {
    app->mainWin->cursor(FL_CURSOR_DEFAULT);
    return;
  }

  if (!app->vncViewer)
    return;

  const VncObject * vnc = app->vncViewer->vnc;
  if (!vnc)
    return;

  // set cursor, if valid
  if (vnc->imgCursor)
  {
    app->mainWin->cursor(vnc->imgCursor, vnc->nCursorXHot, vnc->nCursorYHot);
    Fl::wait();
  }
  else
    app->mainWin->cursor(FL_CURSOR_DEFAULT);
}


/*  create and insert empty listitem if no items were added at startup  */
void svInsertEmptyItem ()
{
  // make and populate a new itm
  HostItem * itm = new HostItem();
  itm->name = "(new connection)";
  itm->hostAddress = "0.0.0.0";
  itm->vncPort = "5900";
  itm->scaling = 'f';
  itm->sshKeyPrivate = "";
  itm->showRemoteCursor = true;
  itm->compressLevel = 5;
  itm->qualityLevel = 5;

  // add empty item to hostlist, set its icon and make it visible
  app->hostList->add(itm->name.c_str(), itm);
  app->hostList->icon(app->hostList->size(), app->iconDisconnected);
  app->hostList->make_visible(app->hostList->size());

  Fl::redraw();
  Fl::check();
}


/*  return hostlist item (integer) that owns host item 'itm'  */
int svItemNumFromItm (const HostItem * itmIn)
{
  if (!itmIn)
    return 0;

  uint16_t nSize = app->hostList->size();

  // go through hostlist and find item owning matching itmIn
  for (uint16_t i = 0; i <= nSize; i ++)
  {
    const HostItem * itm = static_cast<HostItem *>(app->hostList->data(i));
    if (itm && itm == itmIn)
      return i;
  }

  return 0;
}


/* used when 'clearing' a button's callback */
void svNoOpCallback (Fl_Widget *, void *)
{
  // as I lay me down I love tacos!
}


/*
  handle itm option window choose prvkey button presses
  (Fl_Widget * not used so parameter name removed)
*/
void svHandleConnEditChoosePrvKeyBtn (Fl_Widget *, void * data)
{
  SVInput * inPrvKey = static_cast<SVInput *>(data);
  if (!inPrvKey)
    return;

  // set default home path string
  char strHome[FL_PATH_MAX] = {0};
  fl_filename_expand(strHome, sizeof(strHome), "$HOME");

  // create native file chooser
  Fl_Native_File_Chooser fileChooser;
  fileChooser.title("SpiritVNC - Please choose a private key file...");
  fileChooser.type(Fl_Native_File_Chooser::BROWSE_FILE);

  // set default directory
  fileChooser.directory(strHome);

  // show native chooser
  int result = fileChooser.show();

  // get out if we didn't choose anything or canceled
  if (result == -1 || result == 1)
    return;

  const char * chosenFile = fileChooser.filename();

  // get out if null or empty chosen file
  if (!chosenFile || chosenFile[0] == '\0')
    return;

  // set the ssh private key input's text
  inPrvKey->value(chosenFile);
}


/*
  handle itm option window vnc radio button presses
  (void * not used so parameter name removed)
*/
void svItmOptionsRadioButtonsCallback (Fl_Widget * button, void *)
{
  Fl_Radio_Round_Button * btn = static_cast<Fl_Radio_Round_Button *>(button);

  if (!btn)
    return;

  if (btn->value() == 1)
    btn->setonly();
}


/*  send text to log file  */
void svLogToFile (const std::string& strMessage)
{
  std::ofstream ofs;
  char logFileName[FL_PATH_MAX] = {0};
  std::string strLineEnd;

  if (!app->enableLogToFile)
    return;

  if (strMessage.empty())
    return;

  snprintf(logFileName, FL_PATH_MAX, "%s/spiritvnc-fltk.log", app->configPath.c_str());

  ofs.open(logFileName, std::ofstream::out | std::ofstream::app);

  // oops, can't open log file
  if (ofs.fail())
  {
    std::cout << "SpiritVNC ERROR - Could not open log file for writing" <<
      std::endl << std::flush;
    return;
  }

  // add newline, if necessary
  if (strMessage.find('\n') == std::string::npos)
    strLineEnd = "\n";

  // write time-stamp to log
  ofs << svMakeTimeStamp() << "- " << strMessage << strLineEnd;

  ofs.close();
}


/*  return a std::string containing date & time string  */
std::string svMakeTimeStamp (bool dashSeps)
{
  char timeBuf[50] = {0};
  time_t myClock = 0;
  char * strFmt = NULL;

  // build time-stamp
  time(&myClock);

  // dashes optional, ugly in some uses
  if (dashSeps)
    strFmt = strdup("%Y-%m-%d--%H:%M:%S");
  else
    strFmt = strdup("%Y-%m-%d  %H:%M:%S");

  if (strFmt)
    strftime(timeBuf, 50, strFmt, localtime(&myClock));

  free(strFmt);

  return std::string(timeBuf);
}


/*  display a message dialog window  */
void svMessageWindow (const std::string& strMessage, const std::string& strTitle)
{
  // set window position
  int nX = (app->mainWin->w() / 2);
  int nY = (app->mainWin->h() / 2);

  // show message
  Fl::lock();
  fl_message_hotspot(0);
  fl_message_title(strTitle.c_str());
  fl_message_position(nX, nY, 1);
  fl_message("%s", strMessage.c_str());
  Fl::unlock();
}


/*  return number of connected items (integer)  */
bool svThereAreConnectedItems ()
{
  int nSize = app->hostList->size();

  // go through each itm in the host list and
  // return true on the first connected one
  for (uint16_t i = 1; i <= nSize; i ++)
  {
    const HostItem * itm = static_cast<HostItem *>(app->hostList->data(i));
    if (itm && itm->isConnected)
        return true;
  }

  return false;
}


/*  make sure scroller is sized correctly  */
void svResizeScroller ()
{
  int neededWidth = app->mainWin->w() - app->flexLeftSide->w();

  app->scroller->resize(app->flexLeftSide->w(), 0, neededWidth, app->mainWin->h());
}


/*
  restore previous session's window position
  (void * not used so parameter name removed)
*/
void svRestoreWindowSizePosition (void *)
{
  app->mainWin->resize(app->savedX, app->savedY, app->savedW, app->savedH);
  app->mainWin->redraw();
}


/*
  actual command runner
*/
void svRunCommandHelper(const char ** strArgs)
{
  std::string strOops = "Command or command label is NULL";

  // get out if pointer is null
  if (!strArgs)
  {
    svMessageWindow(strOops, "SpiritVNC - Custom command");
    return;
  }

  // if either of the array elements are null, get out
  if (!strArgs[0] || !strArgs[1])
  {
    svMessageWindow(strOops, "SpiritVNC - Custom command");
    return;
  }

  // run the passed command
  int nRes = std::system(strArgs[1]);

  // warn if the command returned a non-zero result
  if (nRes != 0)
  {
    char errMsg[1024] = {0};
    snprintf(errMsg, 1023, "Command failed: '%s'\n\nExit code: %i\n", strArgs[0], nRes);
    svMessageWindow(errMsg, "SpiritVNC - Custom command");
  }

  return;
}


/*
  runs custom command
  (calls svRunCommandHelper as thread)
*/
void svRunCommand(const std::string& label, const std::string& cmd)
{
  // get out if label or command is empty
  if (label.empty() || cmd.empty())
  {
    svMessageWindow("Command or command label is empty", "SpiritVNC - Custom command");
    return;
  }

  // create char array pointer to pass to thread
  const char * strArgs[] = {label.c_str(), cmd.c_str()};

  // launch svRunCommandHelper
  svRunCommandHelper(strArgs);
}


/*
  scan the host list for active connections and pause on each one
  for user-determined time interval
  (void * not used so parameter name removed)
*/
void svScanTimer (void *)
{
  // remove any previously added timer
  Fl::remove_timeout(svScanTimer);

  if (!app->scanIsRunning || !svThereAreConnectedItems())
  {
    app->scanIsRunning = false;
    app->nCurrentScanItem = 0;
    app->mainWin->label("SpiritVNC");
    app->btnListScan->image(new Fl_Pixmap(pmListScan));
    app->btnListScan->redraw();

    if (!svThereAreConnectedItems())
    {
      svQuickInfoSetToEmpty();
      svMessageWindow("No connected viewers. Scanning is now stopped", "SpiritVNC - Scan mode");
    }

    return;
  }

  while (app->scanIsRunning)
  {
    app->nCurrentScanItem ++;

    // start over from the top
    if (app->nCurrentScanItem > app->hostList->size())
    {
      app->nCurrentScanItem = 0;
      svDeselectAllItems();
      svScanTimer(NULL);

      return;
    }

    // hunt for next connected viewer
    HostItem * itm = static_cast<HostItem *>(app->hostList->data(app->nCurrentScanItem));
    if (!itm)
      continue;

    if (itm->isConnected)
    {
      svDeselectAllItems();
      VncObject::hideMainViewer();
      app->hostList->select(app->nCurrentScanItem);
      itm->vnc->setObjectVisible();

      // set quick note label and note text
      svQuickInfoSetLabelAndText(itm);

      // 'tickle' host screen so it doesn't go to screensaver by
      // moving remote mouse back and forth a certain amount
      // (don't do this for view-only connections)
      if (!itm->viewOnly)
      {
        SendPointerEvent(itm->vnc->vncClient, 0, 0, 0);
        Fl::check();
        SendPointerEvent(itm->vnc->vncClient, 100, 100, 0);
        Fl::check();
        SendPointerEvent(itm->vnc->vncClient, 0, 0, 0);
        Fl::check();
      }
      break;
    }
  }

  // call me again
  Fl::add_timeout(app->nScanTimeout, svScanTimer);
}


/* send a stored text string to the vnc host */
void svSendKeyStrokesToHost (const std::string& strIn, const VncObject * vnc)
{
  if (!vnc)
    return;

  size_t stringSize = strIn.size();

  // iterate through string and send each character
  for (uintmax_t i = 0; i < stringSize; i ++)
  {
    // get out if we hit a null character
    if (strIn[i] == '\0')
      return;

    // send everything except newlines
    if (strIn[i] != '\n')
    {
      SendKeyEvent(vnc->vncClient, strIn[i], true);
      SendKeyEvent(vnc->vncClient, strIn[i], false);
    }
  }
}


/* sets and enables / disables tooltips */
void svSetAppTooltips ()
{
  // set app tooltips
  // (item and app options tooltips are set in their svShow.. methods)
  app->btnListAdd->tooltip("Add a new connection");
  app->btnListDelete->tooltip("Delete current connection");
  app->btnListUp->tooltip("Move current connection item up in list");
  app->btnListDown->tooltip("Move current connection item down in list");
  app->btnListScan->tooltip("Scan through connected items in list");
  app->btnListListen->tooltip("Listen for incoming VNC connections");
  app->btnListHelp->tooltip("View About and Help information");
  app->btnListOptions->tooltip("View / edit app options");
  app->hostList->tooltip("Double-click a disconnected item to connect to it\n\n"
    "Right-click a connected item to disconnect from it\n\n"
    "Right-click a disconnected item to connect, edit or delete it");
  app->quickInfoLabel->tooltip("The current item's name");
  app->lastConnected->tooltip("The last time this connection was successfully made");
  app->lastErrorBox->tooltip("The last error when trying to connect to the current item");
  app->quickNoteBox->tooltip("Click here to enter a brief note about the current item");

  // check and set tooltip visibility
  svEnableDisableTooltips();
}


/*  show About / Help info  */
void svShowAboutHelp ()
{
  // check to make sure no other child window is visible
  if (app->childWindowVisible)
    return;

  // set flag so no other child window will show
  app->childWindowVisible = true;

  // window size
  int nWinWidth = 820;
  int nWinHeight = 655;

  // set window position
  int nX = (app->mainWin->w() / 2) - (nWinWidth / 2);
  int nY = (app->mainWin->h() / 2) - (nWinHeight / 2);

  // create messagebox window
  Fl_Window * win = new Fl_Window(nX, nY, nWinWidth, nWinHeight, "About SpiritVNC-FLTK / Help");
  win->set_modal();
  win->callback(svCloseChildWindow, win);

  Fl_Help_View * hv = new Fl_Help_View(10, 10, nWinWidth - 20, nWinHeight - 70);

  if (!hv)
    return;

  hv->color(FL_BACKGROUND2_COLOR);
  hv->textsize(app->nAppFontSize);
  hv->textfont(0);

  char flVersion[20] = {};

  snprintf(flVersion, 19, "%i.%i.%i", FL_MAJOR_VERSION, FL_MINOR_VERSION, FL_PATCH_VERSION);

  std::string strHelp = "<center><font face='sans'><h5>SpiritVNC - FLTK</h5></font></center>"
      "<p><center>"
      "&copy; 2016-" SV_CURRENT_YEAR " Will Brokenbourgh - <a href='https://www.willbrokenbourgh.com/brainout/'>"
      "https://www.willbrokenbourgh.com/brainout/</a>"
      "</center></p>"
      // -
      "<p><center>"
      "Version " SV_APP_VERSION "&nbsp;&bull;&nbsp;"
      "Built " __DATE__ "&nbsp;&bull;&nbsp;" __TIME__ "<br>"
      "FLTK version " + std::string(flVersion) +
      "</center></p>"
      // -
      "<p><center>"
      "SpiritVNC is a multi-viewer VNC client for *nix systems"
      " based on FLTK.  SpiritVNC features VNC-through-SSH, reverse VNC (listening)"
      " connections and automatic timed scanning of each connected viewer."
      "</center></p>"
      // ---
      "<p><center><strong>Instructions</strong></center><br>"
      // -
      "<ul>"
      "<li>To add a new connection, click the [+] button</li>"
      "<li>To delete a connection, single-click it then click the [-] button</li>"
      "<li>To connect a connection, double-click it</li>"
      "<li>To disconnect a normal connection, right-click it while connected</li>"
      "<li>To Connect, Edit, Delete, or 'copy' the F12 macro of a connection, right-click the connection when"
      " not connected</li>"
      "<li>To scan automatically through all connected viewers, click the 'clock' button</li>"
      "<li>To 'listen' for an incoming VNC connection, click the 'ear' button</li>"
      "<li>To hide the host list, action buttons and quick note when connected ('focus mode'), press F11</li>"
      "<li>To disconnect, edit or 'paste' a F12 macro to a 'listening' connection, right-click it while connected</li>"
      "<li>To change application options, click the settings button (looks like three control sliders)</li>"
      "<li>To perform remote actions, press F8 when a remote host is being displayed</li>"
      "<li>See the README.md file for more detailed instructions</li>"
      "</ul>"
      // ---
      "<p><center>"
      "Many thanks to the <em>FLTK</em> and <em>libvncserver</em> projects for their libraries "
      "and example code."
      "</center></p>"
      // -
      "<p><center>"
      "&nbsp;If you have any questions, need to report a bug or have suggestions, please create an issue on "
      "github or "
      " <a href='https://www.willbrokenbourgh.com/brainout/content/spiritvnc.php'>visit the SpiritVNC page</a>"
      "&nbsp;"
      "</center></p>"
      // -
      "<p><center><strong>To God be the glory! :-D</strong></center></p>";

  // set helpview's text
  hv->value(strHelp.c_str());

  // 'OK' button
  Fl_Return_Button * btnOK = new Fl_Return_Button((nWinWidth - 100 - 10), nWinHeight - (35 + 10), 100, 35, "OK");
  btnOK->box(FL_GTK_UP_BOX);
  btnOK->shortcut(FL_Escape);
  btnOK->callback(svCloseChildWindow, win);

  // end help window layout
  win->end();

  // show window
  win->show();
  Fl::redraw();
}


/*  show app options  */
void svShowAppOptions ()
{
  // check to make sure no other child window is visible
  if (app->childWindowVisible)
    return;

  // set flag so no other child window will show
  app->childWindowVisible = true;

  // window size
  int nWinWidth = 675;
  int nWinHeight = 630;

  // set window position
  int nX = app->hostList->w() + 50;
  int nY = (app->mainWin->h() / 2) - (nWinHeight / 2);

  m_appOptions.clear();

  // create window
  Fl_Window * win = new Fl_Window(nX, nY, nWinWidth, nWinHeight, "Application Options");
  m_appOptions["win"] = win;
  win->set_modal();
  win->callback(svCloseChildWindow, win);

  app->childWindowBeingDisplayed = win;

  // add itm value editors / selectors
  int nXPos = 300;
  int nYStep = 32;
  int nYPos = -(nYStep / 2);

  // ############ general options ##########################################################

  // scan viewer time-out
  Fl_Spinner * spinScanTimeout = new Fl_Spinner(nXPos, nYPos += nYStep,
    100, 28, "Scan wait time (seconds) ");
  m_appOptions["spinScanTimeout"] = spinScanTimeout;
  spinScanTimeout->textsize(app->nAppFontSize);
  spinScanTimeout->labelsize(app->nAppFontSize);
  spinScanTimeout->step(1);
  spinScanTimeout->minimum(1);
  spinScanTimeout->maximum(200000);
  spinScanTimeout->value(app->nScanTimeout);
  spinScanTimeout->tooltip("When scanning, this is how long SpiritVNC waits before moving"
    " to the next connected host item");

  // starting local ssh port number
  Fl_Spinner * spinLocalSSHPort = new Fl_Spinner(nXPos, nYPos += nYStep, 100, 28, "Starting local SSH port number ");
  m_appOptions["spinLocalSSHPort"] = spinLocalSSHPort;
  spinLocalSSHPort->textsize(app->nAppFontSize);
  spinLocalSSHPort->labelsize(app->nAppFontSize);
  spinLocalSSHPort->step(1);
  spinLocalSSHPort->minimum(1);
  spinLocalSSHPort->maximum(200000);
  spinLocalSSHPort->value(app->nStartingLocalPort);
  spinLocalSSHPort->tooltip("This is the first SSH port used locally for VNC-over-SSH connections");

  // ssh command
  SVInput * inSSHCommand = new SVInput(nXPos, nYPos += nYStep, 210, 28, "SSH command (eg: ssh or /usr/bin/ssh) ");
  m_appOptions["inSSHCommand"] = inSSHCommand;
  inSSHCommand->textsize(app->nAppFontSize);
  inSSHCommand->labelsize(app->nAppFontSize);
  inSSHCommand->value(app->sshCommand.c_str());
  inSSHCommand->tooltip("This is the command to start the SSH client on your system");

  // log app events to file?
  Fl_Check_Button * chkLogToFile = new Fl_Check_Button(nXPos, nYPos += nYStep, 210, 28, " Log app events to a file");
  m_appOptions["chkLogToFile"] = chkLogToFile;
  chkLogToFile->labelsize(app->nAppFontSize);
  chkLogToFile->tooltip("Check this to log app events to a file");
  if (app->enableLogToFile)
    chkLogToFile->set();

  // right-click to immediately close connection?
  Fl_Check_Button * chkRightClickToClose = new Fl_Check_Button(nXPos, nYPos += nYStep, 210, 28,
    " Right-click immediately closes connection");
  m_appOptions["chkRightClickToClose"] = chkRightClickToClose;
  chkRightClickToClose->labelsize(app->nAppFontSize);
  chkRightClickToClose->tooltip("Check this to immediately close connections by right-clicking them");
  if (app->rightClickToClose)
    chkRightClickToClose->set();

  // adjust message loop wait time
  Fl_Spinner * spinMsgLoopSpeed = new Fl_Spinner(nXPos, nYPos += nYStep, 100, 28, "VNC message loop speed");
  m_appOptions["spinMsgLoopSpeed"] = spinMsgLoopSpeed;
  spinMsgLoopSpeed->textsize(app->nAppFontSize);
  spinMsgLoopSpeed->labelsize(app->nAppFontSize);
  spinMsgLoopSpeed->step(1);
  spinMsgLoopSpeed->minimum(0);
  spinMsgLoopSpeed->maximum(9);
  spinMsgLoopSpeed->value(app->messageLoopSpeed); //std::to_string(app->messageLoopWaitTime).c_str());
  spinMsgLoopSpeed->tooltip("This is the speed setting that the VNC message loop will run when"
    " checking for a new VNC message from the displayed remote host");

  // blurb about message loop wait time
  Fl_Box * msgLoopWaitTimeLabel = new Fl_Box(nXPos - 175, nYPos += nYStep, 465, 55, "Higher speeds draw the "
    "remote host faster but use more CPU\n0 is slowest, 9 is fastest");
  msgLoopWaitTimeLabel->labelsize(app->nAppFontSize);
  msgLoopWaitTimeLabel->labelcolor(fl_rgb_color(40, 64, 0));
  msgLoopWaitTimeLabel->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
  msgLoopWaitTimeLabel->box(FL_NO_BOX);
  msgLoopWaitTimeLabel->labelfont(FL_HELVETICA_ITALIC);

  nYPos += 10;

  // ############ appearance options section ##########################################################

  // appearance options label
  Fl_Box * lblSep01 = new Fl_Box(nXPos, nYPos += nYStep + 14, 100, 28, "Appearance Options");
  lblSep01->labelsize(app->nAppFontSize);
  lblSep01->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
  lblSep01->box(FL_NO_BOX);
  lblSep01->labelfont(FL_BOLD);

  // app font size
  SVIntInput * inAppFontSize = new SVIntInput(nXPos, nYPos += nYStep, 42, 28, "Application font size ");
  m_appOptions["inAppFontSize"] = inAppFontSize;
  inAppFontSize->textsize(app->nAppFontSize);
  inAppFontSize->labelsize(app->nAppFontSize);
  inAppFontSize->value(std::to_string(app->nAppFontSize).c_str());
  inAppFontSize->tooltip("This is the font size used throughout SpiritVNC.  Restart the app to"
    " see any changes");

  // list font
  SVInput * inListFont = new SVInput(nXPos, nYPos += nYStep, 210, 28, "Host list font name (eg: Lucida Sans) ");
  m_appOptions["inListFont"] = inListFont;
  inListFont->textsize(app->nAppFontSize);
  inListFont->labelsize(app->nAppFontSize);
  inListFont->value(app->strListFont.c_str());
  inListFont->tooltip("This is the font used for the host list.  Restart the app to"
    " see any changes");

  // list font size
  SVIntInput * inListFontSize = new SVIntInput(nXPos + 260, nYPos, 42, 28, "Size:");
  m_appOptions["inListFontSize"] = inListFontSize;
  inListFontSize->textsize(app->nAppFontSize);
  inListFontSize->labelsize(app->nAppFontSize);
  inListFontSize->value(std::to_string(app->nListFontSize).c_str());
  inListFontSize->tooltip("This is the font size used for the host list.  Restart the app to"
    " see any changes");

  // list width
  SVIntInput * inListWidth = new SVIntInput(nXPos, nYPos += nYStep, 210, 28, "List width ");
  m_appOptions["inListWidth"] = inListWidth;
  inListWidth->textsize(app->nAppFontSize);
  inListWidth->labelsize(app->nAppFontSize);
  inListWidth->value(std::to_string(app->requestedListWidth).c_str());
  inListWidth->tooltip("Width of the host list.  Restart the app to see any changes");

  // use color-blind icons?
  Fl_Check_Button * chkCBIcons = new Fl_Check_Button(nXPos, nYPos += nYStep, 210, 28, " Use icons for color-blind users");
  m_appOptions["chkCBIcons"] = chkCBIcons;
  chkCBIcons->labelsize(app->nAppFontSize);
  chkCBIcons->tooltip("Check this to enable color-blind-friendly icons in the host list."
    " Restart the app to see any changes");
  if (app->colorBlindIcons)
    chkCBIcons->set();

  // show tooltips?
  Fl_Check_Button * chkShowTooltips = new Fl_Check_Button(nXPos, nYPos += nYStep, 210, 28, " Show tooltips");
  m_appOptions["chkShowTooltips"] = chkShowTooltips;
  chkShowTooltips->labelsize(app->nAppFontSize);
  chkShowTooltips->tooltip("Check this to enable tooltips in SpiritVNC.");
  if (app->showTooltips)
    chkShowTooltips->set();

  // show reverse connection notification?
  Fl_Check_Button * chkShowReverseConnect = new Fl_Check_Button(nXPos, nYPos += nYStep, 210, 28,
    " Show reverse connection notification");
  m_appOptions["chkShowReverseConnect"] = chkShowReverseConnect;
  chkShowReverseConnect->labelsize(app->nAppFontSize);
  chkShowReverseConnect->tooltip("Check this to show a message window when a reverse"
    " connection happens.");
  if (app->showReverseConnect)
    chkShowReverseConnect->set();

  // buttons on top?
  Fl_Check_Button * chkButtonsOnTop = new Fl_Check_Button(nXPos, nYPos += nYStep, 210, 28, " Host list buttons on top");
  m_appOptions["chkButtonsOnTop"] = chkButtonsOnTop;
  chkButtonsOnTop->labelsize(app->nAppFontSize);
  chkButtonsOnTop->tooltip("Check this to position host list buttons on top instead of the bottom");
  if (app->buttonsOnTop)
    chkButtonsOnTop->set();

  // ############ gap and restart advice ##########################################################

  // a little extra vertical gap
  nYPos += nYStep;

  // #### advice to restart SpiritVNC if certain options changed ####
  Fl_Box * boxFontLabel = new Fl_Box(nXPos, nYPos += nYStep, 210, 28,
    "Please restart the app after making font changes");
  boxFontLabel->labelsize(app->nAppFontSize);
  boxFontLabel->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
  boxFontLabel->box(FL_NO_BOX);
  boxFontLabel->labelcolor(fl_rgb_color(40, 64, 0));
  boxFontLabel->labelfont(FL_HELVETICA_BOLD_ITALIC);

  // ############ bottom buttons ##########################################################

  // 'Cancel' button
  Fl_Button * btnCancel = new Fl_Button((nWinWidth - 210 - 10), nWinHeight - (35 + 10), 100, 35, "Cancel");
  m_appOptions["btnCancel"] = btnCancel;
  btnCancel->labelsize(app->nAppFontSize);
  btnCancel->box(FL_GTK_UP_BOX);
  btnCancel->shortcut(FL_Escape);
  btnCancel->callback(svHandleAppOptionsButtons);
  btnCancel->tooltip("Click to abandon any edits and close this window");

  // 'Save' button
  Fl_Button * btnSave = new Fl_Button((nWinWidth - 100 - 10), nWinHeight - (35 + 10), 100, 35, "Save");
  m_appOptions["btnSave"] = btnSave;
  btnSave->labelsize(app->nAppFontSize);
  btnSave->box(FL_GTK_UP_BOX);
  btnSave->shortcut(FL_Enter);
  btnSave->callback(svHandleAppOptionsButtons);
  btnSave->tooltip("Click to save edits and close this window");

  // end app options window layout
  win->end();

  // show window
  win->show();

  // focus the first spinner control
  spinScanTimeout->take_focus();

  // silly code to select all characters in first spinner's text input
  Fl_Input_ * inTemp = static_cast<Fl_Input_ *>(spinScanTimeout->child(0));

  if (inTemp && inTemp->type() == FL_INT_INPUT)
    inTemp->insert_position(0, 1000);

  Fl::redraw();
}


/* show F8 window */
void svShowF8Window ()
{
  // check to make sure no other child window is visible
  if (app->childWindowVisible)
    return;

  // set flag so no other child window will show
  app->childWindowVisible = true;

  m_f8Actions.clear();

  // window size
  int nWinWidth = 230;
  int nWinHeight = 335;

  // set window position
  int nX = (app->mainWin->w() / 2) - (nWinWidth / 2);
  int nY = (app->mainWin->h() / 2) - (nWinHeight / 2);

  // create window
  Fl_Window * win = new Fl_Window(nX, nY, nWinWidth, nWinHeight, "Remote host actions");
  m_f8Actions["win"] = win;
  win->set_modal();
  win->callback(svCloseChildWindow, win);

  app->childWindowBeingDisplayed = win;

  // add itm value editors / selectors
  int nXPos = 15;
  int nYStep = 38;
  int nYPos = -(nYStep / 2);

  // =========================

  Fl_Button * btnCAD = new Fl_Button(nXPos, nYPos += nYStep, 200, 35, "Send Ctrl+Alt+Del");
  m_f8Actions["btnCAD"] = btnCAD;
  btnCAD->box(FL_GTK_UP_BOX);
  btnCAD->callback(svHandleF8Buttons);
  btnCAD->tooltip("Click to send Ctrl+Alt+Delete to the current remote host");

  Fl_Button * btnCSE = new Fl_Button(nXPos, nYPos += nYStep, 200, 35, "Send Ctrl+Shift+Esc");
  m_f8Actions["btnCSE"] = btnCSE;
  btnCSE->box(FL_GTK_UP_BOX);
  btnCSE->callback(svHandleF8Buttons);
  btnCSE->tooltip("Click to send Ctrl+Alt+Esc to the current remote host");

  Fl_Button * btnRefresh = new Fl_Button(nXPos, nYPos += nYStep, 200, 35, "Send refresh request");
  m_f8Actions["btnRefresh"] = btnRefresh;
  btnRefresh->box(FL_GTK_UP_BOX);
  btnRefresh->callback(svHandleF8Buttons);
  btnRefresh->tooltip("Click to send a screen refresh request to the current remote host");

  Fl_Button * btnSendF8 = new Fl_Button(nXPos, nYPos += nYStep, 200, 35, "Send F8");
  m_f8Actions["btnSendF8"] = btnSendF8;
  btnSendF8->box(FL_GTK_UP_BOX);
  btnSendF8->callback(svHandleF8Buttons);
  btnSendF8->tooltip("Click to press the F8 key on the current remote host");

  Fl_Button * btnSendF11 = new Fl_Button(nXPos, nYPos += nYStep, 200, 35, "Send F11");
  m_f8Actions["btnSendF11"] = btnSendF11;
  btnSendF11->box(FL_GTK_UP_BOX);
  btnSendF11->callback(svHandleF8Buttons);
  btnSendF11->tooltip("Click to press the F11 key on the current remote host");

  Fl_Button * btnSendF12 = new Fl_Button(nXPos, nYPos += nYStep, 200, 35, "Send F12");
  m_f8Actions["btnSendF12"] = btnSendF12;
  btnSendF12->box(FL_GTK_UP_BOX);
  btnSendF12->callback(svHandleF8Buttons);
  btnSendF12->tooltip("Click to press the F12 key on the current remote host");

  // ############ bottom button ##########################################################

  // 'Close' button
  Fl_Button * btnCancel = new Fl_Button((nWinWidth - 110), nWinHeight - (35 + 10), 100, 35, "Close");
  m_f8Actions["btnCancel"] = btnCancel;
  btnCancel->box(FL_GTK_UP_BOX);
  btnCancel->shortcut(FL_Escape);
  btnCancel->callback(svHandleF8Buttons);
  btnCancel->tooltip("Click to close this window");

  // end F8 window layout
  win->end();

  // show window
  win->show();

  // focus the first spinner control
  btnCAD->take_focus();

  Fl::redraw();
}


/* handle quick note editor buttons */
/* (void * parameter intentionally missing) */
void svHandleQuickNoteEditorButtons (Fl_Widget * button, void *)
{
  if (!button || m_quickNoteEdit.empty())
  {
    svCloseDeleteFinalizeChildWindow(NULL);
    return;
  }

  Fl_Window * win = reinterpret_cast<Fl_Window *>(m_quickNoteEdit["win"]);
  Fl_Text_Buffer * buf = static_cast<Fl_Text_Buffer *>(m_quickNoteEdit["buf"]);
  HostItem * itm = static_cast<HostItem *>(m_quickNoteEdit["itm"]);

  if (buf && itm)
  {
    if (button == m_quickNoteEdit["btnSave"])
    {
      itm->quickNote = buf->text();
      svQuickInfoSetLabelAndText(itm);
    }
  }

  svCloseDeleteFinalizeChildWindow(win);
}


/* show the quicknote editor window */
void svShowQuickNoteEditorWindow (HostItem * itm)
{
  // check to make sure no other child window is visible
  if (app->childWindowVisible || !itm)
    return;

  // set flag so no other child window will show
  app->childWindowVisible = true;

  // window size
  int nWinWidth = 350;
  int nWinHeight = 275;

  m_quickNoteEdit.clear();

  m_quickNoteEdit["itm"] = itm;

  // set window position
  int nX = (app->mainWin->w() / 4);
  int nY = (app->mainWin->h() / 3);

  // create window
  Fl_Window * win = new Fl_Window(nX, nY, nWinWidth, nWinHeight, "Edit Quick Note");
  m_quickNoteEdit["win"] = win;
  win->set_modal();
  win->callback(svCloseChildWindow, win);

  app->childWindowBeingDisplayed = win;

  Fl_Flex * parentFlex = new Fl_Flex(0, 0, nWinWidth, nWinHeight, Fl_Flex::VERTICAL);
  parentFlex->margin(5);

  Fl_Text_Buffer * buf = new Fl_Text_Buffer();
  m_quickNoteEdit["buf"] = buf;
  SVQuickNoteTextEditor * edit = new SVQuickNoteTextEditor(0, 0, 0, 0);
  edit->buffer(buf);
  edit->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
  buf->text(itm->quickNote.c_str());

  // move cursor to the end
  edit->insert_position(buf->length());

  // scroll to make the cursor visible
  edit->show_insert_position();

  // ############ bottom button ##########################################################

  Fl_Flex * buttonFlex = new Fl_Flex(Fl_Flex::HORIZONTAL);
  buttonFlex->gap(3);
  buttonFlex->margin(150, 5, 2, 0);

  // 'Close' button
  Fl_Button * btnCancel = new Fl_Button(0, 0, 0, 0, "Close");
  m_quickNoteEdit["btnCancel"] = btnCancel;
  btnCancel->box(FL_GTK_UP_BOX);
  btnCancel->shortcut(FL_Escape);
  btnCancel->callback(svHandleQuickNoteEditorButtons);
  btnCancel->tooltip("Click to close this window");

  // 'Save' button
  Fl_Button * btnSave = new Fl_Button(0, 0, 0, 0, "Save");
  m_quickNoteEdit["btnSave"] = btnSave;
  btnSave->box(FL_GTK_UP_BOX);
  btnSave->callback(svHandleQuickNoteEditorButtons);
  btnSave->tooltip("Click to save the quick note");

  buttonFlex->end();

  parentFlex->fixed(buttonFlex, 42);

  // end of parentFlex layout
  parentFlex->end();

  // end window layout
  win->end();

  // show window
  win->show();

  Fl::redraw();
}


/*  create / show connection editor window  */
void svShowConnectionEditor (HostItem * itmIn)
{
  // check to make sure no other child window is visible
  if (app->childWindowVisible)
    return;

  // set flag so no other child window will show
  app->childWindowVisible = true;

  HostItem * itm = static_cast<HostItem *>(itmIn);
  // make a new itm if we're passed a null one
  if (!itm)
  {
    itm = new HostItem();
    itm->name = "(new connection)";
    itm->hostAddress = "0.0.0.0";
    itm->vncPort = "5900";
    itm->sshPort = "";
    itm->scaling = 's';
    itm->sshKeyPrivate = "";
    itm->showRemoteCursor = true;
    itm->compressLevel = 5;
    itm->qualityLevel = 5;
    itm->vnc = NULL;
  }

  m_itmSettings.clear();

  m_itmSettings["itm"] = itm;

  // window size
  int nWinWidth = 545;
  int nWinHeight = 600;

  // set window position
  int nX = app->hostList->w() + 50;
  int nY = (app->mainWin->h() / 2) - (nWinHeight / 2);

  std::string strWinTitle = "Editing '" + itm->name + "'";

  // create window
  Fl_Window * win = new Fl_Window(nX, nY, nWinWidth, nWinHeight, strWinTitle.c_str());
  m_itmSettings["win"] = win;
  win->set_modal();
  win->callback(svCloseChildWindow, win);

  int nXPos = 195;
  int nYStep = 28;
  int nYPos = -24;

  // disable some things if the connection is connected
  bool disableConnectedSettings = itm->isConnected;

  // add itm value editors / selectors

  // create tab control - each tab is a child Fl_Group within the tab / tab->end()
  Fl_Tabs * itemTab = new Fl_Tabs(10, nYPos += nYStep, nWinWidth - 20, nWinHeight - 62);

  // vnc options tab
  Fl_Group * vncGroup = new Fl_Group(0, nYPos += nYStep, nWinWidth - 23, nWinHeight - 20, "VNC options");

  // ############ general options ##########################################################
  nYPos = 16;

  // connection name
  SVInput * inName = new SVInput(nXPos, nYPos += nYStep, 210, 28, "Connection name ");
  m_itmSettings["inName"] = inName;
  if (disableConnectedSettings)
    inName->deactivate();
  inName->value(itm->name.c_str());
  inName->tooltip("The connection name as displayed in the host connection list");

  // connection group
  SVInput * inGroup = new SVInput(nXPos, nYPos += nYStep, 210, 28, "Connection group ");
  m_itmSettings["inGroup"] = inGroup;
  if (disableConnectedSettings)
    inGroup->deactivate();
  inGroup->value(itm->group.c_str());
  inGroup->tooltip("The group name this connection belongs to");

  // remote address
  SVInput * inAddress = new SVInput(nXPos, nYPos += nYStep, 210, 28, "Remote address ");
  m_itmSettings["inAddress"] = inAddress;
  if (disableConnectedSettings)
    inAddress->deactivate();
  inAddress->value(itm->hostAddress.c_str());
  inAddress->tooltip("The IP address of the remote host you want to connect to."
      "  Do NOT include the VNC port number here");

  // f12 macro text
  SVInput * inF12Macro = new SVInput(nXPos, nYPos += nYStep, 210, 28, "F12 macro ");
  m_itmSettings["inF12Macro"] = inF12Macro;
  inF12Macro->value(itm->f12Macro.c_str());
  inF12Macro->tooltip("Key presses that are sent to the remote host when"
      " you press the F12 key");

  // * vnc type buttons *

  // vnc without ssh
  Fl_Radio_Round_Button * rbVNC = new Fl_Radio_Round_Button(nXPos, nYPos += nYStep, 100, 28, "VNC ");
  m_itmSettings["rbVNC"] = rbVNC;
  if (disableConnectedSettings)
    rbVNC->deactivate();
  rbVNC->callback(svItmOptionsRadioButtonsCallback);
  rbVNC->tooltip("Choose this for VNC connections that don't tunnel through SSH");

  // vnc with ssh
  Fl_Radio_Round_Button * rbSVNC = new Fl_Radio_Round_Button(nXPos, nYPos += nYStep, 100, 28,
    "VNC through SSH ");
  m_itmSettings["rbSVNC"] = rbSVNC;
  if (disableConnectedSettings)
    rbSVNC->deactivate();
  rbSVNC->callback(svItmOptionsRadioButtonsCallback);
  rbSVNC->tooltip("Choose this for VNC connections that use SSH to tunnel to the host");

  // set current value
  if (itm->hostType == 'v')
    rbVNC->set();
  else
    rbSVNC->set();

  // vnc port
  SVIntInput * inVNCPort = new SVIntInput(nXPos, nYPos += nYStep, 100, 28, "VNC port ");
  m_itmSettings["inVNCPort"] = inVNCPort;
  if (disableConnectedSettings)
    inVNCPort->deactivate();
  inVNCPort->value(itm->vncPort.c_str());
  inVNCPort->tooltip("The VNC port/display number of the host.  Defaults to 5900");

  // vnc password (shows dots, not cleartext password, for 'password' authentication)
  SVSecretInput * inVNCPassword = new SVSecretInput(nXPos, nYPos += nYStep, 210, 28, "VNC password ");
  m_itmSettings["inVNCPassword"] = inVNCPassword;
  if (disableConnectedSettings)
    inVNCPassword->deactivate();
  inVNCPassword->value(base64Decode(itm->vncPassword).c_str());
  inVNCPassword->tooltip("The VNC password for the host");

  // vnc login name (for 'credential' authentication)
  SVInput * inVNCLoginUser = new SVInput(nXPos, nYPos += nYStep, 210, 28, "VNC login user name ");
  m_itmSettings["inVNCLoginUser"] = inVNCLoginUser;
  if (disableConnectedSettings)
    inVNCLoginUser->deactivate();
  inVNCLoginUser->value(itm->vncLoginUser.c_str());
  inVNCLoginUser->tooltip("The VNC login name for the host");

  // vnc login password (shows dots, not cleartext password, for 'credential' authentication)
  SVSecretInput * inVNCLoginPassword = new SVSecretInput(nXPos, nYPos += nYStep, 210, 28, "VNC login password ");
  m_itmSettings["inVNCLoginPassword"] = inVNCLoginPassword;
  if (disableConnectedSettings)
    inVNCLoginPassword->deactivate();
  inVNCLoginPassword->value(base64Decode(itm->vncLoginPassword).c_str());
  inVNCLoginPassword->tooltip("The VNC login password for the host");

  // vnc compression level
  SVIntInput * inVNCCompressLevel = new SVIntInput(nXPos, nYPos += nYStep, 48, 28, "Compression level (0-9) ");
  m_itmSettings["inVNCCompressLevel"] = inVNCCompressLevel;
  inVNCCompressLevel->value(std::to_string(itm->compressLevel).c_str());
  inVNCCompressLevel->tooltip("The level of compression, from 0 to 9");

  // vnc quality level
  SVIntInput * inVNCQualityLevel = new SVIntInput(nXPos, nYPos += nYStep, 48, 28, "Quality level (0-9) ");
  m_itmSettings["inVNCQualityLevel"] = inVNCQualityLevel;
  inVNCQualityLevel->value(std::to_string(itm->qualityLevel).c_str());
  inVNCQualityLevel->tooltip("The level of image quality, from 0 to 9");

  // ##### scaling start #####

  // * scaling options group *
  Fl_Group * grpScaling = new Fl_Group(nXPos, nYPos += nYStep, 300, 300);

  // scaling off - scroll only
  Fl_Radio_Round_Button * rbScaleOff = new Fl_Radio_Round_Button(nXPos, nYPos, 100, 28, " Scale off (scroll)");
  m_itmSettings["rbScaleOff"] = rbScaleOff;
  rbScaleOff->callback(svItmOptionsRadioButtonsCallback);
  rbScaleOff->tooltip("Choose this if you don't want any scaling."
      "  Scrollbars will appear for hosts with screens larger than the viewer");

  // scale up and down
  Fl_Radio_Round_Button * rbScaleZoom = new Fl_Radio_Round_Button(nXPos, nYPos += nYStep, 100, 28,
    " Scale up and down");
  m_itmSettings["rbScaleZoom"] = rbScaleZoom;
  rbScaleZoom->callback(svItmOptionsRadioButtonsCallback);
  rbScaleZoom->tooltip("Choose this if you want small host screens scaled up to fit the"
      " viewer, or large host screens scaled down to fit the viewer");

  // scale down only
  Fl_Radio_Round_Button * rbScaleFit = new Fl_Radio_Round_Button(nXPos, nYPos += nYStep, 100, 28,
    " Scale down only");
  m_itmSettings["rbScaleFit"] = rbScaleFit;
  rbScaleFit->callback(svItmOptionsRadioButtonsCallback);
  rbScaleFit->tooltip("Choose this to scale down large host screens to fit the viewer but"
      " small host screens are not scaled up");

  // set current value
  if (itm->scaling == 's')
    rbScaleOff->set();
  else if (itm->scaling == 'z')
    rbScaleZoom->set();
  else if (itm->scaling == 'f')
    rbScaleFit->set();

  // fast scaling (instead of the default 'quality' scaling)
  Fl_Check_Button * chkScalingFast = new Fl_Check_Button(nXPos, nYPos += nYStep, 100, 28,
    " Fast scaling (low quality)");
  m_itmSettings["chkScalingFast"] = chkScalingFast;
  if (itm->scalingFast)
    chkScalingFast->set();
  chkScalingFast->tooltip("Check to select fast scaling instead of quality scaling");

  // end of scaling group
   grpScaling->end();

  // ##### scaling end #####

  // show the host's native cursor under our local static cursor
  Fl_Check_Button * chkShowRemoteCursor = new Fl_Check_Button(nXPos, nYPos += nYStep, 100, 28,
    " Use remote cursor locally");
  m_itmSettings["chkShowRemoteCursor"] = chkShowRemoteCursor;
  chkShowRemoteCursor->tooltip("Check to show remote locally");

  // set current value
  if (itm->showRemoteCursor)
    chkShowRemoteCursor->set();

  // end of vnc options tab
  vncGroup->end();

  // ############ vnc-over-ssh options ##########################################################
  nYPos = 4; //10; //28;

  Fl_Group * sshGroup = new Fl_Group(0, nYPos += nYStep, nWinWidth - 20, nWinHeight - 20, "SSH options");

  nYPos = 16;

  // name used for ssh login
  SVInput * inSSHName = new SVInput(nXPos, nYPos += nYStep, 210, 28, "SSH user name ");
  m_itmSettings["inSSHName"] = inSSHName;
  if (disableConnectedSettings)
    inSSHName->deactivate();
  inSSHName->value(itm->sshUser.c_str());
  inSSHName->tooltip("The SSH user name for the host");

  //// password used for ssh login (if used)   // #### DISABLED RIGHT NOW ####
  //SVSecretInput * inSSHPassword = new SVSecretInput(nXPos, nYPos += nYStep, 210, 28, "SSH password (if any) ");
  //m_itmSettings["inSSHPassword"] = inSSHPassword;
  //inSSHPassword->value(itm->sshPass.c_str());
  //inSSHPassword->tooltip("The SSH password for the host (usually not necessary when using"
      //" key files)");
  //inSSHPassword->deactivate();  //  <<<--- We can't really do SSH password right now ---<<<

  // ssh port (on the remote host)
  SVIntInput * inSSHPort = new SVIntInput(nXPos, nYPos += nYStep, 100, 28, "SSH remote port ");
  m_itmSettings["inSSHPort"] = inSSHPort;
  if (disableConnectedSettings)
    inSSHPort->deactivate();
  inSSHPort->value(itm->sshPort.c_str());
  inSSHPort->tooltip("The remote host's port number");

  // ssh private key full path (if used)
  SVInput * inSSHPrvKey = new SVInput(nXPos, nYPos += nYStep, 300, 28, "SSH private key (if any) ");
  m_itmSettings["inSSHPrvKey"] = inSSHPrvKey;
  if (disableConnectedSettings)
    inSSHPrvKey->deactivate();
  inSSHPrvKey->value(itm->sshKeyPrivate.c_str());
  inSSHPrvKey->tooltip("The SSH private key file location (usually not"
      " necessary when using a SSH password");

  // button to select ssh private key
  Fl_Button * btnShowPrvKeyChooser = new Fl_Button(nXPos + 300 + 2, nYPos + 4, 20, 20, "...");
  if (disableConnectedSettings)
  {
    btnShowPrvKeyChooser->deactivate();
    btnShowPrvKeyChooser->callback(svNoOpCallback);
  }
  else
    btnShowPrvKeyChooser->callback(svHandleConnEditChoosePrvKeyBtn, inSSHPrvKey);
  btnShowPrvKeyChooser->tooltip("Click to choose a SSH private key file");

  // set app vars
  app->childWindowBeingDisplayed = win;

  // end of ssh options tab
  sshGroup->end();

  // ############ custom commands ##########################################################
  nYPos = 4;

  Fl_Group * customCommandGroup = new Fl_Group(0, nYPos += nYStep, nWinWidth - 20, nWinHeight - 20,
    "Custom commands");

  nXPos = 15;
  nYPos = 16;

  // custom command 1
  Fl_Check_Button * chkCommand1Enabled = new Fl_Check_Button(nXPos, nYPos += nYStep, 175, 28, "Command 1 enabled");
  m_itmSettings["chkCommand1Enabled"] = chkCommand1Enabled;
  chkCommand1Enabled->tooltip("Check to enable custom command 1");

  if (itm->customCommand1Enabled)
    chkCommand1Enabled->set();

  SVInput * inCommand1Label = new SVInput(nXPos + 180, nYPos, 120, 28);
  m_itmSettings["inCommand1Label"] = inCommand1Label;
  inCommand1Label->value(itm->customCommand1Label.c_str());

  SVInput * inCommand1 = new SVInput(nXPos + 305, nYPos, 200, 28);
  m_itmSettings["inCommand1"] = inCommand1;
  inCommand1->value(itm->customCommand1.c_str());

  // custom command 2
  Fl_Check_Button * chkCommand2Enabled = new Fl_Check_Button(nXPos, nYPos += nYStep, 175, 28, "Command 2 enabled");
  m_itmSettings["chkCommand2Enabled"] = chkCommand2Enabled;
  chkCommand2Enabled->tooltip("Check to enable custom command 2");

  if (itm->customCommand2Enabled)
    chkCommand2Enabled->set();

  SVInput * inCommand2Label = new SVInput(nXPos + 180, nYPos, 120, 28);
  m_itmSettings["inCommand2Label"] = inCommand2Label;
  inCommand2Label->value(itm->customCommand2Label.c_str());

  SVInput * inCommand2 = new SVInput(nXPos + 305, nYPos, 200, 28);
  m_itmSettings["inCommand2"] = inCommand2;
  inCommand2->value(itm->customCommand2.c_str());

  // custom command 3
  Fl_Check_Button * chkCommand3Enabled = new Fl_Check_Button(nXPos, nYPos += nYStep, 175, 28, "Command 3 enabled");
  m_itmSettings["chkCommand3Enabled"] = chkCommand3Enabled;
  chkCommand3Enabled->tooltip("Check to enable custom command 3");

  if (itm->customCommand3Enabled)
    chkCommand3Enabled->set();

  SVInput * inCommand3Label = new SVInput(nXPos + 180, nYPos, 120, 28);
  m_itmSettings["inCommand3Label"] = inCommand3Label;
  inCommand3Label->value(itm->customCommand3Label.c_str());

  SVInput * inCommand3 = new SVInput(nXPos + 305, nYPos, 200, 28);
  m_itmSettings["inCommand3"] = inCommand3;
  inCommand3->value(itm->customCommand3.c_str());

  // end of custom command tab
  customCommandGroup->end();

  // end of tab control
  itemTab->end();

  // ############ bottom buttons ##########################################################

  // make a divider line between the scroller and the buttons
  Fl_Box * sepr = new Fl_Box(0, nWinHeight - (35 + 18), nWinWidth, 1);
  sepr->box(FL_FLAT_BOX);
  sepr->color(fl_lighter(FL_FOREGROUND_COLOR));

  // 'Delete' button
  Fl_Button * btnDel = new Fl_Button(10, nWinHeight - (35 + 7), 100, 35, "Delete");
  m_itmSettings["btnDel"] = btnDel;
  btnDel->box(FL_GTK_UP_BOX);
  btnDel->callback(svHandleConnEditButtons);
  btnDel->tooltip("Click to permanently delete this host connection"
      " (not undoable).  You will be asked to confirm before item is deleted");

  // 'Cancel' button
  Fl_Button * btnCancel = new Fl_Button((nWinWidth - 210 - 10), nWinHeight - (35 + 7), 100, 35, "Cancel");
  m_itmSettings["btnCancel"] = btnCancel;
  btnCancel->box(FL_GTK_UP_BOX);
  btnCancel->shortcut(FL_Escape);
  btnCancel->callback(svHandleConnEditButtons);
  btnCancel->tooltip("Click to abandon any edits to this connection and close this window");

  // 'Save' button
  Fl_Button * btnSave = new Fl_Button((nWinWidth - 100 - 10), nWinHeight - (35 + 7), 100, 35, "Save");
  m_itmSettings["btnSave"] = btnSave;
  btnSave->box(FL_GTK_UP_BOX);
  btnSave->shortcut(FL_Enter);
  btnSave->callback(svHandleConnEditButtons);
  btnSave->tooltip("Click to save edits made to this connection and close this window");

  // end item options window layout
  win->end();

  // focus the first input box and select all text within
  inName->take_focus();
  inName->insert_position(0, strlen(inName->value()) + 1);

  win->show();
  Fl::redraw();
}


/* update text on all host items */
void svUpdateHostListItemText ()
{
  uint16_t nSize = app->hostList->size();

  for (uint16_t i = 0; i <= nSize; i ++)
  {
    HostItem * itm = static_cast<HostItem *>(app->hostList->data(i));
    if (itm)
      app->hostList->text(i, itm->name.c_str());
  }
}
