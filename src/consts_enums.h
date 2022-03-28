/*
 * consts_enums.h - part of SpiritVNC - FLTK
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


#ifndef CONSTS_H
#define CONSTS_H

/* constants */
#define SV_APP_VERSION "0.4.11"

#define SV_CURRENT_YEAR "2022"

#define SV_CONNECTION_TIMEOUT_SECS  30
#define SV_ONE_SECOND               1.00
#define SV_PROP_LINE_MAX            4096
#define SV_APP_FONT_SIZE            14
#define SV_MAX_PROP_LEN             1024

// return type for threads
#define SV_RET_VOID         static_cast<void *>(NULL)

// app options constants
#define SV_OPTS_SCN_TIMEOUT     const_cast<char *>("spinScanTimeout")
#define SV_OPTS_LOCAL_SSH_PORT  const_cast<char *>("spinLocalSSHPort")
#define SV_OPTS_DEAD_TIMEOUT    const_cast<char *>("spinDeadTimeout")
#define SV_OPTS_APP_FONT_SIZE   const_cast<char *>("inAppFontSize")
#define SV_OPTS_LIST_FONT_NAME  const_cast<char *>("inListFont")
#define SV_OPTS_LIST_FONT_SIZE  const_cast<char *>("inListFontSize")
#define SV_OPTS_USE_CB_ICONS    const_cast<char *>("chkCBIcons")
#define SV_OPTS_SHOW_TOOLTIPS   const_cast<char *>("chkShowTooltips")
#define SV_OPTS_SHOW_REV_CON    const_cast<char *>("chkShowReverseConnect")
#define SV_OPTS_CANCEL          const_cast<char *>("btnCancel")
#define SV_OPTS_SAVE            const_cast<char *>("btnSave")

// item options constants
#define SV_ITM_NAME             const_cast<char *>("inName")
#define SV_ITM_GRP              const_cast<char *>("inGroup")
#define SV_ITM_ADDRESS          const_cast<char *>("inAddress")
#define SV_ITM_F12_MACRO        const_cast<char *>("inF12Macro")
#define SV_ITM_CON_VNC          const_cast<char *>("rbVNC")
#define SV_ITM_CON_SVNC         const_cast<char *>("rbSVNC")
#define SV_ITM_VNC_PORT         const_cast<char *>("inVNCPort")
#define SV_ITM_VNC_PASS         const_cast<char *>("inVNCPassword")
#define SV_ITM_VNC_COMP         const_cast<char *>("inVNCCompressLevel")
#define SV_ITM_VNC_QUAL         const_cast<char *>("inVNCQualityLevel")
#define SV_ITM_IGN_DEAD         const_cast<char *>("chkIgnoreInactive")
#define SV_ITM_GRP_SCALE        const_cast<char *>("grpScaling")
#define SV_ITM_SCALE_OFF        const_cast<char *>("rbScaleOff")
#define SV_ITM_SCALE_ZOOM       const_cast<char *>("rbScaleZoom")
#define SV_ITM_SCALE_FIT        const_cast<char *>("rbScaleFit")
#define SV_ITM_FAST_SCALE       const_cast<char *>("chkScalingFast")
#define SV_ITM_SHW_REM_CURSOR   const_cast<char *>("chkShowRemoteCursor")
#define SV_ITM_GRP_SSH          const_cast<char *>("bxSSHSection")
#define SV_ITM_SSH_NAME         const_cast<char *>("inSSHName")
#define SV_ITM_SSH_PASS         const_cast<char *>("inSSHPassword")
#define SV_ITM_SSH_PORT         const_cast<char *>("inSSHPort")
#define SV_ITM_SSH_PUB_KEY      const_cast<char *>("inSSHPubKey")
#define SV_ITM_SSH_PRV_KEY      const_cast<char *>("inSSHPrvKey")
#define SV_ITM_BTN_DEL          const_cast<char *>("btnDelete")
#define SV_ITM_BTN_CANCEL       const_cast<char *>("btnCancel")
#define SV_ITM_BTN_SAVE         const_cast<char *>("btnSave")

// F8 window constants
#define SV_F8_BTN_CAD       const_cast<char *>("btnCAD")
#define SV_F8_BTN_CSE       const_cast<char *>("btnCSE")
#define SV_F8_BTN_REFRESH   const_cast<char *>("btnRefresh")
#define SV_F8_BTN_SEND_F8   const_cast<char *>("btnSendF8")
#define SV_F8_BTN_SEND_F12  const_cast<char *>("btnSendF12")
#define SV_F8_BTN_CLOSE     const_cast<char *>("btnClose")

#endif
