/*
 * ssh.cxx - part of SpiritVNC - FLTK
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
#include "hostitem.h"


/* close and clean up ssh connection */
void svCloseSSHConnection (void * itmData)
{
    HostItem * itm = static_cast<HostItem *>(itmData);

    if (itm == NULL || itm->sshCmdStream == NULL)
        return;

    int sshKillResult = -1;
    
    fprintf(itm->sshCmdStream, "%s\n", "exit");    
    fflush(itm->sshCmdStream);  
    
    sshKillResult = pclose(itm->sshCmdStream);

    (void)sshKillResult;
}


/* create ssh session and ssh forwarding */
void svCreateSSHConnection (void * data)
{
    std::string sshCommandLine;
    std::string strError;

    HostItem * itm = static_cast<HostItem *>(data);

    if (itm == NULL)
        return;

    std::string sshCheck = "which " + app->sshCommand + "5 > /dev/null";

    // first check to see if the ssh command is working
    if (system(sshCheck.c_str()) != 0)
    {
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
    sshCommandLine = app->sshCommand + " " + itm->sshUser + "@" + itm->hostAddress + " -t" + " -t" + //" -N" +
        " -p " + itm->sshPort +
        " -L " + std::to_string(itm->sshLocalPort) + ":127.0.0.1:" + itm->vncPort +
        " -i " + itm->sshKeyPrivate;

    // call the system's ssh client, if available
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
