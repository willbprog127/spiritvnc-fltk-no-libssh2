/*
 * app.cxx - part of SpiritVNC - FLTK
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


#include "app.h"

/*  app options controls  */
struct AppOptionsControls
{
  Fl_Spinner * spinScanTimeout;
  Fl_Spinner * spinLocalSSHPort;
  SVInput * inSSHCommand;
  Fl_Check_Button * chkLogToFile;
  SVIntInput * inAppFontSize;
  SVInput * inListFont;
  SVIntInput * inListFontSize;
  SVIntInput * inListWidth;
  Fl_Check_Button * chkCBIcons;
  Fl_Check_Button * chkShowTooltips;
  Fl_Check_Button * chkShowReverseConnect;
  Fl_Button * btnCancel;
  Fl_Button * btnSave;
} AppOpts;

/*  F8 window controls  */
struct F8Controls
{
  Fl_Button * btnCAD;
  Fl_Button * btnCSE;
  Fl_Button * btnRefresh;
  Fl_Button * btnSendF8;
  Fl_Button * btnSendF11;
  Fl_Button * btnSendF12;
  Fl_Button * btnCancel;
} F8Opts;

/*  itm option controls  */
struct ItemOptionsControls
{
  SVInput * inName;
  SVInput * inGroup;
  SVInput * inAddress;
  SVInput * inF12Macro;
  Fl_Radio_Round_Button * rbVNC;
  Fl_Radio_Round_Button * rbSVNC;
  SVIntInput * inVNCPort;
  SVSecretInput * inVNCPassword;
  SVInput * inVNCLoginUser;
  SVSecretInput * inVNCLoginPassword;
  SVIntInput * inVNCCompressLevel;
  SVIntInput * inVNCQualityLevel;
  Fl_Group * grpScaling;
  Fl_Radio_Round_Button * rbScaleOff;
  Fl_Radio_Round_Button * rbScaleZoom;
  Fl_Radio_Round_Button * rbScaleFit;
  Fl_Check_Button * chkScalingFast;
  Fl_Check_Button * chkShowRemoteCursor;
  Fl_Check_Button * chkCommand1Enabled;
  SVInput * inCommand1Label;
  SVInput * inCommand1;
  Fl_Check_Button * chkCommand2Enabled;
  SVInput * inCommand2Label;
  SVInput * inCommand2;
  Fl_Check_Button * chkCommand3Enabled;
  SVInput * inCommand3Label;
  SVInput * inCommand3;
  Fl_Box * bxSSHSection;
  SVInput * inSSHName;
  //SVSecretInput * inSSHPassword;
  SVIntInput * inSSHPort;
  SVInput * inSSHPrvKey;
  Fl_Button * btnShowPrvKeyChooser;
  // 'Delete' button
  Fl_Button * btnDel;
  Fl_Button * btnCancel;
  Fl_Button * btnSave;
} ItmOpts;


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

  return Fl_Pack::handle(evt);
}


/*
  handle method for SVQuickNoteInput
  (instance method)
*/
int SVQuickNoteInput::handle (int evt)
{
  // handle enter key - save edits
  if (evt == FL_KEYDOWN && Fl::event_key() == FL_Enter)
  {
    // get currently selected list item
    int curLine = app->hostList->value();

    if (curLine > 0)
    {
      HostItem * itm = static_cast<HostItem *>(app->hostList->data(curLine));

      if (itm != NULL)
      {
        // set quickNote text
        itm->quickNote = std::string(app->quickNoteInput->value()).substr(0, 256);

        // clear the quickNote input and hide the quickNote input window
        svHideQuickNoteEditWidgets();

        // set quick note label and note text
        svQuickInfoSetLabelAndText(itm);
        svConfigWrite();
      }
    }
    return 1;
  }

  // escape key pressed -- abandon edits
  if (evt == FL_SHORTCUT && Fl::event_key() == FL_Escape)
  {
    svHideQuickNoteEditWidgets();
    return 1;
  }

  // we will accept focus by returning non-zero
  if (evt == FL_FOCUS || evt == FL_UNFOCUS)
    return 1;

  return SVInput::handle(evt);
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
  if (evt == FL_PUSH && curLine > 0 && app->scanIsRunning == false)
  {
  // show the editor if there's a selected item
    HostItem * itm = static_cast<HostItem *>(app->hostList->data(curLine));

    if (itm != NULL && itm->name != "")
    {
      app->quickNoteInput->value(itm->quickNote.c_str());

      // show editor stuff
      app->quickNotePack->position(app->quickNoteBox->x(), app->quickNoteBox->y());

      // show the quick note edit widgets (hopefully)
      app->quickNotePack->show();
      Fl::check();
      app->quickNotePack->redraw();
      Fl::redraw();

      // set cursor at top left
      if (app->quickNoteInput->take_focus())
        app->quickNoteInput->insert_position(0);

      Fl::add_timeout(SV_BLINK_TIME, svBlinkCursor, app->quickNoteInput);
    }
  }

  return Fl_Multiline_Output::handle(evt);
}


/*  emulates cursor blinking  */
void svBlinkCursor (void * inp)
{
  // if the passed input widget is null, cancel the timeout and get out
  if (inp == NULL)
  {
    Fl::remove_timeout(svBlinkCursor);
    return;
  }

  // cast to fl_input_ from void *
  Fl_Input_ * input = static_cast<Fl_Input_ *>(inp);

  // static blink state for alternating change of cursor color
  static bool blinkState;

  // turn the cursor a different color based on blink state
  if (blinkState == true)
  {
    blinkState = false;
    input->cursor_color(FL_BLACK);
    input->redraw();
  }
  else
  {
    blinkState = true;
    input->cursor_color(FL_BACKGROUND_COLOR);
    input->redraw();
  }

  // keep repeating the timeout until it's removed
  Fl::repeat_timeout(SV_BLINK_TIME, svBlinkCursor, inp);
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


/*
  read from the config file, set app
  options and populate host list
*/
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
    svConfigCreateNewDir();
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
          app->requestedListWidth = atoi(strVal.c_str());

          if (app->requestedListWidth < 10)
            app->requestedListWidth = 10;
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

          if (app->sshCommand == "")
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

        // display debug messages?
        if (strProp == "debugmode")
          app->debugMode = svConvertStringToBoolean(strVal);

        // app font size
        if (strProp == "appfontsize")
        {
          app->nAppFontSize = atoi(strVal.c_str());

          if (app->nAppFontSize < 1)
            // windows
            #ifdef _WIN32
            app->nAppFontSize = 10;
            // *nix-like
            #else
            app->nAppFontSize = 12;
            #endif
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
            // windows
            #ifdef _WIN32
            app->nListFontSize = 10;
            // *nix-like
            #else
            app->nListFontSize = 12;
            #endif
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
          itm->vncPassword = base64Decode(strVal);

        // vnc login user (credential authentication)
        if (strProp == "vncloginuser")
          itm->vncLoginUser = strVal;

        // vnc login password (credential authentication)
        if (strProp == "vncloginpass")
          itm->vncLoginPassword = base64Decode(strVal);

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
      if (itm != NULL && numOfEntries < SV_MAX_HOSTLIST_ENTRIES)
        app->hostList->add(itm->name.c_str(), itm);

      // add a separator
      if (addSep == true)
        // color 16 (@C16.) is supposed to be gray
        app->hostList->add("@C16@.· · ·");

      break;
    }
  }

  ifs.close();
}


