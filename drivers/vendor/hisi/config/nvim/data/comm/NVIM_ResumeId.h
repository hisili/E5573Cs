/*****************************************************************************/
/*                                                                           */
/*                Copyright 1999 - 2003, Huawei Tech. Co., Ltd.              */
/*                           ALL RIGHTS RESERVED                             */
/*                                                                           */
/* FileName: NVIM_ResumeId.h                                                 */
/*                                                                           */
/* Author: Jiang kaibo                                                       */
/*                                                                           */
/* Version: 1.0                                                              */
/*                                                                           */
/* Date: 2008-06                                                             */
/*                                                                           */
/* Description: Def NV Resume ID num.                                        */
/*                                                                           */
/* Others:                                                                   */
/*                                                                           */
/* History:                                                                  */
/* 1. Date: 2008-06                                                          */
/*    Author: Jiang kaibo                                                    */
/*    Modification: Create this file                                         */
/*                                                                           */
/* ------------------------------问题单修改记录------------------------------------
  问题单号                修改人      修改时间      修改说明                             
  DTS2013121105254       王怀勇      2013-12-20   添加在恢复出厂设置机制白名单列表申明
------------------------------------------------------------------------------ */ 

/*****************************************************************************/

#ifndef _NVIM_RESUMEID_H
#define _NVIM_RESUMEID_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "product_config.h"

extern unsigned short   g_ausNvResumeManufactureIdList[];

extern unsigned short   g_ausNvResumeUserIdList[];

extern unsigned short   g_ausNvResumeSecureIdList[];

extern unsigned short   g_ausNvResumeDefualtIdList[];

extern unsigned long bsp_nvm_getRevertNum(unsigned long enNvItem);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif



#endif

