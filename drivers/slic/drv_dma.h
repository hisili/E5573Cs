#ifndef __DRV_DMA_H__
#define __DRV_DMA_H__

#include <linux/kernel.h>
#include "DrvInterface.h"


#ifdef __cplusplus
extern "C" {
#endif


#define DMA_LLI_LINK(ulAddr)    (((ulAddr)&0xffffffe0UL)|(0x2UL))

#define DMA_MEM_SIO_CFG         (0x83311057)
#define DMA_SIO_MEM_CFG         (0x43311047)

#define SC_DMA_SEL_CTRL_ADDR    (0x900004E0U)
#define DMA_BASE_ADDR_PHY       (0x90024000U)
#define DMA_SIZE                (0x1000)

#define DMA_MAX_CHANNEL_NUM         (16)

#define DMA_CPU_NO_HIFI             (2)

#define DMA_BASE_ADDR               (g_ulEdmaBaseAddr)
#define DMA_CX_LLI(j)               (DMA_BASE_ADDR+(0x0800+(0x40*j)))
#define DMA_CX_CNT1(j)              (DMA_BASE_ADDR+(0x080C+(0x40*j)))
#define DMA_CX_CNT0(j)              (DMA_BASE_ADDR+(0x0810+(0x40*j)))
#define DMA_CX_SRC_ADDR(j)          (DMA_BASE_ADDR+(0x0814+(0x40*j)))
#define DMA_CX_DES_ADDR(j)          (DMA_BASE_ADDR+(0x0818+(0x40*j)))
#define DMA_CX_CONFIG(j)            (DMA_BASE_ADDR+(0x081C+(0x40*j)))

#define DMA_INT_TC1_MASK(i)         (DMA_BASE_ADDR+(0x0018+(0x40*i)))
#define DMA_INT_TC2_MASK(i)         (DMA_BASE_ADDR+(0x001c+(0x40*i)))
#define DMA_INT_ERR1_MASK(i)        (DMA_BASE_ADDR+(0x0020+(0x40*i)))
#define DMA_INT_ERR2_MASK(i)        (DMA_BASE_ADDR+(0x0024+(0x40*i)))
#define DMA_INT_ERR3_MASK(i)        (DMA_BASE_ADDR+(0x0028+(0x40*i)))
#define DMA_INT_TC1_RAW             (DMA_BASE_ADDR+(0x0600))
#define DMA_INT_TC2_RAW             (DMA_BASE_ADDR+(0x0608))
#define DMA_INT_ERR1_RAW            (DMA_BASE_ADDR+(0x0610))
#define DMA_INT_ERR2_RAW            (DMA_BASE_ADDR+(0x0618))
#define DMA_INT_ERR3_RAW            (DMA_BASE_ADDR+(0x0620))
#define DMA_INT_TC1_MASK_HIFI       (DMA_INT_TC1_MASK(DMA_CPU_NO_HIFI))
#define DMA_INT_TC2_MASK_HIFI       (DMA_INT_TC2_MASK(DMA_CPU_NO_HIFI))
#define DMA_INT_ERR1_MASK_HIFI      (DMA_INT_ERR1_MASK(DMA_CPU_NO_HIFI))
#define DMA_INT_ERR2_MASK_HIFI      (DMA_INT_ERR2_MASK(DMA_CPU_NO_HIFI))
#define DMA_INT_ERR3_MASK_HIFI      (DMA_INT_ERR3_MASK(DMA_CPU_NO_HIFI))

#define DMA_GetCxIsrPtr()           (&g_astDmaCxIntIsr[0])

enum DMA_INT_TYPE_ENUM
{
    DMA_INT_TYPE_TC1 = 0,
    DMA_INT_TYPE_TC2,
    DMA_INT_TYPE_ERR1,
    DMA_INT_TYPE_ERR2,
    DMA_INT_TYPE_ERR3,
    DMA_INT_TYPE_BUTT
};

typedef BSP_U16 DMA_INT_TYPE_ENUM_UINT16;

typedef struct
{
    BSP_U32 ulLli;
    BSP_U16 usDstBIndex;
    BSP_U16 usSrcBIndex;
    BSP_U16 usDstCIndex;
    BSP_U16 usSrcCIndex;
    BSP_U16 usCCount;
    BSP_U16 usRsv;
    BSP_U16 usACount;
    BSP_U16 usBCount;
    BSP_U32 ulSrcAddr;
    BSP_U32 ulDstAddr;
    BSP_U32 ulCongfig;
}DMA_CXCFG_STRU;

typedef BSP_VOID (*DMA_INT_FUNC)(
    DMA_INT_TYPE_ENUM_UINT16 enIntType,
    BSP_U32 ulPara);

typedef struct
{
    DMA_INT_FUNC pfFunc;
    BSP_U32 ulPara;
}DMA_CXISR_STRU;


//func declare
BSP_VOID DRV_DMA_Init(BSP_VOID);
BSP_VOID DRV_DMA_STOP(BSP_U16 usChNo);
BSP_U32 DMA_StartWithCfg(
    BSP_U16 usChannelNo,
    DMA_CXCFG_STRU *pstCfg,
    DMA_INT_FUNC pfIntHandleFunc,
    BSP_U32 ulPara);
BSP_VOID DMA_Isr(BSP_VOID);


#ifdef __cplusplus
}
#endif


#endif /*end of drv_dma.h*/