/*  write config file  */
void svConfigWrite ()
{
  static bool inConfigWrite;

  if (inConfigWrite == true)
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

  // blank line
  ofs << std::endl;

  // host list entries
  HostItem * itm = NULL;

  ofs << "# host-list entries" << std::endl;

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
    ofs << "vncloginuser=" << itm->vncLoginUser << std::endl;
    ofs << "vncloginpass=" << base64Encode(reinterpret_cast<const unsigned char *>
      (itm->vncLoginPassword.c_str()), itm->vncLoginPassword.size()) << std::endl;
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
  HostItem * itm = NULL;
  VncObject * vnc = NULL;

  // only check if there are waiting viewers
  if (app->nViewersWaiting > 0)
  {
    svDebugLog("svConnectionWatcher - At least one itm ready for processing");

    uint16_t nSize = app->hostList->size();

    // iterate through hostlist items
    for (uint16_t i = 0; i <= nSize; i ++)
    {
      itm = static_cast<HostItem *>(app->hostList->data(i));

      if (itm == NULL)
        continue;

      vnc = itm->vnc;

      if (vnc == NULL)
        continue;

      // if ssh connection faltered, shut down the vnc viewer
      if (itm->isConnected == true && (itm->hostType == 's' && itm->sshReady == false))
      {
        app->nViewersWaiting --;
        svDebugLog("svConnectionWatcher - SSH problem during connection, ending");

        itm->vnc->endViewer();
      }

      // cleanup vnc client structure and delete vnc object
      if (itm->vncNeedsCleanup == true)
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
  if (boolIn == true)
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

  if (strOut == "true" || strOut == "yes"
      || strOut == "on" || strOut == "1")
    return true;

  return false;
}


/*  create icons for app  */
void svCreateAppIcons (const bool fromAppOptions)
{
  // do app icon first
  app->iconApp = new Fl_RGB_Image(new Fl_Pixmap(pmSpiritvnc_xpm));
  app->mainWin->default_icon(app->iconApp);

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

  // set appropriate list icons only when starting up
  if (fromAppOptions == false)
  {
    // set initial icons on hostlist items
    for (uint16_t i = 0; i <= app->hostList->size(); i ++)
      if (app->hostList->data(i) != NULL)
        app->hostList->icon(i, app->iconDisconnected);

    app->hostList->redraw();
  }
}


/*  create program GUI  */
void svCreateGUI ()
{
  // widget tooltips are set in svSetTooltips
  // some additional positioning and sizing is done in svPositionWidgets

  // create main window
  app->mainWin = new Fl_Double_Window(1024, 768);
  app->mainWin->size_range(800, 600, 32767, 32767);
  app->mainWin->label("SpiritVNC");
  app->mainWin->xclass("spiritvncfltk");
  app->mainWin->callback(svHandleMainWindowEvents);

  // explicitly start adding widgets to the app window
  app->mainWin->begin();

  // create host list
  app->hostList = new Fl_Hold_Browser(0, 0, 163, 548);
  app->hostList->clear_visible_focus();
  app->hostList->callback(svHandleHostListEvents, NULL);
  app->hostList->box(FL_THIN_DOWN_BOX);

  // =============== quick info start =======================

  // quick info pack
  app->quickInfoPack = new Fl_Pack(0, 551, 163, 197);
  app->quickInfoPack->type(Fl_Pack::VERTICAL);

  // item name label
  app->quickInfoLabel = new Fl_Box(0, 0, 163, 18);
  app->quickInfoLabel->labelsize(app->nAppFontSize + 2);
  app->quickInfoLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  app->quickInfoLabel->labelfont(FL_HELVETICA);
  app->quickInfoLabel->labelcolor(fl_rgb_color(SV_QUICK_INFO_FG_COLOR));

  // last connected label
  app->lastConnectedLabel = new Fl_Box(0, 0, 163, 18);
  app->lastConnectedLabel->labelsize(app->nAppFontSize + 1);
  app->lastConnectedLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_BOTTOM);
  app->lastConnectedLabel->labelcolor(fl_rgb_color(SV_QUICK_INFO_FG_COLOR));

  // last connected
  app->lastConnected = new Fl_Box(0, 0, 163, 18);
  app->lastConnected->labelsize(app->nAppFontSize + 1);
  app->lastConnected->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_TOP);
  app->lastConnected->labelcolor(fl_rgb_color(SV_QUICK_INFO_FG_COLOR));

  // last error message
  app->lastErrorBox = new Fl_Multiline_Output(0, 0, 163, 45);
  app->lastErrorBox->textsize((app->nAppFontSize + 1));
  app->lastErrorBox->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE);
  app->lastErrorBox->box(FL_THIN_DOWN_BOX);
  app->lastErrorBox->wrap(1);
  app->lastErrorBox->textcolor(FL_DARK_RED);
  app->lastErrorBox->readonly(1);
  app->lastErrorBox->clear_visible_focus();

  // note - very brief item info
  app->quickNoteBox = new SVQuickNoteBox(0, 0, 163, 190);
  app->quickNoteBox->textsize((app->nAppFontSize + 2));
  app->quickNoteBox->textfont(FL_HELVETICA_ITALIC);
  app->quickNoteBox->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE);
  app->quickNoteBox->box(FL_THIN_DOWN_BOX);
  app->quickNoteBox->textcolor(fl_rgb_color(SV_QUICK_INFO_FG_COLOR));
  app->quickNoteBox->wrap(1);
  app->quickNoteBox->readonly(1);
  app->quickNoteBox->clear_visible_focus();

  app->quickInfoPack->end();

  // set quick info text to empty defaults
  svQuickInfoSetToEmpty();
  // =============== quick info end =======================

  // =============== host list buttons start ====================
  // button size constant
  const u_int nBtnSize = 20;

  app->packButtons = new Fl_Pack(0, 747, 170, nBtnSize);
  app->packButtons->type(Fl_Pack::HORIZONTAL);

  app->packButtons->begin();

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

  // create scrolling window we will add viewers to
  app->scroller = new Fl_Scroll(0, 0, 0, 0);
  app->vncViewer = new VncViewer(0, 0, 0, 0);
  app->scroller->box(FL_FLAT_BOX);
  app->scroller->type(0);
  app->scroller->end();

  app->mainWin->end();

  // set resizable widget for whole app
  app->mainWin->resizable(app->scroller);
}


/* create widgets for quick note editing */
void svCreateQuickNoteEditWidgets ()
{
  // quick note parent container (fl_pack)
  app->quickNotePack = new SVQuickNotePack(app->quickNoteBox->x(), app->quickNoteBox->y(),
    app->quickNoteBox->w(), app->quickNoteBox->h() - 3);

  // quick note multi-line wrapped input widget
  app->quickNoteInput = new SVQuickNoteInput(0, 0, app->quickNotePack->w(), app->quickNotePack->h());
  app->quickNoteInput->type(FL_MULTILINE_INPUT_WRAP);
  app->quickNoteInput->textsize(app->nAppFontSize + 2);
  app->quickNoteInput->color(FL_WHITE);
  app->quickNoteInput->box(FL_THIN_DOWN_BOX);

  app->quickNotePack->end();

  // make sure quick note edit widgets are initially hidden
  svHideQuickNoteEditWidgets();

  app->mainWin->add(app->quickNotePack);

  // set quick info text to empty defaults
  svQuickInfoSetToEmpty();
}


/* log debug messages, if enabled in config file */
void svDebugLog (const std::string& strDebugMessage)
{
  if (app->debugMode == true)
    svLogToFile("DEBUG - " + strDebugMessage);
}


/* show confirmation and delete item from hostList */
void svDeleteItem (const int nItem)
{
  static bool inDeleteItem;

  bool okayToDelete = false;

  // prevent re-entry (FLTK menu bug)
  if (inDeleteItem == true)
    return;
  else
    inDeleteItem = true;

  const HostItem * itm = static_cast<HostItem *>(app->hostList->data(nItem));

  // null itm, get out
  if (itm == NULL)
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

    fl_message_hotspot(0);
    fl_message_title("SpiritVNC - Delete Item");

    if (fl_choice("%s", "Cancel", "No", "Yes", strConfirm.c_str()) == SV_CHOICE_BTN_3)
      okayToDelete = true;
  }

  // delete itm if everything is okay
  if (okayToDelete == true)
  {
    app->hostList->remove(nItem);
    app->hostList->redraw();
    svQuickInfoSetToEmpty();
  }

  inDeleteItem = false;
}


/*  sets all host list items to deselected  */
void svDeselectAllItems ()
{
  uint16_t nSize = app->hostList->size();

  // clear list of 'selected' color
  for (uint16_t i = 1; i <= nSize; i ++)
    app->hostList->select(i, 0);
}


/*  enable or disable tooltips  */
void svEnableDisableTooltips ()
{
  // enable or disable ALL app tooltips
  if (app->showTooltips == true)
    Fl_Tooltip::enable();
  else
    Fl_Tooltip::disable();
}


/*  find unused TCP port in a range  */
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


