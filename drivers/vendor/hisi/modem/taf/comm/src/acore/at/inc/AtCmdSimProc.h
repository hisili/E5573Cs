

#ifndef __ATCMDSIMROC_H__
#define __ATCMDSIMPROC_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "AtCtx.h"
#include "AtParse.h"
#include "ATCmdProc.h"
#include "TafNvInterface.h"
#include "siapppih.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

/*****************************************************************************
  2 宏定义
*****************************************************************************/


/*****************************************************************************
  3 枚举定义
*****************************************************************************/


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern VOS_VOID At_Base16Encode(VOS_CHAR *pucSrc, VOS_CHAR *pucDst, VOS_UINT32 ulLen);
extern VOS_UINT32 At_Base16Decode(VOS_CHAR *pcData, VOS_UINT32 ulDataLen, VOS_UINT8* pucDst);
extern VOS_UINT16 At_Hex2Base16(VOS_UINT32 MaxLength,VOS_CHAR *headaddr,VOS_CHAR *pucDst,VOS_UINT8 *pucSrc,VOS_UINT16 usSrcLen);

#if (FEATURE_ON == FEATURE_VSIM)
extern VOS_UINT32 At_SetHvsDHPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 At_QryHvsDHPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 At_TestHvsDHPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 At_QryHvsContPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 At_SetHvsstPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 At_QryHvsstPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 At_TestHvsstPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 At_DealRSFWVsim(VOS_UINT8 ucIndex);
extern VOS_UINT32 At_SetHvpDHPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 At_TestHvpDHPara(VOS_UINT8 ucIndex);
extern VOS_UINT16 At_HvsstQueryCnf(SI_PIH_EVENT_INFO_STRU *pstEvent);
extern VOS_UINT16 AT_HvsDHQueryCnf(SI_PIH_EVENT_INFO_STRU *pstEvent);
#if (FEATURE_ON == MBB_FEATURE_VSIM_HUK)
extern VOS_UINT32 At_SetHvADHPara(VOS_UINT8 ucIndex);
extern VOS_UINT16 At_SetHvADHCnf(SI_PIH_EVENT_INFO_STRU *pstEvent);
extern VOS_UINT32 At_TestHvAHPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 At_SryHvsContPara(VOS_UINT8 ucIndex);
extern VOS_UINT16 At_SetHvsContCnf(SI_PIH_EVENT_INFO_STRU *pstEvent);
extern VOS_UINT32 At_SetHvshDHPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 At_TestHvshDHPara(VOS_UINT8 ucIndex);
extern VOS_UINT16 AT_HvshDHSetCnf(SI_PIH_EVENT_INFO_STRU *pstEvent);
extern VOS_UINT32 At_QryHvdieidPara(VOS_UINT8 ucIndex);
extern VOS_UINT16 AT_HvdieidQueryCnf(SI_PIH_EVENT_INFO_STRU *pstEvent);

extern VOS_UINT32 At_SetHvdDHPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 At_TestHvdDHPara(VOS_UINT8 ucIndex);
extern VOS_UINT16 AT_HvdDHSetCnf(SI_PIH_EVENT_INFO_STRU *pstEvent);

#else

extern VOS_UINT16 At_HvsContQueryCnf(SI_PIH_EVENT_INFO_STRU *pstEvent);
#endif/*end (FEATURE_ON == MBB_FEATURE_VSIM_HUK)*/
#endif/*end (FEATURE_VSIM == FEATURE_ON)*/

extern VOS_UINT16 AT_UiccAuthCnf(TAF_UINT8 ucIndex,SI_PIH_EVENT_INFO_STRU *pstEvent);
extern VOS_UINT16 AT_UiccAccessFileCnf(TAF_UINT8 ucIndex,SI_PIH_EVENT_INFO_STRU *pstEvent);



#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of AtCmdSimProc.h */
