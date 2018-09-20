#ifndef __MED_CTRL_PCM_H__
#define __MED_CTRL_PCM_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/

#include <linux/kernel.h>
#include "DrvInterface.h"
#include "pcm_voip_dma.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define MED_CTRL_PCM_FRAME_INSERT_THD   (5)                                     /* 插帧数阈值 */
#define MED_CTRL_PCM_FRAME_WIN_LEN      (1000)                                  /* 缓冲区数据统计窗长*/

#define MED_CTRL_PCM_SHIFT_RIGHT_16     (16)                                    /* 右移16位*/
#define MED_CTRL_PCM_AHB_ADDR_INC       (4)                                     /* AHB邮箱地址增量*/
#define MED_CTRL_PCM_TX_BUF_SIZE_INIT   (5)                                     /* 上行环形buf初始值*/
#define MED_CTRL_PCM_RX_BUF_SIZE_INIT   (3)                                     /* 上行环形buf初始值*/
#define MED_CTRL_PCM_SIGNAL_RAND_RANGE  (63)                                    /* 小信号随机数幅度 */


/*****************************************************************************
 宏    名  : MED_CTRL_PcmTransferRxDataInd
 功能描述  : 通知OM进行下行数据搬运
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无

 ****************************************************************************/
#define MED_CTRL_PcmTransferRxDataInd() \
{ \
    DRV_IPC_TrigInt(IPC_TARGET_CPU_APPARM, PC_VOICE_RX_DATA_ACPU_IPC_BIT); \
}

#define MED_CODED_FRAME_LENGTH                   ((BSP_U16)36)               /* 一帧编码后的语音数据的最大帧长，单位双字节，最大的为AMR_WB下72个字节*/
#define MED_CODED_FRAME_WITH_OBJ_LEN             ((BSP_U16)80)               /* 一帧编码后的语音数据的最大帧长，单位双字节，带标志*/

#define MED_CTRL_PcmGetMicInBufPtr()    (g_psMedCtrlPcmMicIn)
#define MED_CTRL_PcmGetSpkOutBufPtr()   (g_psMedCtrlPcmSpkOut)

/*****************************************************************************
  3 枚举定义
*****************************************************************************/

/* 3.1 乒乓缓存枚举 */
typedef enum
{
    MED_CTRL_PCM_BUFF_A = 0,
    MED_CTRL_PCM_BUFF_B,
    MED_CTRL_PCM_BUFF_BUTT
} MED_CTRL_PCM_BUFF_ENUM;

/* 3.2 搬运到上下行枚举 */
typedef enum
{
    MED_CTRL_PCM_PLAY_TX = 0,
    MED_CTRL_PCM_PLAY_RX,
    MED_CTRL_PCM_PLAY_TXRX,
    MED_CTRL_PCM_PLAY_BUTT
} MED_CTRL_PCM_PLAY_ENUM;

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


#define VOS_PCVOICE_FLG

#ifndef VOS_TEST_NB
#define VOS_TEST_NB
#endif

#define UCOM_ALIGN(bytes)               __attribute__((aligned(bytes)))

#define MED_CTRL_PCM_MAX_FRAME_LENGTH  160
#define MED_CTRL_PCM_FRAME_LENGTH          160
#define MED_CTRL_PCM_FRAME_LENGTH_DOUBLE 320

#define MED_CTRL_MC_DMAC_CHN_MIC        (9) /* 经分配，V3R2下语音上行SIO使用通道15 */
#define MED_CTRL_MC_DMAC_CHN_SPK        (8) /* 经分配，V3R2下语音下行SIO使用通道14 */

/* 7.1 PCM码流buffer结构体 */
typedef struct
{
    BSP_S16                           asMicInBuffA[MED_CTRL_PCM_MAX_FRAME_LENGTH];
    BSP_S16                           asMicInBuffB[MED_CTRL_PCM_MAX_FRAME_LENGTH];
    BSP_S16                           asSpkOutBuffA[MED_CTRL_PCM_MAX_FRAME_LENGTH];
    BSP_S16                           asSpkOutBuffB[MED_CTRL_PCM_MAX_FRAME_LENGTH];

} MED_CTRL_PCM_BUFFER_STRU;

/* ring buffer状态结构体*/
typedef struct
{
    BSP_U16                          uhwAdpBufferSize;                       /*自适应缓冲大小，单位：帧*/
    BSP_U16                          uhwCntDataSizeIsOne;                    /*缓冲区数据大小等于1的次数*/
    BSP_U16                          uhwCurrDataSize;                        /*当前缓冲区数据大小，单位：帧*/
    BSP_S16                           shwInsertFrameCnt;                      /*插帧数*/
    BSP_U16                          uhwFrameCnt;                            /*帧数计数器*/
    BSP_U16                          uhwReserved;
}MED_CTRL_PCM_RING_BUFFER_STATE_STRU;

typedef struct
{
    BSP_U16               usMicInSwEnable;
	BSP_U16               usSpkOutSwEnable;
	
}MED_CTRL_PCM_DMA_FLAG_STRU;

/* PC Voice对象结构体*/
typedef struct
{
    BSP_U32                          uwRingBuffBaseAddr;
    MED_CTRL_PCM_RING_BUFFER_STATE_STRU stTxRingBufferState;                    /*上行ring buffer状态结构体*/
    MED_CTRL_PCM_RING_BUFFER_STATE_STRU stRxRingBufferState;                    /*下行ring buffer状态结构体*/
}MED_CTRL_PCM_PC_VOICE_OBJ_STRU;
/*****************************************************************************
  8 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  9 全局变量声明
*****************************************************************************/

#define MED_SWITCH_OFF 0
#define MED_SWITCH_ON 1


/*****************************************************************************
  10 函数声明
*****************************************************************************/



extern BSP_VOID MED_CTRL_PcmInit(BSP_VOID);
extern BSP_VOID MED_CTRL_MicDmaIsr(
                       DRV_DMA_INT_TYPE_ENUM_UINT16 enIntType,
                       BSP_U32 uwPara);
extern BSP_U32 MED_CTRL_PcmMicInStartLoopDMA( BSP_U16 usChNum );
extern BSP_U32 MED_CTRL_PcmSpkOutStartLoopDMA( BSP_U16 usChNum );
extern BSP_VOID MED_CTRL_PcmSwitchMicBuff(BSP_VOID);
extern BSP_S16* MED_CTRL_PcmGetWritableSpkBuffPtr(BSP_VOID);
extern BSP_VOID MED_CTRL_PcmSwitchSpkBuff(BSP_VOID);
extern BSP_VOID MED_CTRL_PcmClrLastSpkBuff(BSP_VOID);
extern BSP_VOID MED_PCM_Ctrl_Init(BSP_VOID);
extern BSP_VOID MED_PCM_Ctrl_Start(BSP_VOID);
extern BSP_VOID MED_PCM_Ctrl_Stop(BSP_VOID);

extern BSP_VOID MED_CTRL_SpkDmaIsr(
                       DRV_DMA_INT_TYPE_ENUM_UINT16 enIntType,
                       BSP_U32 uwPara);


#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

#endif /* end of med_pcm.h */
