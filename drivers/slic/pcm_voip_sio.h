

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#ifndef __DRV_SIO_H__
#define __DRV_SIO_H__

#include <linux/kernel.h>
#include "DrvInterface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif



/*****************************************************************************
  2 宏定义
*****************************************************************************/

#define DRV_SC_BASE_ADDR                (0x90000000U)                           /* SC基地址 */


#define DRV_SIO_CLK_CTRL_WORD           (0x0010)                                /* V7R1 CS中SIO总线时钟控制字,第4bit置1 */
#define DRV_SIO_SIO_MODE                (DRV_SC_BASE_ADDR + 0x0030)             /* V7R1 CS中SIO时钟控制寄存器 */
#define DRV_SIO_PCM_MODE                (DRV_SC_BASE_ADDR + 0x0940)             /* PCM接口管脚控制寄存器 */
#define DRV_SIO_UART_PIN_CTRL           (DRV_SC_BASE_ADDR + 0x0988)             /* 管脚复用信息控制寄存器 */
#define DRV_SIO_CLK_FREQ_CTRL           (DRV_SC_BASE_ADDR + 0x0044)             /* V7R1 SIO MASTER分配参数配置 */


#define DRV_SIO_COMBINE_INT_NO          (10)                                    /* SIO组合中断中断号 */
#define DRV_SIO_RX_INT_NO               (8)                                     /* SIO接收中断中断号 */
#define DRV_SIO_TX_INT_NO               (9)                                     /* SIO发送中断中断号 */
extern BSP_U32 g_ulSioBaseAddr_A;

#define SIO_BASE_ADDR_PHY   (0x900A7000U)
#define SIO_SIZE            (0x1000)

#define DRV_SIO_BASE_ADDR       (g_ulSioBaseAddr_A)

#define DRV_SIO_MODE                    (DRV_SIO_BASE_ADDR + 0x40)              /* 模式寄存器                      */
#define DRV_SIO_INTR_STATUS             (DRV_SIO_BASE_ADDR + 0x44)              /* SIO的中断状态指示寄存器         */
#define DRV_SIO_INTR_CLR                (DRV_SIO_BASE_ADDR + 0x48)              /* 中断清除寄存器                  */
#define DRV_SIO_I2S_LEFT_XD             (DRV_SIO_BASE_ADDR + 0x4c)              /* I2S左声道发送数据寄存器         */
#define DRV_SIO_I2S_RIGHT_XD            (DRV_SIO_BASE_ADDR + 0x50)              /* I2S右声道发送数据寄存器         */
#define DRV_SIO_PCM_XD                  (DRV_SIO_BASE_ADDR + 0x50)              /* PCM模式下的数据发送寄存器       */
#define DRV_SIO_I2S_LEFT_RD             (DRV_SIO_BASE_ADDR + 0x54)              /* I2S左声道接收数据寄存器         */
#define DRV_SIO_I2S_RIGHT_RD            (DRV_SIO_BASE_ADDR + 0x58)              /* I2S右声道接收数据寄存器         */
#define DRV_SIO_PCM_RD                  (DRV_SIO_BASE_ADDR + 0x58)              /* PCM接收数据寄存器               */
#define DRV_SIO_CTRL_SET                (DRV_SIO_BASE_ADDR + 0x5c)              /* 设置寄存器                      */
#define DRV_SIO_CTRL_CLR                (DRV_SIO_BASE_ADDR + 0x60)              /* 清除寄存器                      */
#define DRV_SIO_RX_STA                  (DRV_SIO_BASE_ADDR + 0x68)              /* SIO接收状态寄存器               */
#define DRV_SIO_TX_STA                  (DRV_SIO_BASE_ADDR + 0x6c)              /* SIO发送状态寄存器               */
#define DRV_SIO_DATA_WIDTH_SET          (DRV_SIO_BASE_ADDR + 0x78)              /* 在I2S/PCM模式下的数据宽度寄存器 */
#define DRV_SIO_I2S_START_POS           (DRV_SIO_BASE_ADDR + 0x7c)              /* I2S左右声道地址合并后读写起始声道位置设置寄存器 */
#define DRV_SIO_I2S_POS_FLAG            (DRV_SIO_BASE_ADDR + 0x80)              /* I2S声道地址合并后下次读写左右声道位置寄存器     */
#define DRV_SIO_SIGNED_EXT              (DRV_SIO_BASE_ADDR + 0x84)              /* 高位数据符号扩展使能寄存器      */
#define DRV_SIO_INTR_MASK               (DRV_SIO_BASE_ADDR + 0x8c)              /* 中断屏蔽寄存器                  */
#define DRV_SIO_I2S_POS_MERGE_EN        (DRV_SIO_BASE_ADDR + 0x88)              /* I2S左右声道地址合并控制寄存器   */
#define DRV_SIO_I2S_DUAL_RX_CHN         (DRV_SIO_BASE_ADDR + 0xa0)              /* I2S左右声道合并后接收数据寄存器 */
#define DRV_SIO_I2S_DUAL_TX_CHN         (DRV_SIO_BASE_ADDR + 0xc0)              /* I2S声道合并后写发送数据寄存器   */

