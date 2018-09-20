

#ifndef BSP_SPI_DRV_H
#define BSP_SPI_DRV_H

#include <linux/gpio.h>
#include "DrvInterface.h"
#include "data_types.h"


#ifdef __cplusplus
extern "C"
{
#endif

    //#include "product_config.h"
    //#include "BSP_GLOBAL.h"
    //#include "BSP_VERSION.h"
    //#include "arm_pbxa9.h"

    /* SPI管脚复用宏定义 */
#define SSP0_WIRE_4 14//(1<<14)
#define SSP0_CS1_EN 15//(1<<15)

#define SSP1_WIRE_4 30//(1<<30)
#define SSP1_CS1_EN 31//(1UL<<31)

#define SSP0_CTRL       11//(1<<11)
#define SSP1_CTRL       15//(1<<15)
#define HS_UART_CTRL    (3<<18)

    typedef struct tagSPI_ADCFG_S
    {
        BSP_U16 regAddr;        /*寄存器地址*/
        BSP_U8  regValue;       /*寄存器要配置的值*/
    } SPI_ADCFG_S;

#define SPI_LOG_LEVEL_DEBUG 300

    /*配置Ctrl0寄存器时，各成员的偏移*/
#define SPI_PROT_SHIFT_BITS 0x4
#define SPI_COMM_SHIFT_BITS 0xc

    /*FIFO深度*/
#define SPI_FIFO_DEPTH     8

    /*寄存器偏移*/
#define SPI_CTRL0_OFFSET   0x0
#define SPI_CTRL1_OFFSET   0x4
#define SPI_EN_OFFSET      0x8
#define SPI_MWCTRL_OFFSET      0xc
#define SPI_SLAVE_EN_OFFSET    0x10
#define SPI_BAUD_OFFSET    0x14
#define SPI_TXFTL_OFFSET    0x18
#define SPI_RXFTL_OFFSET    0x1c
#define SPI_TXFL_OFFSET    0x20
#define SPI_RXFL_OFFSET    0x24
#define SPI_STATUS_OFFSET    0x28
#define SPI_INT_MASK_OFFSET    0x2c
#define SPI_INT_STATUS_OFFSET    0x30
#define SPI_RAW_INT_STATUS_OFFSET    0x34
#define SPI_TXO_INT_CLEAR_OFFSET    0x38
#define SPI_RXO_INT_CLEAR_OFFSET    0x3c
#define SPI_RXU_INT_CLEAR_OFFSET    0x40
#define SPI_MST_INT_CLEAR_OFFSET    0x44
#define SPI_INT_CLEAR_OFFSET    0x48
#define SPI_DMAC_OFFSET    0x4c
#define SPI_DMATDL_OFFSET    0x50
#define SPI_DMARDL_OFFSET    0x54
#define SPI_ID_OFFSET      0x58
#define SPI_COMP_VERSION_OFFSET      0x5c
#define SPI_DATA_OFFSET_BASE    0x60
#define SPI_DATA_OFFSET(i)    (SPI_DATA_OFFSET_BASE + i*0x4)

#define E5172_SLIC_WAKEUP_DEBUG
#undef E5172_SLIC_WAKEUP_DEBUG

#ifdef E5172_SLIC_WAKEUP_DEBUG
/* 飞线的Slic中断是GPIO2_13 */
#define BASE_GPIO_1 0x9000E000
#define GPIO_23 13
#else
#define BASE_GPIO_1 0x90011000
#define GPIO_23 23
#endif


#define HI_GPIO_SWPORT_DDR 0x4
#define HI_GPIO_INTEN 0x30
#define HI_GPIO_INTMASK 0x34
#define HI_GPIO_INTTYPE_LEVEL 0x38
#define HI_GPIO_INT_PLOARITY 0x3C
#define HI_GPIO_GPIO_INTSTATUS 0x40
#define HI_GPIO_SIZE 0x1000
#define HI_GPIO_PORT_EOI 0x4C

#define SLIC_GPIO_REG(base, reg) (*(volatile UINT32 *)(base + (reg)))
#define SLIC_GPIO_REG_GETBIT(base, reg, pos) ( (SLIC_GPIO_REG(base, reg )>>pos) & 1 )
#define SLIC_GPIO_REG_SETBIT(base, reg, pos) (SLIC_GPIO_REG(base, reg) = SLIC_GPIO_REG(base, reg) | (1<< (pos)) )
#define SLIC_GPIO_REG_CLRBIT(base, reg, pos) (SLIC_GPIO_REG(base, reg) = SLIC_GPIO_REG(base, reg) & (~(1<< (pos)) ) )

//根据Hi6920用户指南，定义软中断号
#define HI6920_I2C_IRQ 111
#define HI6920_GPIO0_IRQ 112

#ifdef E5172_SLIC_WAKEUP_DEBUG
#define HI6920_GPIO1_IRQ 114
#else
#define HI6920_GPIO1_IRQ 113
#endif

    extern UINT32 g_gpioBase;

    /*SPI状态寄存器的值*/
    //#define SPI_STATUS ((*(volatile unsigned int*)(u32TemBaseAddr + SPI_STATUS_OFFSET)))

#if 0
    /*****************************************************************************
    * 函 数 名  : SPI_Init
    *
    * 功能描述  : SPI初始化
    *
    * 输入参数  : 无
    *
    * 输出参数  : 无
    *
    * 返 回 值  : 初始化成功或者失败
    *****************************************************************************/
    BSP_S32 SPI_Init();

    /*****************************************************************************
    * 函 数 名  : SPI_Lock
    *
    * 功能描述  : 锁定SPI总线
    *
    * 输入参数  : enSpiId 需要锁定的SPI号
    *
    * 输出参数  : 无
    *
    * 返 回 值  : 锁定成功或者失败
    *****************************************************************************/
    BSP_S32 SPI_Lock(SPI_DEV_ID_E enSpiId);

    /*****************************************************************************
    * 函 数 名  : SPI_UnLock
    *
    * 功能描述  : 释放SPI总线
    *
    * 输入参数  : enSpiId 需要释放的SPI号
    *
    * 输出参数  : 无
    *
    * 返 回 值  : 释放成功或者失败
    *****************************************************************************/
    BSP_S32 SPI_UnLock(SPI_DEV_ID_E enSpiId);

    /*****************************************************************************
    * 函 数 名  : SPI_Poll_Send
    *
    * 功能描述  : 轮询模式下的数据发送
    *
    * 输入参数  : pSpiId  进行数据发送的SPI号和片选号。
    pData   需要发送的数据缓冲区指针
    *             u32time 要发送数据的次数
    *
    * 输出参数  : 无
    *
    * 返 回 值  : OK     发送成功
    *             ERROR  发送失败
    *****************************************************************************/
    BSP_S32 SPI_POLL_Send(SPI_DEV_S *pSpiId, void *pData, BSP_U32 u32time);

    /*****************************************************************************
    * 函 数 名  : SPI_Poll_Receive
    *
    * 功能描述  : 轮询模式下的数据接收
    *
    * 输入参数  : pSpiId  进行数据接收的SPI号和片选号。
    pData   存储接收数据的缓冲区指针
    *             u32Lens 待接收的数据长度
    *
    * 输出参数  : 无
    *
    * 返 回 值  : OK    接收成功
    *             ERROR 接收失败
    *****************************************************************************/
    BSP_S32 SPI_POLL_Receive(SPI_DEV_S *pSpiId,BSP_VOID *pData, BSP_U32 u32Lens);

    /*****************************************************************************
    * 函 数 名  : SPI_SetSpiOwner
    *
    * 功能描述  : 设置当前SPI的属性
    *
    * 输入参数  : enSpiId 要查询的SPI号。
    *
    * 输出参数  : 无
    *
    * 返 回 值  : SPI当前使用的片选号
    *****************************************************************************/
    BSP_S32 SPI_SetSpiOwner(SPI_DEV_S *pstSpiID);

    BSP_VOID SPI_GetGlobalVariable();

    BSP_VOID SPI_GetRegistValue();
#endif

    typedef int BOOL;

    //add new interface
    INT32 SlicSpiInit(void);
    UINT8 SlicReadReg(UINT8 ucReg);
    void SlicWriteReg(UINT8 ucReg, UINT8 ucVal);
    UINT32 SlicReadRam(UINT16 usRam);
    void SlicWriteRam(UINT16 usRam, UINT32 ulData);
    INT32 SlicSetUserMode(BOOL on);
    void EnableSlicIrq( int bEnable );
    void EnSlicInt(void);
    void DisSlicInt(void);

#ifdef __cplusplus
}
#endif

#endif

