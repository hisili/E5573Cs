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



#ifndef __MBB_TAF_APS_API_H__
#define __MBB_TAF_APS_API_H__

#include "TafApsApi.h"
#include "product_nv_def.h"

#ifdef  __cplusplus
    #if  __cplusplus
    extern "C"{
    #endif
#endif

/*外部函数声明*/
#if (OSA_CPU_ACPU == VOS_OSA_CPU)
extern VOS_UINT32 AT_GetDestPid(
    MN_CLIENT_ID_T                      usClientId,
    VOS_UINT32                          ulRcvPid
);
#endif

extern VOS_UINT32 TAF_PS_SndMsg(
    VOS_UINT32                          ulTaskId,
    VOS_UINT32                          ulMsgId,
    VOS_VOID                           *pData,
    VOS_UINT32                          ulLength
);

#if (FEATURE_ON == MBB_WPG_LTE_ATTACH_APN_LIST)
#define        MAX_LTE_CNF_APN_SUPPORT                           1
/* 用户设置的APN用于NV限制，只保存18个字节，
深度定制表格中可以使用更长的profile名称，查询时现定为64字节 */
#define        LTE_EX_ATTACH_PROFILE_NAME_LEN               (64)

typedef struct
{
    VOS_UINT16                              usIndex;
    VOS_UINT16                              usFilter;
}TAF_PS_CUSTOM_ATTACH_APN_INFO_EXT_STRU;

typedef struct
{
    TAF_CTRL_STRU                       stCtrl;
    TAF_PS_CUSTOM_ATTACH_APN_INFO_EXT_STRU  stExInfo;
}TAF_PS_CUSTOM_ATTACH_APN_INFO_GET_REQ_STRU;

typedef struct
{
    TAF_PS_CUSTOM_ATTACH_APN_INFO_EXT_STRU  stExInfo;
    NV_LTE_ATTACH_PROFILE_STRU                        stNvAttachApnInfo;
    VOS_UINT8                                         ucExProfileNameLen;
    VOS_UINT8                                         aucExProfileName[LTE_EX_ATTACH_PROFILE_NAME_LEN + 1];
}TAF_CUSTOM_ATTACH_APN_INFO;

typedef struct
{
    TAF_CTRL_STRU                            stCtrl;
    TAF_CUSTOM_ATTACH_APN_INFO    stCustomAttachApn;
}TAF_PS_CUSTOM_ATTACH_APN_INFO_SET_REQ_STRU;

typedef struct
{
    TAF_CTRL_STRU                                   stCtrl;

    VOS_UINT32                                      ulResult;
    TAF_CUSTOM_ATTACH_APN_INFO                      stCustomAttachApn[MAX_LTE_CNF_APN_SUPPORT];
}TAF_PS_CUSTOM_ATTACH_APN_INFO_CNF_STRU;

typedef struct
{
    TAF_CTRL_STRU                                   stCtrl;

    VOS_UINT32                                      ulResult;
    VOS_UINT32                                      ulMaxCount;
}TAF_PS_CUSTOM_ATTACH_APN_COUNT_CNF_STRU;

VOS_UINT32 TAF_PS_SetCustomAttachApnReq(
        VOS_UINT32                          ulModuleId,
        VOS_UINT16                          usClientId,
        VOS_UINT8                           ucOpId,
        TAF_CUSTOM_ATTACH_APN_INFO       *pstCustomAttachApn
);

VOS_UINT32 TAF_PS_GetCustomAttachApnReq(
        VOS_UINT32                          ulModuleId,
        VOS_UINT16                          usClientId,
        VOS_UINT8                           ucOpId,
        TAF_PS_CUSTOM_ATTACH_APN_INFO_EXT_STRU       *pstCustomGetInfo
);

VOS_UINT32 TAF_PS_TesttCustomAttachApnReq(
        VOS_UINT32                          ulModuleId,
        VOS_UINT16                          usClientId,
        VOS_UINT8                           ucOpId,
        TAF_PS_CUSTOM_ATTACH_APN_INFO_EXT_STRU       *pstCustomGetInfo
);

VOS_UINT32 TAF_PS_TestCustomAttachApnReq(
        VOS_UINT32                          ulModuleId,
        VOS_UINT16                          usClientId,
        VOS_UINT8                           ucOpId
);


typedef struct
{
    VOS_UINT8     ucFlag;          /*设置的标志*/
    VOS_UINT8    aucReserve[3];  /* 预留后续使用 */
}AT_MTA_ATTACH_SWITCH_INFO_STRU;

typedef struct
{
    TAF_CTRL_STRU                            stCtrl;
    AT_MTA_ATTACH_SWITCH_INFO_STRU       stSwitchFlag;
}TAF_PS_APN_ATTACH_SWITCH_SET_REQ_STRU;

typedef struct
{
    TAF_CTRL_STRU                                   stCtrl;
    VOS_UINT32                                      ulResult;
    AT_MTA_ATTACH_SWITCH_INFO_STRU            stSwitchFlag;
}TAF_PS_APN_ATTACH_SWITCH_SET_CNF_STRU;



typedef struct
{
    VOS_UINT8     ucIsApnAttachOver;          /*设置的标志*/
    VOS_UINT8    aucReserve[3];  /* 预留后续使用 */
}AT_MTA_ATTACH_STATUS_STRU;

typedef struct
{
    TAF_CTRL_STRU    stCtrl;
}TAF_PS_APN_ATTACH_STAUTS_GET_REQ_STRU;

typedef struct
{
    TAF_CTRL_STRU             stCtrl;
    VOS_UINT32                ulResult;
    AT_MTA_ATTACH_STATUS_STRU    stAttachStatus;
}TAF_PS_APN_ATTACH_STATUS_GET_CNF_STRU;

VOS_UINT32 TAF_PS_SetLteApnAttachReq(
        VOS_UINT32                          ulModuleId,
        VOS_UINT16                          usClientId,
        VOS_UINT8                           ucOpId,
        AT_MTA_ATTACH_SWITCH_INFO_STRU       *pstApnOnOff
);


VOS_UINT32 TAF_PS_GetApnAttachStatusReq(
        VOS_UINT32                          ulModuleId,
        VOS_UINT16                          usClientId,
        VOS_UINT8                           ucOpId
);

#endif

#ifdef  __cplusplus
    #if  __cplusplus
    }
    #endif
#endif

#endif
