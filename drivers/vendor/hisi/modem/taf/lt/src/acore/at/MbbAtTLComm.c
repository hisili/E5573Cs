/******************************************************************************
 */
/*
 */
/*                  版权所有 (C), 1998-2009, 华为技术有限公司
 */
/*
 */
/******************************************************************************
 */
/*  文 件 名   : at_lte_eventreport.c
 */
/*  版 本 号   : V1.0
 */

/*  生成日期   : 2011-10-22
 */
/*  功能描述   : LTE命令上报处理
 */
/*
 */
/*  函数列表   : TODO: ...
 */
/*  修改历史   :
 */
/*  1.日    期 : 2011-10-22
 */

/*    修改内容 : 创建文件
 */
/*
 */
/******************************************************************************
 */


#include "MbbPsCsCommon.h"
#include "linux/mlog_lib.h"
#include "at_lte_common.h"
#ifdef  __cplusplus
  #if  __cplusplus
  extern "C"{
  #endif
#endif

#if (FEATURE_ON == MBB_WPG_COMMON)
VOS_UINT32 g_ulHreqInfoDebug = VOS_FALSE;
#define MBB_AT_TL_COMM_DEBUG(str, var) \
    do\
    {\
        if (VOS_TRUE == g_ulHreqInfoDebug)\
        {\
            (VOS_VOID)vos_printf("%s, %d, %s=%d\n", str, var);\
        }\
    }while(0)

#define MBB_AT_TL_COMM_DEBUG_STR(str) \
    do\
    {\
        if (VOS_TRUE == g_ulHreqInfoDebug)\
        {\
            (VOS_VOID)vos_printf("%s, %d, %s\n", str);\
        }\
    }while(0)

#define AT_HCSQ_LTE_RSSI_VALUE_MIN           (-120)

#define AT_HCSQ_LTE_RSSI_VALUE_MAX           (-25)

#define AT_HCSQ_LTE_RSSI_LEVEL_MIN            (0)

#define AT_HCSQ_LTE_RSSI_LEVEL_MAX            (96)   

#define AT_HCSQ_LTE_RSRP_VALUE_MIN           (-140)

#define AT_HCSQ_LTE_RSRP_VALUE_MAX           (-44)

#define AT_HCSQ_LTE_RSRP_LEVEL_MIN            (0)

#define AT_HCSQ_LTE_RSRP_LEVEL_MAX            (97) 

#define AT_HCSQ_LTE_RSRQ_VALUE_MIN           (-39)

#define AT_HCSQ_LTE_RSRQ_VALUE_MAX           (-3)

#define AT_HCSQ_LTE_RSRQ_LEVEL_MIN            (0)

#define AT_HCSQ_LTE_RSRQ_LEVEL_MAX           (34)

#define AT_HCSQ_LTE_SINR_VALUE_MAX            (30)

#define AT_HCSQ_LTE_SINR_VALUE_MIN           (-20)

#define AT_HCSQ_LTE_SINR_LEVEL_MAX            (251)

#define AT_HCSQ_LTE_SINR_LEVEL_MIN            (0)

#define AT_HCSQ_LTE_UNKNOWN                   (99)

#define AT_HCSQ_LTE_INVALID                   (255)

#define AT_HCSQ_LTE_SINR_LEVEL               (23)


#if (FEATURE_ON == MBB_MLOG)
#define AT_HIGH_QULITY_RSRP_MIN              (-110)
#endif
#if(FEATURE_ON == MBB_FEATURE_BOX_FTEN)
extern VOS_UINT8 AT_CalculateLTEAntennaLevel(VOS_INT16 usRsrp);
extern VOS_UINT8 AT_GetSmoothLTEAntennaLevel(AT_CMD_ANTENNA_LEVEL_ENUM_UINT8 enLevel);
static AT_CMD_ANTENNA_LEVEL_ENUM_UINT8     g_oldLteAntLevel = AT_CMD_ANTENNA_LEVEL_BUTT;
extern AT_CMD_CIND_PARA     g_ucCindpara;
extern AT_CMD_CMER_PARA     g_ucCmerpara;
#define CIND_LTE_RESERVED                    (0)         /*+CIND命令LTE 中预留位的默认值0*/
#define TAF_CIND_SRVSTA_NORMAL_SERVICE       (1)         /*+CIND命令返回值表示正常服务 */
#define TAF_CIND_SRVSTA_NO_SERVICE           (0)         /*+CIND命令返回值表示无服务*/
#define CIEV_SIGNAL_FLAG                     (2)         /*AT+CIEV主动上报信号水平标示*/
#define AT_CIND_RPT_SWITCH_ON                (1)          /* +CIND命令标示开关为开*/
#define AT_CMER_RPT_MODE_SWITCH_ON           (3)          /* +CMER命令标示，mode开关为开*/
#define AT_CMER_RPT_SWITCH_ON                (1)          /* +CMER命令标示开关为开*/
#define AT_ANT_TEMPPRT_LEVEL                 (99)
extern VOS_UINT8              g_TempprtEvent_flag;
#endif
VOS_UINT16        gusbandwidth[]  = {1400, 3000, 5000, 10000, 15000, 20000};
LTE_BANDINFO_STRU gstAtBandinfo[] =
{
    {0,     0,     0,     0},                 /*reserved, so that bandno correspond to index directly*/
    {21100, 0,     19200, 18000},             /* FDD:Band1 */
    {19300, 600,   18500, 18600},             /* FDD:Band2 */
    {18050, 1200,  17100, 19200},             /* FDD:Band3 */
    {21100, 1950,  17100, 19950},             /* FDD:Band4 */
    {8690,  2400,  8240,  20400},             /* FDD:Band5 */
    {8750,  2650,  8300,  20650},             /* FDD:Band6 */
    {26200, 2750,  25000, 20750},             /* FDD:Band7 */
    {9250,  3450,  8800,  21450},             /* FDD:Band8 */
    {18449, 3800,  17499, 21800},             /* FDD:Band9 */
    {21100, 4150,  17100, 22150},             /* FDD:Band10 */
    {14759, 4750,  14279, 22750},             /* FDD:Band11 */
    {7290,  5010,  6990,  23010},             /* FDD:Band12 */
    {7460,  5180,  7770,  23180},             /* FDD:Band13 */
    {7580,  5280,  7880,  23280},             /* FDD:Band14 */
    {0,     0,     0,     0},                 /* reserved for b15*/
    {0,     0,     0,     0},                 /* reserved for b16*/
    {7340,  5730,  7040,  23730},             /* FDD:Band17 */
    {8600,  5850,  8150,  23850},             /* FDD:Band18 */
    {8750,  6000,  8300,  24000},             /* FDD:Band19 */
    {7910,  6150,  8320,  24150},             /* FDD:Band20 */
    {14959, 6450,  14479, 24450},             /* FDD:Band21 */
    {35100, 6600,  34100, 24600},             /* FDD:Band22*/
    {21800, 7500,  20000, 25500},             /* FDD:Band23 */
    {15250, 7700,  16265, 25700},             /* FDD:Band24 */
    {19300, 8040,  18500, 26040},             /* FDD:Band25 */
    {8590,  8690,  8140,  26690},             /* FDD:Band26 */
    {0,     0,     0,     0},                 /* reserved for b27 */
    {0,     0,     0,     0},                 /* reserved for b28 */
    {0,     0,     0,     0},                 /* reserved for b29 */
    {0,     0,     0,     0},                 /* reserved for b30 */
    {0,     0,     0,     0},                 /* reserved for b31 */
    {0,     0,     0,     0},                 /* reserved for b32 */
    {19000, 36000, 19000, 36000},             /* TDD:Band33 */
    {20100, 36200, 20100, 36200},             /* TDD:Band34 */
    {18500, 36350, 18500, 36350},             /* TDD:Band35 */
    {19300, 36950, 19300, 36950},             /* TDD:Band36 */
    {19100, 37550, 19100, 37550},             /* TDD:Band37 */
    {25700, 37750, 25700, 37750},             /* TDD:Band38 */
    {18800, 38250, 18800, 38250},             /* TDD:Band39 */
    {23000, 38650, 23000, 38650},             /* TDD:Band40 */
    {24960, 39650, 24960, 39650},             /* TDD:Band41 */
    {34000, 41590, 34000, 41590},             /* TDD:Band42 */
    {36000, 43590, 36000, 43590},             /* TDD:Band43 */
};
AT_HFREQINFO_LTE_INFO_STRU g_LastLteInfo = {0};

VOS_UINT32 atSetHcsqCnfSameProc(VOS_VOID *pMsgBlock);
#if(FEATURE_ON == MBB_FEATURE_BOX_FTEN)
VOS_UINT32 atSetCindCnfSameProc(VOS_VOID *pMsgBlock);
VOS_UINT32 atSetAnlevelCnfantProc(VOS_VOID *pMsgBlock);
#endif
VOS_UINT32 atSetLtersrpCnfSameProc(VOS_VOID *pMsgBlock);
VOS_UINT32 atHcsqIndProc(VOS_VOID *pMsgBlock);
VOS_UINT32 atLtersrpIndProc(VOS_VOID *pMsgBlock);
VOS_UINT32 atHFreqinfoInd(VOS_VOID *pMsgBlock);

extern VOS_UINT32 at_CsqInfoProc(VOS_VOID *pMsgBlock,AT_ANLEVEL_INFO_CNF_STRU* pAnlevelAnqueryInfo);

/* MBB自己增加的AT  命令响应处理*/
static const AT_L4A_MSG_FUN_TABLE_STRU g_astAtL4aCnfMsgFunTableMbb[] = { 
    {ID_MSG_L4A_HCSQ_INFO_CNF, atSetHcsqCnfSameProc     },  
    {ID_MSG_L4A_LTERSRP_INFO_CNF, atSetLtersrpCnfSameProc     },
#if(FEATURE_ON == MBB_FEATURE_BOX_FTEN)
    {ID_MSG_L4A_CIND_INFO_CNF, atSetCindCnfSameProc     },
    {ID_MSG_L4A_ANT_INFO_CNF, atSetAnlevelCnfantProc    },
#endif
};

static const AT_L4A_MSG_FUN_TABLE_STRU g_astAtL4aIndMsgFunTableMbb[] = {
    {ID_MSG_L4A_LWCLASH_IND,    atHFreqinfoInd},
    {ID_MSG_L4A_HCSQ_IND,           atHcsqIndProc},   
    {ID_MSG_L4A_LTERSRP_IND ,    atLtersrpIndProc},   
};


AT_L4A_MSG_FUN_TABLE_STRU* atL4aGetCnfMsgFunMbb(VOS_UINT32 ulMsgId)
{
    VOS_UINT32 i = 0;
    VOS_UINT32 ulTableLen = 0;
    const AT_L4A_MSG_FUN_TABLE_STRU* pTable;

    pTable = g_astAtL4aCnfMsgFunTableMbb;
    ulTableLen = sizeof(g_astAtL4aCnfMsgFunTableMbb);

    for(i = 0; i < (ulTableLen/sizeof(AT_L4A_MSG_FUN_TABLE_STRU)); i++)
    {
        if(ulMsgId == (pTable + i)->ulMsgId)
        {
            return (AT_L4A_MSG_FUN_TABLE_STRU*)(pTable + i);
        }
    }
    return NULL;
}

AT_L4A_MSG_FUN_TABLE_STRU* atL4aGetIndMsgFunMbb(VOS_UINT32 ulMsgId)
{
    VOS_UINT32 i = 0;
    VOS_UINT32 ulTableLen = 0;
    const AT_L4A_MSG_FUN_TABLE_STRU* pTable;

    pTable = g_astAtL4aIndMsgFunTableMbb;
    ulTableLen = sizeof(g_astAtL4aIndMsgFunTableMbb);

    for(i = 0; i < (ulTableLen/sizeof(AT_L4A_MSG_FUN_TABLE_STRU)); i++)
    {
        if(ulMsgId == (pTable + i)->ulMsgId)
        {
            return (AT_L4A_MSG_FUN_TABLE_STRU*)(pTable + i);
        }
    }
    return NULL;
}


VOS_UINT32 atHFreqinfoInd(VOS_VOID *pMsgBlock)
{
    VOS_UINT8  ucSumOfBand = 0;
    VOS_UINT8  ucSumOfBandwidth = 0;  
    VOS_UINT16 usLength = 0;
    VOS_UINT16 usNdl = 0;
    VOS_UINT16 usNul = 0;
    VOS_UINT16 usUlBandwidth = 0;
    VOS_UINT16 usDlBandwidth = 0;
    L4A_READ_LWCLASH_IND_STRU *pstLwclash = NULL;
    LTE_BANDINFO_STRU stCurBandinfo = {0};

    if (VOS_NULL == pMsgBlock)
    {
        return AT_ERROR;
    }
    
    pstLwclash = (L4A_READ_LWCLASH_IND_STRU *)pMsgBlock;
      
    /*获取带宽数组的长度*/
    ucSumOfBandwidth = sizeof(gusbandwidth) / sizeof(gusbandwidth[0]);   
    if((pstLwclash->stLwclashInfo.usUlBandwidth >= ucSumOfBandwidth) || (pstLwclash->stLwclashInfo.usDlBandwidth >= ucSumOfBandwidth))
    {
        MBB_AT_TL_COMM_DEBUG_STR("the receive bandwith beyond length of array");
        return AT_ERROR;
    }
    
    /*band合法性判断*/
    ucSumOfBand = sizeof(gstAtBandinfo) / sizeof(gstAtBandinfo[0]);/*44*/
    if(pstLwclash->stLwclashInfo.usBand >= ucSumOfBand)
    {
        
        MBB_AT_TL_COMM_DEBUG_STR("the received band beyond length of ucSumOfBand");
        return AT_ERROR;
    }

    if((0 == pstLwclash->stLwclashInfo.usDlFreq) || (0 == pstLwclash->stLwclashInfo.usUlFreq))
    {
        /*当从驻留态离开时不再主动上报HFREQINFO*/
        MBB_AT_TL_COMM_DEBUG_STR("");
        return AT_OK;
    }

    /*hfreqinfo消息中判断条件LTE制式的条件无效，删除判断条件*/
    MBB_AT_TL_COMM_DEBUG("pstLwclash->usClientId", pstLwclash->usClientId);

    stCurBandinfo = gstAtBandinfo[pstLwclash->stLwclashInfo.usBand];
    /*下行频点Ndl = (Fdl - Fdl_low) + Noff_dl*/
    usNdl = (pstLwclash->stLwclashInfo.usDlFreq - stCurBandinfo.ulDLfreqlow) + stCurBandinfo.ulNoffdl;
    /*上行频点Nul = (Ful - Ful_low) + Noff_ul*/
    usNul = (pstLwclash->stLwclashInfo.usUlFreq - stCurBandinfo.ulULfreqlow) + stCurBandinfo.ulNofful;
    usUlBandwidth = gusbandwidth[pstLwclash->stLwclashInfo.usUlBandwidth];/*使用lwclash返回的枚举值作为索引获取带宽*/
    usDlBandwidth = gusbandwidth[pstLwclash->stLwclashInfo.usDlBandwidth];/*使用lwclash返回的枚举值作为索引获取带宽*/

    /*本次值合法，保存本次上报值*/
    g_LastLteInfo.usBand = pstLwclash->stLwclashInfo.usBand;
    g_LastLteInfo.usNdl = usNdl;
    g_LastLteInfo.usDlFreq = pstLwclash->stLwclashInfo.usDlFreq;
    g_LastLteInfo.usDlBandwidth = usDlBandwidth;
    g_LastLteInfo.usNul = usNul;
    g_LastLteInfo.usUlFreq = pstLwclash->stLwclashInfo.usUlFreq;
    g_LastLteInfo.usUlBandwidth = usUlBandwidth;

    if (AT_HFREQINFO_REPORT == g_AtHFreqinforeport)
    {
        usLength = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr,
                                           "%s^HFREQINFO:%d,%d,%d,%d,%d,%d,%d,%d,%d%s",
                                           gaucAtCrLf,
                                           g_AtHFreqinforeport,
                                           AT_HFREQINFO_RAT_TYPE_LTE,
                                           pstLwclash->stLwclashInfo.usBand,
                                           usNdl,
                                           pstLwclash->stLwclashInfo.usDlFreq,
                                           usDlBandwidth,
                                           usNul,
                                           pstLwclash->stLwclashInfo.usUlFreq,
                                           usUlBandwidth,
                                           gaucAtCrLf);
        At_SendResultData(AT_BROADCAST_CLIENT_INDEX_MODEM_0, pgucLAtSndCodeAddr, usLength);
    }
    return AT_FW_CLIENT_STATUS_READY;
}


