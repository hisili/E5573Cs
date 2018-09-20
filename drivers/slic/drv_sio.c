
#include <linux/kernel.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/module.h>

#include "DrvInterface.h"
#include "drv_sio.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef enum{
    SLIC_NONE_TONE = 0,                     /* none */
    SLIC_RING,                              /* Ring */
    SLIC_RING_FOR_MONITOR,                  /* Ring for houtai Monitor */
    SLIC_KEEP_RING,                         /* Keep Ring */
    SLIC_DIAL_TONE,                         /* Dial tone */
    SLIC_BUSY_TONE,                         /* Busy tone */
    SLIC_ALARM_TONE,                        /* Alarm tone */
    SLIC_TIP_TONE,                          /* Succeed/fail tone */
    SLIC_FAULT_TONE,                        /* Fault tone */
    SLIC_CALL_WAITING_TONE,                 /* call waiting tone */
    SLIC_RINGBACK_TONE,                     /* Ring Back tone */
    SLIC_CONFIRMATION_TONE,                 /* Confirmation tone */
    SLIC_SUCCEED_TONE,
    SLIC_FAIL_TONE
} slic_tone_enum_type;

extern void slic_sound_start (  slic_tone_enum_type tone );


SIO_CTRL_STRU g_stSioCtrl = {{0}, {0}};

//去掉语间测试时的杂音
//BSP_U32 g_ulPcmPara = 0xA5A5;
BSP_U32 g_ulPcmPara = 0x0;

extern void msleep(unsigned int msec);

BSP_VOID SIO_Init(BSP_VOID)
{
    iounmap((int *)SIO_BASE_ADDR_PHY);
    g_ulSioBaseAddr = (unsigned long)ioremap(SIO_BASE_ADDR_PHY, SIO_SIZE);

    printk("g_ulSioBaseAddr=0x%x\n", g_ulSioBaseAddr);

    /*写SIO_CTRL_CLR 0xffff复位SIO、禁止传输、禁止中断*/
    //复位SIO可能导致已工作的Slic出现问题，可能需要移到Slic驱动中
    BSP_REG_WRITE(SIO_CTRL_CLR, 0, 0xffff);

    /*写SIO_INTR_CLR 0x3f清除所有SIO中断*/
    BSP_REG_WRITE(SIO_INTR_CLR, 0, 0x3f);

    BSP_REG_WRITE(SIO_CTRL_SET, 0, 0x8000);
}

BSP_VOID SIO_Open(
    SIO_INT_MASK_ENUM_UINT16 enIntMask,
    SIO_INT_FUNC pfIntHandleFunc,
    BSP_U32 ulPara)
{
    /*写SIO_INTR_CLR 0xffff复位SIO、禁止传输、禁止中断*/
    BSP_REG_WRITE(SIO_CTRL_CLR, 0, 0xffff);

    /*屏蔽所有SIO中断*/
    BSP_REG_WRITE(SIO_INTR_MASK, 0, 0xffffffff);

    /*清除所有SIO中断*/
    BSP_REG_WRITE(SIO_INTR_CLR, 0, 0x3f);

    /*解复位、设置水线Tx-8(0.5)、RX-8(0.5)、使能中断*/
    BSP_REG_WRITE(SIO_CTRL_SET, 0, 0x8088);

    /*配置为PCM模式*/
    BSP_REG_WRITE(SIO_MODE, 0, 0x1);

    /*配置为发送、接收16bit位宽*/
    BSP_REG_WRITE(SIO_DATA_WIDTH_SET, 0, 0x9);

    /*配置符号扩展禁止*/
    BSP_REG_WRITE(SIO_SIGNED_EXT, 0, 0);

    /*若中断回调函数非空，则打开中断屏蔽，挂接中断回调函数*/
    if(1/*BSP_NULL != pfIntHandleFunc*/)
    {
        /*对enIntMask取反，打开中断屏蔽*/
        BSP_REG_WRITE(SIO_INTR_MASK, 0, ~enIntMask);
    }

    /*使能SIO接收、发送*/
    BSP_REG_WRITE(SIO_CTRL_SET, 0, 0x3000);
}

BSP_VOID SIO_Close(BSP_VOID)
{
    return;
}

BSP_VOID SIO_Isr(BSP_VOID)
{
    return;
}

BSP_U32 Debug_SIO_Read_RX(void)
{
    BSP_U32 ulRegVal = 0;

    return BSP_REG_READ(SIO_PCM_RD, 0, ulRegVal);
}

BSP_U32 Debug_SIO_Read_RX_Sta(void)
{
    BSP_U32 ulRegVal = 0;

    return BSP_REG_READ(SIO_CTRL_RX_STA, 0, ulRegVal);
}

BSP_U32 Debug_SIO_Read_TX_Sta(void)
{
    BSP_U32 ulRegVal = 0;

    return BSP_REG_READ(SIO_CTRL_TX_STA, 0, ulRegVal);
}


BSP_VOID Debug_SIO_Reset(void)
{
    BSP_REG_WRITE(SIO_CTRL_CLR, 0, 0x8000);
    msleep(10);
    BSP_REG_WRITE(SIO_CTRL_SET, 0, 0x8000);
    msleep(10);
}


BSP_VOID Debug_SIO_En_RX_TX(void)
{
    BSP_REG_WRITE(SIO_CTRL_SET, 0, 0x3000);
    BSP_REG_WRITE(SIO_CTRL_CLR, 0, 0x0C00);
}


BSP_VOID Debug_SIO_Dis_RX_TX(void)
{
    BSP_REG_WRITE(SIO_CTRL_CLR, 0, 0x3000);
    BSP_REG_WRITE(SIO_CTRL_SET, 0, 0x0C00);
}

BSP_VOID SetPcmPara(BSP_U32 ulPcmPara)
{
	g_ulPcmPara = ulPcmPara;
}

int Start_Equ_Slic_Test(void)
{
    BSP_U32 ulRegVal = 0x5f9b;
    BSP_U32 ulRegRet = 0;
    BSP_U32 count = 3;

    Debug_SIO_En_RX_TX();

    BSP_REG_WRITE(SIO_PCM_XD,0,ulRegVal);
    printk("---------------check 1 write %x,ulRegVal=%d\n",SIO_PCM_XD,ulRegVal);
    while(count)
    {
        BSP_REG_READ(SIO_PCM_RD, 0, ulRegRet);
        //数据环回校验正确，返回成功
        if (ulRegRet == ulRegVal)
        {
            printk("---------------check 1\n");
            return 1;
        }
        
        count--;
    }
    printk("--------------check 0, %x\n",ulRegRet);
    return 0;
}

EXPORT_SYMBOL(Start_Equ_Slic_Test);


#ifdef __cplusplus
}
#endif

