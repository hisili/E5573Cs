#include <linux/kernel.h>
#include <linux/string.h>
#include <asm/io.h>
#include "DrvInterface.h"
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/earlysuspend.h>
#include <linux/miscdevice.h>
#include <linux/io.h>
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

#include "pcm_voip_dma.h"
#include "ucom_comm.h"
#include "bsp_memmap.h"



#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


static struct platform_device DMA_device = {
    .name = "DMA",
    .id = -1,
};
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/* DMA通道中断处理函数指针数组, 由DRV_DMA_RegisterIsr赋值 */
DRV_DMA_CXISR_STRU g_astDrvDmaCxIntIsr[DRV_DMA_MAX_CHANNEL_NUM];
BSP_U32 g_uldmaBaseAddr = 0;
/*****************************************************************************
  3 函数实现
*****************************************************************************/


irqreturn_t DRV_DMA_Isr_A(BSP_S32 irq, BSP_VOID *dev_id)
{
    BSP_U32                          i;
    BSP_U32                          uwErr1;
    BSP_U32                          uwErr2;
    BSP_U32                          uwErr3;
    BSP_U32                          uwTc1;
    BSP_U32                          uwTc2;
    BSP_U32                          uwIntMask;
    BSP_U32                          uwIntState;
    DRV_DMA_INT_TYPE_ENUM_UINT16        enIntType;
    DRV_DMA_CXISR_STRU                 *pstDmaCxIsr = DRV_DMA_GetCxIsrPtr();

	BSP_REG_READ(DRV_DMA_INT_STAT_HIFI,0,uwIntState);

    /*若通道状态全0表示全通道无中断或中断已处理,直接退出*/
    if (0 == uwIntState)
    {
        return 0;
    }

    /*读取各中断寄存器,查询各中断状态,包括Tc1/Tc2/Err1/Err2/Err3*/
    BSP_REG_READ(DRV_DMA_INT_ERR1_HIFI,0,uwErr1);
    BSP_REG_READ(DRV_DMA_INT_ERR2_HIFI,0,uwErr2);
    BSP_REG_READ(DRV_DMA_INT_ERR3_HIFI,0,uwErr3);
    BSP_REG_READ(DRV_DMA_INT_TC1_HIFI,0,uwTc1);
    BSP_REG_READ(DRV_DMA_INT_TC2_HIFI,0,uwTc2);
    /*写uwIntState依次到各寄存器清除中断,清除本次查询到的通道中断*/
    BSP_REG_WRITE(DRV_DMA_INT_TC1_RAW,0, uwIntState);
    BSP_REG_WRITE(DRV_DMA_INT_TC2_RAW, 0,uwIntState);
    BSP_REG_WRITE(DRV_DMA_INT_ERR1_RAW, 0,uwIntState);
    BSP_REG_WRITE(DRV_DMA_INT_ERR2_RAW, 0,uwIntState);
    BSP_REG_WRITE(DRV_DMA_INT_ERR3_RAW,0, uwIntState);

    /*遍历调用各通道注册的回调处理函数*/
    for ( i = 0; i < DRV_DMA_MAX_CHANNEL_NUM; i++)
    {
        uwIntMask = 0x1L << i;

        /*若该通道有中断产生(对应比特为1)*/
        if (uwIntState & uwIntMask)
        {
            if (BSP_NULL != pstDmaCxIsr[i].pfFunc)
            {
                /*若ERR1中断状态对应比特为1*/
                if (uwErr1 & uwIntMask)
                {
                    enIntType = DRV_DMA_INT_TYPE_ERR1;
                }
                /*若ERR2中断状态对应比特为1*/
                else if (uwErr2 & uwIntMask)
                {
                    enIntType = DRV_DMA_INT_TYPE_ERR2;
                }
                /*若ERR3中断状态对应比特为1*/
                else if (uwErr3 & uwIntMask)
                {
                    enIntType = DRV_DMA_INT_TYPE_ERR3;
                }
                /*若TC1中断状态对应比特为1*/
                else if (uwTc1 & uwIntMask)
                {
                    enIntType = DRV_DMA_INT_TYPE_TC1;
                }
                /*若TC2中断状态对应比特为1*/
                else if (uwTc2 & uwIntMask)
                {
                    enIntType = DRV_DMA_INT_TYPE_TC2;
                }
                /*未知中断*/
                else
                {
                    enIntType = DRV_DMA_INT_TYPE_BUTT;
                }

                /*调用注册的中断处理函数*/
                pstDmaCxIsr[i].pfFunc(enIntType, pstDmaCxIsr[i].uwPara);
            }
        }
    }
	return 1;
}