VOS_UINT32 atSetLtersrpCnfSameProc(VOS_VOID *pMsgBlock)
{
    
    VOS_UINT16 usLength = 0;    
    VOS_UINT32 ulResult = 0;    
    VOS_INT16 sRsrp = 0;    
    VOS_INT16 sRsrq = 0;
    AT_ANLEVEL_INFO_CNF_STRU stLteRsrpInfo = {0};

    if (VOS_NULL == pMsgBlock)
    {
        return AT_ERROR;
    }

    /*调用已有接口函数填充LteRsrpInfo结构体*/
    ulResult = at_CsqInfoProc(pMsgBlock,&stLteRsrpInfo);   
    if(ERR_MSP_SUCCESS == ulResult)    
    {
      
        sRsrp = stLteRsrpInfo.sRsrp;        
        sRsrq = stLteRsrpInfo.sRsrq;        
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,                    
                                        (VOS_CHAR *)pgucLAtSndCodeAddr + usLength,"%s%d,%d",
                                         "^LTERSRP:",sRsrp,
                                        sRsrq);
    }
    else
    {
        HAL_SDMLOG(" atSetLtersrpCnfSameProc ulResult = %d,\n",(int)ulResult);
   
    }
    CmdErrProc((VOS_UINT8)(stLteRsrpInfo.usClientId), stLteRsrpInfo.ulErrorCode,                        
                            usLength, pgucLAtSndCodeAddr);
    
    return AT_FW_CLIENT_STATUS_READY;
}

