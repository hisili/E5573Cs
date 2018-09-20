
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "AdsTimerMgmt.h"
#include "AdsCtx.h"
#include "AdsDownLink.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
/*lint -e767*/
#define    THIS_FILE_ID                 PS_FILE_ID_ADS_TIMERMGMT_C
/*lint +e767*/

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

ADS_TIMER_PRECISION_STRU                g_astAdsTmrPrecisionTbl[] =
{
    { TI_ADS_UL_SEND,                   VOS_TIMER_PRECISION_5  },
    { TI_ADS_DSFLOW_STATS_0,            VOS_TIMER_NO_PRECISION },
    { TI_ADS_DSFLOW_STATS_1,            VOS_TIMER_NO_PRECISION },
    { TI_ADS_DL_ADQ_EMPTY,              VOS_TIMER_PRECISION_0  },
    { TI_ADS_DL_PROTECT,                VOS_TIMER_PRECISION_5  },
    { TI_ADS_RPT_STATS_INFO,            VOS_TIMER_NO_PRECISION }
};


/*****************************************************************************
  3 函数实现
*****************************************************************************/


VOS_VOID  ADS_MNTN_TraceTimerOperation(
    VOS_UINT32                          ulPid,
    ADS_TIMER_ID_ENUM_UINT32            enTimerId,
    VOS_UINT32                          ulTimerLen,
    ADS_TIMER_OPERATION_TYPE_ENUM_UINT8 enTimerAction
)
{
    ADS_TIMER_INFO_STRU                 stMsg;

    PS_MEM_SET(&stMsg, 0x00, sizeof(ADS_TIMER_INFO_STRU));

    stMsg.ulSenderCpuId   = VOS_LOCAL_CPUID;
    stMsg.ulReceiverCpuId = VOS_LOCAL_CPUID;
    stMsg.ulSenderPid     = ulPid;
    stMsg.ulReceiverPid   = VOS_PID_TIMER;
    stMsg.ulLength        = sizeof(ADS_TIMER_INFO_STRU) - VOS_MSG_HEAD_LENGTH;
    stMsg.enTimerId       = enTimerId;
    stMsg.ulTimerLen      = ulTimerLen;
    stMsg.enTimerAction   = enTimerAction;

    OM_AcpuTraceMsgHook(&stMsg);

    return;
}


VOS_TIMER_PRECISION_ENUM_UINT32 ADS_GetTimerPrecision(ADS_TIMER_ID_ENUM_UINT32 enTimerId)
{
    VOS_UINT32                          i;

    for (i = 0; i < ADS_ARRAY_SIZE(g_astAdsTmrPrecisionTbl); i++)
    {
        if (enTimerId == g_astAdsTmrPrecisionTbl[i].enTimerId)
        {
            return g_astAdsTmrPrecisionTbl[i].enPrecision;
        }
    }

    return VOS_TIMER_PRECISION_5;
}

/* Added by l60609 for DSDA Phase II, 2012-12-20, Begin */
#if (FEATURE_OFF == FEATURE_SKB_EXP)

VOS_VOID ADS_DL_StartAdqEmptyTimer(VOS_VOID)
{
    ADS_TIMER_CTX_STRU                 *pstTiCtx;
    VOS_TIMER_PRECISION_ENUM_UINT32     enTmrPrecision;
    VOS_UINT32                          ulRslt;
    VOS_UINT32                          i;

    pstTiCtx = ADS_GetTiCtx();

    /* 如果该定时器已经启动则直接返回 */
    for (i = 0; i < ADS_MAX_TIMER_NUM; i++)
    {
        if ( (ADS_TIMER_STATUS_RUNNING == pstTiCtx[i].enTimerStatus)
          && (TI_ADS_DL_ADQ_EMPTY == pstTiCtx[i].enTimerId) )
        {
            ADS_NORMAL_LOG(ACPU_PID_ADS_DL, "ADS_DL_StartAdqEmptyTimer:timer is already start!");
            return;
        }
    }

    /* 找到一个空闲位置 */
    for (i = 0; i < ADS_MAX_TIMER_NUM; i++)
    {
        if (ADS_TIMER_STATUS_STOP == pstTiCtx[i].enTimerStatus )
        {
            break;
        }
    }

    /* 未找到空闲位置，无法保存该定时器信息，直接返回 */
    if (i >= ADS_MAX_TIMER_NUM)
    {
        ADS_ERROR_LOG1(ACPU_PID_ADS_DL, "ADS_DL_StartAdqEmptyTimer:i is", i);
        return;
    }

    enTmrPrecision = ADS_GetTimerPrecision(TI_ADS_DL_ADQ_EMPTY);

    ulRslt = VOS_StartCallBackRelTimer(&(pstTiCtx[i].hTimer),
                                       ACPU_PID_ADS_DL,
                                       TI_ADS_DL_ADQ_EMPTY_LEN,
                                       TI_ADS_DL_ADQ_EMPTY,
                                       0,
                                       VOS_RELTIMER_NOLOOP,
                                       ADS_DL_RcvTiAdqEmptyExpired,
                                       enTmrPrecision);

    if (VOS_OK != ulRslt)
    {
        ADS_ERROR_LOG(ACPU_PID_ADS_DL, "ADS_DL_StartAdqEmptyTimer:timer is fail start!");
        return;
    }

    pstTiCtx[i].enTimerId     = TI_ADS_DL_ADQ_EMPTY;
    pstTiCtx[i].enTimerStatus = ADS_TIMER_STATUS_RUNNING;

    return;
}
#endif
/* Added by l60609 for DSDA Phase II, 2012-12-20, End */