/*  return config value from input  */
std::string svGetConfigValue (const char * strIn)
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
    app->childWindowVisible = false;
    app->childWindowBeingDisplayed = NULL;
    return;
  }

  // cancel button clicked
  if (btn == AppOpts.btnCancel)
  {
    childWindow->hide();
    app->childWindowVisible = false;
    app->childWindowBeingDisplayed = NULL;
  }

  // save button clicked
  if (btn == AppOpts.btnSave)
  {
    // scan timeout spinner
    app->nScanTimeout = AppOpts.spinScanTimeout->value();

    // local ssh start port number spinner
    app->nStartingLocalPort = AppOpts.spinLocalSSHPort->value();

    // ssh command input
    app->sshCommand = AppOpts.inSSHCommand->value();

    // app font size input
    app->nAppFontSize = atoi(AppOpts.inAppFontSize->value());

    // hostlist font name input
    app->strListFont = AppOpts.inListFont->value();

    // hostlist font size input
    app->nListFontSize = atoi(AppOpts.inListFontSize->value());

    // hostlist requested width input
    app->requestedListWidth = atoi(AppOpts.inListWidth->value());
    svPositionWidgets();

    // user color-blind icons checkbutton
    if (AppOpts.chkCBIcons->value() == 1)
      app->colorBlindIcons = true;
    else
      app->colorBlindIcons = false;

    svCreateAppIcons(true);

    // show tooltips checkbutton
    if (AppOpts.chkShowTooltips->value() == 1)
      app->showTooltips = true;
    else
      app->showTooltips = false;

    svEnableDisableTooltips();

    // show reverse notification checkbutton
    if (AppOpts.chkShowReverseConnect->value() == 1)
      app->showReverseConnect = true;
    else
      app->showReverseConnect = false;

    // log app events to file
    if (AppOpts.chkLogToFile->value() == 1)
      app->enableLogToFile = true;
    else
      app->enableLogToFile = false;

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
  Fl_Button * btn = static_cast<Fl_Button *>(widget);

  if (childWindow == NULL || widget == NULL || btn == NULL)
  {
    app->childWindowVisible = false;
    app->childWindowBeingDisplayed = NULL;
    return;
  }

  VncObject * vnc = app->vncViewer->vnc;

  if (vnc != NULL)
  {
    // ctrl + alt + delete button clicked
    if (btn == F8Opts.btnCAD)
    {
      SendKeyEvent(vnc->vncClient, XK_Control_L, true);
      SendKeyEvent(vnc->vncClient, XK_Alt_L, true);
      SendKeyEvent(vnc->vncClient, XK_Delete, true);

      SendKeyEvent(vnc->vncClient, XK_Control_L, false);
      SendKeyEvent(vnc->vncClient, XK_Alt_L, false);
      SendKeyEvent(vnc->vncClient, XK_Delete, false);
    }

    // ctrl + shift + esc button clicked
    if (btn == F8Opts.btnCSE)
    {
      SendKeyEvent(vnc->vncClient, XK_Control_L, true);
      SendKeyEvent(vnc->vncClient, XK_Shift_L, true);
      SendKeyEvent(vnc->vncClient, XK_Escape, true);

      SendKeyEvent(vnc->vncClient, XK_Control_L, false);
      SendKeyEvent(vnc->vncClient, XK_Shift_L, false);
      SendKeyEvent(vnc->vncClient, XK_Escape, false);
    }

    // ask server for a screen refresh
    if (btn == F8Opts.btnRefresh)
      SendFramebufferUpdateRequest(vnc->vncClient, 0, 0,
        vnc->vncClient->width, vnc->vncClient->height, false);

    // send F8 key
    if (btn == F8Opts.btnSendF8)
    {
      SendKeyEvent(vnc->vncClient, XK_F8, true);
      SendKeyEvent(vnc->vncClient, XK_F8, false);
    }

    // send F11 key
    if (btn == F8Opts.btnSendF11)
    {
      SendKeyEvent(vnc->vncClient, XK_F11, true);
      SendKeyEvent(vnc->vncClient, XK_F11, false);
    }

    // send F12 key
    if (btn == F8Opts.btnSendF12)
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

  if (nListVal > 0 && strcmp(app->hostList->text(nListVal), "@C16@.· · ·") == 0)
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
      svMessageWindow("Nothing was selected so nothing was deleted", "SpiritVNC - Delete Item");
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

        if (vnc != NULL && itm->isListener == true)
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

  if (itm == NULL)
  {
    // set itm's quick note to nothing
    svQuickInfoSetToEmpty();

    return;
  }

  VncObject * vnc = itm->vnc;

  static bool menuUp;

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
    int nF12Flags = 0;
    int nF12Flags2 = 0;
    int nDividerFlag = 0;
    int nCustCommand1Flags = FL_MENU_INVISIBLE;
    int nCustCommand2Flags = FL_MENU_INVISIBLE;
    int nCustCommand3Flags = FL_MENU_INVISIBLE;

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

      vnc->endViewer();

      return;
    }

    // set custom commands visibility
    if (itm->customCommand1Enabled == true)
    {
      nCustCommand1Flags = 0;
      nDividerFlag = FL_MENU_DIVIDER;
    }
    if (itm->customCommand2Enabled == true)
    {
      nCustCommand2Flags = 0;
      nDividerFlag = FL_MENU_DIVIDER;
    }
    if (itm->customCommand3Enabled == true)
    {
      nCustCommand3Flags = 0;
      nDividerFlag = FL_MENU_DIVIDER;
    }

    // show pop-up menu if disconnected and not a listening connection
    if (itm->isConnected == false
        && itm->isConnecting == false
        && itm->hasDisconnectRequest == false
        && itm->isListener == false
        && menuUp == false)
    {
      // enable / disable 'Copy F12 macro' item in menu
      if (itm->f12Macro == "")
        nF12Flags = FL_MENU_INACTIVE;

      // create context menu
      // text,shortcut,callback,user_data,flags,labeltype,labelfont,labelsize
      const Fl_Menu_Item miMain[] = {
        {"Connect",        0, 0, 0, 0,         0, FL_HELVETICA, app->nMenuFontSize},
        {"Edit",           0, 0, 0, 0,         0, FL_HELVETICA, app->nMenuFontSize},
        {"Copy F12 macro", 0, 0, 0, nF12Flags, 0, FL_HELVETICA, app->nMenuFontSize},
        {"Delete...",      0, 0, 0, nDividerFlag, 0, FL_HELVETICA, app->nMenuFontSize},
        {itm->customCommand1Label.c_str(), 0, 0, 0, nCustCommand1Flags, 0, FL_HELVETICA, app->nMenuFontSize},
        {itm->customCommand2Label.c_str(), 0, 0, 0, nCustCommand2Flags, 0, FL_HELVETICA, app->nMenuFontSize},
        {itm->customCommand3Label.c_str(), 0, 0, 0, nCustCommand3Flags, 0, FL_HELVETICA, app->nMenuFontSize},
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
          {"Delete", 0, 0, 0, 0, 0, FL_HELVETICA, app->nMenuFontSize},
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

        // enable / disable 'Clear F12 macro' item in menu
        if (itm->f12Macro == "")
          nF12Flags2 = FL_MENU_INACTIVE;

        // create context menu
        // text,shortcut,callback,user_data,flags,labeltype,labelfont,labelsize
        const Fl_Menu_Item miMain[] = {
          {"Disconnect",      0, 0, 0, 0,          0, FL_HELVETICA, app->nMenuFontSize},
          {"Edit",            0, 0, 0, 0,          0, FL_HELVETICA, app->nMenuFontSize},
          {"Paste F12 macro", 0, 0, 0, nF12Flags,  0, FL_HELVETICA, app->nMenuFontSize},
          {"Clear F12 macro", 0, 0, 0, nF12Flags2, 0, FL_HELVETICA, app->nMenuFontSize},
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

              vnc->endViewer();
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

  // cancel button clicked
  if (btn == ItmOpts.btnCancel)
  {
    childWindow->hide();
    app->childWindowVisible = false;
    app->childWindowBeingDisplayed = NULL;
    app->itmBeingEdited = NULL;
  }

  // delete button clicked
  if (btn == ItmOpts.btnDel)
  {
    fl_message_hotspot(0);
    fl_message_title("SpiritVNC - Delete Item");

    if (fl_choice("Are you sure you want to delete this?", "Cancel", "No", "Yes") == SV_CHOICE_BTN_3)
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
  if (btn == ItmOpts.btnSave)
  {
    // #### vnc tab ########################################

    // connection name text input
    itm->name = ItmOpts.inName->value();

    // connection group text input
    itm->group = ItmOpts.inGroup->value();

    // connection address text input
    itm->hostAddress = ItmOpts.inAddress->value();

    // f12 macro text input
    itm->f12Macro = ItmOpts.inF12Macro->value();

    // vnc connection radio button
    if (ItmOpts.rbVNC->value() == 1)
      itm->hostType = 'v';

    // vnc-through-ssh radio button
    if (ItmOpts.rbSVNC->value() == 1)
      itm->hostType = 's';

    // vnc port text input
    itm->vncPort = ItmOpts.inVNCPort->value();

    // vnc password secret input
    itm->vncPassword = ItmOpts.inVNCPassword->value();

    // vnc login name input
    itm->vncLoginUser = ItmOpts.inVNCLoginUser->value();

    // vnc login password secret input
    itm->vncLoginPassword = ItmOpts.inVNCLoginPassword->value();

    // vnc compression level text input
    itm->compressLevel = atoi(ItmOpts.inVNCCompressLevel->value());

    if (itm->compressLevel > 9)
      itm->compressLevel = 9;

    // vnc quality level text input
    itm->qualityLevel = atoi(ItmOpts.inVNCQualityLevel->value());

    if (itm->qualityLevel > 9)
      itm->qualityLevel = 9;

    // scroll only / no scaling radio button
    if (ItmOpts.rbScaleOff->value() == 1)
      itm->scaling = 's';

    // zoom radio button
    if (ItmOpts.rbScaleZoom->value() == 1)
      itm->scaling = 'z';

    // fit radio button
    if (ItmOpts.rbScaleFit->value() == 1)
      itm->scaling = 'f';

    // fast (jaggy) scaling checkbutton
    if (ItmOpts.chkScalingFast->value() == 1)
      itm->scalingFast = true;
    else
      itm->scalingFast = false;

    // show remote cursor checkbutton
    if (ItmOpts.chkShowRemoteCursor->value() == 1)
      itm->showRemoteCursor = true;
    else
      itm->showRemoteCursor = false;

    // #### ssh tab ###########################################

    // ssh username
    itm->sshUser = ItmOpts.inSSHName->value();

    // ssh port
    itm->sshPort = ItmOpts.inSSHPort->value();

    // ssh private key
    itm->sshKeyPrivate = ItmOpts.inSSHPrvKey->value();

    // #### custom commands ####################################
    // custom command 1
    if (ItmOpts.chkCommand1Enabled->value() == 1)
      itm->customCommand1Enabled = true;
    else
      itm->customCommand1Enabled = false;

    itm->customCommand1Label = ItmOpts.inCommand1Label->value();
    itm->customCommand1 = ItmOpts.inCommand1->value();

    // custom command 2
    if (ItmOpts.chkCommand2Enabled->value() == 1)
      itm->customCommand2Enabled = true;
    else
      itm->customCommand2Enabled = false;

    itm->customCommand2Label = ItmOpts.inCommand2Label->value();
    itm->customCommand2 = ItmOpts.inCommand2->value();

    // custom command 3
    if (ItmOpts.chkCommand3Enabled->value() == 1)
      itm->customCommand3Enabled = true;
    else
      itm->customCommand3Enabled = false;

    itm->customCommand3Label = ItmOpts.inCommand3Label->value();
    itm->customCommand3 = ItmOpts.inCommand3->value();

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

    // refresh any visual changes for connected listeners
    if (itm->isListener == true && itm->vnc != NULL)
      itm->vnc->setObjectVisible();

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


/*  hide quick note edit widgets and empty input value  */
void svHideQuickNoteEditWidgets ()
{
  // stop the text input cursor blink timer
  Fl::remove_timeout(svBlinkCursor);

  // clear quick note input value and hide quick info pack
  app->quickNoteInput->value("");
  app->quickNotePack->hide();
}


/*  popup edit menu in input widgets and handle choice  */
void svPopUpEditMenu (Fl_Input_ * input)
{
  static bool inMenu;

  // prevent re-entry
  if (inMenu == true)
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
  if (miRes != NULL)
  {
    const char * strRes = miRes->text;

    // do edit action based on menu item label text, if any
    if (strRes != NULL)
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
  // create our quick note widgets if they aren't created yet
  if (app->quickNotePack == NULL)
    svCreateQuickNoteEditWidgets();

  // clear quick note input value
  svHideQuickNoteEditWidgets();

  // set itm's quick info label text, if any
  app->quickInfoLabel->copy_label(itm->name.c_str());

  // set last connected text, if any
  if (itm->lastConnectedTime != "")
  {
    if (itm->isListener == false)
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
  if (itm->quickNote == "")
  {
    app->quickNoteBox->textfont(FL_HELVETICA_ITALIC);

    // listening connections don't save any data, so
    // best to call the item 'Temporary Note'
    if (itm->isListener == true)
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
  // create our quick note widgets if they aren't created yet
  if (app->quickNotePack == NULL)
    svCreateQuickNoteEditWidgets();

  // clear quick note input value
  svHideQuickNoteEditWidgets();

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

  // don't allow host list width to be smaller than right-most button's x+w
  if (app->requestedListWidth < (app->packButtons->x() + app->packButtons->w()) - 25)
    app->requestedListWidth = app->packButtons->x() + app->packButtons->w();

  // readjust hostlist width
  app->hostList->size(app->requestedListWidth, app->hostList->h());

  // readjust quickInfoPack width
  app->quickInfoPack->size(app->requestedListWidth, app->packButtons->y() - 3);

  // set scroller scroll position and size
  app->scroller->position(0, 0);  // 'position' is the scroll position, NOT X, Y
  app->scroller->size(app->hostList->x() + app->hostList->w() + 3, app->scroller->y()); // was 'position', for some strange reason
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

  if (itm == NULL)
    return;

  VncObject * vnc = itm->vnc;

  if (vnc == NULL)
    return;

  int nItem = svItemNumFromItm(itm);

  // set viewer as connected
  if (itm->isWaitingForShow == true)
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
      itm->name = strHostViewName;

      app->hostList->text(nListeningItem, itm->name.c_str());

      // (try to) create another listener
      svDebugLog("svConnectionWatcher - Creating Listener object");

      VncObject::createVNCListener();

      if (app->showReverseConnect == true)
        svMessageWindow("A remote VNC host has just reverse-connected"
          "\n\nClick the 'Listen' item(s) in the host list to view");
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
    if (itm->isListener == true)
    {
      // output to console and attempt to log error
      std::string strLErr = svMakeTimeStamp() + " - SpiritVNC-FLTK - Error: Incoming reverse VNC connection failed.  "
        "Attempting to recover";
      svLogToFile(strLErr);

      // log to stdout if we're not using LogToFile
      if (app->enableLogToFile == false)
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
void svHandleThreadCursorChange (void * setToDefault)
{
    bool setDefault = reinterpret_cast<bool *>(setToDefault);

    // if we're just resetting the cursor and nothing else
    if (setDefault == true)
    {
      app->mainWin->cursor(FL_CURSOR_DEFAULT);
      return;
    }

    if (app->vncViewer == NULL)
      return;

    VncObject * vnc = app->vncViewer->vnc;

    if (vnc == NULL)
      return;

    // set cursor, if valid
    if (vnc->imgCursor != NULL)
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
  itm->hostAddress = "192.168.0.1";
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
  if (itmIn == NULL)
    return 0;

  uint16_t nSize = app->hostList->size();

  // go through hostlist and find item owning matching itmIn
  for (uint16_t i = 0; i <= nSize; i ++)
  {
    HostItem * itm = NULL;
    itm = static_cast<HostItem *>(app->hostList->data(i));

    if (itm != NULL && itm == itmIn)
      return i;
  }

  return 0;
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


/*  send text to log file  */
void svLogToFile (const std::string& strMessage)
{
  std::ofstream ofs;
  char logFileName[FL_PATH_MAX] = {0};
  std::string strLineEnd;

  if (app->enableLogToFile == false)
    return;

  if (strMessage.size() == 0 || strMessage == "")
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
  if (dashSeps == true)
    strFmt = strdup("%Y-%m-%d--%H:%M:%S");
  else
    strFmt = strdup("%Y-%m-%d  %H:%M:%S");

  if (strFmt != NULL)
  {
    strftime(timeBuf, 50, strFmt, localtime(&myClock));
    free(strFmt);
  }

  return std::string(timeBuf);
}


/*  display a message dialog window  */
void svMessageWindow (const std::string& strMessage, const std::string& strTitle)
{
  Fl::lock();
  fl_message_hotspot(0);
  fl_message_title(strTitle.c_str())  ;
  fl_message("%s", strMessage.c_str());
  Fl::unlock();
}


/*  return number of connected items (integer)  */
bool svThereAreConnectedItems ()
{
  int nSize = app->hostList->size();

  for (uint16_t i = 1; i <= nSize; i ++)
  {
    HostItem * itm = NULL;
    itm = static_cast<HostItem *>(app->hostList->data(i));

    if (itm != NULL && itm->isConnected == true)
        return true;
  }

  return false;
}


/*  make sure scroller is sized correctly  */
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
  actual command runner
  (called as thread because it may block)
*/
void * svRunCommandHelper(void * strArgsIn)
{
  // detach this thread
  pthread_detach(pthread_self());

  std::string strOops = "Command or command label is NULL";

  // get out if pointer is null
  if (strArgsIn == NULL)
  {
    svMessageWindow(strOops, "SpiritVNC - Custom command");
    return SV_RET_VOID;
  }

  // cast void pointer to char * array
  const char ** strArgs = static_cast<const char **>(strArgsIn);

  // if either of the array elements are null, get out
  if (strArgs[0] == NULL || strArgs[1] == NULL)
  {
    svMessageWindow(strOops, "SpiritVNC - Custom command");
    return SV_RET_VOID;
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

  return SV_RET_VOID;
}


/*
  runs custom command
  (calls svRunCommandHelper as thread)
*/
void svRunCommand(const std::string& label, const std::string& cmd)
{
  pthread_t runThread = 0;

  // get out if label or command is empty
  if (label == "" || cmd == "")
  {
    svMessageWindow("Command or command label is empty", "SpiritVNC - Custom command");
    return;
  }

  // create char array pointer to pass to thread
  char * strArgs[] = {strdup(label.c_str()), strdup(cmd.c_str())};

  // create, launch and detach call to svRunCommandHelper
  if (pthread_create(&runThread, NULL, svRunCommandHelper, strArgs) != 0)
  {
    svMessageWindow("Error: Couldn't create custom command thread", "SpiritVNC - Custom command");
  }
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

    // hunt for next connected viewer
    itm = static_cast<HostItem *>(app->hostList->data(app->nCurrentScanItem));

    if (itm == NULL)
      continue;

    if (itm->isConnected == true)
    {
      svDeselectAllItems();
      VncObject::hideMainViewer();
      app->hostList->select(app->nCurrentScanItem);
      itm->vnc->setObjectVisible();

      // set quick note label and note text
      svQuickInfoSetLabelAndText(itm);

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
void svSendKeyStrokesToHost (const std::string& strIn, const VncObject * vnc)
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
  if (app->childWindowVisible == true)
    return;

  // set flag so no other child window will show
  app->childWindowVisible = true;

  // window size
  int nWinWidth = 765;
  int nWinHeight = 595; //740;

  // set window position
  int nX = (app->mainWin->w() / 2) - (nWinWidth / 2);
  int nY = (app->mainWin->h() / 2) - (nWinHeight / 2);

  // create messagebox window
  Fl_Window * winAboutHelp = new Fl_Window(nX, nY, nWinWidth, nWinHeight, "About SpiritVNC-FLTK / Help");
  winAboutHelp->set_modal();
  winAboutHelp->callback(svCloseChildWindow, winAboutHelp);

  Fl_Help_View * hv = new Fl_Help_View(10, 10, nWinWidth - 20, nWinHeight - 70);

  if (hv == NULL)
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
  Fl_Button * btnOK = new Fl_Button((nWinWidth - 100 - 10), nWinHeight - (35 + 10), 100, 35, "OK");
  btnOK->box(FL_GTK_UP_BOX);
  btnOK->shortcut(FL_Escape);
  btnOK->callback(svCloseChildWindow, winAboutHelp);

  // end help window layout
  winAboutHelp->end();

  // show window
  winAboutHelp->show();
  Fl::redraw();
}


/*  show app options  */
void svShowAppOptions ()
{
  // check to make sure no other child window is visible
  if (app->childWindowVisible == true)
    return;

  // set flag so no other child window will show
  app->childWindowVisible = true;

  // window size
  int nWinWidth = 650;
  int nWinHeight = 550;

  // set window position
  int nX = app->hostList->w() + 50;
  int nY = (app->mainWin->h() / 2) - (nWinHeight / 2);

  // create window
  Fl_Window * winAppOpts = new Fl_Window(nX, nY, nWinWidth, nWinHeight, "Application Options");
  winAppOpts->set_modal();
  winAppOpts->callback(svCloseChildWindow, winAppOpts);

  app->childWindowBeingDisplayed = winAppOpts;

  // add itm value editors / selectors
  int nXPos = 300;
  int nYStep = 32; //31;
  int nYPos = -(nYStep / 2);  //20;

  // ############ general options ##########################################################

  // scan viewer time-out
  AppOpts.spinScanTimeout = new Fl_Spinner(nXPos, nYPos += nYStep,
    100, 28, "Scan wait time (seconds) ");
  AppOpts.spinScanTimeout->textsize(app->nAppFontSize);
  AppOpts.spinScanTimeout->labelsize(app->nAppFontSize);
  AppOpts.spinScanTimeout->step(1);
  AppOpts.spinScanTimeout->minimum(1);
  AppOpts.spinScanTimeout->maximum(200000);
  AppOpts.spinScanTimeout->value(app->nScanTimeout);
  AppOpts.spinScanTimeout->tooltip("When scanning, this is how long SpiritVNC waits before moving"
      " to the next connected host item");

  // starting local ssh port number
  AppOpts.spinLocalSSHPort = new Fl_Spinner(nXPos, nYPos += nYStep,
    100, 28, "Starting local SSH port number ");
  AppOpts.spinLocalSSHPort->textsize(app->nAppFontSize);
  AppOpts.spinLocalSSHPort->labelsize(app->nAppFontSize);
  AppOpts.spinLocalSSHPort->step(1);
  AppOpts.spinLocalSSHPort->minimum(1);
  AppOpts.spinLocalSSHPort->maximum(200000);
  AppOpts.spinLocalSSHPort->value(app->nStartingLocalPort);
  AppOpts.spinLocalSSHPort->tooltip("This is the first SSH port used locally for VNC-over-SSH connections");

  // ssh command
  AppOpts.inSSHCommand = new SVInput(nXPos, nYPos += nYStep, 210, 28,
    "SSH command (eg: ssh or /usr/bin/ssh) ");
  AppOpts.inSSHCommand->textsize(app->nAppFontSize);
  AppOpts.inSSHCommand->labelsize(app->nAppFontSize);
  AppOpts.inSSHCommand->value(app->sshCommand.c_str());
  AppOpts.inSSHCommand->tooltip("This is the command to start the SSH client on your system");

  // log app events to file?
  AppOpts.chkLogToFile = new Fl_Check_Button(nXPos, nYPos += nYStep, 210, 28,
    " Log app events to a file");
  AppOpts.chkLogToFile->labelsize(app->nAppFontSize);
  AppOpts.chkLogToFile->tooltip("Check this to log app events to a file");
  if (app->enableLogToFile == true)
    AppOpts.chkLogToFile->set();

  // ############ appearance options section ##########################################################

  // appearance options label
  Fl_Box * lblSep01 = new Fl_Box(nXPos, nYPos += nYStep + 14, 100, 28, "Appearance Options");
  lblSep01->labelsize(app->nAppFontSize);
  lblSep01->align(FL_ALIGN_CENTER);
  lblSep01->labelfont(1);

  // app font size
  AppOpts.inAppFontSize = new SVIntInput(nXPos, nYPos += nYStep, 42, 28, "Application font size ");
  AppOpts.inAppFontSize->textsize(app->nAppFontSize);
  AppOpts.inAppFontSize->labelsize(app->nAppFontSize);
  AppOpts.inAppFontSize->value(std::to_string(app->nAppFontSize).c_str());
  AppOpts.inAppFontSize->tooltip("This is the font size used throughout SpiritVNC.  Restart the app to"
      " see any changes");

  // list font
  AppOpts.inListFont = new SVInput(nXPos, nYPos += nYStep, 210, 28,
    "List font name (eg: Lucida Sans) ");
  AppOpts.inListFont->textsize(app->nAppFontSize);
  AppOpts.inListFont->labelsize(app->nAppFontSize);
  AppOpts.inListFont->value(app->strListFont.c_str());
  AppOpts.inListFont->tooltip("This is the font used for the host list.  Restart the app to"
      " see any changes");

  // list font size
  AppOpts.inListFontSize = new SVIntInput(nXPos + 260, nYPos, 42, 28, "Size:");
  AppOpts.inListFontSize->textsize(app->nAppFontSize);
  AppOpts.inListFontSize->labelsize(app->nAppFontSize);
  AppOpts.inListFontSize->value(std::to_string(app->nListFontSize).c_str());
  AppOpts.inListFontSize->tooltip("This is the font size used for the host list.  Restart the app to"
      " see any changes");

  // list width
  AppOpts.inListWidth = new SVIntInput(nXPos, nYPos += nYStep, 210, 28, "List width ");
  AppOpts.inListWidth->textsize(app->nAppFontSize);
  AppOpts.inListWidth->labelsize(app->nAppFontSize);
  AppOpts.inListWidth->value(std::to_string(app->requestedListWidth).c_str());
  AppOpts.inListWidth->tooltip("Width of the host list.  Restart the app to see any changes");

  // use color-blind icons?
  AppOpts.chkCBIcons = new Fl_Check_Button(nXPos, nYPos += nYStep, 210, 28,
    " Use icons for color-blind users");
  AppOpts.chkCBIcons->labelsize(app->nAppFontSize);
  AppOpts.chkCBIcons->tooltip("Check this to enable color-blind-friendly icons in the host list."
      " Restart the app to see any changes");
  if (app->colorBlindIcons == true)
    AppOpts.chkCBIcons->set();

  // show tooltips?
  AppOpts.chkShowTooltips = new Fl_Check_Button(nXPos, nYPos += nYStep, 210, 28,
    " Show tooltips");
  AppOpts.chkShowTooltips->labelsize(app->nAppFontSize);
  AppOpts.chkShowTooltips->tooltip("Check this to enable tooltips in SpiritVNC.");
  if (app->showTooltips == true)
    AppOpts.chkShowTooltips->set();

  // show reverse connection notification?
  AppOpts.chkShowReverseConnect = new Fl_Check_Button(nXPos, nYPos += nYStep, 210, 28,
    " Show reverse connection notification");
  AppOpts.chkShowReverseConnect->labelsize(app->nAppFontSize);
  AppOpts.chkShowReverseConnect->tooltip("Check this to show a message window when a reverse"
      " connection happens.");
  if (app->showReverseConnect == true)
    AppOpts.chkShowReverseConnect->set();

  // ############ gap and restart advice ##########################################################

  // a little extra vertical gap
  nYPos += nYStep;

  // #### advice to restart SpiritVNC if certain options changed ####
  Fl_Box * boxFontLabel = new Fl_Box(nXPos, nYPos += nYStep, 210, 28,
    "Restart SpiritVNC for font or icon changes");
  boxFontLabel->labelsize(app->nAppFontSize);
  boxFontLabel->align(FL_ALIGN_CENTER);
  boxFontLabel->labelfont(2);

  // ############ bottom buttons ##########################################################

  // 'Cancel' button
  AppOpts.btnCancel = new Fl_Button((nWinWidth - 210 - 10), nWinHeight - (35 + 10), 100, 35,
    "Cancel");
  AppOpts.btnCancel->labelsize(app->nAppFontSize);
  AppOpts.btnCancel->box(FL_GTK_UP_BOX);
  AppOpts.btnCancel->shortcut(FL_Escape);
  AppOpts.btnCancel->callback(svHandleAppOptionsButtons);
  AppOpts.btnCancel->tooltip("Click to abandon any edits and close this window");

  // 'Save' button
  AppOpts.btnSave = new Fl_Button((nWinWidth - 100 - 10), nWinHeight - (35 + 10), 100, 35,
    "Save");
  AppOpts.btnSave->labelsize(app->nAppFontSize);
  AppOpts.btnSave->box(FL_GTK_UP_BOX);
  AppOpts.btnSave->shortcut(FL_Enter);
  AppOpts.btnSave->callback(svHandleAppOptionsButtons);
  AppOpts.btnSave->tooltip("Click to save edits and close this window");

  // end app options window layout
  winAppOpts->end();

  // show window
  winAppOpts->show();

  // focus the first spinner control
  AppOpts.spinScanTimeout->take_focus();

  // silly code to select all characters in first spinner's text input
  Fl_Input_ * inTemp = static_cast<Fl_Input_ *>(AppOpts.spinScanTimeout->child(0));

  if (inTemp != NULL && inTemp->type() == FL_INT_INPUT)
    inTemp->insert_position(0, 1000);

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
  int nWinHeight = 335;

  // set window position
  int nX = (app->mainWin->w() / 2) - (nWinWidth / 2);
  int nY = (app->mainWin->h() / 2) - (nWinHeight / 2);

  // create window
  Fl_Window * winF8 = new Fl_Window(nX, nY, nWinWidth, nWinHeight, "Remote host actions");
  winF8->set_modal();
  winF8->callback(svCloseChildWindow, winF8);

  app->childWindowBeingDisplayed = winF8;

  // add itm value editors / selectors
  int nXPos = 15;
  int nYStep = 38;
  int nYPos = -(nYStep / 2);  //10; //20;

  // =========================

  F8Opts.btnCAD = new Fl_Button(nXPos, nYPos += nYStep, 200, 35, "Send Ctrl+Alt+Del");
  F8Opts.btnCAD->box(FL_GTK_UP_BOX);
  F8Opts.btnCAD->callback(svHandleF8Buttons);
  F8Opts.btnCAD->tooltip("Click to send Ctrl+Alt+Delete to the current remote host");

  F8Opts.btnCSE = new Fl_Button(nXPos, nYPos += nYStep, 200, 35, "Send Ctrl+Shift+Esc");
  F8Opts.btnCSE->box(FL_GTK_UP_BOX);
  F8Opts.btnCSE->callback(svHandleF8Buttons);
  F8Opts.btnCSE->tooltip("Click to send Ctrl+Alt+Esc to the current remote host");

  F8Opts.btnRefresh = new Fl_Button(nXPos, nYPos += nYStep, 200, 35, "Send refresh request");
  F8Opts.btnRefresh->box(FL_GTK_UP_BOX);
  F8Opts.btnRefresh->callback(svHandleF8Buttons);
  F8Opts.btnRefresh->tooltip("Click to send a screen refresh request to the current remote host");

  F8Opts.btnSendF8 = new Fl_Button(nXPos, nYPos += nYStep, 200, 35, "Send F8");
  F8Opts.btnSendF8->box(FL_GTK_UP_BOX);
  F8Opts.btnSendF8->callback(svHandleF8Buttons);
  F8Opts.btnSendF8->tooltip("Click to press the F8 key on the current remote host");

  F8Opts.btnSendF11 = new Fl_Button(nXPos, nYPos += nYStep, 200, 35, "Send F11");
  F8Opts.btnSendF11->box(FL_GTK_UP_BOX);
  F8Opts.btnSendF11->callback(svHandleF8Buttons);
  F8Opts.btnSendF11->tooltip("Click to press the F11 key on the current remote host");

  F8Opts.btnSendF12 = new Fl_Button(nXPos, nYPos += nYStep, 200, 35, "Send F12");
  F8Opts.btnSendF12->box(FL_GTK_UP_BOX);
  F8Opts.btnSendF12->callback(svHandleF8Buttons);
  F8Opts.btnSendF12->tooltip("Click to press the F12 key on the current remote host");

  // ############ bottom button ##########################################################

  // 'Close' button
  F8Opts.btnCancel = new Fl_Button((nWinWidth - 110), nWinHeight - (35 + 10), 100, 35, "Close");
  F8Opts.btnCancel->box(FL_GTK_UP_BOX);
  F8Opts.btnCancel->shortcut(FL_Escape);
  F8Opts.btnCancel->callback(svHandleF8Buttons);
  F8Opts.btnCancel->tooltip("Click to close this window");

  // end F8 window layout
  winF8->end();

  // show window
  winF8->show();

  // focus the first spinner control
  F8Opts.btnCAD->take_focus();

  Fl::redraw();
}


/*  create / show item options window  */
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
    itm->sshKeyPrivate = "";
    itm->showRemoteCursor = true;
    itm->compressLevel = 5;
    itm->qualityLevel = 5;
    itm->vnc = NULL;
  }

  // window size
  int nWinWidth = 545;
  int nWinHeight = 605;

  // set window position
  int nX = app->hostList->w() + 50;
  int nY = (app->mainWin->h() / 2) - (nWinHeight / 2);

  std::string strWinTitle = "Editing '" + itm->name + "'";

  // create window
  Fl_Window * itmOptWin = new Fl_Window(nX, nY, nWinWidth, nWinHeight, strWinTitle.c_str());
  itmOptWin->set_modal();
  itmOptWin->callback(svCloseChildWindow, itmOptWin);

  int nXPos = 195;
  int nYStep = 28;
  int nYPos = -24;

  // add itm value editors / selectors

  // create tab control - each tab is a child Fl_Group within the tab / tab->end()
  Fl_Tabs * itemTab = new Fl_Tabs(10, nYPos += nYStep, nWinWidth - 20, nWinHeight - 62);

  // vnc options tab
  Fl_Group * vncGroup = new Fl_Group(0, nYPos += nYStep, nWinWidth - 23, nWinHeight - 20, "VNC options");

  // ############ general options ##########################################################
  nYPos = 16;

  // connection name
  ItmOpts.inName = new SVInput(nXPos, nYPos += nYStep, 210, 28, "Connection name ");
  ItmOpts.inName->value(itm->name.c_str());
  ItmOpts.inName->tooltip("The connection name as displayed in the host connection list");

  // connection group
  ItmOpts.inGroup = new SVInput(nXPos, nYPos += nYStep, 210, 28, "Connection group ");
  ItmOpts.inGroup->value(itm->group.c_str());
  ItmOpts.inGroup->tooltip("The group name this connection belongs to");

  // remote address
  ItmOpts.inAddress = new SVInput(nXPos, nYPos += nYStep, 210, 28, "Remote address ");
  ItmOpts.inAddress->value(itm->hostAddress.c_str());
  ItmOpts.inAddress->tooltip("The IP address of the remote host you want to connect to."
      "  Do NOT include the VNC port number here");

  // f12 macro text
  ItmOpts.inF12Macro = new SVInput(nXPos, nYPos += nYStep, 210, 28, "F12 macro ");
  ItmOpts.inF12Macro->value(itm->f12Macro.c_str());
  ItmOpts.inF12Macro->tooltip("Key presses that are sent to the remote host when"
      " you press the F12 key");

  // * vnc type buttons *

  // vnc without ssh
  ItmOpts.rbVNC = new Fl_Radio_Round_Button(nXPos, nYPos += nYStep, 100, 28, "VNC ");
  ItmOpts.rbVNC->callback(svItmOptionsRadioButtonsCallback);
  ItmOpts.rbVNC->tooltip("Choose this for VNC connections that don't tunnel through SSH");

  // vnc with ssh
  ItmOpts.rbSVNC = new Fl_Radio_Round_Button(nXPos, nYPos += nYStep, 100, 28,
    "VNC through SSH ");
  ItmOpts.rbSVNC->callback(svItmOptionsRadioButtonsCallback);
  ItmOpts.rbSVNC->tooltip("Choose this for VNC connections that use SSH to tunnel to the host");

  // set current value
  if (itm->hostType == 'v')
    ItmOpts.rbVNC->set();
  else
    ItmOpts.rbSVNC->set();

  // vnc port
  ItmOpts.inVNCPort = new SVIntInput(nXPos, nYPos += nYStep, 100, 28, "VNC port ");
  ItmOpts.inVNCPort->value(itm->vncPort.c_str());
  ItmOpts.inVNCPort->tooltip("The VNC port/display number of the host.  Defaults to 5900");

  // vnc password (shows dots, not cleartext password, for 'password' authentication)
  ItmOpts.inVNCPassword = new SVSecretInput(nXPos, nYPos += nYStep, 210, 28,
    "VNC password ");
  ItmOpts.inVNCPassword->value(itm->vncPassword.c_str());
  ItmOpts.inVNCPassword->tooltip("The VNC password for the host");

  // vnc login name (for 'credential' authentication)
  ItmOpts.inVNCLoginUser = new SVInput(nXPos, nYPos += nYStep, 210, 28,
    "VNC login user name ");
  ItmOpts.inVNCLoginUser->value(itm->vncLoginUser.c_str());
  ItmOpts.inVNCLoginUser->tooltip("The VNC login name for the host");

  // vnc login password (shows dots, not cleartext password, for 'credential' authentication)
  ItmOpts.inVNCLoginPassword = new SVSecretInput(nXPos, nYPos += nYStep, 210, 28,
    "VNC login password ");
  ItmOpts.inVNCLoginPassword->value(itm->vncLoginPassword.c_str());
  ItmOpts.inVNCLoginPassword->tooltip("The VNC login password for the host");

  // vnc compression level
  ItmOpts.inVNCCompressLevel = new SVIntInput(nXPos, nYPos += nYStep, 48, 28,
    "Compression level (0-9) ");
  ItmOpts.inVNCCompressLevel->value(std::to_string(itm->compressLevel).c_str());
  ItmOpts.inVNCCompressLevel->tooltip("The level of compression, from 0 to 9");

  // vnc quality level
  ItmOpts.inVNCQualityLevel = new SVIntInput(nXPos, nYPos += nYStep, 48, 28, "Quality level (0-9) ");
  ItmOpts.inVNCQualityLevel->value(std::to_string(itm->qualityLevel).c_str());
  ItmOpts.inVNCQualityLevel->tooltip("The level of image quality, from 0 to 9");

  // ##### scaling start #####

  // * scaling options group *
  ItmOpts.grpScaling = new Fl_Group(nXPos, nYPos += nYStep, 300, 300);

  // scaling off - scroll only
  ItmOpts.rbScaleOff = new Fl_Radio_Round_Button(nXPos, nYPos, 100, 28,
    " Scale off (scroll)");
  ItmOpts.rbScaleOff->callback(svItmOptionsRadioButtonsCallback);
  ItmOpts.rbScaleOff->tooltip("Choose this if you don't want any scaling."
      "  Scrollbars will appear for hosts with screens larger than the viewer");

  // scale up and down
  ItmOpts.rbScaleZoom = new Fl_Radio_Round_Button(nXPos, nYPos += nYStep, 100, 28,
    " Scale up and down");
  ItmOpts.rbScaleZoom->callback(svItmOptionsRadioButtonsCallback);
  ItmOpts.rbScaleZoom->tooltip("Choose this if you want small host screens scaled up to fit the"
      " viewer, or large host screens scaled down to fit the viewer");

  // scale down only
  ItmOpts.rbScaleFit = new Fl_Radio_Round_Button(nXPos, nYPos += nYStep, 100, 28,
    " Scale down only");
  ItmOpts.rbScaleFit->callback(svItmOptionsRadioButtonsCallback);
  ItmOpts.rbScaleFit->tooltip("Choose this to scale down large host screens to fit the viewer but"
      " small host screens are not scaled up");

  // set current value
  if (itm->scaling == 's')
    ItmOpts.rbScaleOff->set();
  else if (itm->scaling == 'z')
    ItmOpts.rbScaleZoom->set();
  else if (itm->scaling == 'f')
    ItmOpts.rbScaleFit->set();

  // fast scaling (instead of the default 'quality' scaling)
  ItmOpts.chkScalingFast = new Fl_Check_Button(nXPos, nYPos += nYStep, 100, 28,
    " Fast scaling (low quality)");
  if (itm->scalingFast == true)
    ItmOpts.chkScalingFast->set();
  ItmOpts.chkScalingFast->tooltip("Check to select fast scaling instead of quality scaling");

  // end of scaling group
  ItmOpts.grpScaling->end();

  // ##### scaling end #####

  // show the host's native cursor under our local static cursor
  ItmOpts.chkShowRemoteCursor = new Fl_Check_Button(nXPos, nYPos += nYStep, 100, 28,
    " Use remote cursor locally");
  ItmOpts.chkShowRemoteCursor->tooltip("Check to show remote locally");

  // set current value
  if (itm->showRemoteCursor == true)
    ItmOpts.chkShowRemoteCursor->set();

  // end of vnc options tab
  vncGroup->end();

  // ############ vnc-over-ssh options ##########################################################
  nYPos = 4; //10; //28;

  Fl_Group * sshGroup = new Fl_Group(0, nYPos += nYStep, nWinWidth - 20, nWinHeight - 20, "SSH options");

  nYPos = 16;

  // name used for ssh login
  ItmOpts.inSSHName = new SVInput(nXPos, nYPos += nYStep, 210, 28, "SSH user name ");
  ItmOpts.inSSHName->value(itm->sshUser.c_str());
  ItmOpts.inSSHName->tooltip("The SSH user name for the host");

  //// password used for ssh login (if used)   // #### DISABLED RIGHT NOW ####
  //ItmOpts.inSSHPassword = new SVSecretInput(nXPos, nYPos += nYStep, 210, 28,
    //"SSH password (if any) ");
  //ItmOpts.inSSHPassword->value(itm->sshPass.c_str());
  //ItmOpts.inSSHPassword->tooltip("The SSH password for the host (usually not necessary when using"
      //" key files)");
  //ItmOpts.inSSHPassword->deactivate();  //  <<<--- We can't really do SSH password right now ---<<<

  // ssh port (on the remote host)
  ItmOpts.inSSHPort = new SVIntInput(nXPos, nYPos += nYStep, 100, 28, "SSH remote port ");
  ItmOpts.inSSHPort->value(itm->sshPort.c_str());
  ItmOpts.inSSHPort->tooltip("The remote host's port number");

  // ssh private key full path (if used)
  ItmOpts.inSSHPrvKey = new SVInput(nXPos, nYPos += nYStep, 300, 28, "SSH private key (if any) ");
  ItmOpts.inSSHPrvKey->value(itm->sshKeyPrivate.c_str());
  ItmOpts.inSSHPrvKey->tooltip("The SSH private key file location (usually not"
      " necessary when using a SSH password");

  // button to select ssh private key
  ItmOpts.btnShowPrvKeyChooser = new Fl_Button(nXPos + 300 + 2, nYPos + 4, 20, 20, "...");
  ItmOpts.btnShowPrvKeyChooser->callback(svItmOptionsChoosePrvKeyBtnCallback, ItmOpts.inSSHPrvKey);
  ItmOpts.btnShowPrvKeyChooser->tooltip("Click to choose a SSH private key file");

  // set app vars
  app->childWindowBeingDisplayed = itmOptWin;
  app->itmBeingEdited = itm;

  // end of ssh options tab
  sshGroup->end();

  // ############ custom commands ##########################################################
  nYPos = 4;

  Fl_Group * customCommandGroup = new Fl_Group(0, nYPos += nYStep, nWinWidth - 20, nWinHeight - 20, "Custom commands");

  nXPos = 15;
  nYPos = 16;

  // custom command 1
  ItmOpts.chkCommand1Enabled = new Fl_Check_Button(nXPos, nYPos += nYStep, 175, 28, "Command 1 enabled");
  ItmOpts.chkCommand1Enabled->tooltip("Check to enable custom command 1");

  if (itm->customCommand1Enabled == true)
    ItmOpts.chkCommand1Enabled->set();

  ItmOpts.inCommand1Label = new SVInput(nXPos + 180, nYPos, 120, 28);
  ItmOpts.inCommand1Label->value(itm->customCommand1Label.c_str());

  ItmOpts.inCommand1 = new SVInput(nXPos + 305, nYPos, 200, 28);
  ItmOpts.inCommand1->value(itm->customCommand1.c_str());

  // custom command 2
  ItmOpts.chkCommand2Enabled = new Fl_Check_Button(nXPos, nYPos += nYStep, 175, 28, "Command 2 enabled");
  ItmOpts.chkCommand2Enabled->tooltip("Check to enable custom command 2");

  if (itm->customCommand2Enabled == true)
    ItmOpts.chkCommand2Enabled->set();

  ItmOpts.inCommand2Label = new SVInput(nXPos + 180, nYPos, 120, 28);
  ItmOpts.inCommand2Label->value(itm->customCommand2Label.c_str());

  ItmOpts.inCommand2 = new SVInput(nXPos + 305, nYPos, 200, 28);
  ItmOpts.inCommand2->value(itm->customCommand2.c_str());

  // custom command 3
  ItmOpts.chkCommand3Enabled = new Fl_Check_Button(nXPos, nYPos += nYStep, 175, 28, "Command 3 enabled");
  ItmOpts.chkCommand3Enabled->tooltip("Check to enable custom command 3");

  if (itm->customCommand3Enabled == true)
    ItmOpts.chkCommand3Enabled->set();

  ItmOpts.inCommand3Label = new SVInput(nXPos + 180, nYPos, 120, 28);
  ItmOpts.inCommand3Label->value(itm->customCommand3Label.c_str());

  ItmOpts.inCommand3 = new SVInput(nXPos + 305, nYPos, 200, 28);
  ItmOpts.inCommand3->value(itm->customCommand3.c_str());

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
  ItmOpts.btnDel = new Fl_Button(10, nWinHeight - (35 + 7), 100, 35, "Delete");
  ItmOpts.btnDel->box(FL_GTK_UP_BOX);
  ItmOpts.btnDel->callback(svHandleItmOptionsButtons);
  ItmOpts.btnDel->tooltip("Click to permanently delete this host connection"
      " (not undoable).  You will be asked to confirm before item is deleted");

  // 'Cancel' button
  ItmOpts.btnCancel = new Fl_Button((nWinWidth - 210 - 10), nWinHeight - (35 + 7), 100, 35,
    "Cancel");
  ItmOpts.btnCancel->box(FL_GTK_UP_BOX);
  ItmOpts.btnCancel->shortcut(FL_Escape);
  ItmOpts.btnCancel->callback(svHandleItmOptionsButtons);
  ItmOpts.btnCancel->tooltip("Click to abandon any edits to this connection"
      " and close this window");

  // 'Save' button
  ItmOpts.btnSave = new Fl_Button((nWinWidth - 100 - 10), nWinHeight - (35 + 7), 100, 35,
    "Save");
  ItmOpts.btnSave->box(FL_GTK_UP_BOX);
  ItmOpts.btnSave->shortcut(FL_Enter);
  ItmOpts.btnSave->callback(svHandleItmOptionsButtons);
  ItmOpts.btnSave->tooltip("Click to save edits made to this connection and close this window");

  // end item options window layout
  itmOptWin->end();

  // disable irrelevant widgets for listening connections
  if (itm->isListener)
  {
    ItmOpts.inName->deactivate();
    ItmOpts.inGroup->deactivate();
    ItmOpts.inAddress->deactivate();
    ItmOpts.rbVNC->deactivate();
    ItmOpts.rbSVNC->deactivate();
    ItmOpts.inVNCPort->deactivate();
    ItmOpts.inVNCPassword->deactivate();
    //ItmOpts.bxSSHSection->deactivate();
    ItmOpts.inSSHName->deactivate();
    //ItmOpts.inSSHPassword->deactivate();  //  <<<--- We can't really do SSH password right now ---<<<
    ItmOpts.inSSHPort->deactivate();
    ItmOpts.inSSHPrvKey->deactivate();
    ItmOpts.btnShowPrvKeyChooser->deactivate();
    ItmOpts.btnDel->deactivate();
  }
  else
  {
    ItmOpts.inName->activate();
    ItmOpts.inGroup->activate();
    ItmOpts.inAddress->activate();
    ItmOpts.rbVNC->activate();
    ItmOpts.rbSVNC->activate();
    ItmOpts.inVNCPort->activate();
    ItmOpts.inVNCPassword->activate();
    //ItmOpts.bxSSHSection->activate();
    ItmOpts.inSSHName->activate();
    //ItmOpts.inSSHPassword->activate();  //  <<<--- We can't really do SSH password right now ---<<<
    ItmOpts.inSSHPort->activate();
    ItmOpts.inSSHPrvKey->activate();
    ItmOpts.btnShowPrvKeyChooser->activate();
    ItmOpts.btnDel->activate();
  }

  // focus the first input box and select all text within
  ItmOpts.inName->take_focus();
  ItmOpts.inName->insert_position(0, strlen(ItmOpts.inName->value()) + 1);

  itmOptWin->show();
  Fl::redraw();
}


/* update text on all host items */
void svUpdateHostListItemText ()
{
  uint16_t nSize = app->hostList->size();

  for (uint16_t i = 0; i <= nSize; i ++)
  {
    HostItem * itm = NULL;
    itm = static_cast<HostItem *>(app->hostList->data(i));

    if (itm != NULL)
      app->hostList->text(i, itm->name.c_str());
  }
}
