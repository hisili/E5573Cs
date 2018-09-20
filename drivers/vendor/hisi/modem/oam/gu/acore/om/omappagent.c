
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "OmApp.h"
#include "Omappagent.h"
#include "OmAppRl.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

/*lint -e767 修改人：甘兰47350；检视人：李霄46160；原因简述：LOG方案设计需要*/
#define    THIS_FILE_ID        PS_FILE_ID_ACPU_OMAGENT_C
/*lint +e767 修改人：甘兰47350；检视人：lixiao；*/

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
extern MsgBlock         *g_pstOmAcpuCnfMsg;
extern VOS_UINT32        g_ulOmAcpuCnfSem;
extern VOS_UINT32        g_ulOmAcpuSyncSem;

extern VOS_UINT32        OM_AcpuInit(VOS_VOID);

extern VOS_VOID          Om_HsicConnectProc(VOS_VOID);

extern VOS_UINT32        OM_AutoConfigProc(VOS_VOID);

/*****************************************************************************
 Prototype       : OM_CAgentMsgProc
 Description     : 处理来自CCPU AGENT的消息.
 Input           : pMsg -- The pointer of the msg.
 Output          : None
 Return Value    : VOS_VOID

 History         : ---
    Date         : 2011-07-01
    Author       : g47350
    Modification : Created function
 *****************************************************************************/
VOS_VOID OM_AcpuAgentMsgProc(MsgBlock* pMsg)
{
    VOS_UINT16                  usPrimId;
    VOS_UINT32                  ulRet = VOS_OK;
    OM_AUTOCONFIG_CNF_STRU     *pstSendCnf;

    /* 从消息前两个字节中取出原语ID */
    usPrimId = *(VOS_UINT16*)(pMsg->aucValue);

    OM_RecordInfoStart(VOS_EXC_DUMP_MEM_NUM_1, pMsg->ulSenderPid, ACPU_PID_OMAGENT, *((VOS_UINT32*)pMsg->aucValue));

    /* 判断是否为CCCPU AGENT发来的回复消息 */
    if (IS_CAGENT_CNF_MSG(usPrimId))
    {
        /* 判断消息是否被释放 */
        if (VOS_NULL_PTR == g_pstOmAcpuCnfMsg)
        {
            /* 标记该消息不用释放 */
            VOS_ReserveMsg(ACPU_PID_OMAGENT, pMsg);

            g_pstOmAcpuCnfMsg = pMsg;

            /* 释放信号量，使得调用API任务继续运行 */
            VOS_SmV(g_ulOmAcpuCnfSem);
        }

        /* CCPU 已经OK,可以进行SD卡配置信息 */
        if(OM_AUTOCONFIG_CNF == usPrimId)
        {
            OM_AutoConfigProc();
        }

        OM_RecordInfoEnd(VOS_EXC_DUMP_MEM_NUM_1);

        return;
    }

    /* 否则为CCPU发来不需要回复的请求消息 */
    if (OM_OLED_CLEAR_REQ == usPrimId)
    {
        OM_RecordInfoEnd(VOS_EXC_DUMP_MEM_NUM_1);

        return;
    }

    if (OM_OLED_DISPLAY_REQ == usPrimId)
    {
        OM_RecordInfoEnd(VOS_EXC_DUMP_MEM_NUM_1);

        return;
    }

    /* Modified by h59254 for AP-Modem Personalisation Project, 2012/04/12, begin */
    if (OM_HSIC_CONNECT_REQ == usPrimId)
    {
        Om_HsicConnectProc();

        OM_RecordInfoEnd(VOS_EXC_DUMP_MEM_NUM_1);

        return;
    }
    /* Modified by h59254 for AP-Modem Personalisation Project, 2012/04/12, end */

    if(OM_RECORD_DBU_INFO_REQ == usPrimId)
    {
        pstSendCnf = (OM_AUTOCONFIG_CNF_STRU *)pMsg;

        OM_AcpuLogShowToFile(VOS_FALSE);

        /* 将写文件的结果发送给PC侧 */
        OM_AcpuSendResult(pstSendCnf->aucData[0], ulRet, OM_APP_WRITE_NV_LOG_FILE_CNF);
    }

    if(OM_SET_FTM_MODE_REQ == usPrimId)
    {
        g_ulAcpuFTMFlag = VOS_TRUE;
    }

    OM_RecordInfoEnd(VOS_EXC_DUMP_MEM_NUM_1);

    return;
}


unsigned int MNTN_ErrorLog(char * cFileName, unsigned int ulFileId, unsigned int ulLine,
                unsigned int ulErrNo, void *pRecord, unsigned int ulLen)
{
    return VOS_OK;
}


VOS_UINT32 OM_Acpu_WriteLogFile(VOS_CHAR * cFileName, void *pRecord, VOS_UINT32 ulLen)
{
    OM_WRITELOG_REQ_STRU    *pstLogReq;

    /* 参数检测 */
    if ((VOS_NULL_PTR == cFileName) || (VOS_NULL_PTR == pRecord))
    {
        return OM_ACPU_PARA_ERR;
    }

    pstLogReq = (OM_WRITELOG_REQ_STRU*)VOS_AllocMsg(ACPU_PID_OMAGENT,
                                            (sizeof(OM_WRITELOG_REQ_STRU)-VOS_MSG_HEAD_LENGTH)+ulLen);

    /* 分配消息失败 */
    if (VOS_NULL_PTR == pstLogReq)
    {
        return OM_ACPU_ALLOC_FAIL;
    }

    pstLogReq->ulReceiverPid = CCPU_PID_OMAGENT;
    pstLogReq->usPrimId      = OM_WRITE_LOG_REQ;
    pstLogReq->ulLen         = ulLen;

    /* 为了确保aucFileName最后字节为'\0',拷贝长度需要加1 */
    VOS_MemCpy(pstLogReq->aucFileName, cFileName, VOS_StrLen(cFileName)+1);
    VOS_MemCpy(pstLogReq->aucData, pRecord, ulLen);

    /* 将请求消息发送给CCPU */
    if (VOS_OK != VOS_SendMsg(ACPU_PID_OMAGENT, pstLogReq))
    {
        return OM_ACPU_SEND_FAIL;
    }

    return VOS_OK;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif


