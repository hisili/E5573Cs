#ifndef __DRV_SIO_H__
#define __DRV_SIO_H__


#include <linux/kernel.h>
#include "DrvInterface.h"



#ifdef __cplusplus
extern "C" {
#endif

extern BSP_U32 g_ulSioBaseAddr;

#define SIO_BASE_ADDR_PHY   (0x900A7000U)
#define SIO_SIZE            (0x1000)

#define SIO_BASE_ADDR       (g_ulSioBaseAddr)
#define SIO_MODE            (SIO_BASE_ADDR + 0x40)
#define SIO_INTR_STATUS     (SIO_BASE_ADDR + 0x44)
#define SIO_INTR_CLR        (SIO_BASE_ADDR + 0x48)
#define SIO_PCM_RD          (SIO_BASE_ADDR + 0x58)
#define SIO_PCM_XD          (SIO_BASE_ADDR + 0x50)
#define SIO_CTRL_SET        (SIO_BASE_ADDR + 0x5c)
#define SIO_CTRL_CLR        (SIO_BASE_ADDR + 0x60)
#define SIO_CTRL_RX_STA     (SIO_BASE_ADDR + 0x68)
#define SIO_CTRL_TX_STA     (SIO_BASE_ADDR + 0x6c)
#define SIO_DATA_WIDTH_SET  (SIO_BASE_ADDR + 0x78)
#define SIO_SIGNED_EXT      (SIO_BASE_ADDR + 0x84)
#define SIO_INTR_MASK       (SIO_BASE_ADDR + 0x8c)


#define SIO_GetStatusPtr() (&g_stSioCtrl.stStatus)
#define SIO_GetIntIsrPtr() (&g_stSioCtrl.stIsr)

enum SIO_MODE_ENUM
{
    SIO_MODE_CLOSE = 0,
    SIO_MODE_OPEN,
    SIO_MODE_BUTT
};
typedef BSP_U32 SIO_MODE_ENUM_UINT32;

typedef struct
{
    SIO_MODE_ENUM_UINT32 enMode;
    BSP_U32 ulPinCtrlStatus;
    BSP_U32 ulUartPinCtrlStatus;
    BSP_U32 ulSioClkIomgStatus;
    BSP_U32 ulSioDiIomgStatus;
    BSP_U32 ulSioDoIomgStatus;
}SIO_STATUS_STRU;

enum SIO_INT_MASK_ENUM
{
    SIO_INT_MASK_RX_INTR = 0x1,
    SIO_INT_MASK_TX_INTR = 0x2,
    SIO_INT_MASK_RX_RIGHT_FIFO_OVER = 0x4,
    SIO_INT_MASK_RX_LEFT_FIFO_OVER = 0x8,
    SIO_INT_MASK_TX_RIGHT_FIFO_UNDER = 0x10,
    SIO_INT_MASK_TX_LEFT_FIFO_UNDER = 0x20,
    SIO_INT_MASK_BUTT = 0x40
};
typedef BSP_U16 SIO_INT_MASK_ENUM_UINT16;

typedef BSP_VOID (*SIO_INT_FUNC)(
    SIO_INT_MASK_ENUM_UINT16 enIntType,
    BSP_U32 ulPara);

typedef struct
{
    SIO_INT_FUNC pfFunc;
    BSP_U32 ulPara;
}SIO_INT_ISR_STRU;

typedef struct
{
    SIO_STATUS_STRU stStatus;
    SIO_INT_ISR_STRU stIsr;
}SIO_CTRL_STRU;

//func declare
BSP_VOID SIO_Init(BSP_VOID);
BSP_VOID SIO_Open(
    SIO_INT_MASK_ENUM_UINT16 enIntMask,
    SIO_INT_FUNC pfIntHandleFunc,
    BSP_U32 ulPara);



#ifdef __cplusplus
}
#endif


#endif /*end of drv_sio.h*/
