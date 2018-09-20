

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/

#ifndef __DRV_DMA_H__
#define __DRV_DMA_H__

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
/* 2.1 外部直接引用定义 */
#define DRV_DMA_LLI_LINK(uwAddr)        (((uwAddr) & 0xffffffe0UL) | (0x2UL))   /* 链表链接LLI配置字 */
#define DRV_DMA_LLI_CHAN(uhwChanNo)     (((BSP_U32)(uhwChanNo) << 8)|(0x1UL))/* 通道链接LLI配置字 */
#define DRV_DMA_LLI_DISABLE             (0x0UL)                                 /* 链接模式不使能配置字 */

#define DRV_DMA_MEM_SIO_CFG             (0x83311057)                            /* 典型的MEM->SIO的DMA配置，源地址递增，16bit位宽burst4 */
#define DRV_DMA_SIO_MEM_CFG             (0x43311047)                            /* 典型的SIO->MEM的DMA配置，目的地址递增，16bit位宽burst4 */
#define DRV_DMA_MEM_MEM_CFG             (0xfff33003)                            /* 典型的MEM->MEM的DMA配置，地址双向递增，64bit位宽burst1  */

#define SC_DMA_SEL_CTRL_ADDR    (0x900004E0U)
#define DMA_BASE_ADDR_PHY       (0x90024000U)
#define DMA_SIZE                (0x1000)
extern BSP_U32 g_uldmaBaseAddr ;
/* 2.2 内部引用定义 */
#define DRV_DMA_MAX_CHANNEL_NUM         (16)                                    /* 最大通道数                              */
#define DRV_DMA_INT_NO_HIFI             (120)                                    /* HIFI上EDMA中断的中断编号                */
#define DRV_DMA_BASE_ADDR               (g_uldmaBaseAddr)