BSP_VOID DRV_DMA_Init_A( BSP_VOID )
{
    int err = 0;
    DRV_DMA_CXISR_STRU                 *pstDmaCxIsr = DRV_DMA_GetCxIsrPtr();
    	
    /*初始化g_astDrvDmaCxIntIsr为全零*/
    memset(pstDmaCxIsr, 0, sizeof(DRV_DMA_CXISR_STRU)*DRV_DMA_MAX_CHANNEL_NUM);
    g_uldmaBaseAddr = (unsigned long)ioremap(DMA_BASE_ADDR_PHY, DMA_SIZE);
   
    /* V7R1 选择LTE SIO的DMA为EDMA */
    //BSP_REG_SETBITS(IO_ADDRESS(SC_DMA_SEL_CTRL_ADDR), 0x0, 2, 1, 1);
	

    /*依次写0到如下寄存器默认屏蔽HiFi DMA中断*/
    BSP_REG_WRITE(DRV_DMA_INT_ERR1_MASK_HIFI, 0, 0);
    BSP_REG_WRITE(DRV_DMA_INT_ERR2_MASK_HIFI, 0, 0);
    BSP_REG_WRITE(DRV_DMA_INT_ERR3_MASK_HIFI, 0, 0);
    BSP_REG_WRITE(DRV_DMA_INT_TC1_MASK_HIFI, 0, 0);
    BSP_REG_WRITE(DRV_DMA_INT_TC2_MASK_HIFI, 0, 0);

  
    err = request_irq(DRV_DMA_INT_NO_HIFI,DRV_DMA_Isr_A,IRQF_SHARED,"DMA",&DMA_device);
    if(0 != err)
    {
        printk("request irq for DMA error.\n");
    }
  
}

