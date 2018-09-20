/******************************************************************************
 */
/*
 */
/*                  版权所有 (C), 2001-2011, 华为技术有限公司
 */
/*
 */
/******************************************************************************
 */
/*  文 件 名   : at_lte_eventreport_mbb.h
 */
/*  版 本 号   : V1.0
 */

/*  生成日期   : 2014-06-09
 */
/*  功能描述   : 产品线自研及修改at命令文件
 */
/*  函数列表   :
 */
/*  修改历史   :
 */
/*  1.日    期 : 2011-03-10
 */

/*    修改内容 : 创建文件
 */
/*
 */
/******************************************************************************
 */
/*************************************************************************************
*************************************************************************************/



#ifndef _MBB_AT_GU_COMMON_H__
#define _MBB_AT_GU_COMMON_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "vos.h"
#include "AtCtx.h"
#include "AtInputProc.h"
#include "product_nv_def.h"
#include "product_nv_id.h"
#include "product_config.h"
#include "DrvInterface.h"
#include "at_lte_common.h"
#include "AtEventReport.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if (FEATURE_ON == MBB_WPG_COMMON)

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define AT_HCSQ_RAT_NAME_MAX             (255)
#define AT_HCSQ_RSSI_VALUE_MIN           (-120)
#define AT_HCSQ_RSSI_VALUE_MAX           (-25)
#define AT_HCSQ_LEVEL_MIN                      (0)
#define AT_HCSQ_RSSI_LEVEL_MAX            (96)   
#define AT_HCSQ_RSCP_VALUE_MIN           (-120)
#define AT_HCSQ_RSCP_VALUE_MAX           (-25)
#define AT_HCSQ_RSCP_LEVEL_MAX            (96 )
#define AT_HCSQ_ECIO_VALUE_MIN           (-32)
#define AT_HCSQ_ECIO_VALUE_MAX           (0)
#define AT_HCSQ_ECIO_LEVEL_MAX           (65)
#define AT_HCSQ_VALUE_INVALID             (255)
#define MMA_PLMN_ID_LEN                         (6)
#define AT_COPN_LEN_AND_RT                  (7)
#define AT_MSG_7BIT_MASK                       (0x7f)

#define TIME_INFO_DEBUG_VAR      (3000)

#define SYSCFGEX_MODE_AUTO       (0x00)
#define SYSCFGEX_MODE_GSM        (0x01)
#define SYSCFGEX_MODE_WCDMA      (0x02)
#define SYSCFGEX_MODE_LTE        (0x03)
#define SYSCFGEX_MODE_CDMA       (0x04)
#define SYSCFGEX_MODE_TDSCDMA    (0x05)
#define SYSCFGEX_MODE_WIMAX      (0x06)
#define SYSCFGEX_MODE_NOT_CHANGE (0x99)
#define SYSCFGEX_MODE_INVALID    (0xFF)
#define SYSCFGEX_MAX_RAT_STRNUM  (7) /*gul每个模式2个字符再加上\0*/

extern AT_DEBUG_INFO_STRU g_stAtDebugInfo;

#if(FEATURE_ON == MBB_FEATURE_MPDP)
#define MAX_NDIS_NET                                     (8)
extern UDI_HANDLE g_ulAtUdiNdisMpdpHdl[MAX_NDIS_NET];
#endif/*FEATURE_ON == MBB_FEATURE_MPDP*/

#define AT_DATACLASS_MAX                  AT_DATACLASS_DC_HSPAPLUS
#define AT_DATACLASS_BASE_VALUE           (0X01)
#define AT_DATACLASSLTE_MAX               (0x01)
#define AT_DATACLASS_LTE                  (0x01)
/* 系统模式名称的字符串长度，暂定为16 */
#define AT_DATACLASS_NAME_LEN_MAX         (16)
#define AT_DATACLASS_NOT_SUPPORT          (0)
#define AT_DATACLASS_SUPPORT              (1)
#define AT_DATACLASS_HSPASTATUS_ACTIVED   (1)
#define AT_DATACLASS_ENASRELINDICATOR_R5  (2)
#define AT_DATACLASS_ENASRELINDICATOR_R6  (3)
#define AT_DATACLASS_ENASRELINDICATOR_R7  (4)
#define AT_DATACLASS_ENASRELINDICATOR_R8  (4)
/*****************************************************************************
  3 枚举定义
*****************************************************************************/


/*****************************************************************************
  4 消息头定义
*****************************************************************************/


/*****************************************************************************
  5 消息定义
*****************************************************************************/


/*****************************************************************************
  6 STRUCT定义
*****************************************************************************/
typedef struct
{
    TAF_UINT16   MCC;
    TAF_INT8   Zone;
    TAF_UINT8   Reserved;
}MCC_ZONE_INFO_STRU;


typedef struct
{
    TAF_UINT8  ucLteSupport;
    TAF_UINT8  ucWcdmaSupport;
    TAF_UINT8  ucGsmSupport;
    TAF_UINT8  aucAutoAcqorder[SYSCFGEX_MAX_RAT_STRNUM];
    TAF_UINT8  reserve1;
    TAF_UINT8  reserve2;
}MBB_RAT_SUPPORT_STRU;

extern MBB_RAT_SUPPORT_STRU g_MbbIsRatSupport;

enum AT_DATACLASS_ENUM
{
    AT_DATACLASS_GSM,
    AT_DATACLASS_GPRS,
    AT_DATACLASS_EDGE,
    AT_DATACLASS_WCDMA,
    AT_DATACLASS_HSDPA,
    AT_DATACLASS_HSUPA,
    AT_DATACLASS_HSPA,
    AT_DATACLASS_HSPAPLUS,
    AT_DATACLASS_DC_HSPAPLUS,
};
/*****************************************************************************
  7 UNION定义
*****************************************************************************/


/*****************************************************************************
  8 OTHERS定义
*****************************************************************************/


VOS_UINT32 AT_GetTimeInfoDebugFlag(VOS_VOID);
#define AT_EVENT_REPORT_LOG_1(str1, var1) \
{\
    if (VOS_TRUE == AT_GetTimeInfoDebugFlag())\
    {\
        (VOS_VOID)vos_printf("======>%s,%d, %s=%x\n", __func__, __LINE__, str1, var1);\
    }\
}

#define AT_JUDGE_FDD_TDD(var) ((TAF_UTRANCTRL_UTRAN_MODE_FDD == var) ? MN_PH_SYS_MODE_EX_WCDMA_RAT : MN_PH_SYS_MODE_EX_TDCDMA_RAT)

#if (FEATURE_ON == MBB_MLOG)
#define AT_HIGH_QULITY_RSCP_FDD_MIN      (-95)
#define AT_HIGH_QULITY_RSCP_TDD_MIN      (-84)
#define AT_HIGH_QULITY_RSSI_MIN                (-85)
#define MBB_LOG_RSSI_INFO(var1)\
{\
    if(AT_HIGH_QULITY_RSSI_MIN > var1)\
    {\
        mlog_print("at", mlog_lv_error, "rssi is %d.\n", var1);\
    }\
}

#define MBB_LOG_FDD_RSCP_INFO(var1,var2)\
{\
    if(AT_HIGH_QULITY_RSCP_FDD_MIN > var1)\
    {\
        mlog_print("at",mlog_lv_error,"rscp is %d, ecio is %d.\n",var1, var2);\
    }\
}

#define MBB_LOG_TDD_RSCP_INFO(var1)\
{\
    if(AT_HIGH_QULITY_RSCP_TDD_MIN > var1)\
    {\
        mlog_print("at", mlog_lv_error, "rscp is %d.\n",var1);\
    }\
}
#else
#define MBB_LOG_RSSI_INFO(var1)
#define MBB_LOG_FDD_RSCP_INFO(var1,var2)
#define MBB_LOG_TDD_RSCP_INFO(var1)
#endif