#define DRV_DMA_CPU_NO_HIFI             (1)                                     /* EDMA上HIFI处理器编号                    */
#define DRV_DMA_INT_STAT(i)             (DRV_DMA_BASE_ADDR+(       (0x40*i)))  /* 处理器i的中断状态寄存器                 */
#define DRV_DMA_INT_TC1(i)              (DRV_DMA_BASE_ADDR+(0x0004+(0x40*i)))  /* 处理器i的通道传输完成中断状态寄存器     */
#define DRV_DMA_INT_TC2(i)              (DRV_DMA_BASE_ADDR+(0x0008+(0x40*i)))  /* 处理器i的链表节点传输完成中断状态寄存器 */
#define DRV_DMA_INT_ERR1(i)             (DRV_DMA_BASE_ADDR+(0x000c+(0x40*i)))  /* 处理器i的配置错误中断状态寄存器         */
#define DRV_DMA_INT_ERR2(i)             (DRV_DMA_BASE_ADDR+(0x0010+(0x40*i)))  /* 处理器i的数据传输错误中断状态寄存器     */
#define DRV_DMA_INT_ERR3(i)             (DRV_DMA_BASE_ADDR+(0x0014+(0x40*i)))  /* 处理器i的读链表错误中断状态寄存器       */
#define DRV_DMA_INT_TC1_MASK(i)         (DRV_DMA_BASE_ADDR+(0x0018+(0x40*i)))  /* 处理器i的通道传输完成中断屏蔽寄存器     */
#define DRV_DMA_INT_TC2_MASK(i)         (DRV_DMA_BASE_ADDR+(0x001c+(0x40*i)))  /* 处理器i的链表节点传输完成中断屏蔽寄存器 */
#define DRV_DMA_INT_ERR1_MASK(i)        (DRV_DMA_BASE_ADDR+(0x0020+(0x40*i)))  /* 处理器i的配置错误中断屏蔽寄存器         */
#define DRV_DMA_INT_ERR2_MASK(i)        (DRV_DMA_BASE_ADDR+(0x0024+(0x40*i)))  /* 处理器i的数据传输错误中断屏蔽寄存器     */
#define DRV_DMA_INT_ERR3_MASK(i)        (DRV_DMA_BASE_ADDR+(0x0028+(0x40*i)))  /* 处理器i的链表读取错误中断屏蔽寄存器     */
#define DRV_DMA_INT_TC1_RAW             (DRV_DMA_BASE_ADDR+(0x0600))            /* 原始通道传输完成中断状态寄存器          */
#define DRV_DMA_INT_TC2_RAW             (DRV_DMA_BASE_ADDR+(0x0608))            /* 原始链表节点传输完成中断状态寄存器      */
#define DRV_DMA_INT_ERR1_RAW            (DRV_DMA_BASE_ADDR+(0x0610))            /* 原始配置错误中断状态寄存器              */
#define DRV_DMA_INT_ERR2_RAW            (DRV_DMA_BASE_ADDR+(0x0618))            /* 原始数据传输错误中断状态寄存器          */
#define DRV_DMA_INT_ERR3_RAW            (DRV_DMA_BASE_ADDR+(0x0620))            /* 原始链表读取错误中断状态寄存器          */
#define DRV_DMA_SREQ                    (DRV_DMA_BASE_ADDR+(0x0660))            /* 单传输请求寄存器                        */
#define DRV_DMA_LSREQ                   (DRV_DMA_BASE_ADDR+(0x0664))            /* 末次单传输请求寄存器                    */
#define DRV_DMA_BREQ                    (DRV_DMA_BASE_ADDR+(0x0668))            /* 突发传输请求寄存器                      */
#define DRV_DMA_LBREQ                   (DRV_DMA_BASE_ADDR+(0x066C))            /* 末次突发传输请求寄存器                  */
#define DRV_DMA_FREQ                    (DRV_DMA_BASE_ADDR+(0x0670))            /* 批量传输请求寄存器                      */
#define DRV_DMA_LFREQ                   (DRV_DMA_BASE_ADDR+(0x0674))            /* 末次批量传输请求寄存器                  */
#define DRV_DMA_CH_PRI                  (DRV_DMA_BASE_ADDR+(0x0688))            /* 优先级控制寄存器                        */
#define DRV_DMA_CH_STAT                 (DRV_DMA_BASE_ADDR+(0x0690))            /* 全局DMA状态寄存器                       */
#define DRV_DMA_DMA_CTRL                (DRV_DMA_BASE_ADDR+(0x0698))            /* DMA全局控制寄存器                       */
#define DRV_DMA_CX_CURR_CNT1(j)         (DRV_DMA_BASE_ADDR+(0x0700+(0x10*j)))  /* 通道j的三维传输剩余size状态寄存器       */
#define DRV_DMA_CX_CURR_CNT0(j)         (DRV_DMA_BASE_ADDR+(0x0704+(0x10*j)))  /* 通道j的一、二维传输剩余size状态寄存器   */
#define DRV_DMA_CX_CURR_SRC_ADDR(j)     (DRV_DMA_BASE_ADDR+(0x0708+(0x10*j)))  /* 通道j的源地址寄存器                     */
#define DRV_DMA_CX_CURR_DES_ADDR(j)     (DRV_DMA_BASE_ADDR+(0x070C+(0x10*j)))  /* 通道j的目的地址寄存器                   */
#define DRV_DMA_CX_LLI(j)               (DRV_DMA_BASE_ADDR+(0x0800+(0x40*j)))  /* 通道j的链表地址寄存器                   */
#define DRV_DMA_CX_BINDX(j)             (DRV_DMA_BASE_ADDR+(0x0804+(0x40*j)))  /* 通道j的二维地址偏移量配置寄存器         */
#define DRV_DMA_CX_CINDX(j)             (DRV_DMA_BASE_ADDR+(0x0808+(0x40*j)))  /* 通道j的三维地址偏移量配置寄存器         */
#define DRV_DMA_CX_CNT1(j)              (DRV_DMA_BASE_ADDR+(0x080C+(0x40*j)))  /* 通道j的传输长度1配置寄存器              */
#define DRV_DMA_CX_CNT0(j)              (DRV_DMA_BASE_ADDR+(0x0810+(0x40*j)))  /* 通道j的传输长度配置寄存器               */
#define DRV_DMA_CX_SRC_ADDR(j)          (DRV_DMA_BASE_ADDR+(0x0814+(0x40*j)))  /* 通道j的源地址寄存器                     */
#define DRV_DMA_CX_DES_ADDR(j)          (DRV_DMA_BASE_ADDR+(0x0818+(0x40*j)))  /* 通道j的目的地址寄存器                   */
#define DRV_DMA_CX_CONFIG(j)            (DRV_DMA_BASE_ADDR+(0x081C+(0x40*j)))  /* 通道j的配置寄存器                       */
#define DRV_DMA_CX_AXI_CONF(j)          (DRV_DMA_BASE_ADDR+(0x0820+(0x40*j)))  /* 通道j的AXI特殊操作配置寄存器            */

