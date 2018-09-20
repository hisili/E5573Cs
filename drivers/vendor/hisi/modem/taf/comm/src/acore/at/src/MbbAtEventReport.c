/*
* Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
* foss@huawei.com
*
* If distributed as part of the Linux kernel, the following license terms
* apply:
*
* * This program is free software; you can redistribute it and/or modify
* * it under the terms of the GNU General Public License version 2 and
* * only version 2 as published by the Free Software Foundation.
* *
* * This program is distributed in the hope that it will be useful,
* * but WITHOUT ANY WARRANTY; without even the implied warranty of
* * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* * GNU General Public License for more details.
* *
* * You should have received a copy of the GNU General Public License
* * along with this program; if not, write to the Free Software
* * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
*
* Otherwise, the following license terms apply:
*
* * Redistribution and use in source and binary forms, with or without
* * modification, are permitted provided that the following conditions
* * are met:
* * 1) Redistributions of source code must retain the above copyright
* *    notice, this list of conditions and the following disclaimer.
* * 2) Redistributions in binary form must reproduce the above copyright
* *    notice, this list of conditions and the following disclaimer in the
* *    documentation and/or other materials provided with the distribution.
* * 3) Neither the name of Huawei nor the names of its contributors may
* *    be used to endorse or promote products derived from this software
* *    without specific prior written permission.
*
* * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*/


/*****************************************************************************
   1 头文件包含
*****************************************************************************/
#include "ATCmdProc.h"
#include "AtDataProc.h"
#include "AtEventReport.h"
#include "AtOamInterface.h"
#include "AtInputProc.h"
#include "AtCmdMsgProc.h"

#include "product_nv_def.h"

#if(FEATURE_ON == FEATURE_LTE)
#include "gen_msg.h"
#include "at_lte_common.h"
#endif

#include "TafAppMma.h"
#include "TafApsApi.h"
#include "MbbTafApsApi.h"

/*lint -e767 -e960 */
#ifdef  __cplusplus
    #if  __cplusplus
    extern "C"{
    #endif
#endif
#define    THIS_FILE_ID        PS_FILE_ID_AT_EVENTREPORT_C
/*lint +e767 +e960 */

#if (FEATURE_ON == MBB_WPG_LTE_ATTACH_APN_LIST)

VOS_UINT32 AT_RcvTafPsEvtTestCustomLteAttachApnCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    VOS_UINT32                                                       ulResult = AT_ERROR;
    TAF_PS_CUSTOM_ATTACH_APN_COUNT_CNF_STRU *pstEvtCnf = VOS_NULL_PTR;
    VOS_UINT16                                                       usLength = 0;

    pstEvtCnf = (TAF_PS_CUSTOM_ATTACH_APN_COUNT_CNF_STRU*)pEvtInfo;
    if (VOS_NULL_PTR == pEvtInfo)
    {
        return AT_ERROR;
    }

    if (VOS_OK == pstEvtCnf->ulResult)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                "%s: (0,1),(0-%d),10,32,32,32,(0-3)",
                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                pstEvtCnf->ulMaxCount);
        ulResult = AT_OK;
    }
    
    gstAtSendData.usBufLen = usLength;
    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, ulResult);
    
    return ulResult;
}


VOS_UINT32 AT_RcvTafPsEvtSetCustomLteAttachApnCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    VOS_UINT32      ulResult = AT_ERROR;
    TAF_PS_CUSTOM_ATTACH_APN_INFO_CNF_STRU *pstEvtCnf = VOS_NULL_PTR;

    pstEvtCnf = (TAF_PS_CUSTOM_ATTACH_APN_INFO_CNF_STRU*)pEvtInfo;
    if (VOS_NULL_PTR == pEvtInfo)
    {
        return AT_ERROR;
    }

    if (VOS_OK == pstEvtCnf->ulResult)
    {
        ulResult = AT_OK;
    }
    
    gstAtSendData.usBufLen = 0;
    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, ulResult);
    
    return ulResult;
}


VOS_UINT32 AT_RcvTafPsEvtGetCustomLteAttachApnCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_CUSTOM_ATTACH_APN_INFO_CNF_STRU *pstEvtCnf = VOS_NULL_PTR;
    VOS_UINT32      ulResult = AT_ERROR;
    VOS_UINT32      i = 0;
    VOS_UINT16      usLength = 0;
    VOS_UINT8        aucTempBuffer[MAX_LTE_ATTACH_APN_NAME_LEN + MAX_LTE_ATTACH_APN_USERNAME_LEN];
    VOS_UINT8        ucImsiPrefixBufLen = 0;

    pstEvtCnf = (TAF_PS_CUSTOM_ATTACH_APN_INFO_CNF_STRU*)pEvtInfo;

    if (VOS_NULL_PTR == pEvtInfo)
    {
        return AT_ERROR;
    }

    if (VOS_OK == pstEvtCnf->ulResult)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s: %d,",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                           pstEvtCnf->stCustomAttachApn[i].stExInfo.usIndex);
        
        if (TAF_PDP_IPV4V6 == pstEvtCnf->stCustomAttachApn[i].stNvAttachApnInfo.ucPdpType)
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "\"%s\",",
                                           "IPV4V6");
        }
        else if (TAF_PDP_IPV6 == pstEvtCnf->stCustomAttachApn[i].stNvAttachApnInfo.ucPdpType)
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "\"%s\",",
                                           "IPV6");
        }
        else
        {
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "\"%s\",",
                                               "IP");
        }

        if (0 < pstEvtCnf->stCustomAttachApn[i].stNvAttachApnInfo.ucImsiPrefixLen)
        {
            VOS_MemSet(aucTempBuffer, 0, sizeof(aucTempBuffer));
            if (MAX_LTE_APN_IMSI_PREFIX_SUPPORT <= pstEvtCnf->stCustomAttachApn[i].stNvAttachApnInfo.ucImsiPrefixLen)
            {
                pstEvtCnf->stCustomAttachApn[i].stNvAttachApnInfo.ucImsiPrefixLen = MAX_LTE_APN_IMSI_PREFIX_SUPPORT;
            }
            ucImsiPrefixBufLen = (pstEvtCnf->stCustomAttachApn[i].stNvAttachApnInfo.ucImsiPrefixLen + 1) >> 1; /*2 bcd 1 byte*/
            AT_HexToAsciiString(pstEvtCnf->stCustomAttachApn[i].stNvAttachApnInfo.aucImsiPrefixBcd,
                                        aucTempBuffer, ucImsiPrefixBufLen);
            aucTempBuffer[pstEvtCnf->stCustomAttachApn[i].stNvAttachApnInfo.ucImsiPrefixLen] = '\0';
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "\"%s\",",
                                               aucTempBuffer);
        }
        else
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "\"\",");
        }

        if (0 < pstEvtCnf->stCustomAttachApn[i].stNvAttachApnInfo.ucApnLen)
        {
            VOS_MemCpy(aucTempBuffer, pstEvtCnf->stCustomAttachApn[i].stNvAttachApnInfo.aucApn,
                                MAX_LTE_ATTACH_APN_NAME_LEN);
            aucTempBuffer[MAX_LTE_ATTACH_APN_NAME_LEN] = '\0';
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "\"%s\",",
                                               aucTempBuffer);
        }
        else
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "\"\",");
        }

        if (0 < pstEvtCnf->stCustomAttachApn[i].stNvAttachApnInfo.ucUserNameLen)
        {
            VOS_MemCpy(aucTempBuffer, pstEvtCnf->stCustomAttachApn[i].stNvAttachApnInfo.aucUserName,
                                MAX_LTE_ATTACH_APN_USERNAME_LEN);
            aucTempBuffer[MAX_LTE_ATTACH_APN_USERNAME_LEN] = '\0';
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "\"%s\",",
                                               aucTempBuffer);
        }
        else
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "\"\",");
        }
        
        if (0 < pstEvtCnf->stCustomAttachApn[i].stNvAttachApnInfo.ucPwdLen)
        {
            VOS_MemCpy(aucTempBuffer, pstEvtCnf->stCustomAttachApn[i].stNvAttachApnInfo.aucPwd,
                                MAX_LTE_ATTACH_APN_USERPWD_LEN);
            aucTempBuffer[MAX_LTE_ATTACH_APN_USERPWD_LEN] = '\0';
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "\"%s\",",
                                               aucTempBuffer);
        }
        else
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "\"\",");
        }

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                                       "%d,", pstEvtCnf->stCustomAttachApn[i].stNvAttachApnInfo.ucAuthType);

        if (LTE_EX_ATTACH_PROFILE_NAME_LEN < pstEvtCnf->stCustomAttachApn[i].ucExProfileNameLen)
        {
            pstEvtCnf->stCustomAttachApn[i].ucExProfileNameLen = LTE_EX_ATTACH_PROFILE_NAME_LEN;
        }
        
        if (0 < pstEvtCnf->stCustomAttachApn[i].ucExProfileNameLen)
        {
            /* need change to HEX show */
            VOS_MemSet(aucTempBuffer, 0x00, sizeof(aucTempBuffer));
            (VOS_VOID)AT_HexToAsciiString(pstEvtCnf->stCustomAttachApn[i].aucExProfileName, aucTempBuffer, 
                                            pstEvtCnf->stCustomAttachApn[i].ucExProfileNameLen);
            aucTempBuffer[sizeof(aucTempBuffer) - 1] = '\0';
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                                       "%s", aucTempBuffer);
        }
        
        ulResult = AT_OK;
    }

    gstAtSendData.usBufLen = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, ulResult);
    
    return ulResult;
}


VOS_UINT32 AT_RcvTafPsEvtGetApnAttachStatusCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    VOS_UINT32      ulResult = AT_ERROR;
    TAF_PS_APN_ATTACH_STATUS_GET_CNF_STRU *pstEvtCnf = VOS_NULL_PTR;
    VOS_UINT16   usLength = 0;

    pstEvtCnf = (TAF_PS_APN_ATTACH_STATUS_GET_CNF_STRU*)pEvtInfo;
    if (VOS_NULL_PTR == pEvtInfo)
    {
        return AT_ERROR;
    }

    if (VOS_OK == pstEvtCnf->ulResult)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                "%s:1,%d",
                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                pstEvtCnf->stAttachStatus.ucIsApnAttachOver);
        ulResult = AT_OK;
    }

    gstAtSendData.usBufLen = usLength;
    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, ulResult);

    return ulResult;
}


VOS_UINT32 AT_RcvTafPsEvtSetApnAttachSwitchCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    VOS_UINT32      ulResult = AT_ERROR;
    TAF_PS_APN_ATTACH_SWITCH_SET_CNF_STRU *pstEvtCnf = VOS_NULL_PTR;
    VOS_UINT16   usLength = 0;

    pstEvtCnf = (TAF_PS_APN_ATTACH_SWITCH_SET_CNF_STRU*)pEvtInfo;
    if (VOS_NULL_PTR == pEvtInfo)
    {
        return AT_ERROR;
    }

    if (VOS_OK == pstEvtCnf->ulResult)
    {
        ulResult = AT_OK;
    }

    gstAtSendData.usBufLen = usLength;
    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, ulResult);

    return ulResult;
}

#endif


#if (FEATURE_ON == MBB_WPG_COMMON)
/* MBB定制的PS_CALLBACK事件处理表格 */
const AT_PS_EVT_FUNC_TBL_STRU           g_astMbbAtPsEvtFuncTbl[] = {
#if (FEATURE_ON == MBB_WPG_LTE_ATTACH_APN_LIST)
    {ID_EVT_TAF_PS_SET_CUSTOM_ATTACH_APN_CNF,
        AT_RcvTafPsEvtSetCustomLteAttachApnCnf},
    {ID_EVT_TAF_PS_GET_CUSTOM_ATTACH_APN_CNF,
        AT_RcvTafPsEvtGetCustomLteAttachApnCnf},
    {ID_EVT_TAF_PS_TEST_CUSTOM_ATTACH_APN_CNF,
        AT_RcvTafPsEvtTestCustomLteAttachApnCnf},

    {ID_EVT_TAF_PS_SET_APN_ATTACH_SWITCH_CNF,
        AT_RcvTafPsEvtSetApnAttachSwitchCnf},
    {ID_EVT_TAF_PS_GET_APN_ATTACH_STATUS_CNF,
        AT_RcvTafPsEvtGetApnAttachStatusCnf},
#endif
};


MN_PS_EVT_FUNC AT_MbbGetTafPsEventProcFunc(VOS_UINT32 ulEventId)
{
    MN_PS_EVT_FUNC                      pEvtFunc = VOS_NULL_PTR;
    VOS_UINT32                          i;

    for ( i = 0; i < AT_ARRAY_SIZE(g_astMbbAtPsEvtFuncTbl); i++ )
    {
        if ( ulEventId == g_astMbbAtPsEvtFuncTbl[i].ulEvtId )
        {
            /* 事件ID匹配 */
            pEvtFunc = g_astMbbAtPsEvtFuncTbl[i].pEvtFunc;
            break;
        }
    }

    return pEvtFunc;
}

#endif

#ifdef  __cplusplus
    #if  __cplusplus
    }
    #endif
#endif

