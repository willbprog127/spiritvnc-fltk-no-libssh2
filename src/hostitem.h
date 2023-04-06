/*
 * hostitem.h - part of SpiritVNC - FLTK
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


#ifndef HOSTITEM_H
#define HOSTITEM_H

#include <FL/Fl_Image.H>
#include <iostream>
#include "vnc.h"
#include "consts_enums.h"

// forward declaration of VncObject
class VncObject;

/* host item class */
class HostItem
{
public:
  HostItem () :
    name(""),
    group(""),
    hostAddress(""),
    vncPort(""),
    sshPort(""),
    sshUser(""),
    sshPass(""),
    sshKeyPrivate(""),
    sshLocalPort(0),
    vncPassword(""),
    vncLoginUser(""),
    vncLoginPassword(""),
    hostType('v'),
    vnc(NULL),
    threadRFB(0),
    threadRFBRunning(false),
    sshReady(false),
    vncAddressAndPort(""),
    f12Macro(""),
    scaling('f'),
    scalingFast(false),
    showRemoteCursor(false),
    compressLevel(5),
    qualityLevel(5),
    ignoreInactive(false),
    centerX(false),
    centerY(false),
    isListener(false),
    isConnecting(false),
    isConnected(false),
    isWaitingForShow(false),
    hasCouldntConnect(false),
    hasError(false),
    hasDisconnectRequest(false),
    hasEnded(false),
    icon(NULL),
    lastErrorMessage(""),
    sshWaitTime(5),
    sshCmdStream(NULL),
    sshCloseThread(0),
    quickNote(""),
    lastConnectedTime("")
  {}

  std::string name;
  std::string group;
  std::string hostAddress;
  std::string vncPort;
  std::string sshPort;
  std::string sshUser;
  std::string sshPass;
  std::string sshKeyPrivate;
  int sshLocalPort;
  std::string vncPassword;
  std::string vncLoginUser;
  std::string vncLoginPassword;
  char hostType;
  VncObject * vnc;
  pthread_t threadRFB;
  bool threadRFBRunning;
  bool sshReady;
  std::string vncAddressAndPort;
  std::string f12Macro;
  char scaling;
  bool scalingFast;
  bool showRemoteCursor;
  uint8_t compressLevel;
  uint8_t qualityLevel;
  bool ignoreInactive;
  bool centerX;
  bool centerY;
  //
  bool isListener;
  bool isConnecting;
  bool isConnected;
  bool isWaitingForShow;
  bool hasCouldntConnect;
  bool hasError;
  bool hasDisconnectRequest;
  bool hasEnded;
  Fl_Image * icon;
  std::string lastErrorMessage;
  uint16_t sshWaitTime;
  FILE * sshCmdStream;
  pthread_t sshCloseThread;
  std::string quickNote;
  std::string lastConnectedTime;
};

#endif