#define DRV_DMA_INT_STAT_HIFI           (DRV_DMA_INT_STAT(DRV_DMA_CPU_NO_HIFI))         /* 处理器HIFI的中断状态寄存器                 */
#define DRV_DMA_INT_TC1_HIFI            (DRV_DMA_INT_TC1(DRV_DMA_CPU_NO_HIFI))          /* 处理器HIFI的通道传输完成中断状态寄存器     */
#define DRV_DMA_INT_TC2_HIFI            (DRV_DMA_INT_TC2(DRV_DMA_CPU_NO_HIFI))          /* 处理器HIFI的链表节点传输完成中断状态寄存器 */
#define DRV_DMA_INT_ERR1_HIFI           (DRV_DMA_INT_ERR1(DRV_DMA_CPU_NO_HIFI))         /* 处理器HIFI的配置错误中断状态寄存器         */
#define DRV_DMA_INT_ERR2_HIFI           (DRV_DMA_INT_ERR2(DRV_DMA_CPU_NO_HIFI))         /* 处理器HIFI的数据传输错误中断状态寄存器     */
#define DRV_DMA_INT_ERR3_HIFI           (DRV_DMA_INT_ERR3(DRV_DMA_CPU_NO_HIFI))         /* 处理器HIFI的读链表错误中断状态寄存器       */
#define DRV_DMA_INT_TC1_MASK_HIFI       (DRV_DMA_INT_TC1_MASK(DRV_DMA_CPU_NO_HIFI))     /* 处理器HIFI的通道传输完成中断屏蔽寄存器     */
#define DRV_DMA_INT_TC2_MASK_HIFI       (DRV_DMA_INT_TC2_MASK(DRV_DMA_CPU_NO_HIFI))     /* 处理器HIFI的链表节点传输完成中断屏蔽寄存器 */
#define DRV_DMA_INT_ERR1_MASK_HIFI      (DRV_DMA_INT_ERR1_MASK(DRV_DMA_CPU_NO_HIFI))    /* 处理器HIFI的配置错误中断屏蔽寄存器         */
#define DRV_DMA_INT_ERR2_MASK_HIFI      (DRV_DMA_INT_ERR2_MASK(DRV_DMA_CPU_NO_HIFI))    /* 处理器HIFI的数据传输错误中断屏蔽寄存器     */
#define DRV_DMA_INT_ERR3_MASK_HIFI      (DRV_DMA_INT_ERR3_MASK( DRV_DMA_CPU_NO_HIFI))   /* 处理器HIFI的链表读取错误中断屏蔽寄存器     */

#define DRV_DMA_GetCxIsrPtr()           (&g_astDrvDmaCxIntIsr[0])               /* 获取DMA中断处理函数全局变量首地址 */

/*****************************************************************************
  3 枚举定义
*****************************************************************************/

