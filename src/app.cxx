/*
 * app.cxx - part of SpiritVNC - FLTK
 * 2016-2022 Will Brokenbourgh https://www.pismotek.com/brainout/
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


/* handle method for SVInput
 * (instance method)
 */
int SVInput::handle (int evt)
{
  static bool inMenu;

  // handle child window input controls right-click
  if (evt == FL_PUSH && Fl::event_button3() != 0)
  {
    // prevent re-entry
    if (inMenu == true)
      return 0;

    inMenu = true;

    // create context menu
    // text,shortcut,callback,user_data,flags,labeltype,labelfont,labelsize
    const Fl_Menu_Item miMain[] = {
      {"Undo",  0, 0, 0, FL_MENU_DIVIDER, 0, SV_FLTK_FONT_0, app->nMenuFontSize},
      {"Cut",   0, 0, 0, 0,               0, SV_FLTK_FONT_0, app->nMenuFontSize},
      {"Copy",  0, 0, 0, 0,               0, SV_FLTK_FONT_0, app->nMenuFontSize},
      {"Paste", 0, 0, 0, 0,               0, SV_FLTK_FONT_0, app->nMenuFontSize},
      {0}
    };

    // show context menu and return selected item, if any
    const Fl_Menu_Item * miRes = miMain->popup(Fl::event_x() + 14, Fl::event_y() - 10);

    if (miRes != NULL)
    {
      const char * strRes = miRes->text;

      if (strRes != NULL)
      {
        std::string strR = strRes;

        if (strR == "Undo")
          undo();
        else if (strR == "Cut")
        {
          copy(1);
          cut();
        }
        else if (strR == "Copy")
          copy(1);
        else if (strR == "Paste")
          Fl::paste(*this);
      }
    }

    inMenu = false;

    return 1;
  }

  return Fl_Input::handle(evt);
}


/* handle method for SVSecretInput
 * (instance method)
 */
int SVSecretInput::handle (int evt)
{
  static bool inMenu;

  // handle child window input controls right-click
  if (evt == FL_PUSH && Fl::event_button3() != 0)
  {
    // prevent re-entry
    if (inMenu == true)
      return 0;

    inMenu = true;

    // create context menu
    // text,shortcut,callback,user_data,flags,labeltype,labelfont,labelsize
    const Fl_Menu_Item miMain[] = {
      {"Undo",  0, 0, 0, FL_MENU_DIVIDER,  0, SV_FLTK_FONT_0, app->nMenuFontSize},
      {"Cut",   0, 0, 0, FL_MENU_INACTIVE, 0, SV_FLTK_FONT_0, app->nMenuFontSize},
      {"Copy",  0, 0, 0, FL_MENU_INACTIVE, 0, SV_FLTK_FONT_0, app->nMenuFontSize},
      {"Paste", 0, 0, 0, 0,                0, SV_FLTK_FONT_0, app->nMenuFontSize},
      {0}
    };

    // show context menu and return selected item, if any
    const Fl_Menu_Item * miRes = miMain->popup(Fl::event_x() + 14, Fl::event_y() - 10);

    if (miRes != NULL)
    {
      const char * strRes = miRes->text;

      if (strRes != NULL)
      {
        std::string strR = strRes;

        if (strR == "Undo")
          undo();
        else if (strR == "Paste")
          Fl::paste(*this);
      }
    }

    inMenu = false;

    return 1;
  }

  return Fl_Secret_Input::handle(evt);
}


/*
  child window 'OK' button callback - closes child windows (settings, options, etc
  (Fl_Widget * is unused, so parameter name removed according to 'best practices')
*/
void svCloseChildWindow (Fl_Widget *, void * data)
{
  Fl_Window * childWindow = static_cast<Fl_Window *>(data);

  // close the child window, if valid
  if (childWindow != NULL)
    childWindow->hide();

  // set app child window visible property
  app->childWindowVisible = false;
}


/* creates new configuration if none is found */
void svConfigCreateNew ()
{
  struct stat st;

  if (stat(app->configPath.c_str(), &st) == -1)
  {
    if (mkdir(app->configPath.c_str(), 0700) == -1)
    {
      std::cout << "SpiritVNC ERROR - Cannot create config file directory" << std::endl;
      exit(-1);
    }
  }
}


/* read from the config file, set app options and populate host list */
void svConfigReadCreateHostList ()
{
  std::ifstream ifs;
  char strP[SV_PROP_LINE_MAX] = {0};
  std::string strLastGroup;
  std::string strProp;
  std::string strVal;
  HostItem * itm = NULL;
  bool addSep = false;
  uint32_t numOfEntries = 0;

  app->hostList->clear();

  // try to open config file
  ifs.open(app->configPathAndFile.c_str(), std::ifstream::in);

  // oops, can't open config file
  if (ifs.fail())
  {
    std::cout << "SpiritVNC - Could not open config file.  Using defaults" << std::endl;
    svConfigCreateNew();
    return;
  }

  svLogToFile("--- Program started up ---");

  while (app->shuttingDown == false)
  {
    ifs.getline(strP, SV_PROP_LINE_MAX);

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
          int w = atoi(strVal.c_str());

          if (w < 10)
              w = 10;

          // set size of hostlist
          app->hostList->size(w, app->hostList->h());
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

        // dead connection timeout in seconds
        if (strProp == "deadtimeout")
        {
          int w = atoi(strVal.c_str());

          if (w < 1)
              w = 100;

          app->nDeadTimeout = w;
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

        // display debug messages?
        if (strProp == "debugmode")
          app->debugMode = svConvertStringToBoolean(strVal);

        // app font size
        if (strProp == "appfontsize")
        {
          app->nAppFontSize = atoi(strVal.c_str());

          if (app->nAppFontSize < 1)
            app->nAppFontSize = 10;
        }

        // list font
        if (strProp == "listfont")
        {
          if (strVal != "")
            app->strListFont = strVal;
        }

        // list font size
        if (strProp == "listfontsize")
        {
          app->nListFontSize = atoi(strVal.c_str());

          if (app->nListFontSize < 1)
            app->nListFontSize = 10;
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

        // #############################################################################
        // ######## per-connection options #############################################
        // #############################################################################

        // new host entry
        if (strProp == "host")
        {
          // add last host entry to host list
          if (itm != NULL && numOfEntries < SV_MAX_HOSTLIST_ENTRIES)
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
            if (addSep == true)
              // add a separator
              // color 16 (@C16) is supposed to be gray
              app->hostList->add("@C16@.?? ?? ??");
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

        // ssh password
        if (strProp == "sshpass")
          itm->sshPass = strVal;

        // password
        if (strProp == "vncpass")
        {
          itm->vncPassword = base64Decode(strVal);

          if (itm->vncPassword == "")
            itm->vncPassword = "(empty)";
        }

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

          if (itm->compressLevel < 0)
            itm->compressLevel = 0;

          if (itm->compressLevel > 9)
            itm->compressLevel = 9;
        }

        // quality level
        if (strProp == "quality")
        {
          itm->qualityLevel = atoi(strVal.c_str());

          if (itm->qualityLevel < 0)
            itm->qualityLevel = 0;

          if (itm->qualityLevel > 9)
            itm->qualityLevel = 9;
        }

        // ignore inactive disconnect?
        if (strProp == "ignoreinactive")
          itm->ignoreInactive = svConvertStringToBoolean(strVal);

        // center x?
        if (strProp == "centerx")
          itm->centerX = svConvertStringToBoolean(strVal);

        // center y?
        if (strProp == "centery")
          itm->centerY = svConvertStringToBoolean(strVal);
      }
    }
    else
    {
      // add last host entry to host list
      if (itm != NULL && numOfEntries < SV_MAX_HOSTLIST_ENTRIES)
        app->hostList->add(itm->name.c_str(), itm);

      // add a separator
      if (addSep == true)
        // color 16 (@C16) is supposed to be gray
        app->hostList->add("@C16@.?? ?? ??");

      break;
    }
  }

  ifs.close();

  // set host list font face and size
  Fl::set_font(31, app->strListFont.c_str());
  app->hostList->textfont(31);
  app->hostList->textsize(app->nListFontSize);
  app->nMenuFontSize = app->nListFontSize;
}


/* write config file */
void svConfigWrite ()
{
  std::ofstream ofs;

  // open our config file
  ofs.open(app->configPathAndFile.c_str());

  // oops, can't open config file
  if (ofs.fail())
  {
    std::cout << "SpiritVNC ERROR - Could not open config file for writing" << std::endl;
    return;
  }

  // write header
  ofs << "# SpiritVNC-FLTK config file" <<  std::endl;
  ofs << "#" << std::endl;
  ofs << "# option names / properties should always be lower-case without"
      " spaces" << std::endl;
  ofs << "# host type can be 'v' for vnc and 's' for vnc through ssh" << std::endl;
  ofs << "# scale can be 's' for scrolled, 'z' for scale up/down and 'f' for scale"
      " down only" << std::endl;
  ofs << std::endl;

  ofs << "# program options" << std::endl;

  // hostlist width
  ofs << "hostlistwidth=" << app->hostList->w() << std::endl;

  // colorblind icons
  ofs << "colorblindicons=" << svConvertBooleanToString(app->colorBlindIcons) << std::endl;

  // scan timeout in seconds
  ofs << "scantimeout=" << app->nScanTimeout << std::endl;

  // dead connection timeout
  ofs << "deadtimeout=" << app->nDeadTimeout << std::endl;

  // starting local port number (+99) for ssh connections
  ofs << "startinglocalport=" << app->nStartingLocalPort << std::endl;

  // show tool tips
  ofs << "showtooltips=" << svConvertBooleanToString(app->showTooltips) << std::endl;

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

  // blank line
  ofs << std::endl;

  HostItem * itm = NULL;

  ofs << "# host-list entries" << std::endl;

  // host list entries
  uint16_t nSize = app->hostList->size();

  for (uint16_t i = 0; i <= nSize; i ++)
  {
    itm = static_cast<HostItem *>(app->hostList->data(i));

    if (itm == NULL || itm->isListener == true)
      continue;

    ofs << "host=" << itm->name << std::endl;
    ofs << "group=" << itm->group << std::endl;
    ofs << "hostaddress=" << itm->hostAddress << std::endl;
    ofs << "vncport=" << itm->vncPort << std::endl;
    ofs << "sshport=" << itm->sshPort << std::endl;
    ofs << "vncpass=" << base64Encode(reinterpret_cast<const unsigned char *>
      (itm->vncPassword.c_str()), itm->vncPassword.size()) << std::endl;
    ofs << "type=" << itm->hostType << std::endl;
    //ofs << "sshkeypublic=" << itm->sshKeyPublic << std::endl;
    ofs << "sshkeyprivate=" << itm->sshKeyPrivate << std::endl;
    ofs << "sshuser=" << itm->sshUser << std::endl;
    ofs << "sshpass=" << itm->sshPass << std::endl;
    ofs << "scale=" << itm->scaling << std::endl;
    ofs << "scalefast=" << svConvertBooleanToString(itm->scalingFast) << std::endl;
    ofs << "f12macro=" << itm->f12Macro << std::endl;
    ofs << "showremotecursor=" << svConvertBooleanToString(itm->showRemoteCursor) << std::endl;
    ofs << "compression=" << itm->compressLevel << std::endl;
    ofs << "quality=" << itm->qualityLevel << std::endl;
    ofs << "ignoreinactive=" << svConvertBooleanToString(itm->ignoreInactive) << std::endl;
    ofs << "centerx=" << svConvertBooleanToString(itm->centerX) << std::endl;
    ofs << "centery=" << svConvertBooleanToString(itm->centerY) << std::endl;

    ofs << std::endl;
  }

  ofs << std::endl;

  // close file
  ofs.close();
}