#define DRV_SC_SIO_BCLK_ENABLE_REG_ADDR     (DRV_SC_BASE_ADDR+0x0024U)
#define DRV_SC_SIO_BCLK_DISABLE_REG_ADDR    (DRV_SC_BASE_ADDR+0x0028U)
#define DRV_SC_CLK_CTRL_ENABLE_REG_ADDR     (DRV_SC_BASE_ADDR+0x000CU)          /* V7R1 CS中外设时钟使能寄存器 */
#define DRV_SC_CLK_CTRL_DISABLE_REG_ADDR    (DRV_SC_BASE_ADDR+0x0010U)          /* V7R1 CS中外设时钟关闭寄存器 */

/*
SIO CTRL 寄存器说明:
    b31-16 b15  b14    b13   b12     b11         b10         b9          b8
    res    rst int_en rx_en tx_en rx_fifo_dis tx_fifo_dis rx_merge_en tx_merge_en

      b7-4         b3-0
    rx_fifo_th tx_fifo_th
*/
#define DRV_SIO_GetStatusPtr()          (&g_stDrvSioCtrl.stStatus)              /* 获取SIO状态寄存器指针 */
#define DRV_SIO_GetIntIsrPtr()          (&g_stDrvSioCtrl.stIsr)                 /* 获取SIO中断回调函数全局变量*/

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/*****************************************************************************
 实体名称  : DRV_SIO_INT_MASK_ENUM
 功能描述  : SIO各类中断掩码
*****************************************************************************/
enum DRV_SIO_INT_MASK_ENUM
{
    DRV_SIO_INT_MASK_RX_INTR            = 0x1,                                  /* 接收FIFO高于阈值中断                 */
    DRV_SIO_INT_MASK_TX_INTR            = 0x2,                                  /* 发送FIFO低于阈值中断                 */
    DRV_SIO_INT_MASK_RX_RIGHT_FIFO_OVER = 0x4,                                  /* I2S模式下为右声道接收FIFO上溢出中断  */
                                                                                /* PCM模式下为PCM接收FIFO上溢出中断     */
    DRV_SIO_INT_MASK_RX_LEFT_FIFO_OVER  = 0x8,                                  /* 左声道接收FIFO上溢出,只在I2S模式有效 */
    DRV_SIO_INT_MASK_TX_RIGHT_FIFO_UNDER= 0x10,                                 /* I2S模式下为右声道发送FIFO下溢出中断  */
                                                                                /* PCM模式下为PCM发送FIFO下溢出中断     */
    DRV_SIO_INT_MASK_TX_LEFT_FIFO_UNDER = 0x20,                                 /* 左声道接收FIFO下溢出,只在I2S模式有效 */
    DRV_SIO_INT_MASK_BUTT               = 0x40
};
typedef BSP_U16 DRV_SIO_INT_MASK_ENUM_UINT16;