#if (FEATURE_ON == MBB_WPG_LTE_ATTACH_APN_LIST)
MN_PS_EVT_FUNC AT_MbbGetTafPsEventProcFunc(VOS_UINT32 ulEventId);
#endif
#if (FEATURE_ON == MBB_WPG_HFEATURESTAT)
enum AT_HFEATURE_ID_ENUM
{
    AT_HFEATURE_SINGLE_PDN = 1,/*目前仅支持SINGLE PDN 其他特性待后续扩展*/
    AT_HFEATURE_BUTT,
};
enum AT_HFEATURE_STATE_ENUM
{
    AT_HFEATURE_NOT_OPEN = 0,/*特性未激活*/
    AT_HFEATURE_OPEN,/*特性激活*/
};
#endif/*FEATURE_ON == MBB_WPG_HFEATURESTAT*/
VOS_VOID AT_QryParaRspHcsqProc( VOS_UINT8 ucIndex, VOS_UINT8 ucOpId, VOS_VOID *pPara);
#if(FEATURE_ON == MBB_FEATURE_BOX_FTEN)
VOS_VOID AT_QryParaRspCindProc( VOS_UINT8 ucIndex, VOS_UINT8 ucOpId, VOS_VOID *pPara);
#endif
VOS_VOID AT_RssiConvert(VOS_INT32 lRssiValue, VOS_UINT8 *pucRssiLevel );
VOS_VOID AT_RscpConvert(VOS_INT16 lRscpValue, VOS_UINT8 *pucRscpLevel );
VOS_VOID AT_EcioConvert(VOS_INT16 lEcioValue, VOS_UINT8 *pucEcioLevel );

extern VOS_VOID atLteSetReportSignalFlag(TAF_UINT8 ServiceStatus);
extern VOS_VOID At_FormatAndSndEons0(TAF_UINT8 ucIndex, TAF_PHONE_EVENT_INFO_STRU *pEvent);
extern VOS_VOID At_FormatSyscfgMbb(AT_MODEM_NET_CTX_STRU *pstNetCtx, TAF_PHONE_EVENT_INFO_STRU* pEvent, VOS_UINT8 ucIndex);
extern VOS_UINT32 At_FormatDlckCnf(TAF_UINT16* usLength, TAF_UINT8 ucIndex, TAF_PHONE_EVENT_INFO_STRU  *pEvent);
extern VOS_VOID At_RunQryParaRspProcCus(TAF_UINT8 ucIndex,TAF_UINT8 OpId, TAF_VOID *pPara, TAF_PARA_TYPE QueryType);
extern VOS_VOID At_Pb_VodafoneCPBSCus(TAF_UINT16* usLength, TAF_UINT8 ucIndex);
extern VOS_VOID At_FormatSysinfoExMbb(VOS_UINT16* usLength, TAF_PH_SYSINFO_STRU* stSysInfo);
extern VOS_VOID At_FormatRssiInfo(TAF_UINT8 ucIndex,TAF_PHONE_EVENT_INFO_STRU *pEvent);
extern VOS_VOID AT_ProcSimRefreshInd(VOS_UINT8 ucIndex, const TAF_PHONE_EVENT_INFO_STRU *pstEvent);

extern VOS_VOID AT_ReadNvMbbCustorm(VOS_VOID);
extern VOS_VOID AT_ReadVodafoneCpbsNV( VOS_VOID );
extern VOS_VOID AT_ReadCsimCustomizationNV( VOS_VOID );
extern VOS_UINT16 AT_IsVodafoneCustommed(VOS_VOID);
extern VOS_UINT16 AT_IsCSIMCustommed(VOS_VOID);
extern VOS_UINT32 At_RegisterExPrivateMbbCmdTable(VOS_VOID);
extern VOS_UINT32 AT_PS_ReportDefaultDhcp(VOS_UINT8 ucIndex);
extern VOS_UINT32 AT_PS_ReportDefaultDhcpV6(VOS_UINT8 ucIndex);
extern VOS_UINT32 AT_PS_ReportDefaultApraInfo(TAF_UINT8 ucIndex);

