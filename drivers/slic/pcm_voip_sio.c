

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include <linux/kernel.h>
#include <asm/io.h>
#include <linux/delay.h>
#include "DrvInterface.h"
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

#include "pcm_voip_sio.h"
#include "ucom_comm.h"
#include "bsp_memmap.h"

#ifndef VOS_PRODUCT_E5172
#define VOS_PRODUCT_E5172
#endif

#ifndef VOS_TEST_NB
#define VOS_TEST_NB
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif



static struct platform_device SIO_device = {
    .name = "SIO",
    .id = -1,
};


BSP_U32 g_ulSioBaseAddr_A =0;
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
DRV_SIO_CTRL_STRU   g_stDrvSioCtrl;    /* SIO模块控制全局变量 */

/*****************************************************************************
  3 函数实现
*****************************************************************************/
irqreturn_t DRV_SIO_Isr_A(BSP_S32 irq, BSP_VOID *dev_id)
{
    BSP_U32                          uwIntState;
    DRV_SIO_INT_ISR_STRU               *pstIntIsr = DRV_SIO_GetIntIsrPtr();

    /*读取SIO中断状态*/
    BSP_REG_READ(DRV_SIO_INTR_STATUS,0,uwIntState);

    /*若中断状态为0，直接返回*/
    if (0 == uwIntState)
    {
        return 0;
    }

    /*清除查询到的中断*/
    BSP_REG_WRITE(DRV_SIO_INTR_CLR, 0x0,uwIntState);

    /*若SIO中断回调函数非空,则调用*/
    if (BSP_NULL != pstIntIsr->pfFunc)
    {
        pstIntIsr->pfFunc((DRV_SIO_INT_MASK_ENUM_UINT16)uwIntState, pstIntIsr->uwPara);
    }
	 return 1;
}

BSP_VOID DRV_SIO_Init_A(BSP_VOID)
{
    DRV_SIO_INT_ISR_STRU                *pstIntIsr = DRV_SIO_GetIntIsrPtr();
    DRV_SIO_STATUS_STRU                 *pstStatus = DRV_SIO_GetStatusPtr();
    BSP_S32 nRet = 0;
    /* 初始化SioIntIsr为全零 */
    memset(pstIntIsr,0,sizeof(DRV_SIO_INT_ISR_STRU));

    /* 初始化Sio状态控制为全0 */
    memset(pstStatus,0,sizeof(DRV_SIO_STATUS_STRU));
  
    g_ulSioBaseAddr_A = (unsigned long)ioremap(SIO_BASE_ADDR_PHY, SIO_SIZE);
 

    /* 打开SIO时钟 */
    //BSP_REG_WRITE(IO_ADDRESS(DRV_SC_BASE_ADDR), 0x000c, DRV_SIO_CLK_CTRL_WORD);

	 /*写DRV_SIO_CTRL_CLR 0xfff0复位SIO、禁止传输、禁止中断*/
    BSP_REG_WRITE(DRV_SIO_CTRL_CLR, 0x0,0xffff);

    /*写DRV_SIO_INTR_CLR 0x3f 清除所有SIO中断*/
    BSP_REG_WRITE(DRV_SIO_INTR_CLR,0x0, 0x3f);

    /*注册SIO ISR*/   
    nRet =request_irq(DRV_SIO_COMBINE_INT_NO,DRV_SIO_Isr_A,IRQF_SHARED,"SIO",&SIO_device);
   
    /*禁止sio rx中断*/
    disable_irq(DRV_SIO_RX_INT_NO);

    /*禁止sio tx中断*/
    disable_irq(DRV_SIO_TX_INT_NO);

    /*解复位SIO*/
    BSP_REG_WRITE(DRV_SIO_CTRL_SET,0x0, 0x8000);

    /* 关闭SIO时钟 */
    //BSP_REG_WRITE(IO_ADDRESS(DRV_SC_BASE_ADDR),0x0010, DRV_SIO_CLK_CTRL_WORD);

    /* 标记为关闭 */
    pstStatus->enMode = DRV_SIO_MODE_CLOSE;

}