VOS_UINT32 at_RssiProc(VOS_VOID *pMsgBlock, L4A_HCSQ_STRU* pHcsqInfo)
{
    
    VOS_UINT32 ulResult = ERR_MSP_SUCCESS;    
    L4A_CSQ_INFO_IND_STRU* pCsqInfo = NULL;    
    VOS_UINT8 ucIndexId = 0;    
    /*判断MsgBlock是否为空*/    
    if((NULL == pMsgBlock) || (NULL == pHcsqInfo))    
    {       
        HAL_SDMLOG("pMsgBlock is NULL!\n");
        return AT_ERROR;      
    }
  
    pCsqInfo = (L4A_CSQ_INFO_IND_STRU*)pMsgBlock;
    
    /*对CilentId、OpId、ErrorCode、Rssi、Rsrp、Rsrq赋值*/
    
    (VOS_VOID)At_ClientIdToUserId((VOS_UINT16)pCsqInfo->usClientId, &ucIndexId);    
    pHcsqInfo->stCtrl.ulClientId = (VOS_UINT32)ucIndexId;    
    pHcsqInfo->stCtrl.ulOpId = pCsqInfo->ucOpId;    
    pHcsqInfo->ulErrorCode = pCsqInfo->ulErrorCode;    
    pHcsqInfo->sRssi = pCsqInfo->sRssi;    
    pHcsqInfo->sRsrp = pCsqInfo->sRsrp;    
    pHcsqInfo->sRsrq = pCsqInfo->sRsrq;     
    pHcsqInfo->sSinr  = pCsqInfo->lSINR;    
    return ulResult;
}

