/*****************************************************************************/
/*                                                                           */
/*                Copyright 1999 - 2003, Huawei Tech. Co., Ltd.              */
/*                           ALL RIGHTS RESERVED                             */
/*                                                                           */
/* FileName: LightLED.h                                                      */
/*                                                                           */
/* Author: Xu cheng                                                          */
/*                                                                           */
/* Version: 1.0                                                              */
/*                                                                           */
/* Date: 2008-06                                                             */
/*                                                                           */
/* Description: Turn on LED according to some Events which be supported by PS*/
/*                                                                           */
/* Others:                                                                   */
/*                                                                           */
/* History:                                                                  */
/* 1. Date: 2008-06                                                          */
/*    Author: Xu cheng                                                       */
/*    Modification: Create this file                                         */
/*                                                                           */

/*****************************************************************************/

#ifndef  _LIGHT_LED_H
#define  _LIGHT_LED_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#include "vos.h"
#include "NasOmInterface.h"
#include "WasOmInterface.h"

enum
{
    OAM_LED_STATE_CNF = 0x0001,
    OAM_LED_STATE_BUTT
};

typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                              ulMsgName;
    VOS_INT                                 lOldLedState;
    VOS_INT                                 lNewLedState;
    WAS_MNTN_OM_OUT_OF_SERVICE_ENUM_UINT8   enIsOutService;
    NAS_OM_REGISTER_STATE_ENUM_UINT8        enRegState;
    NAS_OM_SERVICE_TYPE_ENUM_UINT8          enServiceType;
    VOS_UINT8                               ucCardStatus;
    VOS_UINT32                              ulRatType;
}LED_STATE_STRUCT;

#if (FEATURE_ON == MBB_WPG_COMMON)
enum
{
    DEF = 0,                /*0:默认闪灯枚举*/
    HONGKONG,               /*1:香港PCCW闪灯枚举*/
    RUSSIA,                 /*2:俄罗斯MTS闪灯枚举*/
    VODAFONE,               /*3:vodafone闪灯枚举*/   
    CHINAUNION,             /*4:中国联通闪灯枚举*/
                            /*5:预留值*/
    SFR = 6,                /*6:法国SFR闪灯枚举*/
    BSNL,                   /*7:印度BSNL闪灯枚举*/
    KPN,                    /*8:荷兰KPN闪灯枚举*/
    SOFTBANK,               /*9:日本软银闪灯枚举*/
    EMOBILE                /*10:日本Emobile闪灯枚举*/
};

extern VOS_UINT32    g_ulLEDStatus;
extern VOS_UINT32    g_ulNetSearchStatus;

extern VOS_UINT32 OM_GetLedAsSearchStatus(VOS_VOID);
extern VOS_UINT32 OM_GetLedNasSearchStatus(VOS_VOID);
#endif

extern VOS_UINT32 OM_TraceMsgHook(VOS_VOID *pMsg);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* _LIGHT_LED_H */

