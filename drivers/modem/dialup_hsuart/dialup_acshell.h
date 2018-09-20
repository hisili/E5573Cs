
#ifndef    _HSUART_ACSHELL_H_
#define    _HSUART_ACSHELL_H_

#ifdef __cplusplus
extern "C" {
#endif
/**************************************************************************
  头文件包含                            
**************************************************************************/
#include "dialup_hsuart.h"
#include "drv_hsuart.h"
#include "hi_uart.h"

/*****************************************************************************
  宏定义
*****************************************************************************/

#define ACSHELL_TX_BUFF_SIZE 	(8*1024)
#define DIAL_HSUART_CSHELL_BIT	(3)
#define HSUART_BASE_VIRT_ADDR	(0xc2000000)
/*****************************************************************************
  结构体定义
*****************************************************************************/
struct acshell_str
{
	volatile u32 ulread;
	volatile u32 ulwrite;
	u8 uldata[1024];
	u8 *ptxbuff;
	u8 shell_flag;
}acshell_recv_str;

struct  _HSUART_RX_OPS_
{
    int (*shell_rx)(u8 *buff,u32 lenth);
}hsuart_ops;

/*****************************************************************************
  函数声明
*****************************************************************************/
extern struct console* bsp_get_uart_console(void);
extern int cshell_set_bit(int num_from_zero);
extern int cshell_clear_bit(int num_from_zero);
extern int cshell_recv_from_dial(u8 *buf, u32 len);

#ifdef __cplusplus /* __cplusplus */
}
#endif /* __cplusplus */
#endif