VOS_UINT32 atSetHcsqCnfSameProc(VOS_VOID *pMsgBlock)
{   
  
    VOS_UINT16 usLength = 0; 
    VOS_UINT32 ulResult = 0; 
    VOS_UINT8 uRsrp = 0;
    VOS_UINT8 uRsrq = 0; 
    VOS_UINT8 uRssi = 0; 
 
    VOS_UINT8 uSinr = AT_HCSQ_LTE_SINR_LEVEL; 
    L4A_HCSQ_STRU stHcsqInfo = {{0,0,0},0,0,0,0,0,0}; 
  
    if(NULL == pMsgBlock)  
    { 
     
        HAL_SDMLOG("pMsgBlock is NULL!\n");      
        return AT_ERROR;
    }
    
  
    /*调用已有接口函数填充HcsqInfo结构体*/   
    ulResult = at_RssiProc(pMsgBlock,&stHcsqInfo); 
    if(ERR_MSP_SUCCESS == ulResult) 
    {    
        /*对rssi值进行转换*/      
        if(AT_HCSQ_LTE_UNKNOWN <= stHcsqInfo.sRssi)     
        {      
            uRssi = AT_HCSQ_LTE_INVALID; 
        }  
        else if(AT_HCSQ_LTE_RSSI_VALUE_MAX <= stHcsqInfo.sRssi)    
        {       
            uRssi = AT_HCSQ_LTE_RSSI_LEVEL_MAX;   
        } 
        else if (AT_HCSQ_LTE_RSSI_VALUE_MIN > stHcsqInfo.sRssi)      
        {     
            uRssi = AT_HCSQ_LTE_RSSI_LEVEL_MIN;    
        }     
        else    
        {    
            uRssi = (VOS_UINT8)((stHcsqInfo.sRssi - AT_HCSQ_LTE_RSSI_VALUE_MIN) + 1);  
        }
        /*对rsrp进行转换*/
   
        if(AT_HCSQ_LTE_UNKNOWN <= stHcsqInfo.sRsrp)    
        {     
            uRsrp = AT_HCSQ_LTE_INVALID;   
        }    
        else if (AT_HCSQ_LTE_RSRP_VALUE_MAX <= stHcsqInfo.sRsrp)  
        {           
            uRsrp = AT_HCSQ_LTE_RSRP_LEVEL_MAX;     
        }     
        else if (AT_HCSQ_LTE_RSRP_VALUE_MIN > stHcsqInfo.sRsrp)    
        {    
            uRsrp = AT_HCSQ_LTE_RSRP_LEVEL_MIN;    
        } 
        else    
        {      
            uRsrp = (VOS_UINT8)((stHcsqInfo.sRsrp - AT_HCSQ_LTE_RSRP_VALUE_MIN) + 1);  
        }
        
        
        /*对rsrq进行转换*/    
        if(AT_HCSQ_LTE_UNKNOWN <= stHcsqInfo.sRsrq)    
        {      
            uRsrq = AT_HCSQ_LTE_INVALID;       
        }   
        else if (AT_HCSQ_LTE_RSRQ_VALUE_MAX <= stHcsqInfo.sRsrq)    
        {        
            uRsrq = AT_HCSQ_LTE_RSRQ_LEVEL_MAX;  
        }   
        else if (AT_HCSQ_LTE_RSRQ_VALUE_MIN > (stHcsqInfo.sRsrq * 2)) /*check min*/ 
        {   
            uRsrq = AT_HCSQ_LTE_RSRQ_LEVEL_MIN;    
        }       
        else     
        {       
            uRsrq = (VOS_UINT8)(((stHcsqInfo.sRsrq * 2) - AT_HCSQ_LTE_RSRQ_VALUE_MIN) + 1);/*calu value*/            
        }
      
        /*对sinr进行转换*/
      
        if(AT_HCSQ_LTE_UNKNOWN <= stHcsqInfo.sSinr)    
        {         
            uSinr = AT_HCSQ_LTE_INVALID;   
        }  
        else if (stHcsqInfo.sSinr >= AT_HCSQ_LTE_SINR_VALUE_MAX)      
        {       
            uSinr = AT_HCSQ_LTE_SINR_LEVEL_MAX;       
        }   
        else if (stHcsqInfo.sSinr < AT_HCSQ_LTE_SINR_VALUE_MIN)   
        {       
            uSinr = AT_HCSQ_LTE_SINR_LEVEL_MIN;
        }
        else    
        {       
            /* sinr的步长是0.2，这里要转换为整数，所以*5 */
            uSinr = (VOS_UINT8)(((stHcsqInfo.sSinr * 5) - (AT_HCSQ_LTE_SINR_VALUE_MIN * 5)) + 1);
            /*calu value*/    
        }
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,                                        
                                    (VOS_CHAR *)pgucLAtSndCodeAddr,
                                    (VOS_CHAR *)pgucLAtSndCodeAddr + usLength,        
                                    "%s%s%d,%d,%d,%d",
                                    "^HCSQ:",
                                    "\"LTE\",",            
                                    uRssi,             
                                    uRsrp,                 
                                    uSinr,                 
                                    uRsrq);
    }   
    else 
    {        
        HAL_SDMLOG(" atSetHcsqCnfSameProc ulResult = %d,\n",(int)ulResult); 
    }
   
    CmdErrProc((VOS_UINT8)(stHcsqInfo.stCtrl.ulClientId), stHcsqInfo.ulErrorCode,usLength, pgucLAtSndCodeAddr);  
    return AT_FW_CLIENT_STATUS_READY;
}
#if(FEATURE_ON == MBB_FEATURE_BOX_FTEN)

VOS_UINT32 at_RssiCindProc(VOS_VOID *pMsgBlock, L4A_CIND_STRU* pCindInfo)
{
    VOS_UINT32 ulResult = ERR_MSP_SUCCESS;
    L4A_CSQ_INFO_IND_STRU* pCsqInfo = NULL;
    VOS_UINT8 ucIndexId = 0;
    /*判断MsgBlock是否为空*/
    if((NULL == pMsgBlock) || (NULL == pCindInfo))
    {
        HAL_SDMLOG("pMsgBlock is NULL!\n");
        return AT_ERROR;
    }
    pCsqInfo = (L4A_CSQ_INFO_IND_STRU*)pMsgBlock;
    /*对CilentId、OpId、ErrorCode、Rssi、Rsrp、Rsrq赋值*/
    (VOS_VOID)At_ClientIdToUserId((VOS_UINT16)pCsqInfo->usClientId, &ucIndexId);
    pCindInfo->stCtrl.ulClientId = (VOS_UINT32)ucIndexId;
    pCindInfo->stCtrl.ulOpId = pCsqInfo->ucOpId;
    pCindInfo->ulErrorCode = pCsqInfo->ulErrorCode;
    pCindInfo->sRssi = pCsqInfo->sRssi;
    pCindInfo->sRsrp = pCsqInfo->sRsrp;
    pCindInfo->sRsrq = pCsqInfo->sRsrq;
    pCindInfo->sSinr = pCsqInfo->lSINR;
    pCindInfo->ucSrvStatus = pCsqInfo->ucSrvStatus;
    return ulResult;
}
/******************************************************************************
函数名称: at_RssiAntProc
功能描述: 将MsgBlock中的值给L4A_ANT_STRU
参数说明: pMsgBlock，MSP MSG消息结构
返 回 值: U32_T
调用要求: TODO: ...
调用举例: TODO: ...
******************************************************************************/
VOS_UINT32 at_RssiAntProc(VOS_VOID *pMsgBlock, L4A_ANT_STRU* pAntInfo)
{
    VOS_UINT32 ulResult = ERR_MSP_SUCCESS;
    L4A_CSQ_INFO_IND_STRU* pCsqInfo = NULL;
    VOS_UINT8 ucIndexId = 0;
    /*判断MsgBlock是否为空*/
    if((NULL == pMsgBlock) || (NULL == pAntInfo))
    {
        HAL_SDMLOG("pMsgBlock is NULL!\n");
        return AT_ERROR;
    }
    pCsqInfo = (L4A_CSQ_INFO_IND_STRU*)pMsgBlock;
    /*对CilentId、OpId、ErrorCode、Rssi、Rsrp、Rsrq赋值*/
    (VOS_VOID)At_ClientIdToUserId((VOS_UINT16)pCsqInfo->usClientId, &ucIndexId);
    pAntInfo->stCtrl.ulClientId = (VOS_UINT32)ucIndexId;
    pAntInfo->stCtrl.ulOpId = pCsqInfo->ucOpId;
    pAntInfo->ulErrorCode = pCsqInfo->ulErrorCode;
    pAntInfo->sRssi = pCsqInfo->sRssi;
    pAntInfo->sRsrp = pCsqInfo->sRsrp;
    pAntInfo->sRsrq = pCsqInfo->sRsrq;
    pAntInfo->sSinr = pCsqInfo->lSINR;
    pAntInfo->ucSrvStatus = pCsqInfo->ucSrvStatus;
    return ulResult;
}