BSP_U32 DRV_DMA_StartWithCfg_A(
                BSP_U16              uhwChannelNo,
                DRV_DMA_CXCFG_STRU     *pstCfg,
                DRV_DMA_INT_FUNC        pfIntHandleFunc,
                BSP_U32              uwPara)
{
    BSP_U32                          uwChannelMask   = (0x1L << uhwChannelNo);
    DRV_DMA_CXISR_STRU                 *pstCxIsr        = DRV_DMA_GetCxIsrPtr();
  
    /* 检查参数是否非法 */
    if (uhwChannelNo >= DRV_DMA_MAX_CHANNEL_NUM)
    {
        return BSP_ERROR;
    }

    /*写0到DRV_DMA_CX_CONFIG(uhwChannelNo) bit0禁止通道*/
    BSP_REG_SETBITS(DRV_DMA_CX_CONFIG((BSP_U32)uhwChannelNo),
                   0x0,
                  UCOM_BIT0,
                  1,
                  0);

    /*写通道X当前一维传输剩余的Byte数,[15,0]*/
    BSP_REG_SETBITS(DRV_DMA_CX_CNT0((BSP_U32)uhwChannelNo),
                   0x0,
                  UCOM_BIT0,
                  16,
                  pstCfg->uhwACount);
    
    /*写通道X当前二维传输剩余的Array个数,[31,16]*/
    BSP_REG_SETBITS(DRV_DMA_CX_CNT0((BSP_U32)uhwChannelNo),
                   0x0,
                  UCOM_BIT16,
                  16,
                  pstCfg->uhwBCount);

    /*写通道X当前三维传输剩余的Frame数,[15,0]*/
    BSP_REG_SETBITS(DRV_DMA_CX_CNT1((BSP_U32)uhwChannelNo),
                   0x0,
                  UCOM_BIT0,
                  16,
                  pstCfg->uhwCCount);

    /*写通道X的二维源地址偏移量[31,16]及目的地址偏移量[15,0]*/
    BSP_REG_SETBITS(DRV_DMA_CX_BINDX((BSP_U32)uhwChannelNo),
                   0x0,
                  UCOM_BIT0,
                  16,
                  pstCfg->uhwDstBIndex);
    BSP_REG_SETBITS(DRV_DMA_CX_BINDX((BSP_U32)uhwChannelNo),
                   0x0,
                  UCOM_BIT16,
                  16,
                  pstCfg->uhwSrcBIndex);

    /*写通道X的三维源地址偏移量[31,16]及目的地址偏移量[15,0]*/
    BSP_REG_SETBITS(DRV_DMA_CX_CINDX((BSP_U32)uhwChannelNo),
                   0x0,
                  UCOM_BIT0,
                  16,
                  pstCfg->uhwDstCIndex);
    BSP_REG_SETBITS(DRV_DMA_CX_CINDX((BSP_U32)uhwChannelNo),
                   0x0,
                  UCOM_BIT16,
                  16,
                  pstCfg->uhwSrcCIndex);

    /*写通道X的源地址[31,0]及目的地址[31,0]*/
    BSP_REG_WRITE(DRV_DMA_CX_DES_ADDR((BSP_U32)uhwChannelNo),0,pstCfg->uwDstAddr);
    BSP_REG_WRITE(DRV_DMA_CX_SRC_ADDR((BSP_U32)uhwChannelNo),0,pstCfg->uwSrcAddr);

    /*写通道X的链表地址配置*/
    BSP_REG_WRITE(DRV_DMA_CX_LLI((BSP_U32)uhwChannelNo), 0, pstCfg->uwLli);

    /*清除通道X的各种中断状态*/
    BSP_REG_WRITE(DRV_DMA_INT_TC1_RAW,  0,uwChannelMask);
    BSP_REG_WRITE(DRV_DMA_INT_TC2_RAW,  0,uwChannelMask);
    BSP_REG_WRITE(DRV_DMA_INT_ERR1_RAW,  0,uwChannelMask);
    BSP_REG_WRITE(DRV_DMA_INT_ERR2_RAW,  0,uwChannelMask);
    BSP_REG_WRITE(DRV_DMA_INT_ERR3_RAW, 0, uwChannelMask);

    /*若回调函数非空,则保存该值*/
    if (BSP_NULL!= pfIntHandleFunc)
    {
        pstCxIsr[uhwChannelNo].pfFunc  = pfIntHandleFunc;
        pstCxIsr[uhwChannelNo].uwPara  = uwPara;

        /*依次写(uhwChannelNo对应bit为1)到如下寄存器打开HiFi相应DMA通道中断屏蔽*/
        BSP_REG_SETBITS(DRV_DMA_INT_ERR1_MASK_HIFI, 0x0,uhwChannelNo, 1, 1);
        BSP_REG_SETBITS(DRV_DMA_INT_ERR2_MASK_HIFI, 0x0,uhwChannelNo, 1, 1);
        BSP_REG_SETBITS(DRV_DMA_INT_ERR3_MASK_HIFI, 0x0,uhwChannelNo, 1, 1);
        BSP_REG_SETBITS(DRV_DMA_INT_TC1_MASK_HIFI, 0x0,uhwChannelNo, 1, 1);
        BSP_REG_SETBITS(DRV_DMA_INT_TC2_MASK_HIFI, 0x0,uhwChannelNo, 1, 1);

    }

    /*写通道X的配置*/
    BSP_REG_WRITE(DRV_DMA_CX_CONFIG((BSP_U32)uhwChannelNo), 0,pstCfg->uwConfig);
    return BSP_OK;

}

BSP_VOID DRV_DMA_Stop_A(BSP_U16 uhwChannelNo)
{
    DRV_DMA_CXISR_STRU                 *pstCxIsr        = DRV_DMA_GetCxIsrPtr();

    /* 写0到DRV_DMA_CX_CONFIG(uhwChannelNo) bit0 停止对应DMA通道 */
    BSP_REG_SETBITS(DRV_DMA_CX_CONFIG((BSP_U32)uhwChannelNo),0x0,  UCOM_BIT0, 1, 0);

    /*依次写(uhwChannelNo对应bit为0)到如下寄存器屏蔽HiFi相应DMA通道中断*/
    BSP_REG_SETBITS(DRV_DMA_INT_ERR1_MASK_HIFI, 0x0, uhwChannelNo, 1, 0);
    BSP_REG_SETBITS(DRV_DMA_INT_ERR2_MASK_HIFI, 0x0, uhwChannelNo, 1, 0);
    BSP_REG_SETBITS(DRV_DMA_INT_ERR3_MASK_HIFI, 0x0, uhwChannelNo, 1, 0);
    BSP_REG_SETBITS(DRV_DMA_INT_TC1_MASK_HIFI, 0x0, uhwChannelNo, 1, 0);
    BSP_REG_SETBITS(DRV_DMA_INT_TC2_MASK_HIFI, 0x0, uhwChannelNo, 1, 0);

    /*清除全局变量中对应通道处理内容*/
    memset(&pstCxIsr[uhwChannelNo],0,sizeof(DRV_DMA_CXISR_STRU));
}



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