VOS_VOID ADS_DL_StartProtectTimer(VOS_VOID)
{
    ADS_TIMER_CTX_STRU                 *pstTiCtx;
    VOS_TIMER_PRECISION_ENUM_UINT32     enTmrPrecision;
    VOS_UINT32                          ulRslt;
    VOS_UINT32                          i;

    pstTiCtx = ADS_GetTiCtx();

    /* 如果该定时器已经启动则直接返回 */
    for (i = 0; i < ADS_MAX_TIMER_NUM; i++)
    {
        if ( (ADS_TIMER_STATUS_RUNNING == pstTiCtx[i].enTimerStatus)
          && (TI_ADS_DL_PROTECT == pstTiCtx[i].enTimerId) )
        {
            ADS_NORMAL_LOG(ACPU_PID_ADS_DL, "ADS_DL_StartProtectTimer:timer is already start!");
            return;
        }
    }

    /* 找到一个空闲位置 */
    for (i = 0; i < ADS_MAX_TIMER_NUM; i++)
    {
        if (ADS_TIMER_STATUS_STOP == pstTiCtx[i].enTimerStatus )
        {
            break;
        }
    }

    /* 未找到空闲位置，无法保存该定时器信息，直接返回 */
    if (i >= ADS_MAX_TIMER_NUM)
    {
        ADS_ERROR_LOG1(ACPU_PID_ADS_DL, "ADS_DL_StartProtectTimer:i is", i);
        return;
    }

    enTmrPrecision = ADS_GetTimerPrecision(TI_ADS_DL_PROTECT);

    ulRslt = VOS_StartCallBackRelTimer(&(pstTiCtx[i].hTimer),
                                       ACPU_PID_ADS_DL,
                                       TI_ADS_DL_PROTECT_LEN,
                                       TI_ADS_DL_PROTECT,
                                       0,
                                       VOS_RELTIMER_NOLOOP,
                                       ADS_DL_RcvTiProtectExpired,
                                       enTmrPrecision);

    if (VOS_OK != ulRslt)
    {
        ADS_ERROR_LOG(ACPU_PID_ADS_DL, "ADS_DL_StartProtectTimer:timer is fail start!");
        return;
    }

    pstTiCtx[i].enTimerId     = TI_ADS_DL_PROTECT;
    pstTiCtx[i].enTimerStatus = ADS_TIMER_STATUS_RUNNING;

    return;
}


VOS_VOID  ADS_StartTimer(
    VOS_UINT32                          ulPid,
    ADS_TIMER_ID_ENUM_UINT32            enTimerId,
    VOS_UINT32                          ulLen
)
{
    VOS_UINT32                          i;
    VOS_UINT32                          ulRet;
    ADS_TIMER_CTX_STRU                 *pstTiCtx;
    VOS_TIMER_PRECISION_ENUM_UINT32     enTmrPrecision;

    /* 输入参数检查 */
    if (0 == ulLen)
    {
        ADS_ERROR_LOG1(ACPU_PID_ADS_UL, "ADS_StartTimer:ulLen is",ulLen);
        return;
    }

    if (ulLen >= VOS_TIMER_MAX_LENGTH)
    {
        ulLen = VOS_TIMER_MAX_LENGTH - 1;
    }

    pstTiCtx = ADS_GetTiCtx();

    /* 如果该定时器已经启动则直接返回 */
    for (i = 0; i < ADS_MAX_TIMER_NUM; i++)
    {
        if ((ADS_TIMER_STATUS_RUNNING == pstTiCtx[i].enTimerStatus)
         && (enTimerId               == pstTiCtx[i].enTimerId))
        {
            ADS_NORMAL_LOG1(ACPU_PID_ADS_UL, "ADS_StartTimer:timer %d is already start!", enTimerId);
            return;
        }
    }

    /* 找到一个空闲位置 */
    for (i = 0; i < ADS_MAX_TIMER_NUM; i++)
    {
        if (ADS_TIMER_STATUS_STOP == pstTiCtx[i].enTimerStatus )
        {
            break;
        }
    }

    /* 未找到空闲位置，无法保存该定时器信息，直接返回 */
    if (i >= ADS_MAX_TIMER_NUM)
    {
        ADS_ERROR_LOG1(ACPU_PID_ADS_UL, "ADS_StartTimer:i is",i);
        return;
    }

    enTmrPrecision = ADS_GetTimerPrecision(enTimerId);

    ulRet = VOS_StartRelTimer(&(pstTiCtx[i].hTimer),
                              ulPid,
                              ulLen,
                              enTimerId,
                              0,
                              VOS_RELTIMER_NOLOOP,
                              enTmrPrecision);

    if (VOS_OK != ulRet)
    {
        ADS_ERROR_LOG(ulPid, "ADS_StartTimer:timer is fail start!");
        return;
    }

    pstTiCtx[i].enTimerId     = enTimerId;
    pstTiCtx[i].enTimerStatus = ADS_TIMER_STATUS_RUNNING;

    /*勾包ADS_TIMER_INFO_STRU*/
    ADS_MNTN_TraceTimerOperation(ulPid, enTimerId, ulLen, ADS_TIMER_OPERATION_START);

    return;
}


VOS_VOID ADS_StopTimer(
    VOS_UINT32                          ulPid,
    ADS_TIMER_ID_ENUM_UINT32            enTimerId
)
{
    VOS_UINT32                          i;
    ADS_TIMER_CTX_STRU                 *pstTiCtx;

    pstTiCtx = ADS_GetTiCtx();

    /* 如果该定时器已经启动则直接返回 */
    for (i = 0; i < ADS_MAX_TIMER_NUM; i++)
    {
        if ((ADS_TIMER_STATUS_RUNNING == pstTiCtx[i].enTimerStatus)
         && (enTimerId               == pstTiCtx[i].enTimerId))
        {
            break;
        }
    }

    /* 未找到该定时器 */
    if ( i >= ADS_MAX_TIMER_NUM)
    {
        return;
    }

    /* 停止VOS定时器: 当定时器的指针已经为空的时候, 说明其已经停止或者超时 */
    if (VOS_NULL_PTR != pstTiCtx[i].hTimer)
    {
        VOS_StopRelTimer(&(pstTiCtx[i].hTimer));
    }

    pstTiCtx[i].hTimer        = VOS_NULL_PTR;
    pstTiCtx[i].enTimerId     = TI_ADS_TIMER_BUTT;
    pstTiCtx[i].enTimerStatus = ADS_TIMER_STATUS_STOP;

    /*勾包ADS_TIMER_INFO_STRU*/
    ADS_MNTN_TraceTimerOperation(ulPid, enTimerId, 0, ADS_TIMER_OPERATION_STOP);

    return;
}


ADS_TIMER_STATUS_ENUM_UINT8 ADS_GetTimerStatus(
    VOS_UINT32                          ulPid,
    ADS_TIMER_ID_ENUM_UINT32            enTimerId
)
{
    ADS_TIMER_CTX_STRU                 *pstTiCtx;
    ADS_TIMER_STATUS_ENUM_UINT8         enTimerStatus;
    VOS_UINT32                          i;

    pstTiCtx  = ADS_GetTiCtx();

    enTimerStatus = ADS_TIMER_STATUS_STOP;

    for (i = 0; i < ADS_MAX_TIMER_NUM; i++)
    {
        if (enTimerId == pstTiCtx[i].enTimerId)
        {
            enTimerStatus = pstTiCtx[i].enTimerStatus;
            break;
        }
    }

    return enTimerStatus;
}

#if ((FEATURE_ON == MBB_CTF_WAN_QUEUE) || (FEATURE_ON == MBB_FEATURE_FASTIP_WAN_QUEUE))
/* 工作队列参数 */
extern struct work_struct ads_ul_work;
extern struct workqueue_struct *lte_data_process;

/* 上行保护定时器句柄 */
HTIMER g_hAdsUlSndTmrHdr = VOS_NULL_PTR;


VOS_VOID ADS_UL_RcvTiSendExpired(
    VOS_UINT32                          ulParam,
    VOS_UINT32                          ulTimerName
)
{
    queue_work(lte_data_process, &ads_ul_work);
    return;
}


VOS_VOID ADS_UL_StartSendTmr(VOS_VOID)
{
    VOS_UINT32 ulRslt;

    /* 定时器已运行 */
    if (VOS_NULL_PTR != g_hAdsUlSndTmrHdr)
    {
        return;
    }

    /* 启动定时器 */
    ulRslt = VOS_StartCallBackRelTimer(&g_hAdsUlSndTmrHdr,
                                       ACPU_PID_ADS_UL,
                                       ADS_UL_GET_PROTECT_TIMER_LEN(),
                                       TI_ADS_UL_SEND,
                                       0,
                                       VOS_RELTIMER_NOLOOP,
                                       ADS_UL_RcvTiSendExpired,
                                       VOS_TIMER_PRECISION_0);

    if (VOS_OK != ulRslt)
    {
        ADS_ERROR_LOG(ACPU_PID_ADS_UL, "ADS_UL_StartSendTmr: Start timer failed!");
    }

    return;
}
#endif /*(FEATURE_ON == MBB_CTF_WAN_QUEUE) || (FEATURE_ON == MBB_FEATURE_FASTIP_WAN_QUEUE)*/


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