VOS_VOID MN_PH_GetL4aAntLevel (VOS_UINT8 uRsrpValue, VOS_UINT8 uSinrValue, MN_PH_MODE_ANTLEVEL_ENUM_UINT8 *pucRssilv)
{
    VOS_UINT32                          auRsrpValue;
    VOS_UINT32                          auSinrValue;
    VOS_UINT32                          aulRsrp[MN_PH_MODE_ANTLEVEL_BUTT] = { 26, 31, 36, 46, 97};  /*Rsrp门限值*/
    VOS_UINT32                          aulSinr[MN_PH_MODE_ANTLEVEL_BUTT] = { 1, 86, 106, 124, 166}; /*Sinr门限值*/
    AT_CMD_ANTENNA_LEVEL_ENUM_UINT8     auSinrLevel = AT_CMD_ANTENNA_LEVEL_0;
    AT_CMD_ANTENNA_LEVEL_ENUM_UINT8     auRsrpLevel = AT_CMD_ANTENNA_LEVEL_0;
    auRsrpValue = uRsrpValue;
    auSinrValue = uSinrValue;
    /*计算RSRP对应的天线水平值*/
    if (auRsrpValue <= aulRsrp[MN_PH_MODE_ANTLEVEL_0])
    {
        auRsrpLevel = AT_CMD_ANTENNA_LEVEL_0;
    }
    else if (auRsrpValue <= aulRsrp[MN_PH_MODE_ANTLEVEL_1])
    {
        auRsrpLevel = AT_CMD_ANTENNA_LEVEL_1;
    }
    else if (auRsrpValue <= aulRsrp[MN_PH_MODE_ANTLEVEL_2])
    {
        auRsrpLevel = AT_CMD_ANTENNA_LEVEL_2;
    }
    else if (auRsrpValue <= aulRsrp[MN_PH_MODE_ANTLEVEL_3])
    {
        auRsrpLevel = AT_CMD_ANTENNA_LEVEL_3;
    }
    else if ((auRsrpValue >= aulRsrp[MN_PH_MODE_ANTLEVEL_3]) && (auRsrpValue < aulRsrp[MN_PH_MODE_ANTLEVEL_4]))
    {
        auRsrpLevel = AT_CMD_ANTENNA_LEVEL_4;
    }
    /*计算SINR对应的天线水平值*/
    if (auSinrValue <= aulSinr[MN_PH_MODE_ANTLEVEL_1])
    {
        auSinrLevel = AT_CMD_ANTENNA_LEVEL_1;
    }
    else if (auSinrValue <= aulSinr[MN_PH_MODE_ANTLEVEL_2])
    {
        auSinrLevel = AT_CMD_ANTENNA_LEVEL_2;
    }
    else if (auSinrValue <= aulSinr[MN_PH_MODE_ANTLEVEL_3])
    {
        auSinrLevel = AT_CMD_ANTENNA_LEVEL_3;
    }
    else
    {
        auSinrLevel = AT_CMD_ANTENNA_LEVEL_4;
    }
    /*auSinrLevel 与auRsrpLevel取小为天线等级*/
    if (auSinrLevel < auRsrpLevel)
    {
        *pucRssilv = auSinrLevel;
    }
    else
    {
        *pucRssilv = auRsrpLevel;
    }
    return;
}


