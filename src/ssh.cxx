/*
 * ssh.cxx - part of SpiritVNC - FLTK
 * 2016-2024 Will Brokenbourgh https://www.willbrokenbourgh.com/brainout/
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
#include "hostitem.h"


/* attempts to close the popen'd ssh process */
/* (this is called as a thread because it could block) */
void * svSSHCloseHelper (void * itmData)
{
  // detach this thread
  pthread_detach(pthread_self());

  HostItem * itm = static_cast<HostItem *>(itmData);

  if (itm == NULL || itm->sshCmdStream == NULL)
    return SV_RET_VOID;

  // fprintf(itm->sshCmdStream, "%s\r\n", "exit");  //  <<--- trying different way to close (below) ---<<<

  // send 'exit' control-char sequence
  fprintf(itm->sshCmdStream, "\r\n%s", "~.");

  // send extra CRLF, just for fun
  fprintf(itm->sshCmdStream, "\r\n");

  // close the ssh process stream
  pclose(itm->sshCmdStream);

  itm->sshReady = false;

  return SV_RET_VOID;
}


/* close and clean up ssh connection */
void svCloseSSHConnection (void * itmData)
{
  HostItem * itm = static_cast<HostItem *>(itmData);

  if (itm == NULL || itm->sshCmdStream == NULL)
    return;

  // create, launch and detach call to create our vnc connection
  if (pthread_create(&itm->sshCloseThread, NULL, svSSHCloseHelper, itm) != 0)
  {
    svLogToFile("ERROR - Couldn't create SSH closer thread for '" + itm->name +
          "' - " + itm->hostAddress);
    //itm->isConnecting = false;
    itm->hasCouldntConnect = true;
    itm->hasError = true;

    //svHandleThreadConnection(itm);

    //return;
  }

  itm->isConnecting = false;
  svHandleThreadConnection(itm);
}


/* create ssh session and ssh forwarding */
void svCreateSSHConnection (void * data)
{
  std::string sshCommandLine;

  HostItem * itm = static_cast<HostItem *>(data);

  if (itm == NULL)
    return;

  #ifdef _WIN32
  AllocConsole();
  ShowWindow(GetConsoleWindow(), SW_HIDE);

  std::string sshCheck = app->sshCommand;
  #else
  std::string sshCheck = "which " + app->sshCommand + " > /dev/null";
  #endif

  svLogToFile("check command is: " + sshCheck);

  // first check to see if the ssh command is working
  FILE * fResult = popen(sshCheck.c_str(), "w");

  //if (nResult != 0)
  if (fResult == NULL)
  {
    int nResult = errno;

    svLogToFile("ssh check result is: " + std::to_string(nResult));

    fl_beep(FL_BEEP_DEFAULT);
    svMessageWindow("Error: This connection requires SSH and \nthe SSH command isn't working."
        "\n\nCheck that the SSH client program is installed", "SpiritVNC - FLTK");

    svLogToFile("SSH command not working for connection '"
        + itm->name + "' - " + itm->hostAddress);

    itm->lastErrorMessage = "SSH command not working";

    itm->sshReady = false;
    itm->hasError = true;

    return;
  }

  // build the command string for our system() call
  sshCommandLine = app->sshCommand + " " + itm->sshUser + "@" + itm->hostAddress + " -t" + " -t" +
    " -p " + itm->sshPort + " -o ConnectTimeout=" + std::to_string(itm->sshWaitTime) +
    " -L " + std::to_string(itm->sshLocalPort) + ":127.0.0.1:" + itm->vncPort +
    " -i " + itm->sshKeyPrivate;

  // call the system's ssh client, if available and open write stream
  itm->sshCmdStream = popen(sshCommandLine.c_str(), "w");

  if (itm->sshCmdStream != NULL)
    // ssh started okay
    itm->sshReady = true;
  else
  {
    // something -- happened
    svLogToFile("SSH connection disconnected abnormally from '"
        + itm->name + "' - " + itm->hostAddress);

    itm->sshReady = false;
    itm->hasError = true;
  }

  return;
}