/*****************************************************************************
 函 数 名  : DRV_DMA_INT_TYPE_ENUM
 功能描述  : DMA通道中断类型
*****************************************************************************/
enum DRV_DMA_INT_TYPE_ENUM
{
    DRV_DMA_INT_TYPE_TC1 = 0,           /* 通道完成     */
    DRV_DMA_INT_TYPE_TC2,               /* 链表结点完成(除最后一个结点) */
    DRV_DMA_INT_TYPE_ERR1,              /* 配置错误     */
    DRV_DMA_INT_TYPE_ERR2,              /* 数据传输错误 */
    DRV_DMA_INT_TYPE_ERR3,              /* 读取链表错误 */
    DRV_DMA_INT_TYPE_BUTT
};
typedef BSP_U16 DRV_DMA_INT_TYPE_ENUM_UINT16;



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
 实体名称  : DRV_DMA_CXCFG_STRU
 功能描述  : EMAC通道配置实体

LLI寄存器含义:
   b31-5, 链表地址b31-5, 即32byte对齐
   b11-8, 链接通道号
   b1-0,  通道链接使能, 00-不使能;01-通道链接;10-链表链接;

 config寄存器含义:
   b31 b30 b29   b28  b27-24 b23-20 b18-16 b14-12 b9-4   b3-2      b1    b0
   si  di smode dmode   sl     dl    sw     dw    peri flow_ctrl itc_en ch_en
 其中,
   sl/dl, src/dest burst len, 取值0000-1111表示长度为1-16
   sw/dw, src/dest width, 取值000-101表示宽度为8/16/32/64/128/256bit
   flow_ctrl, 00-M2M;01-M2P;10-M2P外设流控
*****************************************************************************/
typedef struct
{
   BSP_U32       uwLli;              /* 通道Linked List Item地址     */
   BSP_U16       uhwDstBIndex;       /* 通道的二维目的地址偏移量配置，占低16bit */
   BSP_U16       uhwSrcBIndex;       /* 通道的二维源地址偏移量配置，占高16bit */
   BSP_U16       uhwDstCIndex;       /* 通道的三维目的地址偏移量配置，占低16bit */
   BSP_U16       uhwSrcCIndex;       /* 通道的三维源地址偏移量配置，占高16bit */
   BSP_U16       uhwCCount;          /* 三维传输长度配置,单位Frame，占低16bit   */
   BSP_U16       uhwRsv;             /* 保留位，占高16bit                    */
   BSP_U16       uhwACount;          /* 一维传输长度配置,单位Byte，占低16bit */
   BSP_U16       uhwBCount;          /* 二维传输长度配置,单位Array，占高16bit  */
   BSP_U32       uwSrcAddr;          /* 通道源地址                   */
   BSP_U32       uwDstAddr;          /* 通道目的地址                 */
   BSP_U32       uwConfig;           /* 通道配置寄存器               */

} DRV_DMA_CXCFG_STRU;

/*****************************************************************************
 函 数 名  : DRV_DMA_INT_FUNC
 功能描述  : DMA中断处理回调函数
 输入参数  : enIntType - 中断类型, 参见DRV_DMA_INT_TYPE_ENUM_UINT16
             uwPara    - 用户参数
 输出参数  : 无
 返 回 值  : VOS_VOID
*****************************************************************************/
typedef BSP_VOID (*DRV_DMA_INT_FUNC)(
                DRV_DMA_INT_TYPE_ENUM_UINT16 enIntType,
                BSP_U32                   uwPara);

/*****************************************************************************
 实体名称  : DRV_DMA_CXISR_STRU
 功能描述  : 定义DMA通道回调函数数据实体
*****************************************************************************/
typedef struct
{
    DRV_DMA_INT_FUNC    pfFunc;         /* 回调函数指针 */
    BSP_U32          uwPara;         /* 回调参数     */

} DRV_DMA_CXISR_STRU;

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

extern BSP_VOID DRV_DMA_Init_A( BSP_VOID );

extern BSP_U32 DRV_DMA_StartWithCfg_A(
                       BSP_U16              uhwChannelNo,
                       DRV_DMA_CXCFG_STRU     *pstCfg,
                       DRV_DMA_INT_FUNC        pfIntHandleFunc,
                       BSP_U32              uwPara);
extern BSP_VOID DRV_DMA_Stop_A(BSP_U16 uhwChannelNo);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of drv_dma.h */