VOS_UINT32 atSetCindCnfSameProc(VOS_VOID *pMsgBlock)
{
    VOS_UINT16 usLength = 0;
    VOS_UINT32 ulResult = 0;
    VOS_UINT8  uRsrp = 0;
    VOS_UINT8  uSinr = AT_HCSQ_LTE_SINR_LEVEL;
    L4A_CIND_STRU stCindInfo = {{0,0,0},0,0,0,0,0,0,0,0};
    AT_CMD_ANTENNA_LEVEL_ENUM_UINT8  uAntLevel = AT_CMD_ANTENNA_LEVEL_0;
    if(NULL == pMsgBlock)
    {
        HAL_SDMLOG("pMsgBlock is NULL!\n");
        return AT_ERROR;
    }
    /*调用已有接口函数填充CindInfo结构体*/
    ulResult = at_RssiCindProc(pMsgBlock,&stCindInfo);
    if(ERR_MSP_SUCCESS == ulResult)
    {
        /*对rsrp进行转换*/
        if(AT_HCSQ_LTE_UNKNOWN <= stCindInfo.sRsrp)
        {
            uRsrp = AT_HCSQ_LTE_INVALID;
        }
        else if (AT_HCSQ_LTE_RSRP_VALUE_MAX <= stCindInfo.sRsrp)
        {
            uRsrp = AT_HCSQ_LTE_RSRP_LEVEL_MAX;
        }
        else if (AT_HCSQ_LTE_RSRP_VALUE_MIN > stCindInfo.sRsrp)
        {
            uRsrp = AT_HCSQ_LTE_RSRP_LEVEL_MIN;
        }
        else
        {
            uRsrp = (VOS_UINT8)((stCindInfo.sRsrp - AT_HCSQ_LTE_RSRP_VALUE_MIN) + 1);
        }
        /*对sinr进行转换*/
        if(AT_HCSQ_LTE_UNKNOWN <= stCindInfo.sSinr)
        {
            uSinr = AT_HCSQ_LTE_INVALID;
        }
        else if (AT_HCSQ_LTE_SINR_VALUE_MAX <= stCindInfo.sSinr)
        {
            uSinr = AT_HCSQ_LTE_SINR_LEVEL_MAX;
        }
        else if (AT_HCSQ_LTE_SINR_VALUE_MIN > stCindInfo.sSinr)
        {
            uSinr = AT_HCSQ_LTE_SINR_LEVEL_MIN;
        }
        else
        {
            /* sinr的步长是0.2，这里要转换为整数，所以*5 */
            uSinr = (VOS_UINT8)(((stCindInfo.sSinr * 5) - (AT_HCSQ_LTE_SINR_VALUE_MIN * 5)) + 1);
        }
        MN_PH_GetL4aAntLevel(uRsrp, uSinr, &uAntLevel);
        stCindInfo.sAntLevel = uAntLevel;
        if (TAF_SDC_REPORT_SRVSTA_NORMAL_SERVICE == stCindInfo.ucSrvStatus)
        {
            stCindInfo.ucSrvStatus = TAF_CIND_SRVSTA_NORMAL_SERVICE;
        }
        else
        {
            stCindInfo.ucSrvStatus = TAF_CIND_SRVSTA_NO_SERVICE;
            stCindInfo.sAntLevel = AT_CMD_ANTENNA_LEVEL_0;
        }
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucLAtSndCodeAddr,
                                           (VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
                                           "%s%d,%d,%d,%d,%d,%d,%d,%d",
                                           "+CIND: ",
                                           CIND_LTE_RESERVED,
                                           stCindInfo.sAntLevel,
                                           stCindInfo.ucSrvStatus,
                                           CIND_LTE_RESERVED,
                                           CIND_LTE_RESERVED,
                                           CIND_LTE_RESERVED,
                                           CIND_LTE_RESERVED,
                                           CIND_LTE_RESERVED);
    }
    else
    {
        HAL_SDMLOG(" atSetCindCnfSameProc ulResult = %d,\n",(int)ulResult);
    }
    CmdErrProc((VOS_UINT8)(stCindInfo.stCtrl.ulClientId), stCindInfo.ulErrorCode, usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;
}
/******************************************************************************
函数名称: atSetAnlevelCnfantProc
功能描述: 
返回值 
参数说明: pMsgBlock，MSP MSG消息结构
返 回 值: AT_FW_CLIENT_STATUS_READY
调用要求: 
调用举例: 
******************************************************************************/
VOS_UINT32 atSetAnlevelCnfantProc(VOS_VOID *pMsgBlock)
{   
    VOS_UINT16 usLength = 0;
    VOS_UINT32 ulResult = 0;
    VOS_UINT8 uRsrp = 0;
    VOS_UINT8 uSinr = AT_HCSQ_LTE_SINR_LEVEL;
    L4A_ANT_STRU stAntInfo = {{0,0,0},0,0,0,0,0,0,0,0};
    AT_CMD_ANTENNA_LEVEL_ENUM_UINT8  uAntLevel = AT_CMD_ANTENNA_LEVEL_0;
    if(NULL == pMsgBlock)
    {
        HAL_SDMLOG("pMsgBlock is NULL!\n");
        return AT_ERROR;
    }
    /*调用已有接口函数填充AntInfo结构体*/   
    ulResult = at_RssiAntProc(pMsgBlock,&stAntInfo);
    if(ERR_MSP_SUCCESS == ulResult)
    {
        /*对rsrp进行转换*/
        if(AT_HCSQ_LTE_UNKNOWN <= stAntInfo.sRsrp)
        {
            uRsrp = AT_HCSQ_LTE_INVALID;
        }
        else if (AT_HCSQ_LTE_RSRP_VALUE_MAX <= stAntInfo.sRsrp)
        {
            uRsrp = AT_HCSQ_LTE_RSRP_LEVEL_MAX;
        }
        else if (AT_HCSQ_LTE_RSRP_VALUE_MIN > stAntInfo.sRsrp)
        {
            uRsrp = AT_HCSQ_LTE_RSRP_LEVEL_MIN;
        }
        else
        {
            uRsrp = (VOS_UINT8)((stAntInfo.sRsrp - AT_HCSQ_LTE_RSRP_VALUE_MIN) + 1);
        }
        /*对sinr进行转换*/
        if(AT_HCSQ_LTE_UNKNOWN <= stAntInfo.sSinr)
        {
            uSinr = AT_HCSQ_LTE_INVALID;
        }
        else if (AT_HCSQ_LTE_SINR_VALUE_MAX <= stAntInfo.sSinr)
        {
            uSinr = AT_HCSQ_LTE_SINR_LEVEL_MAX;
        }
        else if (AT_HCSQ_LTE_SINR_VALUE_MIN > stAntInfo.sSinr)
        {
            uSinr = AT_HCSQ_LTE_SINR_LEVEL_MIN;
        }
        else
        {
            /* sinr的步长是0.2，这里要转换为整数，所以*5 */
            uSinr = (VOS_UINT8)(((stAntInfo.sSinr * 5) - (AT_HCSQ_LTE_SINR_VALUE_MIN * 5)) + 1);
        }
        MN_PH_GetL4aAntLevel(uRsrp, uSinr, &uAntLevel);
        stAntInfo.sAntLevel = uAntLevel;
        if(TAF_SDC_REPORT_SRVSTA_NORMAL_SERVICE != stAntInfo.ucSrvStatus)
        {
            stAntInfo.sAntLevel = AT_CMD_ANTENNA_LEVEL_0;
        }
        if(AT_ANT_TEMPPRTEVENT_H == g_TempprtEvent_flag)/*高温无服务 ant_leve=99*/
        {
            stAntInfo.sAntLevel = AT_ANT_TEMPPRT_LEVEL;
        }
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucLAtSndCodeAddr,
                                           (VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
                                           "%s%d",
                                           "*ANT:",
                                           stAntInfo.sAntLevel);
    }
    else
    {
        HAL_SDMLOG(" atSetAnlevelCnfantProc ulResult = %d,\n",(int)ulResult);
    }
    CmdErrProc((VOS_UINT8)(stAntInfo.stCtrl.ulClientId), stAntInfo.ulErrorCode,usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;
}
#endif
/*******************************************************************************/
/* 函数名称: atHcsqIndProc */
/* 功能描述: 主动上报HCSQ命令 */
/* 参数说明: pMsgBlock，MSP MSG消息结构 */
/* 返 回 值: */
/* 调用要求: TODO: ... */
/* 调用举例: TODO: ... */
/*******************************************************************************/
VOS_UINT32 atHcsqIndProc(VOS_VOID *pMsgBlock)
{
    L4A_CSQ_INFO_IND_STRU * pHcsq = NULL;    
    VOS_UINT16 usLength = 0;    
    VOS_INT16 uRsrp = 0;    
    VOS_INT16 uRsrq = 0;   
    VOS_INT16 uRssi = 0;    
    VOS_INT32 uSinr = 0;
#if(FEATURE_ON == MBB_FEATURE_BOX_FTEN)
    AT_CMD_ANTENNA_LEVEL_ENUM_UINT8  uAntLevel = AT_CMD_ANTENNA_LEVEL_0;
#endif
    if (VOS_NULL == pMsgBlock)
    {
        return AT_ERROR;
    }
    pHcsq = (L4A_CSQ_INFO_IND_STRU*)pMsgBlock;
   
    if((AT_HCSQ_LTE_UNKNOWN == pHcsq->sRssi) || (AT_HCSQ_LTE_UNKNOWN == pHcsq->sRsrq))      
    {
    }   
    else      
    {           
        /*对rssi值进行转换*/           
        if (AT_HCSQ_LTE_UNKNOWN <= pHcsq->sRssi)
        {                
            uRssi = AT_HCSQ_LTE_INVALID;        
        }         
        else if (AT_HCSQ_LTE_RSSI_VALUE_MAX <= pHcsq->sRssi  )
 
        {      
            uRssi = AT_HCSQ_LTE_RSSI_LEVEL_MAX;    
        }       
        else if (AT_HCSQ_LTE_RSSI_VALUE_MIN > pHcsq->sRssi )      
        {       
            uRssi = AT_HCSQ_LTE_RSSI_LEVEL_MIN;      
        }   
        else
        {       
            uRssi = (VOS_UINT8)((pHcsq->sRssi - AT_HCSQ_LTE_RSSI_VALUE_MIN) + 1);     
        }
   
        /*对rsrp进行转换*/
     
        if (AT_HCSQ_LTE_UNKNOWN <= pHcsq->sRsrp)      
        {       
            uRsrp = AT_HCSQ_LTE_INVALID;    
        }
        else if (AT_HCSQ_LTE_RSRP_VALUE_MAX <= pHcsq->sRsrp)     
        {               
            uRsrp = AT_HCSQ_LTE_RSRP_LEVEL_MAX;     
        }   
        else if (AT_HCSQ_LTE_RSRP_VALUE_MIN > pHcsq->sRsrp )   
        {           
            uRsrp = AT_HCSQ_LTE_RSRP_LEVEL_MIN;        
        }   
        else           
        {          
            uRsrp = (VOS_UINT8)((pHcsq->sRsrp - AT_HCSQ_LTE_RSRP_VALUE_MIN) + 1);   
        }
       

        /*对rsrq进行转换*/
        if (AT_HCSQ_LTE_UNKNOWN <= pHcsq->sRsrq)   
        {    
            uRsrq = AT_HCSQ_LTE_INVALID;   
        }  
        else if (AT_HCSQ_LTE_RSRQ_VALUE_MAX <= pHcsq->sRsrq)     
        {       
            uRsrq = AT_HCSQ_LTE_RSRQ_LEVEL_MAX;
        }         
        /*rsrq的精度是0.5，这里要转换成整数，所以乘2*/
        else if (AT_HCSQ_LTE_RSRQ_VALUE_MIN > pHcsq->sRsrq * 2)       
        {     
            uRsrq = AT_HCSQ_LTE_RSRQ_LEVEL_MIN;    
        }       
        else     
        {
            /* rsrq的精度是0.5，这里要转换成整数，所以乘2 */
            uRsrq = (VOS_UINT8)(((pHcsq->sRsrq * 2) - AT_HCSQ_LTE_RSRQ_VALUE_MIN) + 1); 
        }
  
    
        /*对sinr进行转换*/   
        if (AT_HCSQ_LTE_UNKNOWN <= pHcsq->lSINR)      
        {         
            uSinr = AT_HCSQ_LTE_INVALID;     
        }       
        else if (AT_HCSQ_LTE_SINR_VALUE_MAX <= pHcsq->lSINR )     
        {       
            uSinr = AT_HCSQ_LTE_SINR_LEVEL_MAX;       
        }   
        else if (AT_HCSQ_LTE_SINR_VALUE_MIN > pHcsq->lSINR )        
        {    
            uSinr = AT_HCSQ_LTE_SINR_LEVEL_MIN;  
        }       
        else          
        {        
            /* sinr的步长是0.2，这里要转换为整数，所以*5 */
            uSinr = (VOS_UINT8)(((pHcsq->lSINR * 5) - (AT_HCSQ_LTE_SINR_VALUE_MIN * 5)) + 1);/*calu sinr*/   
        }
        
        /*TS 36304 For an E-UTRAN cell, the measured RSRP value shall be greater than or equal to -110 dBm*/
#if (FEATURE_ON == MBB_MLOG)
        g_stSignalInfo.sRsrpValue = pHcsq->sRsrp;
        g_stSignalInfo.sRsrqValue = pHcsq->sRsrq;
        g_stSignalInfo.lSINRValue = pHcsq->lSINR;
        if(AT_HIGH_QULITY_RSRP_MIN > pHcsq->sRsrp)
        {
            mlog_print("at",mlog_lv_error,"rsrp: %d, rsrq: %d sinr: %d.\n", pHcsq->sRsrp,  pHcsq->sRsrq, pHcsq->lSINR);
        }
#endif
#if(FEATURE_ON == MBB_FEATURE_BOX_FTEN)
        if(((AT_CMER_RPT_MODE_SWITCH_ON == g_ucCmerpara.mode)
            && (AT_CMER_RPT_SWITCH_ON == g_ucCmerpara.ind))
            && (AT_CIND_RPT_SWITCH_ON == g_ucCindpara.signal))
        {
            MN_PH_GetL4aAntLevel (uRsrp, uSinr,&uAntLevel);
            uAntLevel = AT_GetSmoothLTEAntennaLevel( uAntLevel);
            if(g_oldLteAntLevel != uAntLevel)
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,
                        (VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
                        "%s%s%d,%d%s",
                        gaucAtCrLf,
                        gastAtStringTab[AT_STRING_CIEV].pucText,
                        CIEV_SIGNAL_FLAG,
                        uAntLevel,
                        gaucAtCrLf);
                g_oldLteAntLevel = uAntLevel;
                At_SendResultData(AT_BROADCAST_CLIENT_INDEX_MODEM_0,pgucLAtSndCodeAddr,usLength);
                usLength = 0;
            }
        }
#endif
      usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,        
                        (VOS_CHAR *)pgucLAtSndCodeAddr + usLength,  
                        "%s%s%s%d,%d,%d,%d%s",    
                        gaucAtCrLf,                  
                        "^HCSQ:",                
                        "\"LTE\",",           
                        uRssi,                   
                        uRsrp,             
                        uSinr,              
                        uRsrq,
                        gaucAtCrLf);

    }        
    At_SendResultData(AT_BROADCAST_CLIENT_INDEX_MODEM_0,pgucLAtSndCodeAddr,usLength);   
    return AT_FW_CLIENT_STATUS_READY;
}