/*****************************************************************************
 实体名称  : DRV_SIO_MODE_ENUM
 功能描述  : SIO工作模式
*****************************************************************************/
enum DRV_SIO_MODE_ENUM
{
    DRV_SIO_MODE_CLOSE                  = 0,                                    /* SIO时钟关闭 */
    DRV_SIO_MODE_OPEN,                                                          /* SIO时钟打开 */
    DRV_SIO_MODE_BUTT
};
typedef BSP_U32 DRV_SIO_MODE_ENUM_UINT32;

/*****************************************************************************
  4 消息头定义
*****************************************************************************/


/*****************************************************************************
  5 消息定义
*****************************************************************************/


/*****************************************************************************
  6 STRUCT定义
*****************************************************************************/

/*****************************************************************************
 函 数 名  : DRV_SIO_INT_FUNC
 功能描述  : SIO中断处理回调函数
 输入参数  : enIntType - 中断类型, 参见DRV_SIO_INT_TYPE_ENUM_UINT16
             uwPara    - 用户参数
 输出参数  : 无
 返 回 值  : VOS_VOID
*****************************************************************************/
typedef BSP_VOID (*DRV_SIO_INT_FUNC)(
                DRV_SIO_INT_MASK_ENUM_UINT16 enIntType,
                BSP_U32                   uwPara);

/*****************************************************************************
 实体名称  : DRV_SIO_INT_ISR_STRU
 功能描述  : 定义SIO中断回调函数数据实体
*****************************************************************************/
typedef struct
{
    DRV_SIO_INT_FUNC                    pfFunc;                                 /* 回调函数指针 */
    BSP_U32                          uwPara;                                 /* 回调参数     */

} DRV_SIO_INT_ISR_STRU;

/*****************************************************************************
 实体名称  : DRV_SIO_STATUS_STRU
 功能描述  : 定义SIO状态数据实体
*****************************************************************************/
typedef struct
{
    DRV_SIO_MODE_ENUM_UINT32            enMode;                                 /* SIO模式 */
    BSP_U32                          uwPinCtrlStatus;                        /* 保存SIO PCM管脚控制字初始值 */
    BSP_U32                          uwUartPinCtrlStatus;                    /* 保存UART管脚控制字初始值 */
    BSP_U32                          uwSioClkIomgStatus;                     /* 保存V3R2 CS下SIO时钟管脚控制字初始值 */
    BSP_U32                          uwSioDiIomgStatus;                      /* 保存V3R2 CS下SIO_DIN管脚控制字初始值 */
    BSP_U32                          uwSioDoIomgStatus;                      /* 保存V3R2 CS下SIO_DOUT管脚控制字初始值 */

} DRV_SIO_STATUS_STRU;

/*****************************************************************************
 实体名称  : DRV_SIO_CTRL_STRU
 功能描述  : 定义SIO控制实体
*****************************************************************************/
typedef struct
{
    DRV_SIO_STATUS_STRU                 stStatus;                               /* 保存SIO模块控制状态 */
    DRV_SIO_INT_ISR_STRU                stIsr;                                  /* 保存SIO中断回调 */

} DRV_SIO_CTRL_STRU;

/*****************************************************************************
  7 UNION定义
*****************************************************************************/


/*****************************************************************************
  8 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  9 全局变量声明
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern BSP_VOID DRV_SIO_Close_A(BSP_VOID);
extern BSP_VOID DRV_SIO_Init_A(BSP_VOID);

extern BSP_VOID DRV_SIO_Open_A(
                       DRV_SIO_INT_MASK_ENUM_UINT16    enIntMask,
                       DRV_SIO_INT_FUNC                pfIntHandleFunc,
                       BSP_U32                      uwPara);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of drv_sio.h */
