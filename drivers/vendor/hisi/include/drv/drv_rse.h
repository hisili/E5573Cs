

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#ifndef __DRV_RSE_H__
#define __DRV_RSE_H__

extern int bsp_rse_on(void);
#define DRV_RSE_ON() bsp_rse_on()

extern int bsp_rse_off(void);
#define DRV_RSE_OFF() bsp_rse_off()

#endif