VOS_UINT32 atLtersrpIndProc(VOS_VOID *pMsgBlock)
{
    VOS_UINT16 usLength = 0;    
    VOS_UINT32 ulResult = 0;    
    VOS_INT16 sRsrp = 0;    
    VOS_INT16 sRsrq = 0;    
    AT_ANLEVEL_INFO_CNF_STRU stLteRsrpInfo = {0};
 
    if (VOS_NULL == pMsgBlock)
    {
        return AT_ERROR;
    }

    /*调用已有接口函数填充LteRsrpInfo结构体*/
    
    ulResult = at_CsqInfoProc(pMsgBlock,&stLteRsrpInfo);
    if(ERR_MSP_SUCCESS == ulResult)    
    {
        
        sRsrp = stLteRsrpInfo.sRsrp;
        sRsrq = stLteRsrpInfo.sRsrq;        
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,                                                                    
                            (VOS_CHAR *)pgucLAtSndCodeAddr + usLength,"%s%s%d,%d%s",                                                                    
                            gaucAtCrLf, "^LTERSRP:",                                                                     
                            sRsrp,                                                                    
                            sRsrq,gaucAtCrLf);   
    }    
    else    
    {        
        HAL_SDMLOG("atLtersrpIndProc ulResult = %d,\n", (int)ulResult);   
    }
    
    At_SendResultData(AT_BROADCAST_CLIENT_INDEX_MODEM_0,pgucLAtSndCodeAddr,usLength);    
    return AT_FW_CLIENT_STATUS_READY;
}