BSP_VOID DRV_SIO_Open_A(
                DRV_SIO_INT_MASK_ENUM_UINT16    enIntMask,
                DRV_SIO_INT_FUNC                pfIntHandleFunc,
                BSP_U32                      uwPara)
{
	DRV_SIO_INT_ISR_STRU                *pstIntIsr = DRV_SIO_GetIntIsrPtr();
	DRV_SIO_STATUS_STRU                 *pstStatus = DRV_SIO_GetStatusPtr();

	
	if(DRV_SIO_MODE_OPEN == pstStatus->enMode)
	{
		return;
	}

	/* 打开SIO总线时钟 */
	//BSP_REG_WRITE(IO_ADDRESS(DRV_SC_BASE_ADDR), 0x000c,DRV_SIO_CLK_CTRL_WORD);

	/* 标记为打开 */
	pstStatus->enMode = DRV_SIO_MODE_OPEN;


	/* 保存SIO管脚控制字当前配置 */
	//BSP_REG_READ(IO_ADDRESS(DRV_SC_BASE_ADDR),0x0988, pstStatus->uwUartPinCtrlStatus);

	/* 设置SIO管脚控制字为复用LTE PCM管脚功能 */
	//BSP_REG_SETBITS(IO_ADDRESS(DRV_SC_BASE_ADDR),0x0988, UCOM_BIT12, 2, 0);
	//BSP_REG_SETBITS(IO_ADDRESS(DRV_SC_BASE_ADDR),0x0988, UCOM_BIT7, 2, 1);

	/* 打开SIO_1比特时钟接口 */
	//BSP_REG_SETBITS(IO_ADDRESS(DRV_SC_BASE_ADDR),0x0024,  UCOM_BIT13, 1, 1);

	
	/* 设置SIO master模式 */
	//BSP_REG_SETBITS(IO_ADDRESS(DRV_SC_BASE_ADDR),0x0030,  UCOM_BIT5, 1, 1);

	/* 设置SIO 时钟与PAD时钟同向 */
	//UCOM_RegBitWr(DRV_SIO_SIO_MODE, UCOM_BIT4, UCOM_BIT4, 0);

	/* 设置SIO master时时钟源频率为19.2M */
	//UCOM_RegBitWr(DRV_SIO_SIO_MODE, UCOM_BIT6, UCOM_BIT6, 0);

	/* 设置PCM接口管脚控制为master模式 */
	//BSP_REG_SETBITS(IO_ADDRESS(DRV_SC_BASE_ADDR),0x0940,  UCOM_BIT20, 1, 1);
    msleep(1);
	/*写DRV_SIO_CTRL_CLRs 0xfff0复位SIO、禁止传输、禁止中断*/
	BSP_REG_WRITE(DRV_SIO_CTRL_CLR, 0x0,0xffff);

	/*屏蔽所有SIO中断，对应比特为1则屏蔽中断，为0则打开中断*/
	BSP_REG_WRITE(DRV_SIO_INTR_MASK,  0x0,0xffffffff);

	/*清除所有SIO中断*/
	BSP_REG_WRITE(DRV_SIO_INTR_CLR,  0x0,0x3f);

	/*解复位、设置水线Tx-8(0.5)、Rx-8(0.5)、使能中断*/
	BSP_REG_WRITE(DRV_SIO_CTRL_SET, 0x0, 0x8088);

	/*配置为PCM模式*/
	//UCOM_RegWr(DRV_SIO_MODE, 0x1);

	/*配置符号扩展禁止*/
	BSP_REG_WRITE(DRV_SIO_SIGNED_EXT,  0x0,0);

	/*配置为PCM模式*/
	BSP_REG_WRITE(DRV_SIO_MODE,  0x0,0x1);

	/*配置为发送、接收16bit位宽*/
	BSP_REG_WRITE(DRV_SIO_DATA_WIDTH_SET, 0x0, 0x9);


	/*关闭双声道合并*/
	BSP_REG_WRITE(DRV_SIO_I2S_POS_MERGE_EN,  0x0,0x0);



	/*若中断回调函数非空，则打开中断屏蔽,保存回调函数信息*/
	if (BSP_NULL != pfIntHandleFunc)
	{
		/*对enIntMask取反，打开中断屏蔽*/
		BSP_REG_WRITE(DRV_SIO_INTR_MASK, 0x0, ~enIntMask);

		pstIntIsr->pfFunc = pfIntHandleFunc;
		pstIntIsr->uwPara = uwPara;
	}

	/*使能SIO接收、发送*/
	BSP_REG_WRITE(DRV_SIO_CTRL_SET, 0x0,0x3000);

    msleep(1);

}

BSP_VOID DRV_SIO_Close_A(BSP_VOID)
{
    DRV_SIO_INT_ISR_STRU                *pstIntIsr = DRV_SIO_GetIntIsrPtr();
    DRV_SIO_STATUS_STRU                 *pstStatus = DRV_SIO_GetStatusPtr();

    if(DRV_SIO_MODE_CLOSE == pstStatus->enMode)
    {
        return;
    }

    /*复位SIO、禁止传输、禁止中断*/
    BSP_REG_WRITE(DRV_SIO_CTRL_CLR, 0x0,0xffff);

    /*屏蔽所有SIO中断*/
    BSP_REG_WRITE(DRV_SIO_INTR_MASK, 0x0, 0xffffffff);

    /*清除SIO中断*/
    BSP_REG_WRITE(DRV_SIO_INTR_CLR,  0x0,0x3f);

    /*解复位SIO*/
    BSP_REG_WRITE(DRV_SIO_CTRL_SET, 0x0, 0x8000);

    /*清除中断回调函数相关信息*/
    memset(pstIntIsr, 0, sizeof(DRV_SIO_INT_ISR_STRU));

    /* 关闭SIO_1比特时钟接口 */
    BSP_REG_SETBITS(IO_ADDRESS(DRV_SC_BASE_ADDR),0x0028,  UCOM_BIT13, 1, 1);

    /* 恢复SIO管脚控制字为原来值 */
   BSP_REG_SETBITS(IO_ADDRESS(DRV_SC_BASE_ADDR),
                  0x0988, 
                  UCOM_BIT12,
                  2,
                  (pstStatus->uwUartPinCtrlStatus>>UCOM_BIT12)&0x3);
    BSP_REG_SETBITS(IO_ADDRESS(DRV_SC_BASE_ADDR),
	           0x0988, 
                  UCOM_BIT7,
                  2,
                  (pstStatus->uwUartPinCtrlStatus>>UCOM_BIT7)&0x3);



    /* 关闭SIO时钟 */
    BSP_REG_WRITE(IO_ADDRESS(DRV_SC_BASE_ADDR),0x0010, DRV_SIO_CLK_CTRL_WORD);

    /* 标记为关闭 */
    pstStatus->enMode = DRV_SIO_MODE_CLOSE;

}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

