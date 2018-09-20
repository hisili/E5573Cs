#include <linux/dma-mapping.h>
//#include "edmacIP.h"
//#include "edmacDrv.h"
#include <linux/kernel.h>
#include "DrvInterface.h"
#include "drv_dma.h"
#include "drv_sio.h"
#include "med_ctrl_pcm.h"
#include "drv_edma.h"

#ifdef __cplusplus
extern "C" {
#endif


DMA_CXCFG_STRU * g_pastPcmInDmaPara[2];
DMA_CXCFG_STRU * g_pastPcmOutDmaPara[2];
BSP_U32 g_aulPcmInDmaParaPhy[2];
BSP_U32 g_aulPcmOutDmaParaPhy[2];

PCM_BUFFER_STRU *g_stPcmBuff;
BSP_U32 g_ulMicInBuffPhyA;
BSP_U32 g_ulMicInBuffPhyB;
BSP_U32 g_ulSpkOutBuffPhyA;
BSP_U32 g_ulSpkOutBuffPhyB;

BSP_VOID PcmInit(BSP_VOID)
{
    BSP_U32 i = 0;
    BSP_U32 ulInDmaCfgPhy = 0;
    BSP_U32 ulOutDmaCfgPhy = 0;
    DMA_CXCFG_STRU *pstInDmaCfg = BSP_NULL;
    DMA_CXCFG_STRU *pstOutDmaCfg = BSP_NULL;
    PCM_BUFFER_STRU *pstPcmBuff = BSP_NULL;
    BSP_U32 ulPcmBuffAddrPhy = 0;

    //debug
    printk("enter PcmInit\n");

    pstInDmaCfg = (DMA_CXCFG_STRU *)dma_alloc_coherent(BSP_NULL,
                                    (2 * sizeof(DMA_CXCFG_STRU)),
                                    &ulInDmaCfgPhy,
                                    GFP_DMA|__GFP_WAIT);

    g_pastPcmInDmaPara[0] = pstInDmaCfg;
    g_pastPcmInDmaPara[1] = pstInDmaCfg + 1;
    g_aulPcmInDmaParaPhy[0] = ulInDmaCfgPhy;
    g_aulPcmInDmaParaPhy[1] = ulInDmaCfgPhy + sizeof(DMA_CXCFG_STRU);

    //debug
    printk("1, g_pastPcmInDmaPara[0]=%p\n, g_pastPcmInDmaPara[1]=%p\n, g_aulPcmInDmaParaPhy[0]=0x%x\n, g_aulPcmInDmaParaPhy[1]=0x%x\n",
                    g_pastPcmInDmaPara[0],
                    g_pastPcmInDmaPara[1],
                    g_aulPcmInDmaParaPhy[0],
                    g_aulPcmInDmaParaPhy[1]);


    pstOutDmaCfg = (DMA_CXCFG_STRU *)dma_alloc_coherent(BSP_NULL,
                                    (2 * sizeof(DMA_CXCFG_STRU)),
                                    &ulOutDmaCfgPhy,
                                    GFP_DMA|__GFP_WAIT);

    g_pastPcmOutDmaPara[0] = pstOutDmaCfg;
    g_pastPcmOutDmaPara[1] = pstOutDmaCfg + 1;
    g_aulPcmOutDmaParaPhy[0] = ulOutDmaCfgPhy;
    g_aulPcmOutDmaParaPhy[1] = ulOutDmaCfgPhy + sizeof(DMA_CXCFG_STRU);

    //debug
    printk("2, g_pastPcmOutDmaPara[0]=%p\n, g_pastPcmOutDmaPara[1]=%p\n, g_aulPcmOutDmaParaPhy[0]=0x%x\n, g_aulPcmOutDmaParaPhy[1]=0x%x\n",
                    g_pastPcmOutDmaPara[0],
                    g_pastPcmOutDmaPara[1],
                    g_aulPcmOutDmaParaPhy[0],
                    g_aulPcmOutDmaParaPhy[1]);

    pstPcmBuff = (PCM_BUFFER_STRU *)dma_alloc_coherent(BSP_NULL,
                                    sizeof(PCM_BUFFER_STRU),
                                    &ulPcmBuffAddrPhy,
                                    GFP_DMA|__GFP_WAIT);

    g_stPcmBuff = pstPcmBuff;
    g_ulMicInBuffPhyA = ulPcmBuffAddrPhy;
    g_ulMicInBuffPhyB = g_ulMicInBuffPhyA + PCM_MAX_FRAME_LENGTH * sizeof(BSP_U16);
    g_ulSpkOutBuffPhyA = g_ulMicInBuffPhyB + PCM_MAX_FRAME_LENGTH * sizeof(BSP_U16);
    g_ulSpkOutBuffPhyB = g_ulSpkOutBuffPhyA + PCM_MAX_FRAME_LENGTH * sizeof(BSP_U16);

    for(i = 0; i < PCM_MAX_FRAME_LENGTH; i++)
    {
        (g_stPcmBuff->ausSpkOutBuffA)[i] = 0xA5A5;
    }

    for(i = 0; i < PCM_MAX_FRAME_LENGTH; i++)
    {
        (g_stPcmBuff->ausSpkOutBuffB)[i] = 0xA5A5;
    }

    //debug
    printk("3, g_ulMicInBuffPhyA=0x%x\n, g_ulMicInBuffPhyB=0x%x\n, g_ulSpkOutBuffPhyA=0x%x\n, g_ulSpkOutBuffPhyB=0x%x\n",
                    g_ulMicInBuffPhyA,
                    g_ulMicInBuffPhyB,
                    g_ulSpkOutBuffPhyA,
                    g_ulSpkOutBuffPhyB);


#if 0
    pInDmaList = malloc(32 + sizeof(DMA_CXCFG_STRU) * 2);
    memset(pInDmaList, 0, 32 + sizeof(DMA_CXCFG_STRU) * 2);

    for(i = 0; i < 32; i++)
    {
        if(0 == (((BSP_U32)pInDmaList) / 32))
        {
            g_pastPcmInDmaPara[0] = (DMA_CXCFG_STRU *)pInDmaList;
            g_pastPcmInDmaPara[1] = (DMA_CXCFG_STRU *)(pInDmaList + sizeof(DMA_CXCFG_STRU));

            g_pastPcmInDmaPara[0] = (DMA_CXCFG_STRU *)TTF_VIRT_TO_PHY((BSP_U32)g_pastPcmInDmaPara[0]);
            g_pastPcmInDmaPara[1] = (DMA_CXCFG_STRU *)TTF_VIRT_TO_PHY((BSP_U32)g_pastPcmInDmaPara[1]);

            printk("alloc in dma list succ, %p, %p\n", g_pastPcmInDmaPara[0], g_pastPcmInDmaPara[1]);
            break;
        }
    }

    if(32 == i)
    {
        printk("alloc in dma list fail\n");
        return;
    }

    pOutDmaList = malloc(32 + sizeof(DMA_CXCFG_STRU) * 2);
    memset(pOutDmaList, 0, 32 + sizeof(DMA_CXCFG_STRU) * 2);

    for(i = 0; i < 32; i++)
    {
        if(0 == (((BSP_U32)pOutDmaList) / 32))
        {
            g_pastPcmOutDmaPara[0] = (DMA_CXCFG_STRU *)pOutDmaList;
            g_pastPcmOutDmaPara[1] = (DMA_CXCFG_STRU *)(pOutDmaList + sizeof(DMA_CXCFG_STRU));

            g_pastPcmOutDmaPara[0] = (DMA_CXCFG_STRU *)TTF_VIRT_TO_PHY((BSP_U32)g_pastPcmOutDmaPara[0]);
            g_pastPcmOutDmaPara[1] = (DMA_CXCFG_STRU *)TTF_VIRT_TO_PHY((BSP_U32)g_pastPcmOutDmaPara[1]);

            printk("alloc out dma list succ, %p, %p\n", g_pastPcmOutDmaPara[0], g_pastPcmOutDmaPara[1]);
            break;
        }
    }

    if(32 == i)
    {
        printk("alloc out dma list fail\n");
        return;
    }
#endif

    /*打开SIO设备*/
    SIO_Open(0x14,
            BSP_NULL,
            0);

    //debug
    printk("out PcmInit\n");
}

BSP_U32 PcmMicInStartLoopDMA(BSP_U16 usChNo)
{
    BSP_U32 ulRet;
    BSP_U32 ulDstAddrA = 0;
    BSP_U32 ulDstAddrB = 0;
    DMA_CXCFG_STRU *pstLLiAddrA = BSP_NULL;
    DMA_CXCFG_STRU *pstLLiAddrB = BSP_NULL;

    //debug
    printk("enter PcmMicInStartLoopDMA\n");

    DRV_DMA_STOP(usChNo);

    //debug
    printk("1\n");


#if 0
    ulDstAddrA = TTF_VIRT_TO_PHY((BSP_U32)(&(g_stPcmBuff.ausMicInBuffA[0])));
    ulDstAddrB = TTF_VIRT_TO_PHY((BSP_U32)(&(g_stPcmBuff.ausMicInBuffB[0])));
#endif

    ulDstAddrA = g_ulMicInBuffPhyA;
    ulDstAddrB = g_ulMicInBuffPhyB;

    pstLLiAddrA = g_pastPcmInDmaPara[0];
    pstLLiAddrB = g_pastPcmInDmaPara[1];

    /*设置DMA配置参数，配置为链表连接，使用MIC通道LOOP配置，每个节点上报一个中断*/
    pstLLiAddrA->ulLli = DMA_LLI_LINK(g_aulPcmInDmaParaPhy[1]);
    pstLLiAddrA->usACount = CODEC_FRAME_LENGTH_NB * sizeof(BSP_U16);
    pstLLiAddrA->ulSrcAddr = SIO_PCM_RD;
    pstLLiAddrA->ulDstAddr = ulDstAddrA;
    pstLLiAddrA->ulCongfig = DMA_SIO_MEM_CFG;

    pstLLiAddrB->ulLli = DMA_LLI_LINK(g_aulPcmInDmaParaPhy[0]);
    pstLLiAddrB->usACount = CODEC_FRAME_LENGTH_NB * sizeof(BSP_U16);
    pstLLiAddrB->ulSrcAddr = SIO_PCM_RD;
    pstLLiAddrB->ulDstAddr = ulDstAddrB;
    pstLLiAddrB->ulCongfig = DMA_SIO_MEM_CFG;

    /*配置MIC播放DMA通道进行上行的数据搬运*/
    ulRet = DMA_StartWithCfg(usChNo,
                    pstLLiAddrA,
                    NULL,
                    0);
    return 0;
}

BSP_U32 PcmSpkOutStartLoopDMA(BSP_U16 usChNo)
{
    BSP_U32 ulRet;
    BSP_U32 ulSrcAddrA = 0;
    BSP_U32 ulSrcAddrB = 0;
    DMA_CXCFG_STRU *pPcmOutDmaListA = BSP_NULL;
    DMA_CXCFG_STRU *pPcmOutDmaListB = BSP_NULL;

    //debug
    printk("enter PcmSpkOutStartLoopDMA\n");

    DRV_DMA_STOP(usChNo);

    //debug
    printk("1\n");

#if 0
    ulSrcAddrA = TTF_VIRT_TO_PHY((BSP_U32)(&(g_stPcmBuff.ausSpkOutBuffA[0])));
    ulSrcAddrB = TTF_VIRT_TO_PHY((BSP_U32)(&(g_stPcmBuff.ausSpkOutBuffB[0])));
#endif

    ulSrcAddrA = g_ulSpkOutBuffPhyA;
    ulSrcAddrB = g_ulSpkOutBuffPhyB;

    pPcmOutDmaListA = g_pastPcmOutDmaPara[0];
    pPcmOutDmaListB = g_pastPcmOutDmaPara[1];

    /*设置DMA配置参数，配置为链表连接，使用SPK通道LOOP配置，每个节点上报一个中断*/
    pPcmOutDmaListA->ulLli = DMA_LLI_LINK(g_aulPcmOutDmaParaPhy[1]);
    pPcmOutDmaListA->usACount = CODEC_FRAME_LENGTH_NB * sizeof(BSP_U16);
    pPcmOutDmaListA->ulSrcAddr = ulSrcAddrA;
    pPcmOutDmaListA->ulDstAddr = SIO_PCM_XD;
    pPcmOutDmaListA->ulCongfig = DMA_MEM_SIO_CFG;

    pPcmOutDmaListB->ulLli = DMA_LLI_LINK(g_aulPcmOutDmaParaPhy[0]);
    pPcmOutDmaListB->usACount = CODEC_FRAME_LENGTH_NB * sizeof(BSP_U16);
    pPcmOutDmaListB->ulSrcAddr = ulSrcAddrB;
    pPcmOutDmaListB->ulDstAddr = SIO_PCM_XD;
    pPcmOutDmaListB->ulCongfig = DMA_MEM_SIO_CFG;

    /*配置SPK播放DMA通道，并进行播放到下行的数据搬运*/
    ulRet = DMA_StartWithCfg(usChNo,
                    pPcmOutDmaListA,
                    NULL,
                    0);

    return ulRet;
}


#ifdef __cplusplus
}
#endif