VOS_UINT32 atLwclashCnfProcMbb(VOS_VOID *pMsgBlock)
{
    VOS_UINT8  ucIndexId = 0;
    VOS_UINT8  ucSumOfBandwidth = 0;
    VOS_UINT8  ucSumOfBand = 0;
    VOS_UINT16 usNdl = 0;
    VOS_UINT16 usNul = 0;
    VOS_UINT16 usUlBandwidth = 0;
    VOS_UINT16 usDlBandwidth = 0;
    VOS_UINT32 ulResult = AT_FAILURE;
    L4A_READ_LWCLASH_CNF_STRU *pstLwclash = NULL;
    LTE_BANDINFO_STRU stCurBandinfo = {0};
    VOS_UINT16 usLength = 0;

    if (VOS_NULL == pMsgBlock)
    {
        return AT_ERROR;
    }   
    pstLwclash = (L4A_READ_LWCLASH_CNF_STRU *)pMsgBlock;
    ulResult = At_ClientIdToUserId(pstLwclash->usClientId, &ucIndexId);
    
    if(AT_FAILURE == ulResult)
    {
        return AT_ERROR;
    }
    
    /*获取带宽数组的长度*/
    ucSumOfBandwidth = sizeof(gusbandwidth) / sizeof(gusbandwidth[0]);   
    if((pstLwclash->stLwclashInfo.usUlBandwidth >= ucSumOfBandwidth) || (pstLwclash->stLwclashInfo.usDlBandwidth >= ucSumOfBandwidth))
    {
        MBB_AT_TL_COMM_DEBUG_STR("the receive bandwith beyond length of array");    
        return AT_ERROR;
    }
    
    /*band合法性判断*/
    ucSumOfBand = sizeof(gstAtBandinfo) / sizeof(gstAtBandinfo[0]);/*44*/
    if(pstLwclash->stLwclashInfo.usBand >= ucSumOfBand)
    {
        MBB_AT_TL_COMM_DEBUG_STR("the received band beyond length of ucSumOfBand");
        return AT_ERROR;
    }
      
    if(VOS_OK == VOS_StrCmp("^HFREQINFO", (VOS_CHAR *)g_stParseContext[ucIndexId].pstCmdElement->pszCmdName))
    {
        stCurBandinfo = gstAtBandinfo[pstLwclash->stLwclashInfo.usBand];
        /*下行频点Ndl = (Fdl - Fdl_low) + Noff_dl*/
        usNdl = (pstLwclash->stLwclashInfo.usDlFreq - stCurBandinfo.ulDLfreqlow) + stCurBandinfo.ulNoffdl;
        /*上行频点Nul = (Ful - Ful_low) + Noff_ul*/
        usNul = (pstLwclash->stLwclashInfo.usUlFreq - stCurBandinfo.ulULfreqlow) + stCurBandinfo.ulNofful;
        usUlBandwidth = gusbandwidth[pstLwclash->stLwclashInfo.usUlBandwidth];/*使用lwclash返回的枚举值作为索引获取带宽*/
        usDlBandwidth = gusbandwidth[pstLwclash->stLwclashInfo.usDlBandwidth];/*使用lwclash返回的枚举值作为索引获取带宽*/
        if ((0 == pstLwclash->stLwclashInfo.usBand)
           || (0 == pstLwclash->stLwclashInfo.usDlFreq)
           || (0 == pstLwclash->stLwclashInfo.usUlFreq))
        {
            usLength = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN, (VOS_CHAR *)pgucLAtSndCodeAddr, (VOS_CHAR *)pgucLAtSndCodeAddr,
                                           "^HFREQINFO:%d,%d,%d,%d,%d,%d,%d,%d,%d",
                                           g_AtHFreqinforeport, 
                                           AT_HFREQINFO_RAT_TYPE_LTE,
                                           g_LastLteInfo.usBand,
                                           g_LastLteInfo.usNdl,
                                           g_LastLteInfo.usDlFreq, 
                                           g_LastLteInfo.usDlBandwidth,
                                           g_LastLteInfo.usNul,
                                           g_LastLteInfo.usUlFreq,
                                           g_LastLteInfo.usUlBandwidth);
        }
        else
        {
            usLength = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN, (VOS_CHAR *)pgucLAtSndCodeAddr, (VOS_CHAR *)pgucLAtSndCodeAddr,
                                           "^HFREQINFO:%d,%d,%d,%d,%d,%d,%d,%d,%d",
                                           g_AtHFreqinforeport, 
                                           AT_HFREQINFO_RAT_TYPE_LTE,
                                           pstLwclash->stLwclashInfo.usBand,
                                           usNdl,
                                           pstLwclash->stLwclashInfo.usDlFreq, 
                                           usDlBandwidth,
                                           usNul,
                                           pstLwclash->stLwclashInfo.usUlFreq,
                                           usUlBandwidth);

            /*本次值合法，保存本次上报值*/
            g_LastLteInfo.usBand = pstLwclash->stLwclashInfo.usBand;
            g_LastLteInfo.usNdl = usNdl;
            g_LastLteInfo.usDlFreq = pstLwclash->stLwclashInfo.usDlFreq;
            g_LastLteInfo.usDlBandwidth = usDlBandwidth;
            g_LastLteInfo.usNul = usNul;
            g_LastLteInfo.usUlFreq = pstLwclash->stLwclashInfo.usUlFreq;
            g_LastLteInfo.usUlBandwidth = usUlBandwidth;
        }
        CmdErrProc((VOS_UINT8)(pstLwclash->usClientId), pstLwclash->ulErrorCode, usLength, pgucLAtSndCodeAddr);
    }
    else
    {
        usLength = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                (VOS_CHAR *)pgucLAtSndCodeAddr,
                (VOS_CHAR *)pgucLAtSndCodeAddr,
                "^LWCLASH: %d,%d,%d,%d,%d,%d",
                pstLwclash->stLwclashInfo.enState,
                pstLwclash->stLwclashInfo.usUlFreq, pstLwclash->stLwclashInfo.usUlBandwidth,
                pstLwclash->stLwclashInfo.usDlFreq, pstLwclash->stLwclashInfo.usDlBandwidth,
                pstLwclash->stLwclashInfo.usBand);

        CmdErrProc((VOS_UINT8)(pstLwclash->usClientId), pstLwclash->ulErrorCode, usLength, pgucLAtSndCodeAddr);
    }
    return AT_FW_CLIENT_STATUS_READY;
}

#endif
#ifdef  __cplusplus
  #if  __cplusplus
  }
  #endif
#endif




