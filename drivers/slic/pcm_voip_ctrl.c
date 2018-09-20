

/*****************************************************************************
   1 头文件包含
******************************************************************************/
#include <linux/dma-mapping.h>
//#include "edmacIP.h"
//#include "edmacDrv.h"
#include <linux/kernel.h>
#include "DrvInterface.h"
#include "pcm_voip_dma.h"
#include "pcm_voip_sio.h"
#include "pcm_voip_ctrl.h"
#include <linux/miscdevice.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/delay.h>

#include <linux/string.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/earlysuspend.h>
#include <linux/miscdevice.h>
#include <linux/clk.h>
#include <linux/jiffies.h>
#include <asm/irq.h>
#include <mach/irqs.h>
#include <linux/spi/spi.h>
#include <linux/platform_device.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include "drv_edma.h"


#ifdef  __cplusplus
#if  __cplusplus
extern "C"{
#endif
#endif



BSP_S16                              *g_psMedCtrlPcmMicIn;                    /*指向当前Mic输入的PCM缓存Buffer的指针*/
BSP_S16                              *g_psMedCtrlPcmSpkOut;                   /*指向当前输出到Speaker的PCM缓存Buffer的指针*/
BSP_S16                               g_shwMedCtrlPcmFrameLength;             /* PCM数据帧长度 */
/* DMA通道配置参数结构体全局变量 */
UCOM_ALIGN(32)
DRV_DMA_CXCFG_STRU * g_pPcmInDmaPara[2];
UCOM_ALIGN(32)
DRV_DMA_CXCFG_STRU * g_pPcmOutDmaPara[2];
BSP_U32 g_ulPcmInDmaParaPhy[2];
BSP_U32 g_ulPcmOutDmaParaPhy[2];

BSP_U32 g_ulMicIn_BuffPhyA;
BSP_U32 g_ulMicIn_BuffPhyB;
BSP_U32 g_ulSpkOut_BuffPhyA;
BSP_U32 g_ulSpkOut_BuffPhyB;


MED_CTRL_PCM_BUFFER_STRU             *g_stMedCtrlPcmBuff;

MED_CTRL_PCM_DMA_FLAG_STRU             g_DMA_Ctrl_Flag ;

extern int slic_pcm_spkout(char *dma_buf,int limit);
extern int slic_pcm_micin(char *dma_buf,int limit);

/*****************************************************************************
   3 外部函数声明
******************************************************************************/


/*****************************************************************************
   4 函数实现
******************************************************************************/

BSP_VOID MED_CTRL_PcmInit(BSP_VOID)
{
    BSP_U32 ulInDmaCfgPhy = 0;
    BSP_U32 ulOutDmaCfgPhy = 0;
    DRV_DMA_CXCFG_STRU *pstInDmaCfg = BSP_NULL;
    DRV_DMA_CXCFG_STRU *pstOutDmaCfg = BSP_NULL;
    MED_CTRL_PCM_BUFFER_STRU *pstPcmBuff = BSP_NULL;
    BSP_U32 ulPcmBuffAddrPhy = 0;
	
    //printk("enter MED_CTRL_PcmInit\n");

	pstInDmaCfg = (DRV_DMA_CXCFG_STRU *)dma_alloc_coherent(BSP_NULL,
                                    (2 * sizeof(DRV_DMA_CXCFG_STRU)),
                                    &ulInDmaCfgPhy,
                                    GFP_DMA|__GFP_WAIT);

    g_pPcmInDmaPara[0] = pstInDmaCfg;
    g_pPcmInDmaPara[1] = pstInDmaCfg + 1;
    g_ulPcmInDmaParaPhy[0] = ulInDmaCfgPhy;
    g_ulPcmInDmaParaPhy[1] = ulInDmaCfgPhy + sizeof(DRV_DMA_CXCFG_STRU);

    /*
    printk("1, g_pastPcmInDmaPara[0]=%p\n, g_pastPcmInDmaPara[1]=%p\n, g_aulPcmInDmaParaPhy[0]=0x%x\n, g_aulPcmInDmaParaPhy[1]=0x%x\n",
                    g_pPcmInDmaPara[0],
                    g_pPcmInDmaPara[1],
                    g_ulPcmInDmaParaPhy[0],
                    g_ulPcmInDmaParaPhy[1]);
                    */


    pstOutDmaCfg = (DRV_DMA_CXCFG_STRU *)dma_alloc_coherent(BSP_NULL,
                                    (2 * sizeof(DRV_DMA_CXCFG_STRU)),
                                    &ulOutDmaCfgPhy,
                                    GFP_DMA|__GFP_WAIT);

    g_pPcmOutDmaPara[0] = pstOutDmaCfg;
    g_pPcmOutDmaPara[1] = pstOutDmaCfg + 1;
    g_ulPcmOutDmaParaPhy[0] = ulOutDmaCfgPhy;
    g_ulPcmOutDmaParaPhy[1] = ulOutDmaCfgPhy + sizeof(DRV_DMA_CXCFG_STRU);

    /*
    printk("2, g_pastPcmOutDmaPara[0]=%p\n, g_pastPcmOutDmaPara[1]=%p\n, g_aulPcmOutDmaParaPhy[0]=0x%x\n, g_aulPcmOutDmaParaPhy[1]=0x%x\n",
                    g_pPcmOutDmaPara[0],
                    g_pPcmOutDmaPara[1],
                    g_ulPcmOutDmaParaPhy[0],
                    g_ulPcmOutDmaParaPhy[1]);
                    */

    pstPcmBuff = (MED_CTRL_PCM_BUFFER_STRU *)dma_alloc_coherent(BSP_NULL,
                                    sizeof(MED_CTRL_PCM_BUFFER_STRU),
                                    &ulPcmBuffAddrPhy,
                                    GFP_DMA|__GFP_WAIT);

    g_stMedCtrlPcmBuff = pstPcmBuff;
    g_ulMicIn_BuffPhyA = ulPcmBuffAddrPhy;
    g_ulMicIn_BuffPhyB = g_ulMicIn_BuffPhyA   + MED_CTRL_PCM_MAX_FRAME_LENGTH * sizeof(BSP_S16);
    g_ulSpkOut_BuffPhyA = g_ulMicIn_BuffPhyB  + MED_CTRL_PCM_MAX_FRAME_LENGTH * sizeof(BSP_S16);
    g_ulSpkOut_BuffPhyB = g_ulSpkOut_BuffPhyA + MED_CTRL_PCM_MAX_FRAME_LENGTH * sizeof(BSP_S16);

    /*
    printk("3, g_stMedCtrlPcmBuff=ox%p\n, g_ulMicIn_BuffPhyA=0x%p\n, g_ulMicIn_BuffPhyB=0x%p\n, g_ulSpkOut_BuffPhyA=0x%p\n,g_ulSpkOut_BuffPhyB=0x%p\n",
	                 g_stMedCtrlPcmBuff,
	                 g_ulMicIn_BuffPhyA,
                    g_ulMicIn_BuffPhyB,
                    g_ulSpkOut_BuffPhyA,
                    g_ulSpkOut_BuffPhyB);
                    */
    g_psMedCtrlPcmMicIn  = g_stMedCtrlPcmBuff->asMicInBuffA;
    g_psMedCtrlPcmSpkOut = g_stMedCtrlPcmBuff->asSpkOutBuffA;
	

   
     //printk("exit MED_CTRL_PcmInit\n ");

}

BSP_VOID MED_CTRL_PcmSwitchMicBuff(BSP_VOID)
{
    BSP_U32      ulOffset = 0;
    BSP_U32      ulDestAddr = 0;
	BSP_U32      ulDestAddrB = 0;   /* MIC采集的DMA通道的目的地址 */

   
    ulOffset = ((BSP_S32)MED_CTRL_PCM_FRAME_LENGTH * sizeof(BSP_S16)) - sizeof(BSP_U32);

    ulDestAddrB = (BSP_S32)g_ulMicIn_BuffPhyB;

    /*读取通道0目的地址寄存器*/
     BSP_REG_READ(DRV_DMA_CX_DES_ADDR(MED_CTRL_MC_DMAC_CHN_MIC),0,ulDestAddr);

	/*当目的地址已经为BufferB范围内(LOOP模式)或目的地址为BufferA的最后一个位宽(非LOOP模式)*/
    /*此逻辑依赖于BufferA与BufferB地址连续，且BufferA在前*/
    if ( (ulDestAddr >= (ulDestAddrB - sizeof(BSP_U32)))
        &&(ulDestAddr < (ulDestAddrB + ulOffset)) )
    {
        g_psMedCtrlPcmMicIn = g_stMedCtrlPcmBuff->asMicInBuffA;
    }
    else
    {
        g_psMedCtrlPcmMicIn = g_stMedCtrlPcmBuff->asMicInBuffB;
    }
}
BSP_S16* MED_CTRL_PcmGetWritableSpkBuffPtr(BSP_VOID)
{
    BSP_U32      ulOffset   = 0;
    BSP_U32      ulSrcAddr  = 0;
	BSP_U32      ulSrcAddrB = 0;   /* SPK播放的DMA通道的源地址 */

   
    ulOffset = ((BSP_S32)MED_CTRL_PCM_FRAME_LENGTH * sizeof(BSP_S16)) - sizeof(BSP_U32);


    ulSrcAddrB  = (BSP_S32)g_ulSpkOut_BuffPhyB;
  
    /*读取通道1源地址寄存器*/
    BSP_REG_READ(DRV_DMA_CX_SRC_ADDR(MED_CTRL_MC_DMAC_CHN_SPK),0,ulSrcAddr);

	/*当源地址已经为BufferB范围内(LOOP模式)或源地址为BufferA的最后一个位宽(非LOOP模式)*/
    /*此逻辑依赖于BufferA与BufferB地址连续，且BufferA在前*/
    if((ulSrcAddr >= (ulSrcAddrB - sizeof(BSP_U32)))
        &&(ulSrcAddr < (ulSrcAddrB + ulOffset)) )
    {
        return g_stMedCtrlPcmBuff->asSpkOutBuffA;
    }
    else
    {
        return g_stMedCtrlPcmBuff->asSpkOutBuffB;
    }
}
	
BSP_VOID MED_CTRL_PcmSwitchSpkBuff(BSP_VOID)
{   
   g_psMedCtrlPcmSpkOut = MED_CTRL_PcmGetWritableSpkBuffPtr();   

}

BSP_VOID MED_CTRL_PcmClrLastSpkBuff(BSP_VOID)
{

    BSP_U16 usBufLen = (BSP_U16)MED_CTRL_PCM_FRAME_LENGTH * sizeof(BSP_U16);

    memset(MED_CTRL_PcmGetWritableSpkBuffPtr(),
                0,
                usBufLen);
}


BSP_U32 MED_CTRL_PcmMicInStartLoopDMA(BSP_U16 usChNum)
{
    BSP_U32              uwRet= BSP_ERROR;
    BSP_U32              uwDestAddrA, uwDestAddrB;   /* MIC采集的DMA通道的目的地址 */
 
    DRV_DMA_CXCFG_STRU *pstLLiAddrA = BSP_NULL;
    DRV_DMA_CXCFG_STRU *pstLLiAddrB = BSP_NULL;
    if (usChNum >= DRV_DMA_MAX_CHANNEL_NUM)
    {
        
        return BSP_ERROR;
    }
    
    /* 禁止MIC采集的DMA */
    DRV_DMA_Stop_A(usChNum);
   
    /*切换buffer并更新DestAddr的值*/
    if ( g_psMedCtrlPcmMicIn == g_stMedCtrlPcmBuff->asMicInBuffA)
    {      
		uwDestAddrA = (BSP_U32)g_ulMicIn_BuffPhyB;
        uwDestAddrB = (BSP_U32)g_ulMicIn_BuffPhyA;			
    }
    else
    {
       
		 uwDestAddrA = (BSP_U32)g_ulMicIn_BuffPhyA;
         uwDestAddrB = (BSP_U32)g_ulMicIn_BuffPhyB;		 
    }
	
    pstLLiAddrA = (DRV_DMA_CXCFG_STRU *)g_pPcmInDmaPara[0];
    pstLLiAddrB = (DRV_DMA_CXCFG_STRU *)g_pPcmInDmaPara[1];
	
    memset(g_pPcmInDmaPara[0], 0, sizeof(DRV_DMA_CXCFG_STRU));
	memset(g_pPcmInDmaPara[1], 0, sizeof(DRV_DMA_CXCFG_STRU));
  
    /*设置DMA配置参数,配置为链表连接，使用MIC通道LOOP配置，每个节点上报一个中断*/
    pstLLiAddrA->uwLli            = DRV_DMA_LLI_LINK((BSP_U32)g_ulPcmInDmaParaPhy[1]);
    pstLLiAddrA->uhwACount        = (BSP_U16)MED_CTRL_PCM_MAX_FRAME_LENGTH;
    pstLLiAddrA->uwSrcAddr        = SIO_BASE_ADDR_PHY+0x58;//DRV_SIO_PCM_RD;
    pstLLiAddrA->uwDstAddr        = uwDestAddrA;
    pstLLiAddrA->uwConfig         = DRV_DMA_SIO_MEM_CFG;

    pstLLiAddrB->uwLli            = DRV_DMA_LLI_LINK((BSP_U32)g_ulPcmInDmaParaPhy[0]);
    pstLLiAddrB->uhwACount        = (BSP_U16)MED_CTRL_PCM_MAX_FRAME_LENGTH;
    pstLLiAddrB->uwSrcAddr        = SIO_BASE_ADDR_PHY+0x58;
    pstLLiAddrB->uwDstAddr        = uwDestAddrB;
    pstLLiAddrB->uwConfig         = DRV_DMA_SIO_MEM_CFG;

    /*配置MIC采集DMA通道进行数据搬运，并注册MIC处DMA中断处理钩子*/
    uwRet = DRV_DMA_StartWithCfg_A(usChNum,
                                 g_pPcmInDmaPara[0],
                                 MED_CTRL_MicDmaIsr,
                                 0);

    return uwRet;
}

BSP_U32 MED_CTRL_PcmSpkOutStartLoopDMA(BSP_U16 usChNum)
{
    BSP_U32              uwRet = BSP_ERROR;
    BSP_U32              uwSrcAddrA, uwSrcAddrB;   /* SPK播放的DMA通道的源地址 */
    DRV_DMA_CXCFG_STRU *pstLLiAddrA = BSP_NULL;
    DRV_DMA_CXCFG_STRU *pstLLiAddrB = BSP_NULL;
    /* 检查参数是否非法,通道号0-15 */
    if (usChNum >= DRV_DMA_MAX_CHANNEL_NUM)
    {
        
        return BSP_ERROR;
    }

    /* 禁止SPK播放的DMA */
    DRV_DMA_Stop_A(usChNum);
 

    /*切换buffer并更新DestAddr的值*/
    if (g_psMedCtrlPcmSpkOut == g_stMedCtrlPcmBuff->asSpkOutBuffA)
    {
        uwSrcAddrA = (BSP_U32)g_ulSpkOut_BuffPhyB;
        uwSrcAddrB = (BSP_U32)g_ulSpkOut_BuffPhyA;
    }
    else
    {
        uwSrcAddrA = (BSP_U32)g_ulSpkOut_BuffPhyA;
        uwSrcAddrB = (BSP_U32)g_ulSpkOut_BuffPhyB;
    }

	pstLLiAddrA = (DRV_DMA_CXCFG_STRU *)g_pPcmOutDmaPara[0];
    pstLLiAddrB = (DRV_DMA_CXCFG_STRU *)g_pPcmOutDmaPara[1];	

    memset(g_pPcmOutDmaPara[0], 0, sizeof(DRV_DMA_CXCFG_STRU));
	memset(g_pPcmOutDmaPara[1], 0, sizeof(DRV_DMA_CXCFG_STRU)); 
    /*设置DMA配置参数,配置为链表连接，使用SPK通道LOOP配置，每个节点上报一个中断*/
    pstLLiAddrA->uwLli     = DRV_DMA_LLI_LINK((BSP_U32)g_ulPcmOutDmaParaPhy[1]);
    pstLLiAddrA->uhwACount = (BSP_U16)MED_CTRL_PCM_MAX_FRAME_LENGTH;
    pstLLiAddrA->uwSrcAddr = uwSrcAddrA;
    pstLLiAddrA->uwDstAddr = SIO_BASE_ADDR_PHY+0x50;//DRV_SIO_PCM_XD;
    pstLLiAddrA->uwConfig  = DRV_DMA_MEM_SIO_CFG;

    pstLLiAddrB->uwLli     = DRV_DMA_LLI_LINK((BSP_U32)g_ulPcmOutDmaParaPhy[0]);
    pstLLiAddrB->uhwACount = (BSP_U16)MED_CTRL_PCM_MAX_FRAME_LENGTH;
    pstLLiAddrB->uwSrcAddr = uwSrcAddrB;
    pstLLiAddrB->uwDstAddr = SIO_BASE_ADDR_PHY+0x50;
    pstLLiAddrB->uwConfig  = DRV_DMA_MEM_SIO_CFG;

  
    /*配置SPEAKER播放DMA通道进行播放到下行的数据搬运, 并注册MIC/SPK处DMA中断处理钩子*/
    uwRet = DRV_DMA_StartWithCfg_A(usChNum,
                                 g_pPcmOutDmaPara[0],
                                 MED_CTRL_SpkDmaIsr,                               
                                 0);

    return uwRet;
}


BSP_VOID MED_CTRL_PcmChkSpkConflick(BSP_VOID)
{
        /* 若SpkOut处DMA中断时刻检测到还未接收到新的下行数据，则认为出现时序冲突 */
        if (MED_SWITCH_OFF == g_DMA_Ctrl_Flag.usSpkOutSwEnable)
        {
          // MED_CTRL_PcmSpkOutStartLoopDMA();
        }
        /* 若未出现冲突,则标志该帧下行数据已使用 */
        else
        {
            g_DMA_Ctrl_Flag.usSpkOutSwEnable = MED_SWITCH_OFF;
        }
   
}

BSP_VOID MED_CTRL_MicDmaIsr(
                DRV_DMA_INT_TYPE_ENUM_UINT16 enIntType,
                BSP_U32 uwPara)
{   
    /* 若中断类型为TC中断,为Mic处DMA正常中断处理流程 */
    if ((DRV_DMA_INT_TYPE_TC1 == enIntType)
        ||(DRV_DMA_INT_TYPE_TC2 == enIntType))
    {
       
           if (MED_SWITCH_ON == g_DMA_Ctrl_Flag.usMicInSwEnable)
           {
                /* 切换上行缓冲区 */
                MED_CTRL_PcmSwitchMicBuff();

				g_DMA_Ctrl_Flag.usMicInSwEnable = MED_SWITCH_OFF ;
               //通知上层取数据 调用接口拷贝上行数据                
                slic_pcm_micin((char *)g_psMedCtrlPcmMicIn,MED_CTRL_PCM_MAX_FRAME_LENGTH);
                memset(g_psMedCtrlPcmMicIn,0,MED_CTRL_PCM_MAX_FRAME_LENGTH*2);
                g_DMA_Ctrl_Flag.usMicInSwEnable = MED_SWITCH_ON ;
            }
		    else
		    {
		        printk("MicIn a frame data will be lost\n");
		    }
          
  
    }
    /* 若中断类型为ERROR中断,记录异常 */
    else
    {
        /*记录异常，出现DMA Error中断*/      
        printk("MED_CTRL_MicDmaIsr ,ISQ failed\n");
    }   
 
}

BSP_VOID MED_CTRL_SpkDmaIsr(
                DRV_DMA_INT_TYPE_ENUM_UINT16 enIntType,
                BSP_U32 uwPara)
{
     /* 若中断类型为TC中断,为SPK处DMA正常中断处理流程 */
    if ((DRV_DMA_INT_TYPE_TC1 == enIntType)
        ||(DRV_DMA_INT_TYPE_TC2 == enIntType))
    {
            if (MED_SWITCH_ON == g_DMA_Ctrl_Flag.usSpkOutSwEnable)
            {
                /* 切换下行缓冲区 */
                MED_CTRL_PcmSwitchSpkBuff();

				g_DMA_Ctrl_Flag.usSpkOutSwEnable = MED_SWITCH_OFF;
				slic_pcm_spkout((char *)g_psMedCtrlPcmSpkOut,MED_CTRL_PCM_MAX_FRAME_LENGTH);
				g_DMA_Ctrl_Flag.usSpkOutSwEnable = MED_SWITCH_ON;
            }
            else
            {
               printk("SpkOut can not get a frame data \n");
                /* 清空上一帧语音数据 */
               MED_CTRL_PcmClrLastSpkBuff();
				
            }

            /* 下行时序监控 */
           // MED_CTRL_PcmChkSpkConflick();
            
     
    }
    /* 若中断类型为ERROR中断,记录异常 */
    else
    {
        /*记录异常，出现DMA Error中断*/
		printk("MED_CTRL_SpkDmaIsr,ISQ failed\n");       
    }

}

BSP_VOID MED_PCM_Ctrl_Init(BSP_VOID)
{
     DRV_DMA_Init_A();
     #if 0
     DRV_SIO_Init_A();
     #endif
     MED_CTRL_PcmInit();
}
BSP_VOID MED_PCM_Ctrl_UnInit(BSP_VOID)
{
    	 
	dma_free_coherent(BSP_NULL,(2 * sizeof(DRV_DMA_CXCFG_STRU)),g_pPcmInDmaPara[0],g_ulPcmInDmaParaPhy[0]);
	dma_free_coherent(BSP_NULL,(2 * sizeof(DRV_DMA_CXCFG_STRU)),g_pPcmOutDmaPara[0],g_ulPcmOutDmaParaPhy[0]);
	dma_free_coherent(BSP_NULL,(2 * sizeof(DRV_DMA_CXCFG_STRU)),g_stMedCtrlPcmBuff,g_ulMicIn_BuffPhyA);
}

BSP_VOID MED_PCM_Ctrl_Start(BSP_VOID)
{
    /* 设置采样率模式和一帧PCM采样点数 */
       memset(&g_DMA_Ctrl_Flag,0,sizeof(MED_CTRL_PCM_DMA_FLAG_STRU));
      g_DMA_Ctrl_Flag.usMicInSwEnable =  MED_SWITCH_ON;
	  g_DMA_Ctrl_Flag.usSpkOutSwEnable = MED_SWITCH_ON;
      #if 0
        /* 调用DRV_SIO_Open接口，打开SIO设备 */
      DRV_SIO_Open_A((enMask | DRV_SIO_INT_MASK_TX_RIGHT_FIFO_UNDER),
                     NULL,
                     0);
      #endif
    MED_CTRL_PcmMicInStartLoopDMA(MED_CTRL_MC_DMAC_CHN_MIC); 
    msleep(2);
    MED_CTRL_PcmSpkOutStartLoopDMA(MED_CTRL_MC_DMAC_CHN_SPK);
}

BSP_VOID MED_PCM_Ctrl_Stop(BSP_VOID)
{
    DRV_DMA_Stop_A(MED_CTRL_MC_DMAC_CHN_MIC);
    DRV_DMA_Stop_A(MED_CTRL_MC_DMAC_CHN_SPK);
    #if 0 
    DRV_SIO_Close_A();
    #endif
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