/*
  a connection 'supervisor' that is called approx. every second by a timer
  (timer callback)
  (void * not used so parameter name removed)
*/
void svConnectionWatcher (void *)
{
  HostItem * itm = NULL;
  VncObject * vnc = NULL;
  uint16_t nSize;

  // only check if there are waiting viewers
  if (app->nViewersWaiting > 0)
  {
    svDebugLog("svConnectionWatcher - At least one itm ready for processing");

    nSize = app->hostList->size();

    // iterate through hostlist items
    for (uint16_t i = 0; i <= nSize; i ++)
    {
      itm = static_cast<HostItem *>(app->hostList->data(i));

      if (itm == NULL)
        continue;

      vnc = itm->vnc;

      if (vnc == NULL)
        continue;

      // advance and check viewer timeout value
      if (itm->isConnecting == true)
      {
        svDebugLog("svConnectionWatcher - itm still 'isConnecting'");

        vnc->waitTime ++;

        // 'soft' time-out reached
        if (vnc->waitTime > app->nConnectionTimeout && itm->isListener == false)
        {
          svDebugLog("svConnectionWatcher - 'Soft' timeout reached, giving up");

          VncObject::endAndDeleteViewer(&vnc);

          app->nViewersWaiting --;

          itm->isConnected = false;

          // set host list item status icon
          itm->icon = app->iconNoConnect;
          svHandleListItemIconChange(NULL);

          svLogToFile(std::string(std::string("Could not connect to '") +
            itm->name + "' - " + itm->hostAddress).c_str());
        }
      }

      // if ssh connection faltered, shut down the vnc viewer
      if (itm->isConnected == true
          && (itm->hostType == 's'
          && itm->sshReady == false))
      {
        app->nViewersWaiting --;
        svDebugLog("svConnectionWatcher - SSH problem during connection, ending");
        VncObject::endAndDeleteViewer(&itm->vnc);
      }
    }
  }

  // do an inactive connection check
  nSize = app->hostList->size();

  // iterate through hostlist items
  for (uint16_t i = 0; i <= nSize; i ++)
  {
    itm = static_cast<HostItem *>(app->hostList->data(i));

    if (itm == NULL)
      continue;

    vnc = itm->vnc;

    if (vnc == NULL || itm->isConnected == false)
      continue;

    // check if connection has been inactive, unless this itm is ignoring
    if (vnc->inactiveSeconds >= app->nDeadTimeout && itm->ignoreInactive == false)
      // remote host hasn't responded in time allotted, disconnect
      VncObject::endAndDeleteViewer(&vnc);
    else
      vnc->inactiveSeconds ++;
  }

  // set timer to call this function again in 1 second
  // (do NOT change this interval as connection timeout
  // values rely on this being at or near 1 second)
  Fl::repeat_timeout(SV_ONE_SECOND, svConnectionWatcher);
}


/* convert boolean to string */
std::string svConvertBooleanToString (bool boolIn)
{
  if (boolIn == true)
    return "true";
  else
    return "false";
}


/* convert string to boolean */
bool svConvertStringToBoolean (const std::string& strIn)
{
  if (strIn == "TRUE" || strIn == "True" || strIn == "true"
      || strIn == "1"
      || strIn == "YES" || strIn == "Yes" || strIn == "yes"
      || strIn == "ON"  || strIn == "On"  || strIn == "on")
        return true;

  return false;
}


/* create icons for app */
void svCreateAppIcons (bool fromAppOptions)
{
  // default or colorblind icons
  if (app->colorBlindIcons == false)
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

  if (fromAppOptions == false)
  {
    // set initial icons on hostlist items
    for (uint16_t i = 0; i <= app->hostList->size(); i ++)
      if (app->hostList->data(i) != NULL)
        app->hostList->icon(i, app->iconDisconnected);

    app->hostList->redraw();
  }
}


/* create program GUI */
void svCreateGUI ()
{
  // widget tooltips are set (if enabled) in svSetTooltips

  // create main window
  app->mainWin = new Fl_Double_Window(1024, 768);
  app->mainWin->size_range(800, 600, 32767, 32767);
  app->mainWin->label("SpiritVNC");
  app->mainWin->xclass("spiritvncfltk");
  app->mainWin->callback(svHandleMainWindowEvents);

  // create host list buttons
  // (positioning is done in svHandleMiscEvents)
  int nBtnSize = 20;

  app->packButtons = new Fl_Pack(0, 0, 1, nBtnSize);
  app->packButtons->type(Fl_Pack::HORIZONTAL);
  app->packButtons->begin();

  app->btnListAdd = new Fl_Button(0, 0, nBtnSize, nBtnSize);
  app->btnListAdd->clear_visible_focus();
  app->btnListAdd->callback(svHandleHostListButtons);

  app->btnListDelete = new Fl_Button(0, 0, nBtnSize, nBtnSize);
  app->btnListDelete->clear_visible_focus();
  app->btnListDelete->callback(svHandleHostListButtons);

  //--
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

  // create scrolling window we will add viewers to
  app->scroller = new Fl_Scroll(0, 0, 0, 0);
  app->vncViewer = new VncViewer(0, 0, 0, 0);
  app->scroller->box(FL_FLAT_BOX);
  app->scroller->type(0);
  app->scroller->end();

  // create host list
  app->hostList = new Fl_Hold_Browser(0, 0, 0, 0);
  app->hostList->clear_visible_focus();
  app->hostList->callback(svHandleHostListEvents, NULL);
  app->hostList->box(FL_THIN_DOWN_BOX);

  app->mainWin->resizable(app->scroller);
}


/* log debug messages, if enabled in config file */
void svDebugLog (const std::string& strDebugMessage)
{
  if (app->debugMode == true)
    svLogToFile("DEBUG - " + strDebugMessage);
}


/* show confirmation and delete item from hostList */
void svDeleteItem (int nItem)
{
  static bool inDeleteItem;

  bool okayToDelete = false;

  // prevent re-entry (FLTK menu bug)
  if (inDeleteItem == true)
    return;
  else
    inDeleteItem = true;

  const HostItem * itm = static_cast<HostItem *>(app->hostList->data(nItem));

  if (itm == NULL)
  {
    fl_beep(FL_BEEP_DEFAULT);
    inDeleteItem = false;
    return;
  }

  // listening connection
  if (itm->isListener)
    okayToDelete = true;
  else
  {
    // standard (non-listening) connection
    std::string strConfirm = "Are you sure you want to delete '" + itm->name + "'?";

    fl_message_hotspot(0);
    fl_message_title("SpiritVNC - Delete Item");

    if (fl_choice("%s", "Cancel", "No", "Yes", strConfirm.c_str()) == SV_CHOICE_2)
      okayToDelete = true;
  }

  if (okayToDelete == true)
  {
    app->hostList->remove(nItem);
    app->hostList->redraw();
  }

  inDeleteItem = false;
}


/* sets all host list items to deselected */
void svDeselectAllItems ()
{
  uint16_t nSize = app->hostList->size();

  // clear list of 'selected' color
  for (uint16_t i = 1; i <= nSize; i ++)
    app->hostList->select(i, 0);
}


/* find unused TCP port in a range */
int svFindFreeTcpPort ()
{
  int nSock = 0;
  struct sockaddr_in structSockAddress;

  structSockAddress.sin_family = AF_INET;
  structSockAddress.sin_addr.s_addr = htonl(INADDR_ANY);

  nSock = socket(AF_INET, SOCK_STREAM, 0);

  if (nSock < 0)
  {
    svLogToFile("ERROR - Cannot create socket for svFindFreeTCPPort");
    return 0;
  }

  // go through a range of startingLocalPort + 99 to see if we can use for ssh forwarding
  for (uint16_t nPort = app->nStartingLocalPort + 99; nPort > app->nStartingLocalPort; nPort --)
  {
    structSockAddress.sin_port = htons((unsigned short)nPort);

    // if nothing is on this port and it's not the reverse vnc port, return the port number
    if (bind(nSock, reinterpret_cast<sockaddr *>(&structSockAddress),
      sizeof(structSockAddress)) == 0 && nPort != 5500)
    {
      close(nSock);
      return nPort;
    }
  }

  close(nSock);

  return 0;
}


/* return config property from input */
std::string svGetConfigProperty (char * strIn)
{
  std::string strTemp;

  for (int i = 0; i < SV_MAX_PROP_LEN; i ++)
  {
    if (strIn[i] != '=' && strIn[i] != '\0' && strIn[i] != ' ')
      strTemp.push_back(strIn[i]);
    else
      return strTemp;
  }

  return "";
}


/* return config value from input */
std::string svGetConfigValue (char * strIn)
{
  std::string strTemp;
  int i = 0;

  // find first '=' character
  for (; i < SV_MAX_PROP_LEN; i ++)
  {
    if (strIn[i] == '=')
      break;
  }

  i ++;

  // read in value
  for (; i < SV_MAX_PROP_LEN; i ++)
  {
    if (strIn[i] != '\0')
      strTemp.push_back(strIn[i]);
    else
      return strTemp;
  }

  return "";
}


/*
  handle app options buttons
  (void * not used so parameter name removed)
*/
void svHandleAppOptionsButtons (Fl_Widget * widget, void *)
{
  Fl_Window * childWindow = app->childWindowBeingDisplayed;
  Fl_Button * btn = static_cast<Fl_Button *>(widget);

  if (childWindow == NULL || btn == NULL)
  {
    //childWindow->hide();
    app->childWindowVisible = false;
    app->childWindowBeingDisplayed = NULL;
    return;
  }

  // get the widget's name through user_data()
  const char * strCtlName = static_cast<char *>(widget->user_data());

  if (strCtlName == NULL)
    return;

  // cancel button clicked
  if (strcmp(strCtlName, "btnCancel") == 0)
  {
    childWindow->hide();
    app->childWindowVisible = false;
    app->childWindowBeingDisplayed = NULL;
  }

  // save button clicked
  if (strcmp(strCtlName, "btnSave") == 0)
  {
    Fl_Widget * wid = NULL;

    int nChildCount = childWindow->children();

    char * strTemp = NULL;
    std::string strName = strCtlName;

    for (int i = 0; i < nChildCount; i ++)
    {
      wid = childWindow->child(i);

      if (wid != NULL)
      {
        strTemp = static_cast<char *>(wid->user_data());

        if (strTemp == NULL || strTemp[0] == '\0')
          continue;

        strName = strTemp;

        if (strName == "")
          continue;

        if (strName == "spinScanTimeout")
          app->nScanTimeout = static_cast<Fl_Spinner *>(wid)->value();

        if (strName == "spinLocalSSHPort")
          app->nStartingLocalPort = static_cast<Fl_Spinner *>(wid)->value();

        if (strName == "spinDeadTimeout")
          app->nDeadTimeout = static_cast<Fl_Spinner *>(wid)->value();

        if (strName == "inAppFontSize")
          app->nAppFontSize = atoi(static_cast<SVInput *>(wid)->value());

        if (strName == "inListFont")
          app->strListFont = static_cast<SVInput *>(wid)->value();

        if (strName == "inListFontSize")
          app->nListFontSize = atoi(static_cast<SVInput *>(wid)->value());

        if (strName == "chkCBIcons")
        {
          if (static_cast<Fl_Check_Button *>(wid)->value() == 1)
            app->colorBlindIcons = true;
          else
            app->colorBlindIcons = false;

          svCreateAppIcons(true);
        }

        if (strName == "chkShowTooltips")
        {
          if (static_cast<Fl_Check_Button *>(wid)->value() == 1)
            app->showTooltips = true;
          else
            app->showTooltips = false;

          svSetUnsetMainWindowTooltips();
        }

        if (strName == "chkShowReverseConnect")
        {
          if (static_cast<Fl_Check_Button *>(wid)->value() == 1)
            app->showReverseConnect = true;
          else
            app->showReverseConnect = false;
        }

        if (strName == "chkDebugMode")
        {
          if (static_cast<Fl_Check_Button *>(wid)->value() == 1)
            app->debugMode = true;
          else
            app->debugMode = false;
        }
      }
    }

    Fl::redraw();
    childWindow->hide();
    app->childWindowVisible = false;
    app->childWindowBeingDisplayed = NULL;

    svConfigWrite();
  }
}


/*
  handle F8 buttons
  (void * not used so parameter name removed)
*/
void svHandleF8Buttons (Fl_Widget * widget, void *)
{
  Fl_Window * childWindow = app->childWindowBeingDisplayed;

  if (childWindow == NULL || widget == NULL)
  {
    //childWindow->hide();
    app->childWindowVisible = false;
    app->childWindowBeingDisplayed = NULL;
    return;
  }

  // get the widget's name through user_data()
  const char * strName = static_cast<char *>(widget->user_data());

  if (strName == NULL)
    return;

  VncObject * vnc = app->vncViewer->vnc;

  if (vnc != NULL)
  {
    // ctrl+alt+delete button clicked
    if (strcmp(strName, SV_F8_BTN_CAD) == 0)
    {
      SendKeyEvent(vnc->vncClient, XK_Control_L, true);
      SendKeyEvent(vnc->vncClient, XK_Alt_L, true);
      SendKeyEvent(vnc->vncClient, XK_Delete, true);

      SendKeyEvent(vnc->vncClient, XK_Control_L, false);
      SendKeyEvent(vnc->vncClient, XK_Alt_L, false);
      SendKeyEvent(vnc->vncClient, XK_Delete, false);
    }

    // ctrl+shift+esc button clicked
    if (strcmp(strName, SV_F8_BTN_CSE) == 0)
    {
      SendKeyEvent(vnc->vncClient, XK_Control_L, true);
      SendKeyEvent(vnc->vncClient, XK_Shift_L, true);
      SendKeyEvent(vnc->vncClient, XK_Escape, true);

      SendKeyEvent(vnc->vncClient, XK_Control_L, false);
      SendKeyEvent(vnc->vncClient, XK_Shift_L, false);
      SendKeyEvent(vnc->vncClient, XK_Escape, false);
    }

    // ask server for a screen refresh
    if (strcmp(strName, SV_F8_BTN_REFRESH) == 0)
      SendFramebufferUpdateRequest(vnc->vncClient, 0, 0,
        vnc->vncClient->width, vnc->vncClient->height, false);

    // send F8 key
    if (strcmp(strName, SV_F8_BTN_SEND_F8) == 0)
    {
      SendKeyEvent(vnc->vncClient, XK_F8, true);
      SendKeyEvent(vnc->vncClient, XK_F8, false);
    }

    // send F12 key
    if (strcmp(strName, SV_F8_BTN_SEND_F12) == 0)
    {
      SendKeyEvent(vnc->vncClient, XK_F12, true);
      SendKeyEvent(vnc->vncClient, XK_F12, false);
    }
  }

  Fl::redraw();
  childWindow->hide();
  app->childWindowVisible = false;
  app->childWindowBeingDisplayed = NULL;
}


