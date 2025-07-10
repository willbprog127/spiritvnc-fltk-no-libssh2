/*
 * spiritvnc.cxx - part of SpiritVNC - FLTK
 * 2016-2025 Will Brokenbourgh https://www.willbrokenbourgh.com/brainout/
 *
 * To God be the glory!  In Jesus name! :-D
 *
 * I don't typically use 'shortcuts' when checking for NULL or 0
 * i.e.: if (variable) or if (!variable)
 * because I want people to better understand what's
 * going on in the code
 *
 * Some code comes from examples provided by the
 * libvncserver project
 *
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


AppVars * app = new AppVars();


/*  main program  */
int main (int argc, char **argv)
{
  // tells FLTK we're a multithreaded app
  Fl::lock();

  // set graphics / display options
  Fl::visual(FL_DOUBLE | FL_RGB);

  // create program UI
  svCreateGUI();

  // read config file, set app options and populate host list
  svConfigReadCreateHostList();

  // set app tooltips
  svSetAppTooltips();

  // add default empty host list item if no items added from config file
  if (app->hostList->size() == 0)
  {
    svInsertEmptyItem();
    app->hostList->size(170, app->hostList->h());
  }

  // set host list font face and size
  Fl::set_font(SV_LIST_FONT_ID, app->strListFont.c_str());
  app->hostList->textfont(SV_LIST_FONT_ID);
  app->hostList->textsize(app->nListFontSize);
  app->nMenuFontSize = app->nListFontSize;

  // set app icons
  svCreateAppIcons();

  // manually trigger misc events callback
  svPositionWidgets();

  // end widget adding and show app window
  app->mainWin->end();
  app->mainWin->show(argc, argv);

  Fl::focus(app->hostList);
  app->hostList->take_focus();

  // ignore SIGPIPE from libvncclient socket calls
  // (not sure if this does anything...?)
  #ifndef _WIN32
  signal(SIGPIPE, SIG_IGN);
  #endif

  // start up the connection 'supervisor' timer callback
  // do NOT change the interval of this timer because program
  // logic expects this to always be near 1 second
  Fl::add_timeout(SV_ONE_SECOND, svConnectionWatcher);

  // start watching the clipboard
  Fl::add_clipboard_notify(svHandleLocalClipboard);

  svResizeScroller();

  // restore main window saved position and size on
  // Mac OS X / macOS, Windows immediately
  #if defined __APPLE__ || defined _WIN32
  svRestoreWindowSizePosition(NULL);
  #endif

  Fl::redraw();
  Fl::wait();

  // restore main window saved postition and size
  #if !defined __APPLE__ && !defined _WIN32
  // x11 window managers usually need delayed repositioning
  Fl::add_timeout(0.7, svRestoreWindowSizePosition);
  #endif

  // maximize window if last state was maximized
  if (app->maximized)
    app->mainWin->maximize();

  VncObject::masterMessageLoop();

  return Fl::run();
}