extern VOS_UINT8                AT_GetIPv6VerFlag(VOS_VOID);
extern VOS_VOID                 AT_SetIPv6VerFlag( VOS_UINT8 ucFlag );
extern TAF_PDP_TYPE_ENUM_UINT8  AT_GetPdpTypeForNdisDialup(VOS_VOID);
extern VOS_UINT32 At_IsForbiddenAtCmd(VOS_CHAR* pszCmdName, VOS_UINT8 ucClientId);
#ifdef BSP_CONFIG_BOARD_WINGLE_MS2172S_818
#if(FEATURE_OFF == MBB_FEATURE_BOX_FTEN)
extern VOS_UINT32 At_IsGlobalForbiddenAtCmd(VOS_CHAR* pszCmdName, VOS_UINT8 ucClientId);
#endif
#endif
extern VOS_UINT8 AT_GetSyscfgexModeRestrictFlag(VOS_VOID);
extern VOS_UINT32 At_CheckSpecRatOrderInModeList(AT_SYSCFGEX_RAT_ORDER_STRU *pstSysCfgExRatOrder);
extern VOS_VOID AT_PS_SndIPV4FailedResult(VOS_UINT8 ucCallId, VOS_UINT16 usClientId);
extern VOS_VOID AT_PS_ProcIpv4CallRejectEx(VOS_UINT8 ucCallId, TAF_PS_CALL_PDP_ACTIVATE_REJ_STRU  *pstEvent);
extern VOS_UINT32 AT_PS_ValidateDialParamEx(VOS_UINT8 ucIndex);
extern VOS_UINT32 AT_PS_ParseUsrDialParamEx(VOS_UINT8 ucIndex, 
    AT_DIAL_PARAM_STRU *pstUsrDialParam, 
    TAF_PDP_PRIM_CONTEXT_STRU* stPdpCtxInfo);
extern VOS_UINT32 AT_PS_CheckSyscfgexModeRestrictPara(VOS_UINT32* ulRst, AT_SYSCFGEX_RAT_ORDER_STRU* stSyscfgExRatOrder);
extern TAF_UINT32 AT_SetDsFlowQryParaEx(TAF_UINT8 ucIndex);
extern VOS_UINT32 AT_GetNdisDialParamEx(TAF_PS_DIAL_PARA_STRU *pstDialParaInfo, VOS_UINT8 ucIndex);
extern VOS_VOID AT_UpdateCclkInfo(NAS_MM_INFO_IND_STRU* pStMmInfo);
/*向PC上报连接状态的at函数*/
extern VOS_VOID AT_PS_ReportDendNDISSTATEX(VOS_UINT8 ucCid,
    VOS_UINT8                           ucPortIndex,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType,
    TAF_PS_CAUSE_ENUM_UINT32            enCause);
/*向PC上报断开状态的at函数*/
extern VOS_VOID AT_PS_ReportDconnNDISSTATEX(VOS_UINT8 ucCid,
    VOS_UINT8                           ucPortIndex,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType);
extern VOS_UINT32 AT_QryNdisStatParaEx( VOS_UINT8 ucIndex );

extern VOS_VOID AT_MBBConverAutoMode(TAF_PH_RAT_ORDER_STRU    *pstSysCfgRatOrder);
#endif

#if (FEATURE_OFF == MBB_WPG_COMMON)
#define MBB_LOG_RSSI_INFO(var1)
#define MBB_LOG_FDD_RSCP_INFO(var1,var2)
#define MBB_LOG_TDD_RSCP_INFO(var1)
#define AP_CONN_DEBUG_STR_VAR(str, var)
#define AP_CONN_DEBUG()
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


#endif /*__AT_H__
 */