/*
  handle host list button events
  (void * not used so parameter name removed)
*/
void svHandleHostListButtons (Fl_Widget * button, void *)
{
  Fl_Button * btn = static_cast<Fl_Button *>(button);

  if (btn == NULL)
    return;

  bool isSeparator;

  int nListVal = app->hostList->value();

  if (nListVal > 0 && strcmp(app->hostList->text(nListVal), "@C16@.?? ?? ??") == 0)
    isSeparator = true;
  else
    isSeparator = false;

  // add new item button
  if (btn == app->btnListAdd)
    svShowItemOptions(NULL);

  // delete current item button
  if (btn == app->btnListDelete && isSeparator == false)
  {
    fl_message_hotspot(0);
    fl_message_title("SpiritVNC - Delete Item");

    if (nListVal > 0)
      svDeleteItem(nListVal);
    else
      svMessageWindow("Nothing was selected so nothing was deleted",
        "SpiritVNC - Delete Item");
  }

  // show app options
  if (btn == app->btnListOptions)
    svShowAppOptions();

  // show About / Help
  if (btn == app->btnListHelp)
    svShowAboutHelp();

  // move item up button
  if (btn == app->btnListUp && isSeparator == false)
  {
    if (nListVal > 1)
    {
      app->hostList->swap(nListVal, nListVal - 1);
      app->hostList->select(nListVal - 1);
      app->hostList->redraw();
    }
  }

  // move item down button
  if (btn == app->btnListDown && isSeparator == false)
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
    if (app->scanIsRunning == true)
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
    HostItem * itm = NULL;
    VncObject * vnc = NULL;

    uint16_t nSize = app->hostList->size();

    // check the host list for other listening viewers
    for (uint16_t i = 0; i <= nSize; i ++)
    {
      itm = static_cast<HostItem *>(app->hostList->data(i));

      if (itm != NULL)
      {
        vnc = itm->vnc;

        if (vnc != NULL)
        {
          if (itm->isListener == true)
          {
            svMessageWindow("Only one active listening viewer is allowed");
            return;
          }
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

  if (itm == NULL)
    return;

  VncObject * vnc = itm->vnc;

  static bool menuUp;

  // *** DO *NOT* CHECK vnc FOR NULL HERE!!! ***
  // *** IT'S OKAY IF vnc IS NULL AT THIS POINT!!! ***

  // left mouse button
  if (Fl::event_button() == FL_LEFT_MOUSE)
  {
    // double-click
    if (Fl::event_clicks() != 0 && nHostItemNum > 0)
    {
      if (app->childWindowVisible == true)
        return;

      // start new connection
      if (itm->isConnected == false
        && itm->isConnecting == false
        && itm->isListener == false)
      {
        VncObject::hideMainViewer();
        VncObject::createVNCObject(itm);
      }

      return;
    }

    // single-click
    if (event == FL_WHEN_RELEASE_ALWAYS || event == FL_WHEN_RELEASE)
    {
      if (app->childWindowVisible == true)
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
    int nF12Flags;

    if (app->childWindowVisible == true)
      return;

    VncObject::hideMainViewer();

    // special visibility handling for listening item
    if (itm->isListener == true)
      if (vnc != NULL && itm->isConnected)
        vnc->setObjectVisible();

    // disconnect right single-clicked viewer
    if ((itm->isConnected == true || itm->isConnecting == true)
        && itm->hasDisconnectRequest == false
        && itm->isListener == false)
    {
      itm->hasDisconnectRequest = true;

      VncObject::endAndDeleteViewer(&vnc);

      return;
    }

    // show pop-up menu if disconnected and not a listening connection
    if (itm->isConnected == false
        && itm->isConnecting == false
        && itm->hasDisconnectRequest == false
        && itm->isListener == false
        && menuUp == false)
    {
      // include any error message in menu
      int nFlags = FL_MENU_INVISIBLE;
      char strError[FL_PATH_MAX] = {0};

      strncat(strError, itm->lastErrorMessage.c_str(), FL_PATH_MAX - 1);

      // enable / disable error message in menu
      if (itm->lastErrorMessage != "")
        nFlags = FL_MENU_INACTIVE | FL_MENU_DIVIDER;

      // enable / disable 'Copy F12 macro' item in menu
      if (itm->f12Macro == "")
        nF12Flags = FL_MENU_INACTIVE;
      else
        nF12Flags = 0;

      // create context menu
      // text,shortcut,callback,user_data,flags,labeltype,labelfont,labelsize
      const Fl_Menu_Item miMain[] = {
        {strError,         0, 0, 0, nFlags,    0, SV_FLTK_FONT_0, app->nMenuFontSize},
        {"Connect",        0, 0, 0, 0,         0, SV_FLTK_FONT_0, app->nMenuFontSize},
        {"Edit",           0, 0, 0, 0,         0, SV_FLTK_FONT_0, app->nMenuFontSize},
        {"Copy F12 macro", 0, 0, 0, nF12Flags, 0, SV_FLTK_FONT_0, app->nMenuFontSize},
        {"Delete...",      0, 0, 0, 0,         0, SV_FLTK_FONT_0, app->nMenuFontSize},
        {0}
      };

      // prevent re-entry (FLTK menu bug)
      menuUp = true;

      // show context menu and return selected item, if any
      const Fl_Menu_Item * miRes = miMain->popup(Fl::event_x() + 14, Fl::event_y() - 10);

      if (miRes != NULL)
      {
        const char * strRes = miRes->text;

        if (strRes != NULL)
        {
          // connect
          if (strcmp(strRes, "Connect") == 0)
            VncObject::createVNCObject(itm);

          // edit itm
          if (strcmp(strRes, "Edit") == 0)
            svShowItemOptions(itm);

          // copy this item's F12 macro to clipboard and app f12 macro variable
          if (strcmp(strRes, "Copy F12 macro") == 0)
          {
            Fl::copy(itm->f12Macro.c_str(), itm->f12Macro.size(), 1);
            app->strF12ClipVar = itm->f12Macro;
          }

          // delete item (and itm)
          if (strcmp(strRes, "Delete...") == 0)
            svDeleteItem(nHostItemNum);
        }
      }

      menuUp = false;
    }

    // show pop-up menu for reverse / listening connections
    if (itm->isListener == true && menuUp == false)
    {
      // prevent re-entry (FLTK menu bug)
      menuUp = true;

      // listener is not connected
      if (itm->isConnected == false
          && itm->isWaitingForShow == false)
      {
        // create context menu
        // text,shortcut,callback,user_data,flags,labeltype,labelfont,labelsize
        const Fl_Menu_Item miM[] = {
          {"Delete", 0, 0, 0, 0, 0, SV_FLTK_FONT_0, app->nMenuFontSize},
          {0}
        };

        // show context menu and return selected item, if any
        const Fl_Menu_Item * miRes = miM->popup(Fl::event_x() + 14, Fl::event_y() - 10);

        if (miRes != NULL)
        {
          const char * strRes = miRes->text;

          // delete item (and itm)
          if (strRes != NULL && strcmp(strRes, "Delete") == 0)
            svDeleteItem(nHostItemNum);
        }

        menuUp = false;
        return;
      }
      else
      {
        // listener is connected

        // enable / disable 'Paste F12 macro' item in menu
        if (app->strF12ClipVar == "")
          nF12Flags = FL_MENU_INACTIVE;
        else
          nF12Flags = 0;

        // create context menu
        // text,shortcut,callback,user_data,flags,labeltype,labelfont,labelsize
        const Fl_Menu_Item miMain[] = {
          {"Disconnect",      0, 0, 0, 0,         0, SV_FLTK_FONT_0, app->nMenuFontSize},
          {"Edit",            0, 0, 0, 0,         0, SV_FLTK_FONT_0, app->nMenuFontSize},
          {"Paste F12 macro", 0, 0, 0, nF12Flags, 0, SV_FLTK_FONT_0, app->nMenuFontSize},
          {0}
        };

        menuUp = true;

        // show context menu and return selected item, if any
        const Fl_Menu_Item * miRes = miMain->popup(Fl::event_x() + 14, Fl::event_y() - 10);

        if (miRes != NULL)
        {
          const char * strRes = miRes->text;

          if (strRes != NULL)
          {
            // disconnect
            if (strcmp(strRes, "Disconnect") == 0)
            {
              int iIndx = svItemNumFromItm(itm);

              VncObject::endAndDeleteViewer(&vnc);
              svDeleteItem(iIndx);

              menuUp = false;

              return;
            }

            // edit itm
            if (strcmp(strRes, "Edit") == 0)
              svShowItemOptions(itm);

            // 'paste' app-private F12 macro variable to listening item
            if (strcmp(strRes, "Paste F12 macro") == 0)
            {
              itm->f12Macro = app->strF12ClipVar;
              app->strF12ClipVar = "";
            }
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
void svHandleItmOptionsButtons (Fl_Widget * widget, void *)
{
  Fl_Window * childWindow = app->childWindowBeingDisplayed;
  HostItem * itm = app->itmBeingEdited;
  Fl_Button * btn = static_cast<Fl_Button *>(widget);

  if (itm == NULL || childWindow == NULL || btn == NULL)
  {
    app->childWindowVisible = false;
    app->childWindowBeingDisplayed = NULL;
    app->itmBeingEdited = NULL;
    return;
  }

  // get the widget's name through user_data()
  const char * strCtlName = static_cast<char *>(widget->user_data());

  if (strCtlName == NULL)
    return;

  // cancel button clicked
  if (strcmp(strCtlName, SV_ITM_BTN_CANCEL) == 0)
  {
    childWindow->hide();
    app->childWindowVisible = false;
    app->childWindowBeingDisplayed = NULL;
    app->itmBeingEdited = NULL;
  }

  // delete button clicked
  if (strcmp(strCtlName, SV_ITM_BTN_DEL) == 0)
  {
    fl_message_hotspot(0);
    fl_message_title("SpiritVNC - Delete Item");

    if (fl_choice("Are you sure you want to delete this?", "Cancel", "No", "Yes") == SV_CHOICE_2)
    {
      int nItem = svItemNumFromItm(itm);

      if (nItem > 0)
      {
        itm = NULL;
        app->hostList->remove(nItem);
        childWindow->hide();
        app->childWindowVisible = false;
        app->childWindowBeingDisplayed = NULL;
        app->itmBeingEdited = NULL;
      }
    }

    return;
  }

  // save button clicked
  if (strcmp(strCtlName, SV_ITM_BTN_SAVE) == 0)
  {
    Fl_Widget * wid = NULL;

    int nChildCount = childWindow->children();

    char * strTemp = NULL;
    std::string strName = strCtlName;

    // iterate through child controls and save their settings
    for (int i = 0; i < nChildCount; i ++)
    {
      wid = childWindow->child(i);

      if (wid != NULL)
      {
        strTemp = static_cast<char *>(wid->user_data());

        if (strTemp == NULL || strTemp[0] == '\0')
          continue;

        strName = strTemp;

        if (strName == "")
          continue;

        // connection name text input
        if (strName == SV_ITM_NAME)
          itm->name = static_cast<SVInput *>(wid)->value();

        // connection group text input
        if (strName == SV_ITM_GRP)
          itm->group = static_cast<SVInput *>(wid)->value();

        // connection address text input
        if (strName == SV_ITM_ADDRESS)
          itm->hostAddress = static_cast<SVInput *>(wid)->value();

        // f12 macro text input
        if (strName == SV_ITM_F12_MACRO)
          itm->f12Macro = static_cast<SVInput *>(wid)->value();

        // vnc connection radio button
        if (strName == SV_ITM_CON_VNC)
          if (static_cast<Fl_Radio_Round_Button *>(wid)->value() == 1)
            itm->hostType = 'v';

        // vnc-through-ssh radio button
        if (strName == SV_ITM_CON_SVNC)
          if (static_cast<Fl_Radio_Round_Button *>(wid)->value() == 1)
            itm->hostType = 's';

        // vnc port text input
        if (strName == SV_ITM_VNC_PORT)
          itm->vncPort = static_cast<SVInput *>(wid)->value();

        // vnc password secret input
        if (strName == SV_ITM_VNC_PASS)
          itm->vncPassword = static_cast<Fl_Secret_Input *>(wid)->value();

        // vnc compression level text input
        if (strName == SV_ITM_VNC_COMP)
        {
          itm->compressLevel = atoi(static_cast<SVInput *>(wid)->value());
          if (itm->compressLevel < 0)
            itm->compressLevel = 0;

          if (itm->compressLevel > 9)
            itm->compressLevel = 9;
        }

        // vnc quality level text input
        if (strName == SV_ITM_VNC_QUAL)
        {
          itm->qualityLevel = atoi(static_cast<SVInput *>(wid)->value());

          if (itm->qualityLevel < 0)
            itm->qualityLevel = 0;

          if (itm->qualityLevel > 9)
            itm->qualityLevel = 9;
        }

        // ignore inactive connection checkbutton
        if (strName == SV_ITM_IGN_DEAD)
        {
          if (static_cast<Fl_Check_Button *>(wid)->value() == 1)
            itm->ignoreInactive = true;
          else
            itm->ignoreInactive = false;
        }

        // scaling options group
        if (strName == SV_ITM_GRP_SCALE)
        {
          Fl_Group * grp = static_cast<Fl_Group *>(wid);
          int nGrpChildCount = grp->children();
          Fl_Widget * chld = NULL;

          // iterate through this group's children and save their settings
          for (uint8_t j = 0; j < nGrpChildCount; j ++)
          {
            chld = grp->child(j);

            char * strChildTemp = static_cast<char *>(chld->user_data());

            if (strChildTemp == NULL || strChildTemp[0] == '\0')
              continue;

            std::string strChildName = strChildTemp;

            if (strChildName == "")
              continue;

            // scroll only / no scaling radio button
            if (strChildName == SV_ITM_SCALE_OFF)
              if (static_cast<Fl_Radio_Round_Button *>(chld)->value() == 1)
                itm->scaling = 's';

            // zoom radio button
            if (strChildName == SV_ITM_SCALE_ZOOM)
              if (static_cast<Fl_Radio_Round_Button *>(chld)->value() == 1)
                itm->scaling = 'z';

            // fit radio button
            if (strChildName == SV_ITM_SCALE_FIT)
              if (static_cast<Fl_Radio_Round_Button *>(chld)->value() == 1)
                itm->scaling = 'f';

            // fast (jaggy) scaling checkbutton
            if (strChildName == SV_ITM_FAST_SCALE)
            {
              if (static_cast<Fl_Check_Button *>(chld)->value() == 1)
                itm->scalingFast = true;
              else
                itm->scalingFast = false;
            }
          }
        }

        // show remote cursor checkbutton
        if (strName == SV_ITM_SHW_REM_CURSOR)
        {
          if (static_cast<Fl_Check_Button *>(wid)->value() == 1)
            itm->showRemoteCursor = true;
          else
            itm->showRemoteCursor = false;
        }

        if (strName == SV_ITM_SSH_NAME)
          itm->sshUser = static_cast<SVInput *>(wid)->value();

        if (strName == SV_ITM_SSH_PASS)
          itm->sshPass = static_cast<SVInput *>(wid)->value();

        if (strName == SV_ITM_SSH_PORT)
          itm->sshPort = static_cast<SVInput *>(wid)->value();

        if (strName == SV_ITM_SSH_PRV_KEY)
          itm->sshKeyPrivate = static_cast<SVInput *>(wid)->value();
      }
    }

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

    childWindow->hide();
    app->childWindowVisible = false;
    app->childWindowBeingDisplayed = NULL;
    app->itmBeingEdited = NULL;

    svConfigWrite();
  }
}


/*
  sends new clipboard text to the currently displayed vnc host
  (void * not used so parameter name removed)
*/
void svHandleLocalClipboard (int source, void *)
{
  // don't process clipboard if there's no remote server being displayed
  // of it's the selection buffer
  if (app->vncViewer->vnc == NULL || source != 1)
    return;

  // if we received a cliboard event from the active server,
  // we don't want an event loop
  if (app->blockLocalClipboardHandling == true)
    return;

  app->blockLocalClipboardHandling = true;

  Fl::paste(*app->vncViewer, 1);

  app->blockLocalClipboardHandling = false;
}


/*
  handle main window close event
  (void * not used so parameter name removed)
*/
void svHandleMainWindowEvents (Fl_Widget * window, void *)
{
  int event = window->when();

  // window closing
  if (event == FL_LEAVE)
  {
    app->shuttingDown = true;

    VncObject::endAllViewers();

    svLogToFile("--- Program shutting down ---");

    // save main window position and size
    app->savedX = app->mainWin->x();
    app->savedY = app->mainWin->y();
    app->savedW = app->mainWin->w();
    app->savedH = app->mainWin->h();

    svConfigWrite();

    Fl::check();

    exit(0);
  }
}


/*
 * handle app and main window events, such as resize, move, etc
 * and resize gui elements
 */
void svPositionWidgets ()
{
  // only continue if main window geometry changes
  if (app->mainWin->w() != app->nMainWinPreviousW
      || app->mainWin->h() != app->nMainWinPreviousH)
  {
    svDebugLog("svPositionWidgets - Resizing GUI elements");

    // store window geometry for comparison later
    app->nMainWinPreviousW = app->mainWin->w();
    app->nMainWinPreviousH = app->mainWin->h();

    // resize the host list vertically if the main window resizes
    app->hostList->size(app->hostList->w(), app->mainWin->h() - 26);

    // set list buttons position
    app->packButtons->position(3, app->hostList->x() + app->hostList->h() + 3);

    // don't allow host list width to be smaller than right-most button's x+w
    if (app->hostList->w() < (app->packButtons->x() + app->packButtons->w()))
      app->hostList->size((app->packButtons->x() + app->packButtons->w()), app->hostList->h());

    // set scroller x
    app->scroller->position(app->hostList->x() + app->hostList->w() + 3, app->scroller->y());
    app->scroller->redraw();

    VncObject * vnc = app->vncViewer->vnc;

    // reset scroller position
    if (vnc != NULL)
    {
      svResizeScroller();
      vnc->setObjectVisible();
    }

    Fl::redraw();
    Fl::check();

    return;
  }
}


/*
  handle host item icon change
  (void * not used so parameter name removed)
*/
void svHandleListItemIconChange (void *)
{
  uint16_t nSize = app->hostList->size();
  HostItem * itm = NULL;

  // iterate through host list and set status icons for items
  for (uint16_t i = 0; i < nSize; i++)
  {
    itm = static_cast<HostItem *>(app->hostList->data(i));

    if (itm != NULL && itm->icon != NULL)
    {
      Fl::lock();
      app->hostList->icon(i, itm->icon);
      Fl::unlock();
      Fl::check();
    }
  }

  app->hostList->redraw();
}


/* handle messages from child threads */
void svHandleThreadConnection (void * data)
{
  HostItem * itm = static_cast<HostItem *>(data);

  if (itm == NULL)
    return;

  VncObject * vnc = itm->vnc;

  if (vnc == NULL)
    return;

  int nItem = svItemNumFromItm(itm);

  // set viewer as connected
  if (itm->isWaitingForShow == true)
  {
    svDebugLog("svConnectionWatcher - itm changing from"
      " 'isWaitingToShow' to 'isConnected'");

    itm->isWaitingForShow = false;

    app->nViewersWaiting --;

    // set host list item status icon
    itm->icon = app->iconConnected;
    svHandleListItemIconChange(NULL);

    svLogToFile("Connected to '" + itm->name + "' - " + itm->hostAddress);

    // show viewer if it matches the selected host list item
    uint16_t nSelectedHost = app->hostList->value();

    if (nItem == nSelectedHost && itm->isListener == false)
    {
      svDebugLog("svConnectionWatcher - Showing viewer because it's selected");
      vnc->setObjectVisible();
    }

    // append desktop name to this listener and create another listening viewer
    if (itm->isListener == true)
    {
      // add remote desktop's name to this Listening item
      int nListeningItem = svItemNumFromItm(itm);

      std::string strHostViewName = "Listening - ";
      strHostViewName.append(vnc->vncClient->desktopName);

      app->hostList->text(nListeningItem, strHostViewName.c_str());

      // (try to) create another listener
      svDebugLog("svConnectionWatcher - Creating Listener object");

      VncObject::createVNCListener();

      if (app->showReverseConnect == true)
      {
        svMessageWindow("A remote VNC host has just reverse-connected"
          "\n\nClick the 'Listen' item(s) in the host list to view");
        //app->mBar = new SVMessageBar("A remote VNC host has just reverse-connected"
          //"\n\nClick the 'Listen' item(s) in the host list to view");
        //svResizeScroller();
        //// app->scroller->add(app->mBar);
      }
    }
  }

  // set no connect icon
  if (itm->hasCouldntConnect == true)
  {
    svDebugLog("svConnectionWatcher - itm changing from 'hasCouldntConnect' to"
      " 'isConnected = false'");

    itm->isConnected = false;
    app->nViewersWaiting --;

    // set host list item status icon
    if (itm->lastErrorMessage != "")
      itm->icon = app->iconDisconnectedBigError;
    else
      itm->icon = app->iconNoConnect;

    svHandleListItemIconChange(NULL);

    if (itm->isListener == true)
    {
      app->hostList->remove(svItemNumFromItm(itm));
      svMessageWindow("Error: Unable to create a listening viewer at this time"
        "\n\nTry exiting the program, then restarting");
    }

    itm->vnc = NULL;
  }

  // advance and check viewer timeout value
  if (itm->isConnecting == true)
  {
    svDebugLog("svConnectionWatcher - itm still 'isConnecting'");

    vnc->waitTime ++;

    if (vnc->waitTime > app->nConnectionTimeout && itm->isListener == false)
    {
      svDebugLog("svConnectionWatcher - 'Soft' timeout reached, giving up");

      VncObject::endAndDeleteViewer(&vnc);

      app->nViewersWaiting --;

      // set host list item status icon
      itm->icon = app->iconNoConnect;
      svHandleListItemIconChange(NULL);

      // stop this thread because our 'soft' timeout was reached
      svDebugLog("svConnectionWatcher - Canceling itm->threadRFB");

      if (itm->threadRFB != 0)
        pthread_cancel(itm->threadRFB);

      svLogToFile("Could not connect to '" + itm->name + "' - " + itm->hostAddress);
    }
  }
}


/*
  handle thread cursor change
  (void * not used so parameter name removed)
*/
void svHandleThreadCursorChange (void *)
{
    if (app->vncViewer == NULL)
      return;

    VncObject * vnc = app->vncViewer->vnc;

    if (vnc == NULL)
      return;

    Fl::lock();

    // set cursor, if valid
    if (vnc->imgCursor != NULL)
    {
      app->mainWin->cursor(vnc->imgCursor, vnc->nCursorXHot, vnc->nCursorYHot);
      Fl::wait();
    } else
      app->mainWin->cursor(FL_CURSOR_DEFAULT);

    Fl::unlock();
}


/* create and insert empty listitem if no items were added at startup */
void svInsertEmptyItem ()
{
  // make a new itm
  HostItem * itm = new HostItem();
  itm->name = "(new connection)";
  itm->hostAddress = "192.168.0.1";
  itm->vncPort = "5900";
  itm->scaling = 'f';
  // itm->sshKeyPublic = "";
  itm->sshKeyPrivate = "";
  itm->showRemoteCursor = true;
  itm->compressLevel = 5;
  itm->qualityLevel = 5;

  Fl::lock();
  app->hostList->add(itm->name.c_str(), itm);
  app->hostList->icon(app->hostList->size(), app->iconDisconnected);
  app->hostList->make_visible(app->hostList->size());
  Fl::unlock();

  Fl::redraw();
  Fl::check();
}


/* return hostlist item (integer) that owns host item 'im' */
int svItemNumFromItm (const HostItem * itmIn)
{
  if (itmIn == NULL)
    return 0;

  HostItem * itm = NULL;
  uint16_t nSize = app->hostList->size();

  Fl::lock();

  // go through hostlist and find item owning matching itmIn
  for (uint16_t i = 0; i <= nSize; i ++)
  {
    itm = NULL;
    itm = static_cast<HostItem *>(app->hostList->data(i));

    if (itm != NULL && itm == itmIn)
    {
      Fl::unlock();

      return i;
    }
  }

  Fl::unlock();

  return 0;
}


/* return hostlist itm (HostItem) that owns vnc object 'v' */
HostItem * svItmFromVnc (const VncObject * vncIn)
{
  if (vncIn == NULL)
    return NULL;

  HostItem * itm = NULL;
  uint16_t nSize = app->hostList->size();

  Fl::lock();

  for (uint16_t i = 0; i <= nSize; i ++)
  {
      itm = NULL;
      itm = static_cast<HostItem *>(app->hostList->data(i));

      if (itm != NULL && itm->vnc != NULL)
      {
        if (itm->vnc == vncIn)
        {
          Fl::unlock();

          return itm;
        }
      }
  }

  Fl::unlock();

  return NULL;
}


/*
  handle itm option window choose prvkey button presses
  (Fl_Widget * not used so parameter name removed)
*/
void svItmOptionsChoosePrvKeyBtnCallback (Fl_Widget *, void * data)
{
  SVInput * inPrvKey = static_cast<SVInput *>(data);

  if (inPrvKey == NULL)
    return;

  char strHome[FL_PATH_MAX] = {0};

  fl_filename_expand(strHome, sizeof(strHome), "$HOME");

  const char * strFilename = fl_file_chooser("SpiritVNC - Please choose a private key file...",
    "*", strHome, 0);

  if (strFilename == NULL)
    return;

  inPrvKey->value(strFilename);
}


/*
  handle itm option window vnc radio button presses
  (void * not used so parameter name removed)
*/
void svItmOptionsRadioButtonsCallback (Fl_Widget * button, void *)
{
  Fl_Radio_Round_Button * btn = static_cast<Fl_Radio_Round_Button *>(button);

  if (btn == NULL)
    return;

  if (btn->value() == 1)
    btn->setonly();
}


/* send text to log file */
void svLogToFile (const std::string& strMessage)
{
  std::ofstream ofs;
  char logFileName[FL_PATH_MAX] = {0};
  char timeBuf[50] = {0};
  time_t logClock = 0;
  std::string strLineEnd;

  if (strMessage.size() == 0 || strMessage == "")
    return;

  // build time-stamp
  time(&logClock);
  strftime(timeBuf, 50, "%Y-%m-%d %X ", localtime(&logClock));

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
  ofs << timeBuf << "- " << strMessage << strLineEnd;

  ofs.close();
}


/* display a message dialog window */
void svMessageWindow (const std::string& strMessage, const std::string& strTitle)
{
  Fl::lock();
  fl_message_hotspot(0);
  fl_message_title(strTitle.c_str())  ;
  fl_message("%s", strMessage.c_str());
  Fl::unlock();
}


/* return number of connected items (integer) */
bool svThereAreConnectedItems ()
{
  int nSize = app->hostList->size();
  HostItem * itm = NULL;

  for (uint16_t i = 1; i <= nSize; i ++)
  {
    itm = static_cast<HostItem *>(app->hostList->data(i));

    if (itm != NULL)
    {
      if (itm->isConnected == true)
        return true;
    }
  }

  return false;
}


/* make sure scroller is sized correctly */
void svResizeScroller ()
{
  svDebugLog("svResizeScroller");

  if (app->scroller->x() != (app->hostList->w() + app->hostList->x() + 3))
    app->scroller->position((app->hostList->w() + app->hostList->x() + 3), app->scroller->y());

  if (app->scroller->w() != app->mainWin->w() - (app->hostList->w() + app->hostList->x() + 3))
    app->scroller->size(app->mainWin->w() - (app->hostList->w() + app->hostList->x() + 3),
      app->scroller->h());

  if (app->scroller->h() != app->mainWin->h())
    app->scroller->size(app->scroller->w(), app->mainWin->h());
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
  scan the host list for active connections and pause on each one
  for user-determined time interval
  (void * not used so parameter name removed)
*/
void svScanTimer (void *)
{
  if (app->scanIsRunning == false || svThereAreConnectedItems() == false)
  {
    app->scanIsRunning = false;
    app->nCurrentScanItem = 0;
    app->mainWin->label("SpiritVNC");
    app->btnListScan->image(new Fl_Pixmap(pmListScan));
    app->btnListScan->redraw();

    if (svThereAreConnectedItems() == false)
        svMessageWindow("No connected viewers. Scanning is now stopped", "SpiritVNC - Scan mode");

    return;
  }

  HostItem * itm = NULL;

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

    // hunt for a new viewer
    itm = static_cast<HostItem *>(app->hostList->data(app->nCurrentScanItem));

    if (itm == NULL)
      continue;

    if (itm->isConnected == true)
    {
      svDeselectAllItems();
      VncObject::hideMainViewer();
      app->hostList->select(app->nCurrentScanItem);
      itm->vnc->setObjectVisible();

      // 'tickle' host screen so it doesn't go to screensaver by
      // moving remote mouse back and forth a certain amount
      SendPointerEvent(itm->vnc->vncClient, 0, 0, 0);
      Fl::check();
      SendPointerEvent(itm->vnc->vncClient, 100, 100, 0);
      Fl::check();
      SendPointerEvent(itm->vnc->vncClient, 0, 0, 0);
      Fl::check();
      break;
    }
  }

  // call me again
  Fl::add_timeout(app->nScanTimeout, svScanTimer);
}


/* send a stored text string to the vnc host */
void svSendKeyStrokesToHost (std::string& strIn, VncObject * vnc)
{
  if (vnc == NULL)
    return;

  for (uint32_t i = 0;; i ++)
  {
    if (strIn[i] == '\0')
      return;

    if (strIn[i] != '\n')
    {
      SendKeyEvent(vnc->vncClient, strIn[i], true);
      SendKeyEvent(vnc->vncClient, strIn[i], false);
    }
  }
}


/* sets or unsets tooltips */
void svSetUnsetMainWindowTooltips ()
{
  if (app->showTooltips == true)
    app->btnListAdd->tooltip("Add a new connection");
  else
    app->btnListAdd->tooltip(NULL);

  if (app->showTooltips == true)
    app->btnListDelete->tooltip("Delete current connection");
  else
    app->btnListDelete->tooltip(NULL);

  if (app->showTooltips == true)
    app->btnListUp->tooltip("Move current connection item up in list");
  else
    app->btnListUp->tooltip(NULL);

  if (app->showTooltips == true)
    app->btnListDown->tooltip("Move current connection item down in list");
  else
    app->btnListDown->tooltip(NULL);

  if (app->showTooltips == true)
    app->btnListScan->tooltip("Scan through connected items in list");
  else
    app->btnListScan->tooltip(NULL);

  if (app->showTooltips == true)
    app->btnListListen->tooltip("Listen for incoming VNC connections");
  else
    app->btnListListen->tooltip(NULL);

  if (app->showTooltips == true)
    app->btnListHelp->tooltip("View About and Help information");
  else
    app->btnListHelp->tooltip(NULL);

  if (app->showTooltips == true)
    app->btnListOptions->tooltip("View / edit app options");
  else
    app->btnListOptions->tooltip(NULL);

  if (app->showTooltips == true)
    app->hostList->tooltip("Double-click a disconnected item to connect to it\n\n"
      "Right-click a connected item to disconnect from it\n\n"
      "Right-click a disconnected item to connect, edit or delete it");
  else
    app->hostList->tooltip(NULL);
}


/* show About / Help info */
void svShowAboutHelp ()
{
  // check to make sure no other child window is visible
  if (app->childWindowVisible == true)
    return;

  // set flag so no other child window will show
  app->childWindowVisible = true;

  // window size
  int nWinWidth = 700;
  int nWinHeight = 740;

  // set window position
  int nX = (app->mainWin->w() / 2) - (nWinWidth / 2);
  int nY = (app->mainWin->h() / 2) - (nWinHeight / 2);

  // create messagebox window
  Fl_Window * winAboutHelp = new Fl_Window(nX, nY, nWinWidth, nWinHeight, "About / Help"); //NULL);
  winAboutHelp->set_non_modal();
  //winAboutHelp->box(FL_GTK_UP_BOX);

  Fl_Help_View * hv = new Fl_Help_View(10, 10, nWinWidth - 20, nWinHeight - 70);

  hv->color(FL_BACKGROUND2_COLOR);
  hv->textsize(app->nAppFontSize);
  hv->textfont(0);

  const char strHelp[] = "<center><strong><h3>SpiritVNC"
      " - FLTK</strong></center></font>"
      "<p><center><font face='sans'>&copy; 2016-" SV_CURRENT_YEAR " Will Brokenbourgh - <a"
      " href='https://www.pismotek.com/brainout/'>"
      "https://www.pismotek.com/brainout/</a></font></center></p>"
      "<p><center><font face='sans'>Version " SV_APP_VERSION "<br>"
      "Built " __DATE__ "&nbsp;&bull;&nbsp;" __TIME__ "</font></center></p>"
      "<p><center><font face='sans'>SpiritVNC is a multi-viewer VNC client for *nix systems"
      " based on FLTK.  SpiritVNC features VNC-through-SSH, reverse VNC (listening)"
      " connections and automatic timed scanning of each connected viewer."
      "</center></font></p>"
      "<hr>"
      "<p><center><strong>Instructions</strong></center><br>"
      "<ul><li>To add a new connection, click the [+] button</li>"
      "<li>To delete a connection, click it one time, then click the [-] button</li>"
      "<li>To connect a connection, double-click it</li>"
      "<li>To disconnect a normal connection, right-click it while connected</li>"
      "<li>To Connect, Edit, Delete or copy the F12 macro of a connection, right-click the connection when"
      " not connected</li>"
      "<li>To move a connection up or down in the list, click the 'up' or 'down'"
      " arrow buttons</li>"
      "<li>To scan automatically through all connected viewers, click the 'clock' button</li>"
      "<li>To 'listen' for an incoming VNC connection, click the 'ear' button</li>"
      "<li>To disconnect or edit a 'listening' connection, right-click it while connected</li>"
      "<li>To change application options, click the settings button (looks like three control sliders)</li>"
      "<li>To perform remote actions, press F8 when a remote host is being displayed</li></ul>"
      "<hr>"
      "<p><center>Many thanks to the <em>FLTK</em> and <em>libvncserver</em>"
      " projects for their libraries and example code.</center></p>"
      "<p><center>&nbsp;If you have any questions, need to report a bug or have suggestions, please"
      " <a href='https://www.pismotek.com/brainout/content/spiritvnc.php'>visit the SpiritVNC page</a>"
      "&nbsp;</center></p>"
      "<p><center><strong>To God be the glory! :-D</strong></center></p>";

  // set helpview's text
  hv->value(strHelp);

  // 'OK' button
  Fl_Button * btnOK = new Fl_Button((nWinWidth - 100 - 10), nWinHeight - (35 + 10), 100, 35, "OK");
  btnOK->box(FL_GTK_UP_BOX);
  btnOK->shortcut(FL_Escape);
  btnOK->callback(svCloseChildWindow, winAboutHelp);

  // end adding stuff to the options window
  winAboutHelp->end();

  // make this window an embedded window in the parent
  //app->mainWin->add(winAboutHelp);

  // show window
  winAboutHelp->show();
  Fl::redraw();
}


/* show app options */
void svShowAppOptions ()
{
  // check to make sure no other child window is visible
  if (app->childWindowVisible == true)
    return;

  // set flag so no other child window will show
  app->childWindowVisible = true;

  // window size
  int nWinWidth = 650;
  int nWinHeight = 500;

  // set window position
  int nX = (app->mainWin->w() / 2) - (nWinWidth / 2);
  int nY = (app->mainWin->h() / 2) - (nWinHeight / 2);

  // create window
  Fl_Window * winAppOpts = new Fl_Window(nX, nY, nWinWidth, nWinHeight, "Application Options"); //NULL);
  winAppOpts->set_non_modal();
  //winAppOpts->box(FL_GTK_UP_BOX);
  app->childWindowBeingDisplayed = winAppOpts;

  //// add main top label, showing itm name
  //Fl_Box * bxTopLabel = new Fl_Box(0, 0, nWinWidth, 28, "Application Options");
  //bxTopLabel->align(FL_ALIGN_CENTER);
  //bxTopLabel->labelfont(1);
  //bxTopLabel->labelsize(app->nAppFontSize);
  //bxTopLabel->box(FL_GTK_UP_BOX);
  //bxTopLabel->color(17);

  // add itm value editors / selectors
  int nXPos = 300;
  int nYStep = 31;
  int nYPos = 20;

  // widgetName->user_data() is used to assign the widget's name to itself
  // so it can be easily handled in the callback

  // * general options *

  // scan viewer time-out
  Fl_Spinner * spinScanTimeout = new Fl_Spinner(nXPos, nYPos += nYStep,
    100, 28, "Scan wait time (seconds) ");
  spinScanTimeout->textsize(app->nAppFontSize);
  spinScanTimeout->labelsize(app->nAppFontSize);
  spinScanTimeout->step(1);
  spinScanTimeout->minimum(1);
  spinScanTimeout->maximum(200000);
  spinScanTimeout->user_data(SV_OPTS_SCN_TIMEOUT);
  spinScanTimeout->value(app->nScanTimeout);
  if (app->showTooltips == true)
    spinScanTimeout->tooltip("When scanning, this is how long SpiritVNC waits before moving"
      " to the next connected host item");

  // starting local ssh port number
  Fl_Spinner * spinLocalSSHPort = new Fl_Spinner(nXPos, nYPos += nYStep,
    100, 28, "Starting local SSH port number ");
  spinLocalSSHPort->textsize(app->nAppFontSize);
  spinLocalSSHPort->labelsize(app->nAppFontSize);
  spinLocalSSHPort->step(1);
  spinLocalSSHPort->minimum(1);
  spinLocalSSHPort->maximum(200000);
  spinLocalSSHPort->user_data(SV_OPTS_LOCAL_SSH_PORT);
  spinLocalSSHPort->value(app->nStartingLocalPort);
  if (app->showTooltips == true)
    spinLocalSSHPort->tooltip("This is the first SSH port used locally for VNC-over-SSH"
      " connections");

  // inactive connection timeout
  Fl_Spinner * spinDeadTimeout = new Fl_Spinner(nXPos, nYPos += nYStep,
    100, 28, "Inactive connection timeout (seconds) ");
  spinDeadTimeout->textsize(app->nAppFontSize);
  spinDeadTimeout->labelsize(app->nAppFontSize);
  spinDeadTimeout->step(1);
  spinDeadTimeout->minimum(1);
  spinDeadTimeout->maximum(200000);
  spinDeadTimeout->user_data(SV_OPTS_DEAD_TIMEOUT);
  spinDeadTimeout->value(app->nDeadTimeout);
  if (app->showTooltips == true)
    spinDeadTimeout->tooltip("This is the time, in seconds, SpiritVNC waits before"
      " disconnecting a remote host due to inactivity");


  Fl_Box * lblSep01 = new Fl_Box(nXPos, nYPos += nYStep + 14, 100, 28, "Appearance Options");
  lblSep01->labelsize(app->nAppFontSize);
  lblSep01->align(FL_ALIGN_CENTER);
  lblSep01->labelfont(1);

  // app font size
  SVInput * inAppFontSize = new SVInput(nXPos, nYPos += nYStep, 42, 28, "Application font size ");
  inAppFontSize->textsize(app->nAppFontSize);
  inAppFontSize->labelsize(app->nAppFontSize);
  inAppFontSize->user_data(SV_OPTS_APP_FONT_SIZE);
  char strTmp01[32] = {};
  snprintf(strTmp01, 32, "%i", app->nAppFontSize);
  inAppFontSize->value(strTmp01);
  if (app->showTooltips == true)
    inAppFontSize->tooltip("This is the font size used throughout SpiritVNC.  Restart the app to"
      " see any changes");

  // list font
  SVInput * inListFont = new SVInput(nXPos, nYPos += nYStep, 210, 28,
    "List font name (eg: Lucida Sans) ");
  inListFont->textsize(app->nAppFontSize);
  inListFont->labelsize(app->nAppFontSize);
  inListFont->user_data(SV_OPTS_LIST_FONT_NAME);
  inListFont->value(app->strListFont.c_str());
  if (app->showTooltips == true)
    inListFont->tooltip("This is the font used for the host list.  Restart the app to"
      " see any changes");

  // list font size
  SVInput * inListFontSize = new SVInput(nXPos + 260, nYPos, 42, 28, "Size:");
  inListFontSize->textsize(app->nAppFontSize);
  inListFontSize->labelsize(app->nAppFontSize);
  inListFontSize->user_data(SV_OPTS_LIST_FONT_SIZE);
  char strTmp02[32] = {};
  snprintf(strTmp02, 32, "%i", app->nListFontSize);
  inListFontSize->value(strTmp02);
  if (app->showTooltips == true)
    inListFontSize->tooltip("This is the font size used for the host list.  Restart the app to"
      " see any changes");

  // use color-blind icons?
  Fl_Check_Button * chkCBIcons = new Fl_Check_Button(nXPos, nYPos += nYStep, 210, 28,
    " Use icons for color-blind users");
  chkCBIcons->labelsize(app->nAppFontSize);
  chkCBIcons->user_data(SV_OPTS_USE_CB_ICONS);
  if (app->colorBlindIcons == true)
    chkCBIcons->set();
  if (app->showTooltips == true)
    chkCBIcons->tooltip("Check this to enable color-blind-friendly icons in the host list."
      " Restart the app to see any changes");

  // show tooltips?
  Fl_Check_Button * chkShowTooltips = new Fl_Check_Button(nXPos, nYPos += nYStep, 210, 28,
    " Show tooltips");
  chkShowTooltips->labelsize(app->nAppFontSize);
  chkShowTooltips->user_data(SV_OPTS_SHOW_TOOLTIPS);
  if (app->showTooltips == true)
  {
    chkShowTooltips->set();
    chkShowTooltips->tooltip("Check this to enable tooltips in SpiritVNC.");
  }

  // show reverse connection notification?
  Fl_Check_Button * chkShowReverseConnect = new Fl_Check_Button(nXPos, nYPos += nYStep, 210, 28,
    " Show reverse connection notification");
  chkShowReverseConnect->labelsize(app->nAppFontSize);
  chkShowReverseConnect->user_data(SV_OPTS_SHOW_REV_CON);
  if (app->showReverseConnect == true)
    chkShowReverseConnect->set();
  if (app->showTooltips == true)
    chkShowReverseConnect->tooltip("Check this to show a message window when a reverse"
      " connection happens.");

  nYPos += nYStep;

  Fl_Box * boxFontLabel = new Fl_Box(nXPos, nYPos += nYStep, 210, 28,
    "Restart SpiritVNC for font or icon changes");
  boxFontLabel->labelsize(app->nAppFontSize);
  boxFontLabel->align(FL_ALIGN_CENTER);
  boxFontLabel->labelfont(2);

  // ############ bottom buttons ##########################################################

  // 'Cancel' button
  Fl_Button * btnCancel = new Fl_Button((nWinWidth - 210 - 10), nWinHeight - (35 + 10), 100, 35,
    "Cancel");
  btnCancel->labelsize(app->nAppFontSize);
  btnCancel->box(FL_GTK_UP_BOX);
  btnCancel->shortcut(FL_Escape);
  btnCancel->user_data(SV_OPTS_CANCEL);
  btnCancel->callback(svHandleAppOptionsButtons);
  if (app->showTooltips == true)
    btnCancel->tooltip("Click to abandon any edits and close this window");

  // 'Save' button
  Fl_Button * btnSave = new Fl_Button((nWinWidth - 100 - 10), nWinHeight - (35 + 10), 100, 35,
    "Save");
  btnSave->labelsize(app->nAppFontSize);
  btnSave->box(FL_GTK_UP_BOX);
  btnSave->shortcut(FL_Enter);
  btnSave->user_data(SV_OPTS_SAVE);
  btnSave->callback(svHandleAppOptionsButtons);
  if (app->showTooltips == true)
    btnSave->tooltip("Click to save edits and close this window");

  // end adding stuff to the options window
  winAppOpts->end();

  // make this window an embedded window in the parent
  //app->mainWin->add(winAppOpts);

  // show window
  winAppOpts->show();

  // focus the first spinner control
  spinScanTimeout->take_focus();

  // silly code to select all characters in first spinner's text input
  Fl_Input_ * inTemp = static_cast<Fl_Input_ *>(spinScanTimeout->child(0));

  if (inTemp != NULL && inTemp->type() == FL_INT_INPUT)
    inTemp->position(0, 1000);

  Fl::redraw();
}


/* show F8 window */
void svShowF8Window ()
{
  // check to make sure no other child window is visible
  if (app->childWindowVisible == true)
    return;

  // set flag so no other child window will show
  app->childWindowVisible = true;

  // window size
  int nWinWidth = 230;
  int nWinHeight = 300;

  // set window position
  int nX = (app->mainWin->w() / 2) - (nWinWidth / 2);
  int nY = (app->mainWin->h() / 2) - (nWinHeight / 2);

  // create window
  Fl_Window * winF8 = new Fl_Window(nX, nY, nWinWidth, nWinHeight, "Remote host actions"); // NULL);
  winF8->set_non_modal();
  //winF8->box(FL_GTK_UP_BOX);
  app->childWindowBeingDisplayed = winF8;

  //// add main top label
  //Fl_Box * bxTopLabel = new Fl_Box(0, 0, nWinWidth, 28, "Remote host actions");
  //bxTopLabel->align(FL_ALIGN_CENTER);
  //bxTopLabel->labelfont(1);
  //bxTopLabel->box(FL_GTK_UP_BOX);
  //bxTopLabel->color(17);

  // add itm value editors / selectors
  int nXPos = 15;
  int nYStep = 40;
  int nYPos = 20;

  // widgetName->user_data() is used to assign the widget's name to itself
  // so it can be easily handled in the callback

  // =========================

  Fl_Button * btnCAD = new Fl_Button(nXPos, nYPos += 20, 200, 35, "Send Ctrl+Alt+Del");
  btnCAD->box(FL_GTK_UP_BOX);
  btnCAD->user_data(SV_F8_BTN_CAD);
  btnCAD->callback(svHandleF8Buttons);
  if (app->showTooltips == true)
    btnCAD->tooltip("Click to send Ctrl+Alt+Delete to the current remote host");

  Fl_Button * btnCSE = new Fl_Button(nXPos, nYPos += nYStep, 200, 35, "Send Ctrl+Shift+Esc");
  btnCSE->box(FL_GTK_UP_BOX);
  btnCSE->user_data(SV_F8_BTN_CSE);
  btnCSE->callback(svHandleF8Buttons);
  if (app->showTooltips == true)
    btnCSE->tooltip("Click to send Ctrl+Alt+Delete to the current remote host");

  Fl_Button * btnRefresh = new Fl_Button(nXPos, nYPos += nYStep, 200, 35, "Send refresh request");
  btnRefresh->box(FL_GTK_UP_BOX);
  btnRefresh->user_data(SV_F8_BTN_REFRESH);
  btnRefresh->callback(svHandleF8Buttons);
  if (app->showTooltips == true)
    btnRefresh->tooltip("Click to send a screen refresh request to the current remote host");

  Fl_Button * btnSendF8 = new Fl_Button(nXPos, nYPos += nYStep, 200, 35, "Send F8");
  btnSendF8->box(FL_GTK_UP_BOX);
  btnSendF8->user_data(SV_F8_BTN_SEND_F8);
  btnSendF8->callback(svHandleF8Buttons);
  if (app->showTooltips == true)
    btnSendF8->tooltip("Click to press the F8 key on the current remote host");

  Fl_Button * btnSendF12 = new Fl_Button(nXPos, nYPos += nYStep, 200, 35, "Send F12");
  btnSendF12->box(FL_GTK_UP_BOX);
  btnSendF12->user_data(SV_F8_BTN_SEND_F12);
  btnSendF12->callback(svHandleF8Buttons);
  if (app->showTooltips == true)
    btnSendF12->tooltip("Click to press the F12 key on the current remote host");

  // ############ bottom button ##########################################################

  // 'Close' button
  Fl_Button * btnCancel = new Fl_Button((nWinWidth - 110), nWinHeight - (35 + 10), 100, 35, "Close");
  btnCancel->box(FL_GTK_UP_BOX);
  btnCancel->shortcut(FL_Escape);
  btnCancel->user_data(SV_F8_BTN_CLOSE);
  btnCancel->callback(svHandleF8Buttons);
  if (app->showTooltips == true)
    btnCancel->tooltip("Click to close this window");

  // end adding stuff to the F8 window
  winF8->end();

  // make this window an embedded window in the parent
  //app->mainWin->add(winF8);

  // show window
  winF8->show();

  // focus the first spinner control
  btnCAD->take_focus();

  Fl::redraw();
}


/* create / show item options window */
void svShowItemOptions (HostItem * im)
{
  // check to make sure no other child window is visible
  if (app->childWindowVisible == true)
    return;

  // set flag so no other child window will show
  app->childWindowVisible = true;

  HostItem * itm = static_cast<HostItem *>(im);

  // make a new itm if we're passed a null one
  if (itm == NULL)
  {
    itm = new HostItem();
    itm->name = "(new connection)";
    itm->vncPort = "5900";
    itm->sshPort = "";
    itm->scaling = 's';
    // itm->sshKeyPublic = "";
    itm->sshKeyPrivate = "";
    itm->showRemoteCursor = true;
    itm->compressLevel = 5;
    itm->qualityLevel = 5;
    itm->vnc = NULL;
  }

  // window size
  int nWinWidth = 545;
  int nWinHeight = 750;

  // set window position
  int nX = app->hostList->w() + 50;
  int nY = (app->mainWin->h() / 2) - (nWinHeight / 2);

  // create window
  Fl_Window * itmOptWin = new Fl_Window(nX, nY, nWinWidth, nWinHeight, itm->name.c_str()); //NULL);
  itmOptWin->set_non_modal();
  //itmOptWin->box(FL_GTK_UP_BOX);

  //// add main top label, showing itm name
  //Fl_Box * bxTopLabel = new Fl_Box(0, 0, nWinWidth, 28, itm->name.c_str());
  //bxTopLabel->align(FL_ALIGN_CENTER);
  //bxTopLabel->labelfont(1);
  //bxTopLabel->box(FL_GTK_UP_BOX);
  //bxTopLabel->color(17);

  // add itm value editors / selectors
  int nXPos = 195;
  int nYStep = 28;
  int nYPos = 20;

  // widgetName->user_data() is used to assign the widget's name to itself
  // so it can be easily handled in the callbacks

  // * general options *

  // connection name
  SVInput * inName = new SVInput(nXPos, nYPos += nYStep, 210, 28, "Connection name ");
  inName->value(itm->name.c_str());
  inName->user_data(SV_ITM_NAME);
  if (app->showTooltips == true)
    inName->tooltip("The connection name as displayed in the host connection list");

  // connection group
  SVInput * inGroup = new SVInput(nXPos, nYPos += nYStep, 210, 28, "Connection group ");
  inGroup->value(itm->group.c_str());
  inGroup->user_data(SV_ITM_GRP);
  if (app->showTooltips == true)
    inGroup->tooltip("The group name this connection belongs to");

  // remote address
  SVInput * inAddress = new SVInput(nXPos, nYPos += nYStep, 210, 28, "Remote address ");
  inAddress->value(itm->hostAddress.c_str());
  inAddress->user_data(SV_ITM_ADDRESS);
  if (app->showTooltips == true)
    inAddress->tooltip("The IP address of the remote host you want to connect to."
      "  Do NOT include the VNC port number here");

  // f12 macro text
  SVInput * inF12Macro = new SVInput(nXPos, nYPos += nYStep, 210, 28, "F12 macro ");
  inF12Macro->value(itm->f12Macro.c_str());
  inF12Macro->user_data(SV_ITM_F12_MACRO);
  if (app->showTooltips == true)
    inF12Macro->tooltip("Key presses that are sent to the remote host when"
      " you press the F12 key");

  // * vnc type buttons *

  // vnc without ssh
  Fl_Radio_Round_Button * rbVNC = new Fl_Radio_Round_Button(nXPos, nYPos += nYStep, 100, 28, "VNC ");
  rbVNC->user_data(SV_ITM_CON_VNC);
  rbVNC->callback(svItmOptionsRadioButtonsCallback);
  if (app->showTooltips == true)
    rbVNC->tooltip("Choose this for VNC connections that don't tunnel through SSH");

  // vnc with ssh
  Fl_Radio_Round_Button * rbSVNC = new Fl_Radio_Round_Button(nXPos, nYPos += nYStep, 100, 28,
    "VNC through SSH ");
  rbSVNC->user_data(SV_ITM_CON_SVNC);
  rbSVNC->callback(svItmOptionsRadioButtonsCallback);
  if (app->showTooltips == true)
    rbSVNC->tooltip("Choose this for VNC connections that use SSH to tunnel to the host");

  // set pre-existing value
  if (itm->hostType == 'v')
    rbVNC->set();
  else
    rbSVNC->set();

  // vnc port
  SVInput * inVNCPort = new SVInput(nXPos, nYPos += nYStep, 100, 28, "VNC port ");
  inVNCPort->value(itm->vncPort.c_str());
  inVNCPort->user_data(SV_ITM_VNC_PORT);
  if (app->showTooltips == true)
    inVNCPort->tooltip("The VNC port/display number of the host.  Defaults to 5900");

  // vnc password (shows dots, not cleartext password)
  SVSecretInput * inVNCPassword = new SVSecretInput(nXPos, nYPos += nYStep, 210, 28,
    "VNC password ");
  inVNCPassword->value(itm->vncPassword.c_str());
  inVNCPassword->user_data(SV_ITM_VNC_PASS);
  if (app->showTooltips == true)
    inVNCPassword->tooltip("The VNC password for the host");

  // vnc compression level
  SVInput * inVNCCompressLevel = new SVInput(nXPos, nYPos += nYStep, 48, 28,
    "Compression level (0-9) ");
  char strCompress[15] = {0};
  sprintf(strCompress, "%i", itm->compressLevel);
  inVNCCompressLevel->value(strCompress);
  inVNCCompressLevel->user_data(SV_ITM_VNC_COMP);
  if (app->showTooltips == true)
    inVNCCompressLevel->tooltip("The level of compression, from 0 to 9");

  // vnc quality level
  SVInput * inVNCQualityLevel = new SVInput(nXPos, nYPos += nYStep, 48, 28, "Quality level (0-9) ");
  char strQuality[15] = {0};
  sprintf(strQuality, "%i", itm->qualityLevel);
  inVNCQualityLevel->value(strQuality);
  inVNCQualityLevel->user_data(SV_ITM_VNC_QUAL);
  if (app->showTooltips == true)
    inVNCQualityLevel->tooltip("The level of image quality, from 0 to 9");

  // ignore inactive connection checking
  Fl_Check_Button * chkIgnoreInactive = new Fl_Check_Button(nXPos, nYPos += nYStep, 100, 28,
    " Don't auto-disconnect when inactive");
  chkIgnoreInactive->user_data(SV_ITM_IGN_DEAD);
  if (app->showTooltips == true)
    chkIgnoreInactive->tooltip("Check to prevent auto-disconnect due to remote"
        " server inactivity");
  if (itm->ignoreInactive == true)
    chkIgnoreInactive->set();

  // * scaling options group *
  Fl_Group * grpScaling = new Fl_Group(nXPos, nYPos += nYStep, 300, 300);
  grpScaling->user_data(SV_ITM_GRP_SCALE);

  // scaling off - scroll only
  Fl_Radio_Round_Button * rbScaleOff = new Fl_Radio_Round_Button(nXPos, nYPos, 100, 28,
    " Scale off (scroll)");
  rbScaleOff->user_data(SV_ITM_SCALE_OFF);
  rbScaleOff->callback(svItmOptionsRadioButtonsCallback);
  if (app->showTooltips == true)
    rbScaleOff->tooltip("Choose this if you don't want any scaling."
      "  Scrollbars will appear for hosts with screens larger than the viewer");

  // scale up and down
  Fl_Radio_Round_Button * rbScaleZoom = new Fl_Radio_Round_Button(nXPos, nYPos += nYStep, 100, 28,
    " Scale up and down");
  rbScaleZoom->user_data(SV_ITM_SCALE_ZOOM);
  rbScaleZoom->callback(svItmOptionsRadioButtonsCallback);
  if (app->showTooltips == true)
    rbScaleZoom->tooltip("Choose this if you want small host screens scaled up to fit the"
      " viewer, or large host screens scaled down to fit the viewer");

  // scale down only
  Fl_Radio_Round_Button * rbScaleFit = new Fl_Radio_Round_Button(nXPos, nYPos += nYStep, 100, 28,
    " Scale down only");
  rbScaleFit->user_data(SV_ITM_SCALE_FIT);
  rbScaleFit->callback(svItmOptionsRadioButtonsCallback);
  if (app->showTooltips == true)
    rbScaleFit->tooltip("Choose this to scale down large host screens to fit the viewer but"
      " small host screens are not scaled up");

  // set pre-existing value
  if (itm->scaling == 's')
    rbScaleOff->set();
  else if (itm->scaling == 'z')
    rbScaleZoom->set();
  else if (itm->scaling == 'f')
    rbScaleFit->set();

  // fast scaling (instead of the default 'quality' scaling)
  Fl_Check_Button * chkScalingFast = new Fl_Check_Button(nXPos, nYPos += nYStep, 100, 28,
    " Fast scaling (low quality)");
  chkScalingFast->user_data(SV_ITM_FAST_SCALE);
  if (itm->scalingFast == true)
    chkScalingFast->set();
  if (app->showTooltips == true)
    chkScalingFast->tooltip("Check to select fast scaling instead of quality scaling");

  // end of scaling group
  grpScaling->end();

  // show the host's native cursor under our local static cursor
  Fl_Check_Button * chkShowRemoteCursor = new Fl_Check_Button(nXPos, nYPos += nYStep, 100, 28,
    " Use remote cursor locally");
  chkShowRemoteCursor->user_data(SV_ITM_SHW_REM_CURSOR);
  if (app->showTooltips == true)
    chkShowRemoteCursor->tooltip("Check to show remote locally");

  // set pre-existing value
  if (itm->showRemoteCursor == true)
    chkShowRemoteCursor->set();

  // * vnc over ssh options *

  // separate these values a little from above controls
  nYPos += 15;

  // section label
  Fl_Box * bxSSHSection = new Fl_Box(nXPos, nYPos += nYStep, 210, 28, "VNC through SSH options");
  bxSSHSection->labelfont(1);
  bxSSHSection->user_data(SV_ITM_GRP_SSH);

  // name used for ssh login
  SVInput * inSSHName = new SVInput(nXPos, nYPos += nYStep, 210, 28, "SSH user name ");
  inSSHName->value(itm->sshUser.c_str());
  inSSHName->user_data(SV_ITM_SSH_NAME);
  if (app->showTooltips == true)
    inSSHName->tooltip("The SSH user name for the host");

  // password used for ssh login (if used)
  SVSecretInput * inSSHPassword = new SVSecretInput(nXPos, nYPos += nYStep, 210, 28,
    "SSH password (if any) ");
  inSSHPassword->value(itm->sshPass.c_str());
  inSSHPassword->user_data(SV_ITM_SSH_PASS);
  if (app->showTooltips == true)
    inSSHPassword->tooltip("The SSH password for the host (usually not necessary when using"
      " key files)");

  // ssh port (on the remote host)
  SVInput * inSSHPort = new SVInput(nXPos, nYPos += nYStep, 100, 28, "SSH remote port ");
  inSSHPort->value(itm->sshPort.c_str());
  inSSHPort->user_data(SV_ITM_SSH_PORT);
  if (app->showTooltips == true)
    inSSHPort->tooltip("The remote host's port number");

  // ssh private key full path (if used)
  SVInput * inSSHPrvKey = new SVInput(nXPos, nYPos += nYStep, 300, 28, "SSH private key (if any) ");
  inSSHPrvKey->value(itm->sshKeyPrivate.c_str());
  inSSHPrvKey->user_data(SV_ITM_SSH_PRV_KEY);
  if (app->showTooltips == true)
    inSSHPrvKey->tooltip("The SSH private key file location (usually not"
      " necessary when using a SSH password");

  // button to select ssh private key
  Fl_Button * btnShowPrvKeyChooser = new Fl_Button(nXPos + 300 + 2, nYPos + 4, 20, 20, "...");
  btnShowPrvKeyChooser->callback(svItmOptionsChoosePrvKeyBtnCallback, inSSHPrvKey);
  if (app->showTooltips == true)
    btnShowPrvKeyChooser->tooltip("Click to choose a SSH private key file");

  // set app vars
  app->childWindowBeingDisplayed = itmOptWin;
  app->itmBeingEdited = itm;

  // 'Delete' button
  Fl_Button * btnDel = new Fl_Button(10, nWinHeight - (35 + 10), 100, 35, "Delete");
  btnDel->box(FL_GTK_UP_BOX);
  btnDel->user_data(SV_ITM_BTN_DEL);
  btnDel->callback(svHandleItmOptionsButtons);
  if (app->showTooltips == true)
    btnDel->tooltip("Click to permanently delete this host connection"
      " (not undoable).  You will be asked to confirm before item is deleted");

  // 'Cancel' button
  Fl_Button * btnCancel = new Fl_Button((nWinWidth - 210 - 10), nWinHeight - (35 + 10), 100, 35,
    "Cancel");
  btnCancel->box(FL_GTK_UP_BOX);
  btnCancel->shortcut(FL_Escape);
  btnCancel->user_data(SV_ITM_BTN_CANCEL);
  btnCancel->callback(svHandleItmOptionsButtons);
  if (app->showTooltips == true)
    btnCancel->tooltip("Click to abandon any edits to this connection"
      " and close this window");

  // 'Save' button
  Fl_Button * btnSave = new Fl_Button((nWinWidth - 100 - 10), nWinHeight - (35 + 10), 100, 35,
    "Save");
  btnSave->box(FL_GTK_UP_BOX);
  btnSave->shortcut(FL_Enter);
  btnSave->user_data(SV_ITM_BTN_SAVE);
  btnSave->callback(svHandleItmOptionsButtons);
  if (app->showTooltips == true)
    btnSave->tooltip("Click to save edits made to this connection and close this window");

  // end adding stuff to the options window
  itmOptWin->end();

  // disable irrelevant things for listening connections
  if (itm->isListener)
  {
    inName->deactivate();
    inGroup->deactivate();
    inAddress->deactivate();
    rbVNC->deactivate();
    rbSVNC->deactivate();
    inVNCPort->deactivate();
    inVNCPassword->deactivate();
    bxSSHSection->deactivate();
    inSSHName->deactivate();
    inSSHPassword->deactivate();
    inSSHPort->deactivate();
    inSSHPrvKey->deactivate();
    btnShowPrvKeyChooser->deactivate();
    btnDel->deactivate();
  }
  else
  {
    inName->activate();
    inGroup->activate();
    inAddress->activate();
    rbVNC->activate();
    rbSVNC->activate();
    inVNCPort->activate();
    inVNCPassword->activate();
    bxSSHSection->activate();
    inSSHName->activate();
    inSSHPassword->activate();
    inSSHPort->activate();
    inSSHPrvKey->activate();
    btnShowPrvKeyChooser->activate();
    btnDel->activate();
  }

  // make this window an embedded window in the parent
  //app->mainWin->add(itmOptWin);

  // focus the first input box and select all text within
  inName->take_focus();
  inName->position(0, strlen(inName->value()) + 1);

  itmOptWin->show();
  Fl::redraw();
}


/* update text on all host items */
void svUpdateHostListItemText ()
{
  HostItem * itm = NULL;
  uint16_t nSize = app->hostList->size();

  for (uint16_t i = 0; i <= nSize; i ++)
  {
    itm = static_cast<HostItem *>(app->hostList->data(i));

    if (itm != NULL)
      app->hostList->text(i, strdup(itm->name.c_str()));
  }
}
